package com.hodoan.local_push_connectivity.sockets

import android.content.ContentResolver
import android.util.Log
import com.hodoan.local_push_connectivity.services.ReceiverCallback
import com.hodoan.local_push_connectivity.wrapper.PigeonWrapper
import com.hodoan.local_push_connectivity.wrapper.PingModel
import io.ktor.client.HttpClient
import io.ktor.client.engine.cio.CIO
import io.ktor.client.plugins.HttpTimeout
import io.ktor.client.plugins.websocket.WebSockets
import io.ktor.client.plugins.websocket.webSocket
import io.ktor.websocket.Frame
import io.ktor.websocket.WebSocketSession
import io.ktor.websocket.readText
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.cancel
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import kotlinx.coroutines.withTimeout

class WebSocketClient(
    contentResolver: ContentResolver,
    private val receiverCallback: ReceiverCallback
) : ISocket(contentResolver, receiverCallback) {
    private val client = HttpClient(CIO) {
        install(WebSockets) {
            pingInterval = 3_000
        }
        install(HttpTimeout) {
            requestTimeoutMillis = 5_000
            connectTimeoutMillis = 5_000
        }
    }

    private fun WebSocketSession.heartbeat() {
        scope?.launch {
            while (true) {
                try {
                    val ping = PingModel("ping")
                    Log.e(TAG, "heartbeat: send ping ${PigeonWrapper.toString(ping)}")
                    withTimeout(5_000) {
                        send(Frame.Text(PigeonWrapper.toString(ping)))
                    }
                } catch (e: Exception) {
                    isConnected = false
                    println("Heartbeat failed: ${e.message}")
                    release()
                    connect()
                    break
                }
                delay(3_000)
            }
        }
    }

    private var scope: CoroutineScope? = CoroutineScope(Job() + Dispatchers.Default)

    private suspend fun mConnect() {
        val url =
            "${if (settings.wss == true) "wss" else "ws"}://${settings.host}:${settings.port}${settings.wsPath}"

        Log.d(TAG, "mConnect: $url")
        try {
            client.webSocket(url) {
                isConnected = true
                receiverCallback.newMessage("reconnect")
                launch { heartbeat() }
                send(Frame.Text(messageRegister()))
                Log.e(TAG, "mConnect: ${messageRegister()}")
                for (message in incoming) {
                    when (message) {
                        is Frame.Text -> receiverCallback.newMessage(message.readText())
                        else -> TODO()
                    }
                }
            }
        } catch (e: Exception) {
            Log.e(TAG, "mConnect: $e")
            throw e
        }
    }

    @Synchronized
    override fun connect() {
        disconnect(false)
        startTimer()
        try {
            Log.d(TAG, "connect: ${hasSettingConnect()}")
            if (hasSettingConnect()) return
            scope = CoroutineScope(Job() + Dispatchers.Default)
            scope?.launch {
                try {
                    mConnect()
                } catch (e: Exception) {
                    isConnected = false
                    Log.e(TAG, "connect: error ${e.message}")
                    delay(5000)
                    createSocket()
                }
            }
        } catch (e: Exception) {
            isConnected = false
            Log.e(TAG, "connect: error ${e.message}")
            Thread.sleep(5000)
            createSocket()
        }
    }

    @Synchronized
    override fun disconnect(isRemoveSetting: Boolean) {
        isConnected = false
        if (isRemoveSetting) {
            settings.connectorID = null
        }
        thread?.interrupt()
        thread = null
        scope?.cancel()
        scope = null
    }
}