import 'dart:convert';

import 'package:json_annotation/json_annotation.dart';

part 'model.g.dart';

@JsonSerializable()
class Notification {
  Notification({required this.title, required this.body});

  @JsonKey(name: 'Title')
  final String title;
  @JsonKey(name: 'Body')
  final String body;

  factory Notification.fromJson(Map<String, dynamic> json) =>
      _$NotificationFromJson(json);
  Map<String, dynamic> toJson() => _$NotificationToJson(this);
}

@JsonSerializable()
class MessageResponse {
  const MessageResponse({required this.notification, required this.mPayload});

  @JsonKey(name: 'Notification')
  final Notification notification;
  @JsonKey(name: 'mPayload')
  final String? mPayload;

  factory MessageResponse.fromJson(Map<String, dynamic> json) =>
      _$MessageResponseFromJson(json);

  factory MessageResponse.fromString(String str) =>
      MessageResponse.fromJson(jsonDecode(str));
  Map<String, dynamic> toJson() => _$MessageResponseToJson(this);
}

@JsonSerializable()
class RegisterMessage {
  RegisterMessage({
    required this.messageType,
    required this.sendConnectorID,
    required this.sendDeviceId,
    required this.systemType,
  });

  @JsonKey(name: 'MessageType')
  final String messageType;

  @JsonKey(name: 'SendConnectorID')
  final String sendConnectorID;
  @JsonKey(name: 'SendDeviceId')
  final String sendDeviceId;
  @JsonKey(name: 'SystemType')
  final int systemType;

  factory RegisterMessage.fromJson(Map<String, dynamic> json) =>
      _$RegisterMessageFromJson(json);
  Map<String, dynamic> toJson() => _$RegisterMessageToJson(this);
}
