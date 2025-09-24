import 'dart:async';
import 'dart:convert';
import 'dart:developer';
import 'dart:js_interop';
import 'dart:math' as math;
import 'package:web/web.dart' as web;

import 'package:flutter/services.dart';
import 'package:web_socket_channel/web_socket_channel.dart';

import 'messages.g.dart';
import 'model.dart';

class LocalPushConnectivity
    implements
        LocalPushConnectivityPigeonHostApi,
        LocalPushConnectivityPigeonFlutterApi {
  LocalPushConnectivity._() {
    LocalPushConnectivityPigeonFlutterApi.setUp(this);
    _controller = StreamController<MessageSystemPigeon>.broadcast();
    message = _controller.stream;
    _init();
  }

  late final StreamController<MessageSystemPigeon> _controller;

  static final LocalPushConnectivity instance = LocalPushConnectivity._();

  late final Stream<MessageSystemPigeon> message;

  PluginSettingsPigeon pluginSettings = PluginSettingsPigeon();

  bool _isFocus = false;

  @override
  Future<void> onMessage(MessageResponsePigeon message) async {
    _controller.add(MessageSystemPigeon(inApp: true, mrp: message));
    return;
  }

  void dispose() {
    _controller.close();
  }

  void _init() {
    final isFocus = web.document.hasFocus();
    _isFocus = isFocus;
    web.window.addEventListener(
      'focus',
      ((web.Event _) {
        _isFocus = true;
        log('Window focused');
      }).toJS,
    );
    web.window.addEventListener(
      'blur',
      ((web.Event _) {
        _isFocus = false;
        log('Window blurred');
      }).toJS,
    );
  }

  @override
  Future<bool> config(TCPModePigeon mode, [String? ssid]) async {
    pluginSettings.host = mode.host;
    pluginSettings.publicKey = mode.publicHasKey;
    pluginSettings.port = mode.port;
    pluginSettings.wsPath = mode.path;
    pluginSettings.wss = mode.connectionType == ConnectionType.wss;
    pluginSettings.useTcp = mode.connectionType == ConnectionType.tcp;
    pluginSettings.systemType = 77;
    // TODO: get connector id
    pluginSettings.connectorID = '4';
    pluginSettings.deviceId = getDeviceID();
    await start();
    return true;
  }

  @override
  Future<bool> initialize({
    AndroidSettingsPigeon? android,
    WindowsSettingsPigeon? windows,
    IosSettingsPigeon? ios,
    required TCPModePigeon mode,
  }) async {
    pluginSettings.host = mode.host;
    pluginSettings.publicKey = mode.publicHasKey;
    pluginSettings.port = mode.port;
    pluginSettings.wsPath = mode.path;
    pluginSettings.wss = mode.connectionType == ConnectionType.wss;
    pluginSettings.useTcp = mode.connectionType == ConnectionType.tcp;
    pluginSettings.systemType = 77;
    // TODO: get connector id
    pluginSettings.connectorID = '4';
    pluginSettings.deviceId = getDeviceID();
    return true;
  }

  @override
  // ignore: non_constant_identifier_names
  BinaryMessenger? get pigeonVar_binaryMessenger => null;

  @override
  // ignore: non_constant_identifier_names
  String get pigeonVar_messageChannelSuffix => '';

  @override
  Future<bool> requestPermission() async {
    final permission = await web.Notification.requestPermission().toDart;
    if (permission.toDart == 'granted') {
      log('Permission granted');
      return true;
    } else {
      log('Permission denied');
      return false;
    }
  }

  WebSocketChannel? _webSocketChannel;

  @override
  Future<bool> start() async {
    await stop();
    log('Starting WebSocket connection');
    _webSocketChannel = WebSocketChannel.connect(
      Uri.parse(
        'ws://${pluginSettings.host}:${pluginSettings.port}${pluginSettings.wsPath}',
      ),
    );
    await _webSocketChannel?.ready;
    _webSocketChannel?.sink.add(
      jsonEncode(
        RegisterMessage(
          messageType: 'register',
          sendConnectorID: pluginSettings.connectorID ?? '',
          systemType: pluginSettings.systemType ?? 0,
          sendDeviceId: pluginSettings.deviceId ?? '',
        ).toJson(),
      ),
    );
    _webSocketChannel?.stream.listen(
      (event) {
        final message = MessageResponse.fromString(event.toString());
        if (!_isFocus) {
          _sendNotification(
            MessageResponse(
              notification: message.notification,
              mPayload: event.toString(),
            ),
            onNotificationClick: (value) {
              _controller.add(
                MessageSystemPigeon(
                  inApp: true,
                  mrp: MessageResponsePigeon(
                    notification: NotificationPigeon(
                      title: message.notification.title,
                      body: message.notification.body,
                    ),
                    mPayload: value,
                  ),
                ),
              );
            },
          );
        }
      },
      onDone: () {
        log('WebSocket connection closed');
        if (!_closeConnection) {
          start();
        }
      },
      onError: (error) {
        log('WebSocket connection error: $error');
        if (!_closeConnection) {
          start();
        }
      },
      cancelOnError: true,
    );
    return true;
  }

  bool _closeConnection = false;

  @override
  Future<bool> stop() async {
    _closeConnection = true;
    _webSocketChannel?.sink.close();
    _webSocketChannel = null;
    return true;
  }

  bool _sendNotification(
    MessageResponse message, {
    ValueChanged<String>? onNotificationClick,
  }) {
    if (message.notification.title == '') return false;

    var notification = web.Notification(
      message.notification.title,
      web.NotificationOptions(body: message.notification.body),
    );

    notification.onclick = (web.Event event) {
      event.preventDefault();
      web.window.focus();
      notification.close();
      onNotificationClick?.call(message.mPayload ?? '');
    }.toJS;

    return true;
  }

  // Function to generate a UUID (v4)
  String generateUUID() {
    final random = math.Random();
    return 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replaceAllMapped(
      RegExp(r'x|y'),
      (match) {
        final r = random.nextInt(16);
        final v = match[0] == 'x' ? r : (r & 0x3 | 0x8);
        return v.toRadixString(16);
      },
    );
  }

  /// Function to set a cookie
  void setCookie(String name, String value, int days) {
    DateTime now = DateTime.now();
    DateTime expiryDate = now.add(Duration(days: days));
    String expires = 'expires=${expiryDate.toUtc().toIso8601String()}';
    web.document.cookie = '$name=$value; $expires; path=/';
  }

  /// Function to retrieve a specific cookie
  String? getCookie(String name) {
    String nameEQ = '$name=';
    List<String> cookies = web.document.cookie.split(';');
    for (var cookie in cookies) {
      String trimmedCookie = cookie.trim();
      if (trimmedCookie.startsWith(nameEQ)) {
        return trimmedCookie.substring(nameEQ.length);
      }
    }
    return null;
  }

  /// Main function to get or create a device ID
  String getDeviceID() {
    String? deviceId = getCookie("deviceId");
    if (deviceId == null) {
      deviceId = generateUUID(); // Generate a UUID
      setCookie("deviceId", deviceId, 365); // Cookie expires in 365 days
    }
    return deviceId;
  }

  //#endregion
}
