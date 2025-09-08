import 'dart:async';

import 'src/messages.g.dart';
import 'src/model.dart';
import 'src/streams.g.dart' as streams;

export 'src/messages.g.dart';

class LocalPushConnectivity extends LocalPushConnectivityPigeonHostApi
    implements LocalPushConnectivityPigeonFlutterApi {
  LocalPushConnectivity._()
    : super(binaryMessenger: null, messageChannelSuffix: '') {
    LocalPushConnectivityPigeonFlutterApi.setUp(this);
    _controller = StreamController<MessageSystemPigeon>.broadcast();
    message = _controller.stream;
  }

  @override
  Future<bool> initialize({
    AndroidSettingsPigeon? android,
    WindowsSettingsPigeon? windows,
    IosSettingsPigeon? ios,
    required TCPModePigeon mode,
  }) async {
    _subscription = streams
        .onMessage()
        .map((e) {
          final messageResponse = MessageResponse.fromString(e);
          return MessageSystemPigeon(
            inApp: false,
            message: MessageResponsePigeon(
              notification: NotificationPigeon(
                title: messageResponse.notification.title,
                body: messageResponse.notification.body,
              ),
              mPayload: messageResponse.mPayload ?? e,
            ),
          );
        })
        .listen(_controller.add);
    return super.initialize(
      android: android,
      windows: windows,
      ios: ios,
      mode: mode,
    );
  }

  late final StreamSubscription _subscription;
  late final StreamController<MessageSystemPigeon> _controller;

  static final LocalPushConnectivity instance = LocalPushConnectivity._();

  late final Stream<MessageSystemPigeon> message;

  @override
  Future<void> onMessage(MessageResponsePigeon message) async {
    _controller.add(MessageSystemPigeon(inApp: true, message: message));
    return;
  }

  void dispose() {
    _subscription.cancel();
    _controller.close();
  }
}
