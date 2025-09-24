import 'dart:async';

import 'messages.g.dart';

class LocalPushConnectivity extends LocalPushConnectivityPigeonHostApi
    implements LocalPushConnectivityPigeonFlutterApi {
  LocalPushConnectivity._()
    : super(binaryMessenger: null, messageChannelSuffix: '') {
    LocalPushConnectivityPigeonFlutterApi.setUp(this);
    _controller = StreamController<MessageSystemPigeon>.broadcast();
    message = _controller.stream;
  }

  late final StreamController<MessageSystemPigeon> _controller;

  static final LocalPushConnectivity instance = LocalPushConnectivity._();

  late final Stream<MessageSystemPigeon> message;

  @override
  Future<void> onMessage(MessageResponsePigeon message) async {
    _controller.add(MessageSystemPigeon(inApp: true, mrp: message));
    return;
  }

  void dispose() {
    _controller.close();
  }
}
