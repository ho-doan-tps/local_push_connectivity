#include "local_push_connectivity_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>

// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <VersionHelpers.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <memory>
#include <sstream>

#include "messages.g.h"
#include "win_toast.h"
#include "utils.h"

#include <cstdlib>

#include <winrt/windows.networking.h>
#include <winrt/windows.networking.sockets.h>
#include <winrt/windows.storage.streams.h>
#include <winrt/windows.foundation.h>

#include <TlHelp32.h>

using namespace winrt;
using namespace Windows::Networking;
using namespace Windows::Networking::Sockets;
using namespace Windows::Storage::Streams;
using namespace Windows::Foundation;

namespace local_push_connectivity {

    // static
    void LocalPushConnectivityPlugin::RegisterWithRegistrar(
        flutter::PluginRegistrarWindows* registrar) {
        auto plugin = std::make_unique<LocalPushConnectivityPlugin>();
        LocalPushConnectivityPigeonHostApi::SetUp(registrar->messenger(), plugin.get());
        registrar->AddPlugin(std::move(plugin));
    }

    LocalPushConnectivityPlugin::LocalPushConnectivityPlugin() {}

    LocalPushConnectivityPlugin::~LocalPushConnectivityPlugin() {}

    int LocalPushConnectivityPlugin::RegisterProcess(std::wstring title,
        _In_ wchar_t* command_line) {
        int numArgs;
        LPWSTR* argv = CommandLineToArgvW(command_line, &numArgs);
        std::wcout << "path: " << command_line;
        if (argv == nullptr || numArgs == 1) {
            return 1;
        }
        std::wstring processName = argv[0];
        if (processName != L"child") {
            return -1;
        }

        auto oldPid = read_pid();
        if (oldPid != -1) {
            killProcess(oldPid);
        }

        DWORD pid = GetCurrentProcessId();
        write_pid(pid);

        std::wstring appBundle = argv[1];
        std::wstring displayName = argv[2];
        std::wstring iconPath = argv[3];
        std::wstring iconContent = argv[4];
        std::wstring host = argv[5];
        std::wstring port = argv[6];
        std::wstring systemType = argv[7];
        std::wstring publicHasKey = argv[8];
        std::wstring wss = argv[9];
        std::wstring path = argv[10];
        std::wstring tcp = argv[11];
        auto setting = PluginSetting{
            wide_to_utf8(appBundle),
            wide_to_utf8(displayName),
            wide_to_utf8(iconPath),
            wide_to_utf8(iconContent),
            wide_to_utf8(host),
            std::stoll(port),
            std::stoll(systemType),
            wide_to_utf8(publicHasKey),
            wss == L"true",
            wide_to_utf8(path),
            tcp == L"true"
        };
        LocalPushConnectivityPlugin::saveSetting(setting);
        LocalPushConnectivityPlugin::startThreadInChildProcess([result]() {

        });
        LocalFree(argv);
        return 0;
    }
    void LocalPushConnectivityPlugin::HandleMessage(HWND window, UINT const message,
        LPARAM const lparam) {
        if (message == WM_COPYDATA) {
            auto cp_struct = reinterpret_cast<COPYDATASTRUCT*>(lparam);
            try {
                const wchar_t* data = reinterpret_cast<const wchar_t*>(cp_struct->lpData);
                std::wstring strs(data, cp_struct->cbData / sizeof(wchar_t));
                // show notifi
            }
            catch (hresult_error const& e) {
                std::wcout << "Error message: " << e.message().c_str();
            }
        }
    }

    LRESULT CALLBACK LocalPushConnectivityPlugin::HiddenWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
        case WM_COPYDATA: {
            auto* pCDS = reinterpret_cast<PCOPYDATASTRUCT>(lParam);
            MessageBoxA(NULL, "ChildHiddenWindow", "receiver ok", MB_OK);
            if (pCDS->dwData == 100) {
                std::wstring receiver((wchar_t*)pCDS->lpData);
                // TODO: handle data settings
                // SocketClient::updateSettings
                auto argv = _split(receiver, L"***");

                std::wstring appBundle = argv[1];
                std::wstring displayName = argv[2];
                std::wstring iconPath = argv[3];
                std::wstring iconContent = argv[4];
                std::wstring host = argv[5];
                std::wstring port = argv[6];
                std::wstring systemType = argv[7];
                std::wstring publicHasKey = argv[8];
                std::wstring wss = argv[9];
                std::wstring path = argv[10];
                std::wstring tcp = argv[11];
                auto settings = PluginSetting{
                    wide_to_utf8(appBundle),
                    wide_to_utf8(displayName),
                    wide_to_utf8(iconPath),
                    wide_to_utf8(iconContent),
                    wide_to_utf8(host),
                    std::stoll(port),
                    std::stoll(systemType),
                    wide_to_utf8(publicHasKey),
                    wss == L"true",
                    wide_to_utf8(path),
                    tcp == L"true"
                };

                SocketClient::saveSettings(settings);

                MessageBoxA(NULL, wide_to_utf8(receiver).c_str(), wide_to_utf8(receiver).c_str(), MB_OK);
                std::wcout << L"[Child] receiver: " << receiver;
            }
            return TRUE;
        }
        }
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    HWND LocalPushConnectivityPlugin::createHiddenWindow(HINSTANCE hInstance) {
        WNDCLASSW wc = { 0 };
        auto settings = gSetting();
        wc.lpfnWndProc = HiddenWndProc;
        wc.hInstance = hInstance;
        //wc.lpszClassName = utf8_to_wide(settings.appBundle).c_str();
        wc.lpszClassName = L"ChildHiddenWindow";

        RegisterClassW(&wc);
        return CreateWindowExW(0, wc.lpszClassName, L"ChildHiddenWindow",
            0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL);
    }

    DWORD WINAPI LocalPushConnectivityPlugin::ThreadFunction(LPVOID lpParam) {
        PluginSetting* param = (PluginSetting*)lpParam;
        createHiddenWindow(GetModuleHandle(NULL));
        try{
        // connect ws
        }
        catch (hresult_error& e) {
            DesktopNotificationManagerCompat::sendToastProcess(
                utf8_to_wide(param->appBundle),
                L"error", e.message().c_str()
            );
        }
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        return 0;
    }

    void LocalPushConnectivityPlugin::sendSettings(const std::wstring& settings) {
        HWND hwndChild = FindWindow(L"ChildHiddenWindow", NULL);
        if (!hwndChild) {
            MessageBoxA(NULL, "ChildHiddenWindow", "ChildHiddenWindow is null", MB_OK);
            return;
        }
        COPYDATASTRUCT cds;
        cds.dwData = 100;
        cds.cbData = (DWORD)(settings.size() + 1) * sizeof(wchar_t);
        cds.lpData = (PVOID)settings.c_str();
        SendMessage(hwndChild, WM_COPYDATA, (WPARAM)GetCurrentProcessId(),
            (LPARAM)&cds);
    }

    int LocalPushConnectivityPlugin::createBackgroundProcess(std::function<void(ErrorOr<bool> reply)> result) {
        auto oldPid = read_pid();
        if (oldPid != -1) {
            killProcess(oldPid);
        }

        PROCESS_INFORMATION pi;
        STARTUPINFO si = { 0 };
        si.cb = sizeof(STARTUPINFO);
        wchar_t execuablePath[MAX_PATH];

        if (GetModuleFileName(NULL, execuablePath, MAX_PATH) == 0) {
            std::wcerr << L"get module file name error: " << GetLastError() << std::endl;
            return -1;
        }

        PluginSetting settings = LocalPushConnectivityPlugin::gSetting();

        wchar_t commandLine[700];
        swprintf(commandLine, 700, L"\"%s\" child \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%lld\" \"%lld\" \"%s\" \"%s\" \"%s\" \"%s\"",
            execuablePath,
            utf8_to_wide(settings.appBundle).c_str(),
            utf8_to_wide(settings.displayName).c_str(),
            utf8_to_wide(settings.iconPath).c_str(),
            utf8_to_wide(settings.iconContent).c_str(),
            utf8_to_wide(settings.host).c_str(),
            settings.port,
            settings.systemType,
            utf8_to_wide(settings.publicHasKey).c_str(),
            settings.wss ? L"true" : L"false",
            utf8_to_wide(settings.path).c_str(),
            settings.tcp ? L"true" : L"false"
        );

        std::wcout << "exe service notification: " << commandLine << L"\n";
        if (CreateProcess(NULL, commandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
            std::wcout << L"create process ok";
            return 0;
        }
        else {
            std::wcerr << L"create process error" << GetLastError() << std::endl;
        }
        return -1;
    }

    void LocalPushConnectivityPlugin::killProcess(int pid) {
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE) {
            std::wcout << "Failed to take process snapshot!";
            return;
        }
        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hSnapshot, &pe)) {
            do {
                auto peid = static_cast<int>(pe.th32ProcessID);
                if (peid == pid) {
                    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                    if (hProcess) {
                        TerminateProcess(hProcess, 0);
                        CloseHandle(hProcess);
                    }
                }
                else {
                    continue;
                }
            } while (Process32Next(hSnapshot, &pe));
        }
        write_pid(-1);
        CloseHandle(hSnapshot);
    }

    void LocalPushConnectivityPlugin::startThreadInChildProcess() {
        auto settings = LocalPushConnectivityPlugin::gSetting();
        HANDLE hThread = CreateThread(
            NULL, 0, LocalPushConnectivityPlugin::ThreadFunction, &settings,
            0, NULL
        );

        if (hThread == NULL) {
            std::wcerr << "Error create thread: " << GetLastError() << std::endl;
        }
        ::WaitForSingleObject(hThread, INFINITE);
        ::CloseHandle(hThread);
    }

    void LocalPushConnectivityPlugin::Initialize(const AndroidSettingsPigeon* android,
        const WindowsSettingsPigeon* windows,
        const IosSettingsPigeon* ios,
        const TCPModePigeon& mode,
        std::function<void(ErrorOr<bool> reply)> result) {
        //result(FlutterError("NNN", "detail"));
        std::string public_has_key = "-";
        std::string path = "-";
        if (mode.public_has_key() != nullptr) {
            public_has_key = wide_to_utf8(utf8_to_wide_2(mode.public_has_key()));
        }
        if (mode.path() != nullptr) {
            path = wide_to_utf8(utf8_to_wide_2(mode.path()));
        }
        try {
            LocalPushConnectivityPlugin::saveSetting(PluginSetting{
                windows->bundle_id(),
                windows->display_name(),
                windows->icon(),
                windows->icon_content(),
                mode.host(),
                mode.port(),
                77,
                public_has_key,
                mode.connection_type() == ConnectionType::kWss,
                path,
                mode.connection_type() == ConnectionType::kTcp
                });
            if (windows != nullptr) {
                std::wstring pathIcon = get_current_path() + std::wstring(L"\\data\\flutter_assets\\") + utf8_to_wide(windows->icon());
                DesktopNotificationManagerCompat::Register(utf8_to_wide(windows->bundle_id()),
                    utf8_to_wide(windows->display_name()), pathIcon);

                DesktopNotificationManagerCompat::OnActivated([this](
                    DesktopNotificationActivatedEventArgsCompat data) {
                        std::wstring tag = data.Argument();
                        //std::wstring m = L"\"type\""
                    });

                LocalPushConnectivityPlugin::createBackgroundProcess();
            }
        }
        catch (hresult_error const& e) {
            std::wcout << "ERROR register notification: " << e.message().c_str();
        }
        result(true);
    }

    void LocalPushConnectivityPlugin::Config(
        const TCPModePigeon& mode,
        const std::string* ssid,
        std::function<void(ErrorOr<bool> reply)> result) {
        PluginSetting settings = LocalPushConnectivityPlugin::gSetting();
        settings.host = mode.host();
        settings.port = mode.port();
        settings.publicHasKey = mode.public_has_key() == nullptr ? "-" :
            wide_to_utf8(utf8_to_wide_2(mode.public_has_key()));
        settings.wss = mode.connection_type() == ConnectionType::kWss;
        settings.path = mode.path() == nullptr ? "-" :
            wide_to_utf8(utf8_to_wide_2(mode.path()));
        wchar_t commandLine[700];
        swprintf(commandLine, 700, L"%s***%s***%s***%s***%s***%lld***%lld***%s***%s***%s***%s",
            utf8_to_wide(settings.appBundle).c_str(),
            utf8_to_wide(settings.displayName).c_str(),
            utf8_to_wide(settings.iconPath).c_str(),
            utf8_to_wide(settings.iconContent).c_str(),
            utf8_to_wide(settings.host).c_str(),
            settings.port,
            settings.systemType,
            utf8_to_wide(settings.publicHasKey).c_str(),
            settings.wss ? L"true" : L"false",
            utf8_to_wide(settings.path).c_str(),
            settings.tcp ? L"true" : L"false"
        );
        sendSettings(commandLine);
        result(true);
    }

    void LocalPushConnectivityPlugin::RequestPermission(std::function<void(ErrorOr<bool> reply)> result) {
        result(true);
    }

    void LocalPushConnectivityPlugin::Start(std::function<void(ErrorOr<bool> reply)> result) {
        result(true);
    }

    void LocalPushConnectivityPlugin::Stop(std::function<void(ErrorOr<bool> reply)> result) {
        result(true);
    }

    PluginSetting LocalPushConnectivityPlugin::_settings{};
    PluginSetting SocketClient::_settings{};
    std::function<void()> SocketClient::_callback = nullptr;

    void SocketClient::m_connect(PluginSetting* param, std::function<void()> callback) {
        SocketClient::saveSettings(*param);
        return SocketClient::connectWss();
    }
   
    void SocketClient::connectWss() {
        try {
            wchar_t uri[256];
            auto settings = SocketClient::gSettings();
            std::wstring m = settings.wss ? L"wss" : L"ws";
            swprintf(uri, 256, L"%ls://%ls:%lld%ls",
                m.c_str(),
                utf8_to_wide(settings.host).c_str(),
                settings.port,
                utf8_to_wide(settings.path).c_str()
            );

            std::wstring uk(uri);
            Uri serverUri(uri);
            StreamWebSocket _socket;

            _socket.ConnectAsync(serverUri).get();
            DataWriter writer(_socket.OutputStream());
            auto deviceId = get_sys_device_id();

            writer.WriteString(L"");
            writer.StoreAsync().get();
            writer.FlushAsync().get();
            //receiverWss(_socket);
            writer.DetachStream();
            return;
        }
        catch (hresult_error& e) {
            std::wcerr << L"Error connect websocket: " << e.message().c_str() << std::endl;
        }
    }

    void SocketClient::receiverWss(){}
}  // namespace local_push_connectivity