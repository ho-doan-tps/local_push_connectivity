#ifndef FLUTTER_PLUGIN_LOCAL_PUSH_CONNECTIVITY_PLUGIN_H_
#define FLUTTER_PLUGIN_LOCAL_PUSH_CONNECTIVITY_PLUGIN_H_

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>

#include "messages.g.h"

#include <memory>

namespace local_push_connectivity {

    struct PluginSetting {
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
            const AndroidSettingsPigeon* android,
            const WindowsSettingsPigeon* windows,
            const IosSettingsPigeon* ios,
            const TCPModePigeon& mode,
            std::function<void(ErrorOr<bool> reply)> result) override;
        void Config(
            const TCPModePigeon& mode,
            const std::string* ssid,
            std::function<void(ErrorOr<bool> reply)> result) override;
        void RequestPermission(std::function<void(ErrorOr<bool> reply)> result) override;
        void Start(std::function<void(ErrorOr<bool> reply)> result) override;
        void Stop(std::function<void(ErrorOr<bool> reply)> result) override;
    private:
        static PluginSetting gSetting() { return _settings; }
        static void saveSetting(PluginSetting settings) {
            _settings = settings;
        }
        static void startThreadInChildProcess();
        static int createBackgroundProcess(std::function<void(ErrorOr<bool> reply)> result);
        static void killProcess(int pid);
        static LRESULT CALLBACK HiddenWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
        static HWND createHiddenWindow(HINSTANCE hInstance);
        static DWORD WINAPI ThreadFunction(LPVOID lpParam);
        static void sendSettings(const std::wstring& settings);
    };

    class SocketClient {
    public:
        static PluginSetting _settings;
        static  std::function<void()> _callback;
        static void saveSettings(PluginSetting settings) { _settings = settings; };
        static void m_connect(PluginSetting* param, std::function<void()> callback);
    private:
        static PluginSetting gSettings() { return _settings; };
        static std::function<void()> callback() { return _callback; };
        static void saveCallback(std::function<void()> callback) {
            _callback = callback;
        };
        static void connectWss();
        static void receiverWss();
    };
}  // namespace local_push_connectivity

#endif  // FLUTTER_PLUGIN_LOCAL_PUSH_CONNECTIVITY_PLUGIN_H_
