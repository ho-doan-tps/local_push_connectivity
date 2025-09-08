package com.hodoan.local_push_connectivity.wrapper

import com.hodoan.local_push_connectivity.MessageResponsePigeon
import com.hodoan.local_push_connectivity.NotificationPigeon
import com.hodoan.local_push_connectivity.PluginSettingsPigeon
import com.hodoan.local_push_connectivity.RegisterMessagePigeon
import kotlinx.serialization.SerialName
import kotlinx.serialization.Serializable
import kotlinx.serialization.json.Json

class PigeonWrapper {
    companion object {
        inline fun <reified T> toString(model: T): String {
            val json = Json {
                ignoreUnknownKeys = true
            }
            return json.encodeToString<T>(model)
        }

        inline fun <reified T> fromString(str: String): T {
            val json = Json {
                ignoreUnknownKeys = true
            }
            return json.decodeFromString<T>(str)
        }
    }
}

@Serializable
data class RegisterMessage(
    @SerialName("MessageType")
    val messageType: String,
    @SerialName("SendConnectorID")
    val sendConnectorID: String,
    @SerialName("SendDeviceId")
    val sendDeviceId: String,
    @SerialName("SystemType")
    val systemType: Long
) {
    companion object {
        fun fromPigeon(model: RegisterMessagePigeon): RegisterMessage {
            return RegisterMessage(
                messageType = model.messageType,
                systemType = model.systemType,
                sendConnectorID = model.sendConnectorID,
                sendDeviceId = model.sendDeviceId,
            )
        }
    }
}

@Serializable
data class PluginSettings(
    val host: String? = null,
    var deviceId: String? = null,
    var connectorID: String? = null,
    val systemType: Long? = null,
    val iconNotification: String? = null,
    val port: Long? = null,
    val channelNotification: String? = null,
    val wss: Boolean? = null,
    val wsPath: String? = null,
    val useTcp: Boolean? = null,
    val publicKey: String? = null
) {
    companion object {
        fun fromPigeon(model: PluginSettingsPigeon): PluginSettings {
            return PluginSettings(
                model.host,
                model.deviceId,
                model.connectorID,
                model.systemType,
                model.iconNotification,
                model.port,
                model.channelNotification,
                model.wss,
                model.wsPath,
                model.useTcp,
                model.publicKey
            )
        }
    }

    fun copyWith(model: PluginSettings, useOldUser: Boolean = true): PluginSettings =
        PluginSettings(
            host = model.host ?: host,
            deviceId = model.deviceId ?: deviceId,
            connectorID = model.connectorID ?: (if (useOldUser) connectorID else null),
            systemType = model.systemType ?: systemType,
            iconNotification = model.iconNotification ?: iconNotification,
            channelNotification = model.channelNotification ?: channelNotification,
            port = model.port ?: port,
            wss = model.wss ?: wss,
            wsPath = model.wsPath ?: wsPath,
            useTcp = model.useTcp ?: useTcp,
            publicKey = model.publicKey ?: publicKey
        )
}

@Serializable
data class Notification(
    @SerialName("Title")
    val title: String,
    @SerialName("Body")
    val body: String
) {
    companion object {
        fun fromPigeon(model: NotificationPigeon): Notification {
            return Notification(
                title = model.title,
                body = model.body
            )
        }
    }
}

@Serializable
data class MessageResponse(
    @SerialName("Notification")
    val notification: Notification,
    val mPayload: String? = null
) {
    companion object {
        fun fromPigeon(model: MessageResponsePigeon): MessageResponse {
            return MessageResponse(
                notification = Notification.fromPigeon(model.notification)
            )
        }
    }

    fun toPigeon(str: String): MessageResponsePigeon {
        return MessageResponsePigeon(
            NotificationPigeon(notification.title, notification.body),
            str
        )
    }
}