package com.hodoan.local_push_connectivity.services

import android.Manifest
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.PendingIntent
import android.app.Service
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.content.pm.PackageManager
import android.graphics.Color
import android.os.Build
import android.os.Handler
import android.os.IBinder
import android.os.Looper
import android.util.Log
import androidx.core.app.ActivityCompat
import androidx.core.app.NotificationCompat
import androidx.core.app.NotificationManagerCompat
import androidx.core.content.ContextCompat
import com.hodoan.local_push_connectivity.LocalPushConnectivityPlugin
import com.hodoan.local_push_connectivity.sockets.ISocket
import com.hodoan.local_push_connectivity.wrapper.MessageResponse
import com.hodoan.local_push_connectivity.wrapper.PigeonWrapper
import com.hodoan.local_push_connectivity.wrapper.PluginSettings

interface ReceiverCallback {
    fun newMessage(message: String)
}

class PushNotificationService : Service(), ReceiverCallback {
    companion object {
        val TAG = PushNotificationService::class.simpleName

        const val CHANGE_SETTING: String = "com.hodoan.CHANGE_SETTING"
        const val SETTINGS_EXTRA: String = "SETTINGS_EXTRA"
        const val PREF_NAME: String = "LocalPushFlutter"
        const val SETTINGS_PREF: String = "SETTINGS_PREF"

        const val SUMMARY_ID = 0
        const val GROUP_KEY = "com.GROUP_KEY"

        fun createNotificationChannel(context: Context, channelId: String): String {
            val channelName = "My Background Service"
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                val channel = NotificationChannel(
                    channelId,
                    channelName, NotificationManager.IMPORTANCE_HIGH
                )
                val notificationManager: NotificationManager =
                    context.getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
                notificationManager.createNotificationChannel(channel)
            }
            return channelId
        }
    }

    private val content = this
    private var notificationManagerId = 1
    private var settings = PluginSettings()

    private var socket: ISocket? = null

    private val receiver = object : BroadcastReceiver() {
        override fun onReceive(p0: Context?, p1: Intent?) {
            Log.e(TAG, "onReceive: ${p1?.action}")
            when (p1?.action) {
                CHANGE_SETTING -> {
                    p1.getStringExtra(SETTINGS_EXTRA)?.let {
                        settings = PigeonWrapper.fromString<PluginSettings>(it)
                        socket?.release()
                        socket = null
                        socket = ISocket.register(
                            contentResolver,
                            this@PushNotificationService,
                            settings
                        )
                    }
                }

                else -> {
                    Log.d(TAG, "onReceive: do not support action ${p1?.action}")
                }
            }
        }
    }

    override fun onCreate() {
        super.onCreate()
        startForeground()
        val filter = IntentFilter(CHANGE_SETTING)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            registerReceiver(receiver, filter, RECEIVER_EXPORTED)
        } else {
            ActivityCompat.registerReceiver(
                this, receiver, filter, ContextCompat.RECEIVER_EXPORTED
            )
        }
    }

    private fun startForeground() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            createNotificationChannel(content, content.packageName)
        }
    }

    private fun showNotification(str: String) {
        val notificationResponse = PigeonWrapper.fromString<MessageResponse>(str)

        val channelId =
            createNotificationChannel(
                applicationContext,
                settings.channelNotification ?: content.packageName
            )

        val packageName = content.packageName
        val packageManager = content.packageManager
        val activityIntent = packageManager
            .getLaunchIntentForPackage(packageName)
            ?.apply {
                putExtra("payload", str)
            }
        val pendingIntent = PendingIntent.getActivity(
            content,
            notificationManagerId,
            activityIntent,
            PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE
        )

        val notification = createNotification(
            channelId, notificationResponse.notification.title,
            notificationResponse.notification.body, pendingIntent
        )
        with(NotificationManagerCompat.from(this)) {
            if (ActivityCompat
                    .checkSelfPermission(
                        content,
                        Manifest.permission.POST_NOTIFICATIONS
                    ) != PackageManager.PERMISSION_GRANTED
            ) {
                Log.d(TAG, "showNotification: permission denied")
                return
            }
            notificationManagerId++
            notify(notificationManagerId, notification)

            val summaryNotification = NotificationCompat.Builder(applicationContext, channelId)
                .setSmallIcon(getIcon())
                .setColor(Color.GRAY)
                .setContentTitle(notificationResponse.notification.title)
                .setContentText(notificationResponse.notification.body)
                .setGroup(GROUP_KEY)
                .setGroupSummary(true)
                .build()
            notify(SUMMARY_ID, summaryNotification)
        }
    }

    private fun getIcon(): Int {
        val icons = (settings.iconNotification ?: "")
            .replaceFirst("@", "").split("/")
        return applicationContext.resources.getIdentifier(
            icons.last(),
            icons.first(),
            content.packageName
        )
    }

    private fun createNotification(
        channelId: String,
        title: String,
        body: String,
        pendingIntent: PendingIntent? = null
    ): android.app.Notification {
        val notificationBuilder = NotificationCompat.Builder(applicationContext)
        val mBuilder = notificationBuilder
            .setSmallIcon(getIcon())
            .setDefaults(NotificationCompat.DEFAULT_ALL)
            .setPriority(NotificationManager.IMPORTANCE_HIGH)
            .setColor(Color.GRAY)
            .setChannelId(channelId)
            .setContentTitle(title)
            .setContentText(body)
            .setGroup(GROUP_KEY)
            .setAutoCancel(true)
            .setVibrate(longArrayOf(0))
        if (pendingIntent != null) {
            mBuilder.setContentIntent(pendingIntent)
        }
        return mBuilder.build()
    }

    override fun onBind(p0: Intent?): IBinder? {
        val bundle = p0?.extras
        bundle?.getString(SETTINGS_EXTRA)?.let {
            settings = settings.copyWith(PigeonWrapper.fromString(it))
            socket?.release()
            socket = null
            socket = ISocket.register(contentResolver, this, settings)
            socket?.settings = settings
        }
        return null
    }

    override fun onUnbind(intent: Intent?): Boolean {
        socket?.release()
        socket = null
        return super.onUnbind(intent)
    }

    override fun onDestroy() {
        onUnbind(null)
        super.onDestroy()
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        val bundle = intent?.extras
        bundle?.getString(SETTINGS_EXTRA)?.let {
            settings = settings.copyWith(PigeonWrapper.fromString(it))
            socket?.release()
            socket = null
            socket = ISocket.register(contentResolver, this, settings)
            socket?.settings = settings
        }
        super.onStartCommand(intent, flags, startId)

        val channelId =
            createNotificationChannel(
                applicationContext,
                settings.channelNotification ?: content.packageName
            )
        val notification = createNotification(
            channelId, "Message Service",
            "This is a Message service"
        )
        startForeground(101, notification)
        socket?.createSocket()

        return START_NOT_STICKY
    }

    override fun newMessage(message: String) {
        try {
            if(message == "{\"type\":\"pong\"}") return
            val notificationResponse = PigeonWrapper.fromString<MessageResponse>(message)
            if (LocalPushConnectivityPlugin.flutterApi != null) {
                Handler(Looper.getMainLooper()).post {
                    LocalPushConnectivityPlugin.flutterApi!!.onMessage(
                        notificationResponse.toPigeon(message)
                    ) {
                        if (it.isSuccess) {
                            Log.d(TAG, "newMessage: receive success")
                        } else {
                            Log.e(TAG, "newMessage: receive failure")
                            throw Exception("send failure")
                        }
                    }
                }
            } else {
                throw Exception("LocalPushConnectivityPlugin.flutterApi is null")
            }
        } catch (e: Exception) {
            Log.e(TAG, "newMessage: ${e.message}")
            showNotification(message)
        }
    }
}