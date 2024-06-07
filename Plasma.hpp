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
#include "imgui-1.90.7/imgui.h"
#include "imgui-1.90.7/backends/imgui_impl_sdl2.h"
#include "imgui-1.90.7/backends/imgui_impl_sdlrenderer2.h"
#include <stdio.h>
#include "Sdl2/SDL.h"


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



	namespace Ui {
		namespace Wigets{
			struct BaseWiget;
			struct Text;
			struct Button;
			struct Checkbox;
			struct RadioButton;
			struct ComboBox;
            struct ListBox;
            struct SliderInt;
            struct SliderFloat;
            struct InputText;
            struct InputInt;
            struct InputFloat;
            struct ColorPicker;
            struct Tooltip;
            struct ProgressBar;
            struct Separator;
            struct Spacing;
            struct Indent;
            struct Unindent;
            struct ChildWindow;
            struct Group;
            struct TabBar;
            struct Tab;
            struct CollapsingHeader;
            struct TreeNode;
            struct MenuBar;
            struct Menu;
            struct MenuItem;
            struct ContextMenu;
            struct PlotLines;
            struct PlotHistogram;
            struct Columns;
            struct NextColumn;
            struct Table;
            struct DragSource;
            struct DragTarget;
            struct Tree;
            struct Tables;
            struct Image;
            struct ImageButton;
            struct Selectable;
            struct Bullet;
            struct Hyperlink;


		}
		class WindowManager {
		private:
			SDL_Renderer* GuiRenderer;
			SDL_Window* GuiWindowContext;
			bool Running;
		public:
			WindowManager(std::string& Title, unsigned int Width, unsigned int Height, bool FullScreen);
			void Render();
			bool IsRunning() const;
			
			~WindowManager();
		};
	}

} // namespace Plasma
