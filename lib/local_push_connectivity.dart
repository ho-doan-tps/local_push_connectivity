
import 'local_push_connectivity_platform_interface.dart';

class LocalPushConnectivity {
  Future<String?> getPlatformVersion() {
    return LocalPushConnectivityPlatform.instance.getPlatformVersion();
  }
}
