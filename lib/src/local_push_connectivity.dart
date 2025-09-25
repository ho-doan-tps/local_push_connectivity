import 'dart:async';
import 'dart:io';

import 'messages.g.dart';

class LocalPushConnectivity extends LocalPushConnectivityPigeonHostApi
    implements LocalPushConnectivityPigeonFlutterApi {
  LocalPushConnectivity._()
    : super(binaryMessenger: null, messageChannelSuffix: '') {
    LocalPushConnectivityPigeonFlutterApi.setUp(this);
    _controller = StreamController<MessageSystemPigeon>.broadcast();
    message = _controller.stream.map((e) {
      if (Platform.isWindows) {
        return MessageSystemPigeon(
          fromNotification: e.fromNotification,
          mrp: MessageResponsePigeon(
            notification: e.mrp.notification,
            mPayload: e.mrp.mPayload.replaceFirst(RegExp(r'\[\]$'), ''),
          ),
        );
      }
      return e;
    });
  }

  late final StreamController<MessageSystemPigeon> _controller;

  static final LocalPushConnectivity instance = LocalPushConnectivity._();

  late final Stream<MessageSystemPigeon> message;

  @override
  Future<void> onMessage(MessageSystemPigeon message) async {
    _controller.add(message);
    return;
  }

  void dispose() {
    _controller.close();
  }
}
