#include "Plasma.hpp"

namespace Plasma {
	namespace Concurrency {

		template<typename ReturnType, typename... Args>
		MutexedThreadManager<ReturnType, Args...>::MutexedThreadManager(std::function<ReturnType(Args...)> Func, Args&&... Args)
			: Function(Func), ArgsTuple(std::forward<Args>(Args)...), ReturnValue(std::nullopt) {}

		template<typename ReturnType, typename... Args>
		void MutexedThreadManager<ReturnType, Args...>::StartThread() {
			this->Thread = std::thread([this]() {
				std::lock_guard<std::mutex> lock(this->MutexForThread);
				this->ReturnValue = std::apply(this->Function, this->ArgsTuple);
				});
		}

		template<typename ReturnType, typename... Args>
		void MutexedThreadManager<ReturnType, Args...>::JoinThread() {
			if (this->Thread.joinable()) {
				this->Thread.join();
			}
		}

		template<typename ReturnType, typename... Args>
		std::optional<ReturnType> MutexedThreadManager<ReturnType, Args...>::GetReturnValue() {
			std::lock_guard<std::mutex> lock(this->MutexForThread);
			return this->ReturnValue;
		}

		template<typename ReturnType, typename... Args>
		MutexedThreadManager<ReturnType, Args...>::~MutexedThreadManager() {
			if (this->Thread.joinable()) {
				this->Thread.join();
			}
		}

		template<typename ReturnType, typename ...Args>
		ThreadManager<ReturnType, Args...>::ThreadManager(std::function<ReturnType(Args...)> Func, Args && ...Args)
			: Function(Func), ArgsTuple(std::forward<Args>(Args)...), ReturnValue(std::nullopt) {}

		template<typename ReturnType, typename ...Args>
		void ThreadManager<ReturnType, Args...>::StartThread() {
			this->Thread = std::thread([this]() {
				this->ReturnValue = std::apply(this->Function, this->ArgsTuple);
				});
		}

		template<typename ReturnType, typename ...Args>
		void ThreadManager<ReturnType, Args...>::JoinThread() {
			if (this->Thread.joinable()) {
				this->Thread.join();
			}
		}

		template<typename ReturnType, typename ...Args>
		ThreadManager<ReturnType, Args...>::~ThreadManager() {
			if (this->Thread.joinable()) {
				this->Thread.join();
			}
		}

		ProcessManager::ProcessManager() {
			ZeroMemory(&this->StartupInfo, sizeof(this->StartupInfo));
			this->StartupInfo.cb = sizeof(this->StartupInfo);
			ZeroMemory(&this->ProcessInfo, sizeof(this->ProcessInfo));

			SECURITY_ATTRIBUTES saAttr;
			saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
			saAttr.bInheritHandle = TRUE;
			saAttr.lpSecurityDescriptor = NULL;

			if (!CreatePipe(&this->hOutputRead, &this->hOutputWrite, &saAttr, 0)) {
				throw std::runtime_error("Failed to create stdout pipe");
			}
			SetHandleInformation(this->hOutputRead, HANDLE_FLAG_INHERIT, 0);

			if (!CreatePipe(&this->hInputRead, &this->hInputWrite, &saAttr, 0)) {
				CloseHandle(this->hOutputRead);
				CloseHandle(this->hOutputWrite);
				throw std::runtime_error("Failed to create stdin pipe");
			}
			SetHandleInformation(this->hInputWrite, HANDLE_FLAG_INHERIT, 0);
		}

		bool ProcessManager::StartProcess(const std::string& command) {
			this->StartupInfo.hStdError = this->hOutputWrite;
			this->StartupInfo.hStdOutput = this->hOutputWrite;
			this->StartupInfo.hStdInput = this->hInputRead;
			this->StartupInfo.dwFlags |= STARTF_USESTDHANDLES;

			if (!CreateProcessA(NULL, const_cast<char*>(command.c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &this->StartupInfo, &this->ProcessInfo)) {
				return false;
			}
			return true;
		}

		void ProcessManager::WriteToProcess(const std::string& input) {
			DWORD written;
			if (!WriteFile(this->hInputWrite, input.c_str(), input.length(), &written, NULL)) {
				throw std::runtime_error("Failed to write to process");
			}
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

	namespace Gui {
		WindowManager::WindowManager(const std::string& WindowTitle, unsigned int Width, unsigned int Height, bool FullScreen, bool Resizeable, const std::string& WindowIconPath) {
			// Initialize SDL
			if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
				throw std::runtime_error("Failed to initialize SDL");
			}

			// Set SDL window flags
			Uint32 flags = SDL_WINDOW_SHOWN;
			if (FullScreen) {
				flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
			}
			if (Resizeable) {
				flags |= SDL_WINDOW_RESIZABLE;
			}

			// Create SDL window
			this->GuiWindow = SDL_CreateWindow(WindowTitle.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Width, Height, flags);
			if (!this->GuiWindow) {
				throw std::runtime_error("Failed to create SDL window");
			}

			// Load window icon
			SDL_Surface* icon = SDL_LoadBMP(WindowIconPath.c_str());
			if (icon) {
				SDL_SetWindowIcon(this->GuiWindow, icon);
				SDL_FreeSurface(icon);
			}

			// Create SDL renderer
			this->GuiRenderer = SDL_CreateRenderer(this->GuiWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (!this->GuiRenderer) {
				throw std::runtime_error("Failed to create SDL renderer");
			}

			// Initialize ImGui context
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO(); (void)io;

			// Initialize ImGui for SDL and the renderer
			ImGui_ImplSDL2_InitForSDLRenderer(this->GuiWindow, this->GuiRenderer);
			ImGui_ImplSDLRenderer2_Init(this->GuiRenderer);

			// Setup ImGui style
			ImGui::StyleColorsDark();
		}

		WindowManager::~WindowManager() {
			// Cleanup ImGui
			ImGui_ImplSDLRenderer2_Shutdown();
			ImGui_ImplSDL2_Shutdown();
			ImGui::DestroyContext();

			// Destroy SDL renderer
			if (this->GuiRenderer) {
				SDL_DestroyRenderer(this->GuiRenderer);
			}

			// Destroy SDL window
			if (this->GuiWindow) {
				SDL_DestroyWindow(this->GuiWindow);
			}

			// Quit SDL subsystems
			SDL_Quit();
		}

		void WindowManager::NewFrame() {
			// Start the ImGui frame
			ImGui_ImplSDLRenderer2_NewFrame();
			ImGui_ImplSDL2_NewFrame();
			ImGui::NewFrame();
		}

		void WindowManager::Render() {
			// Render ImGui
			ImGui::Render();
			SDL_SetRenderDrawColor(this->GuiRenderer, 0, 0, 0, 255);
			SDL_RenderClear(this->GuiRenderer);
			ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), this->GuiRenderer);
			SDL_RenderPresent(this->GuiRenderer);
		}

		void WindowManager::ShowDebugInfo() {
			ImGui::ShowMetricsWindow();
			ImGui::ShowDebugLogWindow();
			ImGui::ShowIDStackToolWindow();
		}

		template<typename Func, typename... Args>
		void WindowManager::AddButton(const char* label, Func&& onClick, Args&&... args) {
			if (ImGui::Button(label)) {
				std::invoke(std::forward<Func>(onClick), std::forward<Args>(args)...);
			}
		}

		void WindowManager::AddText(const char* text) {
			ImGui::Text("%s", text);
		}

		template<typename Func, typename... Args>
		void WindowManager::AddSliderFloat(const char* label, float* v, float v_min, float v_max, Func&& onChange, Args&&... args) {
			if (ImGui::SliderFloat(label, v, v_min, v_max)) {
				std::invoke(std::forward<Func>(onChange), std::forward<Args>(args)...);
			}
		}

		void WindowManager::AddInputText(const char* label, char* buf, size_t buf_size) {
			ImGui::InputText(label, buf, buf_size);
		}

		template<typename Func, typename... Args>
		void WindowManager::AddCheckbox(const char* label, bool* v, Func&& onChange, Args&&... args) {
			if (ImGui::Checkbox(label, v)) {
				std::invoke(std::forward<Func>(onChange), std::forward<Args>(args)...);
			}
		}

	} // namespace Gui

} // namespace Plasma