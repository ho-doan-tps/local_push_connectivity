import 'package:pigeon/pigeon.dart';

@ConfigurePigeon(
  PigeonOptions(
    dartPackageName: 'local_push_connectivity',
    dartOut: 'lib/src/messages.g.dart',
    swiftOut:
        'ios/local_push_connectivity/Sources/local_push_connectivity/Messages.g.swift',
    swiftOptions: SwiftOptions(),
    kotlinOut:
        'android/src/main/kotlin/com/hodoan/local_push_connectivity/Messages.g.kt',
    kotlinOptions: KotlinOptions(package: 'com.hodoan.local_push_connectivity'),
    cppHeaderOut: 'windows/runner/messages.g.h',
    cppOptions: CppOptions(namespace: 'local_push_connectivity'),
    cppSourceOut: 'windows/runner/messages.g.cpp',
  ),
)
enum ConnectionType { tcp, tcpTls, ws, wss }

class TCPModePigeon {
  final String host;
  final int port;
  final ConnectionType connectionType;

  /// Path for ws and wss
  final String? path;

  /// for tcpTls
  final String? publicHasKey;

  /// for tcpTls & Platform windows
  final String? cnName;

  /// for tcpTls & Platform windows
  final String? dnsName;

  const TCPModePigeon({
    required this.host,
    required this.port,
    required this.connectionType,
    this.path,
    this.publicHasKey,
    this.cnName,
    this.dnsName,
  });
}

class AndroidSettingsPigeon {
  final String icon;
  final String channelNotification;

  const AndroidSettingsPigeon({
    required this.icon,
    required this.channelNotification,
  });
}

class WindowsSettingsPigeon {
  final String displayName;
  final String bundleId;
  final String icon;
  final String iconContent;

  const WindowsSettingsPigeon({
    required this.displayName,
    required this.bundleId,
    required this.icon,
    required this.iconContent,
  });
}

class IosSettingsPigeon {
  final String? ssid;
  final bool enableSSID;

  const IosSettingsPigeon({this.ssid, this.enableSSID = true});
}

@FlutterApi()
abstract class LocalPushConnectivityPigeonFlutterApi {
  @async
  void onMessage(MessageResponsePigeon message);
}

@HostApi()
abstract class LocalPushConnectivityPigeonHostApi {
  @async
  bool initialize({
    AndroidSettingsPigeon? android,
    WindowsSettingsPigeon? windows,
    // WebSettingsPigeon? web,
    IosSettingsPigeon? ios,
    required TCPModePigeon mode,
  });
  @async
  bool config(TCPModePigeon mode, [String? ssid]);
  @async
  bool requestPermission();
  @async
  bool start();
  @async
  bool stop();
}

class MessageSystemPigeon {
  final bool inApp;
  final MessageResponsePigeon message;

  const MessageSystemPigeon({required this.inApp, required this.message});
}

class NotificationPigeon {
  final String title;
  final String body;

  NotificationPigeon({required this.title, required this.body});
}

class MessageResponsePigeon {
  final NotificationPigeon notification;
  final String mPayload;

  MessageResponsePigeon({required this.mPayload, required this.notification});
}

class RegisterMessagePigeon {
  final String messageType;
  final String sendConnectorID;
  final String sendDeviceId;
  final int systemType;

  const RegisterMessagePigeon({
    required this.messageType,
    required this.sendConnectorID,
    required this.systemType,
    required this.sendDeviceId,
  });
}

class PluginSettingsPigeon {
  final String? host;
  final String? deviceId;
  final String? connectorID;
  final int? systemType;
  final String? iconNotification;
  final int? port;
  final String? channelNotification;

  final bool? wss;
  //default for ws path
  final String? wsPath;
  final bool? useTcp;
  final String? publicKey;

  const PluginSettingsPigeon({
    required this.host,
    required this.deviceId,
    required this.connectorID,
    required this.systemType,
    required this.iconNotification,
    required this.port,
    required this.channelNotification,
    required this.wss,
    required this.wsPath,
    required this.useTcp,
    required this.publicKey,
  });
}
