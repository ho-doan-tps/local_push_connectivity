package com.hodoan.local_push_connectivity.sockets

import android.annotation.SuppressLint
import android.content.ContentResolver
import android.provider.Settings
import android.util.Log
import com.hodoan.local_push_connectivity.services.ReceiverCallback
import com.hodoan.local_push_connectivity.wrapper.DataRegister
import com.hodoan.local_push_connectivity.wrapper.PigeonWrapper
import com.hodoan.local_push_connectivity.wrapper.PluginSettings
import com.hodoan.local_push_connectivity.wrapper.RegisterModel
import com.hodoan.local_push_connectivity.wrapper.SenderRegister
import java.util.Timer
import java.util.TimerTask

abstract class ISocket(
    private val contentResolver: ContentResolver,
    receiverCallback: ReceiverCallback
) {
    companion object {
        val TAG = ISocket::class.simpleName
        fun register(
            contentResolver: ContentResolver,
            receiverCallback: ReceiverCallback,
            settings: PluginSettings
        ): ISocket {
//            if
            val socket = WebSocketClient(contentResolver, receiverCallback)
            socket.settings = settings
            return socket
        }
    }

    var settings: PluginSettings = PluginSettings()
        @SuppressLint("HardwareIds")
        set(value) {
            field = settings.copyWith(value, false)
            if (field.deviceId == null) {
                field.deviceId =
                    Settings.Secure.getString(
                        contentResolver, Settings.Secure.ANDROID_ID
                    )
            }
            createSocket()
        }

    abstract fun connect()

    abstract fun disconnect(isRemoveSetting: Boolean = true)

    fun release() {
        stopTimer()
        disconnect(false)
    }

    fun hasSettingConnect(): Boolean =
        settings.connectorID == null
                || settings.connectorID!!.isEmpty()
                || settings.port == null
                || settings.systemType == null
                || settings.host == null
                || settings.deviceId == null

    fun messageRegister(): String = PigeonWrapper.toString(
        RegisterModel(
            messageType = "register",
            systemType = settings.systemType ?: -1,
            sender = SenderRegister(
                connectorTag = settings.connectorTag ?: "",
                connectorID = settings.connectorID ?: "",
                deviceID = settings.deviceId ?: "",
            ),
            data = DataRegister(null, null, null)
        )
    )

    private var timer: Timer? = null
    private var timerTask: TimerTask? = null
    var isConnected = false

    @Synchronized
    fun startTimer() {
        stopTimer()
        timer = Timer()
        timerTask = object : TimerTask() {
            override fun run() {
                Log.d(
                    TAG,
                    "run: $isConnected $settings"
                )
            }
        }
        timer?.schedule(timerTask, 0, 8000)
    }

    @Synchronized
    private fun stopTimer() {
        timerTask?.cancel()
        timerTask = null
        timer?.cancel()
        timer?.purge()
        timer = null
    }

    var thread: Thread? = null

    @Synchronized
    fun createSocket() {
        disconnect(false)
        stopTimer()
        thread?.interrupt()
        thread = null
        thread = Thread {
            connect()
        }
        thread?.start()
    }
}