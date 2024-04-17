#include <windows.h>
#include <iostream>

enum class ShutdownReason {
    PressCtrlC = 0,
    PressCtrlBreak = 1,
    ConsoleClosing = 2,
    WindowsLogOff = 5,
    WindowsShutdown = 6,
    ReachEndOfMain = 1000,
    Exception = 1001
};


struct ShutdownEventArgs {
    std::exception* Exception;
    ShutdownReason Reason;

    ShutdownEventArgs(ShutdownReason reason) : Reason(reason), Exception(nullptr) {}

    ShutdownEventArgs(std::exception* exception) : Reason(ShutdownReason::Exception), Exception(exception) {}
};


class ShutdownEventCatcher {
public:
    static void RaiseShutdownEvent(ShutdownEventArgs args) {
        if (nullptr != Shutdown) {
            Shutdown(args);
        }
    }

    static void Kernel32_ProcessShuttingDown(ShutdownEventArgs args) {
        RaiseShutdownEvent(args);
    }

    static bool SetConsoleCtrlHandler(int handler, bool add) {
        return ::SetConsoleCtrlHandler(reinterpret_cast<PHANDLER_ROUTINE>(handler), add);
    }

    static bool SetProcessShutdownParameters(DWORD dwLevel, DWORD dwFlags) {
        return ::SetProcessShutdownParameters(dwLevel, dwFlags);
    }

    static bool ExitWindowsEx(UINT uFlags, DWORD dwReason) {
        return ::ExitWindowsEx(uFlags, dwReason);
    }

   
    using ShutdownHandler = void (*)(ShutdownEventArgs);
    static ShutdownHandler Shutdown;

private:
    using Kernel32ShutdownHandler = ShutdownHandler;
};


ShutdownEventCatcher::ShutdownHandler ShutdownEventCatcher::Shutdown = nullptr;


void ShowMessageBox(const char* functionName) {
    MessageBoxA(NULL, functionName, "Current Function", MB_OK | MB_ICONINFORMATION);
}

int main() {
    while (true) {
        
        ShowMessageBox(__FUNCTION__);
        if (!ShutdownEventCatcher::SetProcessShutdownParameters(0x00040000 | 0x00000000, SHUTDOWN_NORETRY)) {
            DWORD dwLastError = GetLastError();
            std::cerr << "Failed to set shutdown parameters. Error code: " << dwLastError << std::endl;
            return 1;
        }

      
        ShowMessageBox(__FUNCTION__);
        ShutdownEventCatcher::Shutdown = ShutdownEventCatcher::Kernel32_ProcessShuttingDown;
        ShutdownEventCatcher::SetConsoleCtrlHandler(reinterpret_cast<int>(ShutdownEventCatcher::Kernel32_ProcessShuttingDown), true);

        
        ShowMessageBox(__FUNCTION__);
        if (!ShutdownEventCatcher::ExitWindowsEx(EWX_SHUTDOWN | EWX_POWEROFF, 0)) {
            DWORD dwLastError = GetLastError();
            std::cerr << "Failed to initiate shutdown. Error code: " << dwLastError << std::endl;
        } else {
            std::cout << "System is shutting down..." << std::endl;
            break; 
        }
    }
    
    return 0;
}
