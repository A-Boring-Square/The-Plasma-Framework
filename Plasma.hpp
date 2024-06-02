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
#include <stdexcept>
#include "imgui-1.90.7/imgui.h"
#include "imgui-1.90.7/backends/imgui_impl_sdl2.h"
#include "imgui-1.90.7/backends/imgui_impl_sdlrenderer2.h"
#include <stdio.h>
#include "SDL.h"

#if !SDL_VERSION_ATLEAST(2,0,17)
#error DearImGui requires SDL 2.0.17+ because of SDL_RenderGeometry() function The Gui Functions Will NOT WORK
#endif


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



	namespace Gui {

		class WindowManager {
		private:
			SDL_Window* GuiWindow = nullptr;
			SDL_Renderer* GuiRenderer = nullptr;
		public:
			WindowManager(const std::string& WindowTitle, unsigned int Width, unsigned int Height, bool FullScreen, bool Resizeable, const std::string& WindowIconPath);
			~WindowManager();

			void NewFrame();
			void Render();
			void ShowDebugInfo();

			template<typename Func, typename... Args>
			void AddButton(const char* label, Func&& onClick, Args&&... args);

			void AddText(const char* text);

			template<typename Func, typename... Args>
			void AddSliderFloat(const char* label, float* v, float v_min, float v_max, Func&& onChange, Args&&... args);

			void AddInputText(const char* label, char* buf, size_t buf_size);

			template<typename Func, typename... Args>
			void AddCheckbox(const char* label, bool* v, Func&& onChange, Args&&... args);


		};

	} // namespace Gui

} // namespace Plasma
