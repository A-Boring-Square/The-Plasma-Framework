#pragma once
#define WINDOWS_LEAN_AND_MEAN
#include <Windows.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <functional>
#include <tuple>
#include <optional>
#include <vector>


namespace Plasma {
	namespace Concurrency {

		template<typename ReturnType, typename... Args>
		class MutexedThreadManager {
		private:
			std::thread MutexedThread;
			std::mutex MutexForThread;
			std::function<ReturnType(Args...)> Function;
			std::tuple<Args...> ArgsTuple;
			std::optional<ReturnType> ReturnValue;

		public:
			MutexedThreadManager(std::function<ReturnType(Args...)> Func, Args&&... Args);
			void StartThread();
			void JoinThread();
			std::optional<ReturnType> GetReturnValue();
			~MutexedThreadManager();
		};

		template<typename ReturnType, typename... Args>
		class ThreadManager {
		private:
			std::thread Thread;
			std::function<ReturnType(Args...)> Function;
			std::tuple<Args...> ArgsTuple;
			std::optional<ReturnType> ReturnValue;
		public:
			ThreadManager(std::function<ReturnType(Args...)> Func, Args&&... Args);
			void StartThread();
			void JoinThread();
		};

		class ProcessManager {
		private:
			PROCESS_INFORMATION ProcessInfo;
			STARTUPINFOA StartupInfo;
			HANDLE hInputWrite, hInputRead, hOutputWrite, hOutputRead;

		public:
			ProcessManager();

			bool StartProcess(const std::string& command);

			void WriteToProcess(const std::string& input);

			std::string ReadFromProcess();

			void Wait();

			~ProcessManager();
		};
		
	} // namespace Concurrency



	namespace Ui {

		class WindowManager {
		private:


		};
	}

} // namespace Plasma
