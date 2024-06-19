#include "Plasma.hpp"


namespace Plasma {

	namespace Concurrency {
        template<typename ReturnType, typename... Args>
        MutexedThreadManager<ReturnType, Args...>::MutexedThreadManager(std::function<ReturnType(Args...)> Func, Args&&... args)
            : Function(std::move(Func)), ArgsTuple(std::forward<Args>(args)...) {
            DebugInfo("Mutexed Thread Manager Created");
        }

        template<typename ReturnType, typename... Args>
        void MutexedThreadManager<ReturnType, Args...>::StartThread() {
            this->MutexedThread = std::thread([this]() {
                std::lock_guard<std::mutex> lock(this->MutexForThread);
                this->ReturnValue = std::apply(Function, ArgsTuple);
                });
            DebugInfo("Mutexed Thread Started Id: " + (std::string)this->MutexedThread.get_id());

        }

        template<typename ReturnType, typename... Args>
        void MutexedThreadManager<ReturnType, Args...>::JoinThread() {
            if (this->MutexedThread.joinable()) {
                DebugInfo("Mutexed Thread Joined Id: " + (std::string)this->MutexedThread.get_id());
                this->MutexedThread.join();
            }
        }

        template<typename ReturnType, typename... Args>
        std::optional<ReturnType> MutexedThreadManager<ReturnType, Args...>::GetReturnValue() {
            std::lock_guard<std::mutex> lock(this->MutexForThread);
            return this->ReturnValue;
        }

        template<typename ReturnType, typename... Args>
        MutexedThreadManager<ReturnType, Args...>::~MutexedThreadManager() {
            if (this->MutexedThread.joinable()) {
                DebugInfo("Mutexed Thread Joined Silently Due To It's Thread Manager Going Out Of Scope Id: " + (std::string)this->MutexedThread.get_id());
                this->MutexedThread.join();
            }
        }

        template<typename ReturnType, typename... Args>
        ThreadManager<ReturnType, Args...>::ThreadManager(std::function<ReturnType(Args...)> Func, Args&&... args)
            : Function(std::move(Func)), ArgsTuple(std::forward<Args>(args)...) {
            DebugInfo("Thread Manager Created");
        }

        template<typename ReturnType, typename... Args>
        void ThreadManager<ReturnType, Args...>::StartThread() {
            this->Thread = std::thread([this]() {
                this->ReturnValue = std::apply(Function, ArgsTuple);
                });
            DebugInfo("Thread Created Id: " + (std::string)this->Thread.get_id());
        }

        template<typename ReturnType, typename... Args>
        void ThreadManager<ReturnType, Args...>::JoinThread() {
            if (this->Thread.joinable()) {
                DebugInfo("Thread Joined Id: " + (std::string)this->MutexedThread.get_id());
                this->Thread.join();
            }
        }

        template<typename ReturnType, typename... Args>
        std::optional<ReturnType> ThreadManager<ReturnType, Args...>::GetReturnValue() {
            return std::make_optional<ReturnType>(this->ReturnValue);
        }

        template<typename ReturnType, typename ...Args>
        ThreadManager<ReturnType, Args...>::~ThreadManager() {
            if (this->Thread.joinable()) {
                DebugInfo("Thread Joined Silently Due To It's Thread Manager Going Out Of Scope Id: " + (std::string)this->Thread.get_id());
                this->Thread.join();
            }
        }



        ProcessManager::ProcessManager() : hInputWrite(NULL), hInputRead(NULL), hOutputWrite(NULL), hOutputRead(NULL) {
            ZeroMemory(&ProcessInfo, sizeof(PROCESS_INFORMATION));
            ZeroMemory(&StartupInfo, sizeof(STARTUPINFOA));
            StartupInfo.cb = sizeof(STARTUPINFOA);
        }

        bool ProcessManager::StartProcess(const std::string& command) {
            SECURITY_ATTRIBUTES saAttr;
            saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
            saAttr.bInheritHandle = TRUE;
            saAttr.lpSecurityDescriptor = NULL;

            // Create pipes for standard input
            if (!CreatePipe(&hInputRead, &hInputWrite, &saAttr, 0)) {
                std::cerr << "StdIn CreatePipe failed" << std::endl;
                return false;
            }
            if (!SetHandleInformation(hInputWrite, HANDLE_FLAG_INHERIT, 0)) {
                std::cerr << "StdIn SetHandleInformation failed" << std::endl;
                return false;
            }

            // Create pipes for standard output
            if (!CreatePipe(&hOutputRead, &hOutputWrite, &saAttr, 0)) {
                std::cerr << "StdOut CreatePipe failed" << std::endl;
                return false;
            }
            if (!SetHandleInformation(hOutputRead, HANDLE_FLAG_INHERIT, 0)) {
                std::cerr << "StdOut SetHandleInformation failed" << std::endl;
                return false;
            }

            StartupInfo.hStdError = hOutputWrite;
            StartupInfo.hStdOutput = hOutputWrite;
            StartupInfo.hStdInput = hInputRead;
            StartupInfo.dwFlags |= STARTF_USESTDHANDLES;

            // Create the child process
            if (!CreateProcessA(NULL, (LPSTR)command.c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &StartupInfo, &ProcessInfo)) {
                std::cerr << "CreateProcess failed (" << GetLastError() << ")" << std::endl;
                return false;
            }

            // Close handles that are no longer needed
            CloseHandle(hInputRead);
            CloseHandle(hOutputWrite);

            return true;
        }

        void ProcessManager::WriteToProcess(const std::string& input) {
            DWORD written;
            if (!WriteFile(hInputWrite, input.c_str(), input.size(), &written, NULL)) {
                std::cerr << "WriteFile to process failed" << std::endl;
            }
        }

        std::string ProcessManager::ReadFromProcess() {
            DWORD read;
            char buffer[4096];
            std::string output;

            while (true) {
                if (!ReadFile(hOutputRead, buffer, sizeof(buffer) - 1, &read, NULL) || read == 0) {
                    break;
                }
                buffer[read] = '\0';
                output += buffer;
            }

            return output;
        }

        void ProcessManager::Wait() {
            WaitForSingleObject(ProcessInfo.hProcess, INFINITE);
        }

        ProcessManager::~ProcessManager() {
            CloseHandle(ProcessInfo.hProcess);
            CloseHandle(ProcessInfo.hThread);
            CloseHandle(hInputWrite);
            CloseHandle(hOutputRead);
        }


	} // namespace Concurrency


} // namespace Plasma