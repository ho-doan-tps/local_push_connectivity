package com.hodoan.local_push_connectivity.sockets

import android.content.ContentResolver
import android.util.Log
import com.hodoan.local_push_connectivity.services.ReceiverCallback
import io.ktor.client.HttpClient
import io.ktor.client.engine.cio.CIO
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

class WebSocketClient(
    contentResolver: ContentResolver,
    private val receiverCallback: ReceiverCallback
) : ISocket(contentResolver, receiverCallback) {
    private val client = HttpClient(CIO) {
        install(WebSockets) {
            pingInterval = 20_000
        }
    }

    private suspend fun WebSocketSession.heartbeat() {
        while (isConnected) {
            try {
                send(Frame.Text("""{"type":"ping"}"""))
            } catch (e: Exception) {
                println("Heartbeat failed: ${e.message}")
                disconnect(false)
                connect()
                break
            }
            delay(5000)
        }
    }

    private var scope:CoroutineScope? = CoroutineScope(Job() + Dispatchers.Default)

    private suspend fun mConnect() {
        val url =
            "${if (settings.wss == true) "wss" else "ws"}://${settings.host}:${settings.port}${settings.wsPath}"

        Log.d(TAG, "mConnect: $url")

        client.webSocket(url) {
            isConnected = true
            launch { heartbeat() }
            send(Frame.Text(messageRegister()))
            for (message in incoming) {
                when (message) {
                    is Frame.Text -> receiverCallback.newMessage(message.readText())
                    else -> TODO()
                }
            }
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
        if(isRemoveSetting) {
            settings.connectorID = null
        }
        thread?.interrupt()
        thread = null
        scope?.cancel()
        scope = null
    }
}