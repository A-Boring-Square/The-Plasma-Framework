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
#include <string>
#include <stdio.h>


#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

#if _DEBUG
#define DebugInfo(Message) std::cout << "[PlasmaDebug]: " << Message << std::endl;
#else
#define DebugInfo(Message)
#endif


namespace Plasma {
	namespace Concurrency {

		template<typename ReturnType, typename... Args>
		class MutexedThreadManager {
		private:
			std::thread MutexedThread;
			std::lock_guard<std::mutex> MutexForThread;
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
			std::optional<ReturnType> GetReturnValue();
			~ThreadManager();
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

} // namespace Plasma
