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

#include <TlHelp32.h>

namespace local_push_connectivity {

    std::string PluginWrapper::toJson(PluginSetting settings) {
        json j = settings;

        auto k = j.dump();
        return base64_encode(k);
    }
    std::string PluginWrapper::toJson2(PluginSetting settings) {
        json j = settings;

        return j.dump();
    }
    PluginSetting PluginWrapper::fromJson(std::string json_str) {
        std::string decode = base64_decode(json_str);
        json j = json::parse(decode);
        PluginSetting p = j.get<PluginSetting>();
        return p;
    }

    // static
    void LocalPushConnectivityPlugin::RegisterWithRegistrar(
        flutter::PluginRegistrarWindows* registrar) {
        auto plugin = std::make_unique<LocalPushConnectivityPlugin>();
        LocalPushConnectivityPigeonFlutterApi api =
            LocalPushConnectivityPigeonFlutterApi(registrar->messenger());
        LocalPushConnectivityPlugin::setApi(api);
        LocalPushConnectivityPigeonHostApi::SetUp(registrar->messenger(), plugin.get());
        registrar->AddPlugin(std::move(plugin));
        sendMessageFromNoti(true);
    }

    LocalPushConnectivityPlugin::LocalPushConnectivityPlugin() {}

    LocalPushConnectivityPlugin::~LocalPushConnectivityPlugin() {}
    std::optional<LocalPushConnectivityPigeonFlutterApi> LocalPushConnectivityPlugin::_flutterApi = nullptr;
    std::wstring LocalPushConnectivityPlugin::_title = L"";
    int LocalPushConnectivityPlugin::RegisterProcess(std::wstring title,
        _In_ wchar_t* command_line) {
        set_title(title);
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

        std::string setting_str = wide_to_utf8(argv[1]);
        auto settings = PluginWrapper::fromJson(setting_str);

        auto oldPid = read_pid();
        if (oldPid != -1) {
            killProcess(oldPid);
        }

        DWORD pid = GetCurrentProcessId();
        write_pid(pid);

        LocalPushConnectivityPlugin::saveSetting(settings);
        LocalPushConnectivityPlugin::startThreadInChildProcess();
        LocalFree(argv);
        return 0;
    }
    void LocalPushConnectivityPlugin::HandleMessage(HWND window, UINT const message,
        LPARAM const lparam) {
        if (message == WM_COPYDATA) {
            auto cp_struct = reinterpret_cast<COPYDATASTRUCT*>(lparam);
            try {
                const wchar_t* data = reinterpret_cast<const wchar_t*>(cp_struct->lpData);
                std::wstring str(data, cp_struct->cbData / sizeof(wchar_t));
                auto api = gFlutterApi();

                if (api) {
                    NotificationPigeon n = NotificationPigeon("n", "n");
                    MessageResponsePigeon mR = MessageResponsePigeon(n, wide_to_utf8(str));
                    MessageSystemPigeon m = MessageSystemPigeon(false, mR);
                    m.set_from_notification(true);
                    api.value().OnMessage(m,
                        []() {
                            std::cout << "Message sent ok";
                        },
                        [](const FlutterError& e) {
                            std::cout << "Message send error: " << e.code()
                                << " - " << e.message();
                        });
                }
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
            try {
                if (pCDS->dwData == 100) {
                    std::wstring receiver((wchar_t*)pCDS->lpData);
                    auto settings = PluginWrapper::fromJson(wide_to_utf8(receiver));

                    SocketClient::saveSettings(settings);
                    std::wcout << L"[Child] receiver: " << receiver;
                }
            }
            catch(hresult_error &e){
                MessageBoxA(NULL, wide_to_utf8(e.message().c_str()).c_str(), "wide_to_utf8(e.message().c_str()).c_str()", MB_OK);
            }
            return TRUE;
        }
        }
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    HWND LocalPushConnectivityPlugin::createHiddenWindow(HINSTANCE hInstance) {
        WNDCLASSW wc = { 0 };
        //auto settings = gSetting();
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
            SocketClient::m_connect(param);
        }
        catch (hresult_error& e) {
            DesktopNotificationManagerCompat::sendToastProcess(
                utf8_to_wide(param->appBundle),
                L"error", L"-", L"-", e.message().c_str()
            );
        }
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        return 0;
    }

    void LocalPushConnectivityPlugin::sendSettings() {
        PluginSetting settings = gSetting();
        std::string commandLine = PluginWrapper::toJson(settings);
        HWND hwndChild = FindWindow(L"ChildHiddenWindow", NULL);
        if (!hwndChild) {
            MessageBoxA(NULL, commandLine.c_str(), "ChildHiddenWindow is null", MB_OK);
            return;
        }
        std::wstring c = utf8_to_wide(commandLine);
        COPYDATASTRUCT cds;
        cds.dwData = 100;
        cds.cbData = (DWORD)(wcslen(c.c_str()) + 1) * sizeof(wchar_t);
        cds.lpData = (PVOID)c.c_str();
        SendMessageW(hwndChild, WM_COPYDATA, (WPARAM)GetCurrentProcessId(),
            (LPARAM)&cds);
    }

    int LocalPushConnectivityPlugin::createBackgroundProcess() {
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
        std::string m_settings = PluginWrapper::toJson(settings);

        wchar_t commandLine[700];
        swprintf(commandLine, 700, L"\"%s\" child \"%s\"",
            execuablePath,
            utf8_to_wide(m_settings).c_str()
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

    std::optional<MessageSystemPigeon> LocalPushConnectivityPlugin::_m = std::nullopt;

    void LocalPushConnectivityPlugin::sendMessageFromNoti(bool isDelay) {
        if (isDelay) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
        auto api = gFlutterApi();
        if (api && _m) {
            auto m = *_m;
            api.value().OnMessage(m,
                []() {
                    std::cout << "Message sent ok";
                },
                [](const FlutterError& e) {
                    MessageBoxA(NULL, "send failllll", "ChildHiddenWindow is null", MB_OK);
                    std::cout << "Message send error: " << e.code()
                        << " - " << e.message();
                });
            return;
        }
    }

    void LocalPushConnectivityPlugin::Initialize(int64_t system_type, const AndroidSettingsPigeon* android,
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
            auto settings = PluginSetting{
                wide_to_utf8(title()),
                windows->bundle_id(),
                windows->display_name(),
                windows->icon(),
                windows->icon_content(),
                mode.host(),
                mode.port(),
                system_type,
                public_has_key,
                mode.connection_type() == ConnectionType::kWss,
                path,
                mode.connection_type() == ConnectionType::kTcp
            };
            LocalPushConnectivityPlugin::saveSetting(settings);
            if (windows != nullptr) {
                std::wstring pathIcon = get_current_path() + std::wstring(L"\\data\\flutter_assets\\") + utf8_to_wide(windows->icon());
                DesktopNotificationManagerCompat::Register(utf8_to_wide(windows->bundle_id()),
                    utf8_to_wide(windows->display_name()), pathIcon);

                settings.iconPath = wide_to_utf8(pathIcon);
                LocalPushConnectivityPlugin::saveSetting(settings);

                DesktopNotificationManagerCompat::OnActivated([this](
                    DesktopNotificationActivatedEventArgsCompat data) {
                        std::wstring tag = data.Argument();

                        NotificationPigeon n = NotificationPigeon("n", "n");
                        MessageResponsePigeon mR = MessageResponsePigeon(n, wide_to_utf8(tag));
                        MessageSystemPigeon m = MessageSystemPigeon(false, mR);
                        m.set_from_notification(true);

                        LocalPushConnectivityPlugin::_m = m;
                        sendMessageFromNoti(false);
                    });
                HWND hwndChild = FindWindow(L"ChildHiddenWindow", NULL);
                if (!hwndChild) {
                    LocalPushConnectivityPlugin::createBackgroundProcess();
                    return;
                }
                else {
                    LocalPushConnectivityPlugin::sendSettings();
                }
                
                result(true);
            }
            else {
                result(false);
            }
        }
        catch (hresult_error const& e) {
            result(false);
            std::wcout << "ERROR register notification: " << e.message().c_str();
        }
    }

    void LocalPushConnectivityPlugin::RegisterUser(
        const UserPigeon& user,
        std::function<void(ErrorOr<bool> reply)> result) {
        PluginSetting settings = gSetting();
        settings.connector_id = user.connector_i_d();
        settings.connector_tag = user.connector_tag();
        saveSetting(settings);
        sendSettings();
        result(true);
    }
    void LocalPushConnectivityPlugin::DeviceID(std::function<void(ErrorOr<std::string> reply)> result) {
        result(wide_to_utf8(get_sys_device_id()));
    }

    void LocalPushConnectivityPlugin::Config(
        const TCPModePigeon& mode,
        const flutter::EncodableList* ssids,
        std::function<void(ErrorOr<bool> reply)> result) {
        PluginSetting settings = LocalPushConnectivityPlugin::gSetting();
        settings.host = mode.host();
        settings.port = mode.port();
        settings.publicHasKey = mode.public_has_key() == nullptr ? "-" :
            wide_to_utf8(utf8_to_wide_2(mode.public_has_key()));
        settings.wss = mode.connection_type() == ConnectionType::kWss;
        settings.path = mode.path() == nullptr ? "-" :
            wide_to_utf8(utf8_to_wide_2(mode.path()));
        saveSetting(settings);
        sendSettings();
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
    DataWriter SocketClient::_writer;

    void SocketClient::m_connect(PluginSetting* param) {
        SocketClient::saveSettings(*param);
        return SocketClient::connectWss();
    }

    void SocketClient::_heartbeat() {
        try {
            json j = PingModel{ "ping" };
            _writer.WriteString(utf8_to_wide(j.dump()));
            _writer.StoreAsync().get();
            _writer.FlushAsync().get();
        }
        catch (const hresult_error& e) {
            std::wcout << "Error " << e.message();
            MessageBoxA(NULL, wide_to_utf8(e.message().c_str()).c_str(), "soc heartbeat err", MB_OK);
            connectWss();
        }
        std::this_thread::sleep_for(std::chrono::seconds(3));
        std::thread(_heartbeat).detach();
    }

    StreamWebSocket SocketClient::_socket;
   
    void SocketClient::connectWss() {
        try {
            wchar_t uri[256];
            PluginSetting settings = SocketClient::gSettings();
            if (settings.connector_id.empty()) {
                throw std::runtime_error("connector is null or empty");
            }
            std::wstring m = settings.wss ? L"wss" : L"ws";
            swprintf(uri, 256, L"%ls://%ls:%lld%ls",
                m.c_str(),
                utf8_to_wide(settings.host).c_str(),
                settings.port,
                utf8_to_wide(settings.path).c_str()
            );

            std::wstring uk(uri);
            Uri serverUri(uri);
            

            _socket.Closed([](IInspectable const&, WebSocketClosedEventArgs const& args) {
                MessageBoxA(NULL, "ddsss", "soc is disconnected", MB_OK);
                std::this_thread::sleep_for(std::chrono::seconds(5));
                std::thread(connectWss).detach();
                });

            _socket.ConnectAsync(serverUri).get();
            _writer = DataWriter(_socket.OutputStream());
            auto deviceId = get_sys_device_id();

            HWND hwnd = FindWindow(nullptr, utf8_to_wide(settings.title).c_str());
            std::wstring reconnect = L"reconnect";
            if ((hwnd != nullptr)) {
                COPYDATASTRUCT cds;
                cds.dwData = 1;
                cds.cbData = (DWORD)(wcslen(reconnect.c_str()) + 1) * sizeof(wchar_t);
                cds.lpData = (PVOID)reconnect.c_str();
                SendMessageW(hwnd, WM_COPYDATA, (WPARAM)GetCurrentProcessId(), (LPARAM)&cds);
            }

            RegisterModel registerModel{
                "register",
                static_cast<int>(settings.systemType),
                Sender{
                    settings.connector_id,
                    settings.connector_tag,
                    wide_to_utf8(deviceId)
                },
                DataRegister{}
            };

            json j = registerModel;

            _writer.WriteString(utf8_to_wide(j.dump()));
            _writer.StoreAsync().get();
            _writer.FlushAsync().get();

            _heartbeat();
            receiverWss(_socket);
            _writer.DetachStream();
            return;
        }
        catch (...) {
            std::wstring s = L"SEH: " + GetLastError();
            std::this_thread::sleep_for(std::chrono::seconds(5));
            std::thread(connectWss).detach();
        }
    }

    void SocketClient::receiverWss(StreamWebSocket& socket){
        DataReader reader(socket.InputStream());
        reader.InputStreamOptions(InputStreamOptions::Partial);
        std::wstring buffer;
        while (true) {
            auto byteRead = reader.LoadAsync(4096).get();
            if (byteRead == 0) {
                MessageBoxA(NULL, "wide_to_utf8(buffer).c_str()", "reconnect werwerafter 5s", MB_OK);
                reader.DetachStream();
                return;
            }
            hstring partH = reader.ReadString(byteRead);
            auto part = utf8_to_wide(winrt::to_string(partH));
            buffer += part;
            try {
                if (!buffer.empty() && buffer.back() == L'}') {
                    auto settings = gSettings();
                    MessageResponse p;
                    PongModel pong;
                    try {
                        json j = json::parse(wide_to_utf8(buffer));
                        p = j.get<MessageResponse>();
                    }
                    catch (...) {}
                    try {
                        json j = json::parse(wide_to_utf8(buffer));
                        pong = j.get<PongModel>();
                    }
                    catch (...) {}
                    if (!p.notification) { 
                        buffer.clear();
                        continue; 
                    }
                    if (pong.pong) {
                        buffer.clear();
                        continue;
                    }
                    HWND hwnd = FindWindow(nullptr, utf8_to_wide(settings.title).c_str());
                    if ((hwnd != nullptr)) {
                        COPYDATASTRUCT cds;
                        cds.dwData = 1;
                        cds.cbData = (DWORD)(wcslen(buffer.c_str()) + 1) * sizeof(wchar_t);
                        cds.lpData = (PVOID)buffer.c_str();
                        SendMessageW(hwnd, WM_COPYDATA, (WPARAM)GetCurrentProcessId(), (LPARAM)&cds);
                    }
                    else {
                        DesktopNotificationManagerCompat::sendToastProcess(
                            utf8_to_wide(settings.appBundle),
                            utf8_to_wide(settings.iconContent),
                            utf8_to_wide(p.notification.value().title),
                            utf8_to_wide(p.notification.value().body),
                            buffer.c_str()
                        );
                    }
                    buffer.clear();
                }
            }
            catch (...) {
                MessageBoxA(NULL, wide_to_utf8(buffer).c_str(), "reconnect after 5s", MB_OK);
            }
        }

        reader.DetachStream();
    }
}  // namespace local_push_connectivity