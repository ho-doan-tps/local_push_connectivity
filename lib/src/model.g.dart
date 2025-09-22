// GENERATED CODE - DO NOT MODIFY BY HAND

part of 'model.dart';

// **************************************************************************
// JsonSerializableGenerator
// **************************************************************************

Notification _$NotificationFromJson(Map<String, dynamic> json) =>
    Notification(title: json['Title'] as String, body: json['Body'] as String);

Map<String, dynamic> _$NotificationToJson(Notification instance) =>
    <String, dynamic>{'Title': instance.title, 'Body': instance.body};

MessageResponse _$MessageResponseFromJson(Map<String, dynamic> json) =>
    MessageResponse(
      notification: Notification.fromJson(
        json['Notification'] as Map<String, dynamic>,
      ),
      mPayload: json['mPayload'] as String?,
    );

Map<String, dynamic> _$MessageResponseToJson(MessageResponse instance) =>
    <String, dynamic>{
      'Notification': instance.notification,
      'mPayload': instance.mPayload,
    };

RegisterMessage _$RegisterMessageFromJson(Map<String, dynamic> json) =>
    RegisterMessage(
      messageType: json['MessageType'] as String,
      sendConnectorID: json['SendConnectorID'] as String,
      sendDeviceId: json['SendDeviceId'] as String,
      systemType: (json['SystemType'] as num).toInt(),
    );

Map<String, dynamic> _$RegisterMessageToJson(RegisterMessage instance) =>
    <String, dynamic>{
      'MessageType': instance.messageType,
      'SendConnectorID': instance.sendConnectorID,
      'SendDeviceId': instance.sendDeviceId,
      'SystemType': instance.systemType,
    };
