#include "Plasma.hpp"

namespace Plasma {
	namespace Concurrency {

		template<typename ReturnType, typename... Args>
		MutexedThreadManager<ReturnType, Args...>::MutexedThreadManager(std::function<ReturnType(Args...)> Func, Args&&... Args)
			: Function(Func), ArgsTuple(std::forward<Args>(Args)...), ReturnValue(std::nullopt) {}

		template<typename ReturnType, typename... Args>
		void MutexedThreadManager<ReturnType, Args...>::StartThread() {
			this->MutexedThread = std::thread([this]() {
				std::lock_guard<std::mutex> lock(this->MutexForThread);
				this->ReturnValue = std::apply(this->Function, this->ArgsTuple);
				});
		}

		template<typename ReturnType, typename... Args>
		void MutexedThreadManager<ReturnType, Args...>::JoinThread() {
			if (this->MutexedThread.joinable()) {
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
				this->MutexedThread.join();
			}
		}

		template<typename ReturnType, typename ...Args>
		ThreadManager<ReturnType, Args...>::ThreadManager(std::function<ReturnType(Args...)> Func, Args && ...Args)
			: Function(Func), ArgsTuple(std::forward<Args>(Args)...), ReturnValue(std::nullopt) {

		}

		template<typename ReturnType, typename ...Args>
		void ThreadManager<ReturnType, Args...>::StartThread() {
			this->Thread = std::thread([this]() {
				this->ReturnValue = std::apply(this->Function, this->ArgsTuple);
				});

		}

		template<typename ReturnType, typename ...Args>
		void ThreadManager<ReturnType, Args...>::JoinThread() {
			if (this->MutexedThread.joinable()) {
				this->MutexedThread.join();
			}
		}

		ProcessManager::ProcessManager() {
				ZeroMemory(&this->StartupInfo, sizeof(this->StartupInfo));
				this->StartupInfo.cb = sizeof(this->StartupInfo);
				ZeroMemory(&this->ProcessInfo, sizeof(this->ProcessInfo));

				// Create pipes for stdin and stdout
				SECURITY_ATTRIBUTES saAttr;
				saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
				saAttr.bInheritHandle = TRUE;
				saAttr.lpSecurityDescriptor = NULL;

				// Create a pipe for the child process's STDOUT.
				CreatePipe(&this->hOutputRead, &this->hOutputWrite, &saAttr, 0);
				SetHandleInformation(this->hOutputRead, HANDLE_FLAG_INHERIT, 0);

				// Create a pipe for the child process's STDIN.
				CreatePipe(&this->hInputRead, &this->hInputWrite, &saAttr, 0);
				SetHandleInformation(this->hInputWrite, HANDLE_FLAG_INHERIT, 0);
		}

		bool ProcessManager::StartProcess(const std::string& command) {
			this->StartupInfo.hStdError = this->hOutputWrite;
			this->StartupInfo.hStdOutput = this->hOutputWrite;
			this->StartupInfo.hStdInput = this->hInputRead;
			this->StartupInfo.dwFlags |= STARTF_USESTDHANDLES;

			return CreateProcessA(NULL, const_cast<char*>(command.c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &this->StartupInfo, &this->ProcessInfo);
		}

		void ProcessManager::WriteToProcess(const std::string& input) {
			DWORD written;
			WriteFile(this->hInputWrite, input.c_str(), input.length(), &written, NULL);
		}

		std::string ProcessManager::ReadFromProcess() {
			char buffer[4096];
			DWORD read;
			std::string output;

			while (ReadFile(this->hOutputRead, buffer, sizeof(buffer) - 1, &read, NULL) && read > 0) {
				buffer[read] = '\0';
				output += buffer;
			}
			return output;
		}
		void ProcessManager::Wait() {
			WaitForSingleObject(this->ProcessInfo.hProcess, INFINITE);
		}

		ProcessManager::~ProcessManager() {
			CloseHandle(this->ProcessInfo.hProcess);
			CloseHandle(this->ProcessInfo.hThread);
			CloseHandle(this->hInputWrite);
			CloseHandle(this->hInputRead);
			CloseHandle(this->hOutputWrite);
			CloseHandle(this->hOutputRead);
		}


	} // namespace Concurrency



} // namespace Plasma
