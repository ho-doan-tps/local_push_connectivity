import 'package:pigeon/pigeon.dart';

@ConfigurePigeon(
  PigeonOptions(
    dartPackageName: 'local_push_connectivity',
    dartOut: 'lib/src/streams.g.dart',
    swiftOut:
        'ios/local_push_connectivity/Sources/local_push_connectivity/Streams.g.swift',
    swiftOptions: SwiftOptions(),
    kotlinOut:
        'android/src/main/kotlin/com/hodoan/local_push_connectivity/Streams.g.kt',
    kotlinOptions: KotlinOptions(package: 'com.hodoan.local_push_connectivity'),
    // TODO: pigeon not supported yet
    // cppHeaderOut: 'windows/runner/streams.g.h',
    // cppOptions: CppOptions(namespace: 'local_push_connectivity'),
    // cppSourceOut: 'windows/runner/streams.g.cpp',
  ),
)
@EventChannelApi()
abstract class StreamsEventChannel {
  String onMessage();
}
