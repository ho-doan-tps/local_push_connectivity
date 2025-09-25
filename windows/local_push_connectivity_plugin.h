#ifndef FLUTTER_PLUGIN_LOCAL_PUSH_CONNECTIVITY_PLUGIN_H_
#define FLUTTER_PLUGIN_LOCAL_PUSH_CONNECTIVITY_PLUGIN_H_

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>

#include "messages.g.h"

#include <memory>

#include "nlohmann/json.hpp"

#include <winrt/windows.networking.h>
#include <winrt/windows.networking.sockets.h>
#include <winrt/windows.storage.streams.h>
#include <winrt/windows.foundation.h>

using json = nlohmann::json;

using namespace winrt;
using namespace Windows::Networking;
using namespace Windows::Networking::Sockets;
using namespace Windows::Storage::Streams;
using namespace Windows::Foundation;

struct PluginSetting {
    std::string title;
    std::string appBundle;
    std::string displayName;
    std::string iconPath;
    std::string iconContent;
    std::string host;
    std::int64_t port;
    std::int64_t systemType;
    std::string publicHasKey;
    bool wss;
    std::string path;
    bool tcp;
    std::string connector_tag;
    std::string connector_id;
};

struct NotificationResponse {
    std::string title;
    std::string body;
    std::optional<std::string> subtitle;
};
// ========= NotificationResponse ======
inline void to_json(json& j, const NotificationResponse& s) {
    j = json{
        {"Title", s.title},
        {"Body", s.body},
    };
    if (s.subtitle)j["Subtitle"] = *s.subtitle;
}

inline void from_json(const json& j, NotificationResponse& p) {
    j.at("Title").get_to(p.title);
    j.at("Body").get_to(p.body);
    if (j.contains("Subtitle")) p.subtitle = j.at("Subtitle").get<std::string>();
}
// ========= NotificationResponse ======
struct MessageResponse {
    std::optional<NotificationResponse> notification;
};
//// ========= MessageResponse ======
inline void to_json(json& j, const MessageResponse& s) {
    j = json{};
    if (s.notification) j["Notification"] = *s.notification;
}

inline void from_json(const json& j, MessageResponse& p) {
    if (j.contains("Notification")) p.notification = j.at("Notification").get<NotificationResponse>();
}
// ========= MessageResponse ======
struct PongModel {
    std::optional<std::string> pong;
};
//// ========= MessageResponse ======
inline void to_json(json& j, const PongModel& s) {
    j = json{};
    if (s.pong) j["pong"] = *s.pong;
}

inline void from_json(const json& j, PongModel& p) {
    if (j.contains("pong")) p.pong = j.at("pong").get<std::string>();
}
// ========= MessageResponse ======

struct PingModel {
    std::string messageType;
};

struct Sender {
    std::string connectorID;
    std::string connectorTag;
    std::string deviceID;
};

struct DataRegister {
    std::optional<std::string> apnsToken;
    std::optional<std::string> applicationID;
    std::optional<int> apnsServerType;
};

struct RegisterModel {
    std::string messageType;
    int systemType;
    Sender sender;
    DataRegister data;

};

// ================== plugin settings ================================
inline void to_json(json& j, const PluginSetting& s) {
    j = json{
        {"title", s.title},
        {"appBundle", s.appBundle},
        {"displayName", s.displayName},
        {"iconPath",s.iconPath},
        {"iconContent",s.iconContent},
        {"host",s.host},
        {"port", s.port},
        {"systemType", s.systemType},
        {"publicHasKey", s.publicHasKey},
        {"wss",s.wss},
        {"path", s.path},
        {"tcp", s.tcp},
        {"connector_tag",s.connector_tag},
        {"connector_id",s.connector_id},
    };
}

inline void from_json(const json& j, PluginSetting& p) {
    j.at("title").get_to(p.title);
    j.at("appBundle").get_to(p.appBundle);
    j.at("displayName").get_to(p.displayName);
    j.at("iconPath").get_to(p.iconPath);
    j.at("iconContent").get_to(p.iconContent);
    j.at("host").get_to(p.host);
    j.at("port").get_to(p.port);
    j.at("systemType").get_to(p.systemType);
    j.at("publicHasKey").get_to(p.publicHasKey);
    j.at("wss").get_to(p.wss);
    j.at("path").get_to(p.path);
    j.at("tcp").get_to(p.tcp);
    j.at("connector_tag").get_to(p.connector_tag);
    j.at("connector_id").get_to(p.connector_id);
}
// ================== plugin settings ================================
// ================== ping ================================
inline void to_json(json& j, const PingModel& s) {
    j = json{
        {"messageType",s.messageType},
    };
}

inline void from_json(const json& j, PingModel& s) {
    j.at("messageType").get_to(s.messageType);
}
// ================== ping ================================
// ================== sender ================================
inline void to_json(json& j, const Sender& s) {
    j = json{
        {"connectorID",s.connectorID},
        {"connectorTag",s.connectorTag},
        {"deviceID",s.deviceID},
    };
}

inline void from_json(const json& j, Sender& s) {
    j.at("connectorID").get_to(s.connectorID);
    j.at("connectorTag").get_to(s.connectorTag);
    j.at("deviceID").get_to(s.deviceID);
}
// ================== sender ================================
// ================== data ================================
inline void to_json(json& j, const DataRegister& s) {
    j = json{};
    if (s.apnsToken)j["apnsToken"] = *s.apnsToken;
    if (s.applicationID)j["applicationID"] = *s.applicationID;
    if (s.apnsServerType)j["apnsServerType"] = *s.apnsServerType;
}

inline void from_json(const json& j, DataRegister& s) {
    if (j.contains("apnsToken"))
        s.apnsToken = j.at("apnsToken").get<std::string>();
    if (j.contains("applicationID"))
        s.applicationID = j.at("applicationID").get<std::string>();
    if (j.contains("apnsServerType"))
        s.apnsServerType = j.at("apnsServerType").get<int>();
}
// ================== data ================================
// ================== register ================================
inline void to_json(json& j, const RegisterModel& s) {
    j = json{
        {"messageType",s.messageType},
        {"systemType",s.systemType},
        {"sender",s.sender},
        {"data",s.data},
    };
}

inline void from_json(const json& j, RegisterModel& s) {
    j.at("messageType").get_to(s.messageType);
    j.at("systemType").get_to(s.systemType);
    j.at("sender").get_to(s.sender);
    j.at("data").get_to(s.data);
}
// ================== register ================================

namespace local_push_connectivity {
    class PluginWrapper {
    public:
        static std::string toJson(PluginSetting settings);
        static std::string toJson2(PluginSetting settings);
        static PluginSetting fromJson(std::string json_str);
    };

    class LocalPushConnectivityPlugin : public flutter::Plugin, public LocalPushConnectivityPigeonHostApi {
    public:
        static void RegisterWithRegistrar(flutter::PluginRegistrarWindows* registrar);

        LocalPushConnectivityPlugin();

        virtual ~LocalPushConnectivityPlugin();

        static PluginSetting _settings;

        static int RegisterProcess(std::wstring title, _In_ wchar_t* command_line);
        static void HandleMessage(HWND window, UINT const message,
            LPARAM const lparam);

        void Initialize(
            int64_t system_type,
            const AndroidSettingsPigeon* android,
            const WindowsSettingsPigeon* windows,
            const IosSettingsPigeon* ios,
            const TCPModePigeon& mode,
            std::function<void(ErrorOr<bool> reply)> result) override;
        void Config(
            const TCPModePigeon& mode,
            const flutter::EncodableList* ssids,
            std::function<void(ErrorOr<bool> reply)> result) override;
        void RequestPermission(std::function<void(ErrorOr<bool> reply)> result) override;
        void Start(std::function<void(ErrorOr<bool> reply)> result) override;
        void Stop(std::function<void(ErrorOr<bool> reply)> result) override;
        void RegisterUser(
            const UserPigeon& user,
            std::function<void(ErrorOr<bool> reply)> result) override;
        void DeviceID(std::function<void(ErrorOr<std::string> reply)> result) override;
    private:
        static std::wstring _title;
        static std::wstring  title() { return _title; }
        static PluginSetting gSetting() { return _settings; }
        static void set_title(std::wstring title) {
            _title = title;
        }
        static void saveSetting(PluginSetting settings) {
            _settings = settings;
        }
        static std::optional<LocalPushConnectivityPigeonFlutterApi> _flutterApi;
        static std::optional<LocalPushConnectivityPigeonFlutterApi> gFlutterApi() { return _flutterApi; }
        static void setApi(LocalPushConnectivityPigeonFlutterApi api) { _flutterApi = api; }
        static void startThreadInChildProcess();
        static int createBackgroundProcess();
        static void killProcess(int pid);
        static LRESULT CALLBACK HiddenWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
        static HWND createHiddenWindow(HINSTANCE hInstance);
        static DWORD WINAPI ThreadFunction(LPVOID lpParam);
        static void sendSettings();
        static void sendMessageFromNoti(bool isDelay);
        static std::optional<MessageSystemPigeon> _m;
    };

    class SocketClient {
    public:
        static PluginSetting _settings;
        static void saveSettings(PluginSetting settings) { _settings = settings; };
        static void m_connect(PluginSetting* param);
    private:
        static PluginSetting gSettings() { return _settings; };
        static void connectWss();
        static void _heartbeat();
        static void receiverWss(StreamWebSocket& socket);
        static StreamWebSocket _socket;
        static DataWriter _writer;
    };
}  // namespace local_push_connectivity

#endif  // FLUTTER_PLUGIN_LOCAL_PUSH_CONNECTIVITY_PLUGIN_H_
