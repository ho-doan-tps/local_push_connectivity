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
data class RegisterModel(
    val messageType: String,
    val systemType: Long,
    val sender: SenderRegister,
    val data: DataRegister
)

@Serializable
data class SenderRegister(
    val connectorID: String,
    val connectorTag: String,
    val deviceID: String
)

@Serializable
data class DataRegister(
    val apnsToken: String?,
    val applicationID: String?,
    val apnsServerType: Int?
)

@Serializable
data class PluginSettings(
    val host: String? = null,
    var deviceId: String? = null,
    var connectorID: String? = null,
    var connectorTag: String? = null,
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
                model.connectorTag,
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
            connectorTag = model.connectorTag,
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
data class NotificationModel(
    val Title: String,
    val Body: String
) {
    companion object {
        fun fromPigeon(model: NotificationPigeon): NotificationModel {
            return NotificationModel(
                Title = model.title,
                Body = model.body
            )
        }
    }
}

@Serializable
data class MessageResponse(
    val Notification: NotificationModel,
    val mPayload: String? = null
) {
    companion object {
        fun fromPigeon(model: MessageResponsePigeon): MessageResponse {
            return MessageResponse(
                Notification = NotificationModel.fromPigeon(model.notification)
            )
        }
    }

    fun toPigeon(str: String): MessageResponsePigeon {
        return MessageResponsePigeon(
            NotificationPigeon(Notification.Title, Notification.Body),
            str
        )
    }
}

@Serializable
data class PingModel(
    val messageType: String
)

@Serializable
data class PongModel(
    val pong: String?
)