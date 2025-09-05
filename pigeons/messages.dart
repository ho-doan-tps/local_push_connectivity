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
class BaseModePigeon {
  const BaseModePigeon();
}

class TCPModePigeon extends BaseModePigeon {
  final String host;
  final int port;

  const TCPModePigeon({required this.host, required this.port});
}
