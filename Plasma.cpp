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


    namespace Ui {
        namespace Widgets {
            struct BaseWiget {
                ~BaseWiget() {}
            };

            struct Text : BaseWiget {
                const char* Label;
            };

            struct Button : BaseWiget {
                const char* Label;
                std::function<void()> OnClick;
            };

            struct Checkbox : BaseWiget {
                const char* Label;
                bool* Value;
                std::function<void(bool)> OnChange;
            };

            struct RadioButton : BaseWiget {
                const char* Label;
                int* Value;
                int ButtonId;
                std::function<void(int)> OnChange;
            };

            struct ComboBox : BaseWiget {
                const char* Label;
                const char** Items;
                int ItemCount;
                int* CurrentItem;
                std::function<void(int)> OnChange;
            };

            struct ListBox : BaseWiget {
                const char* Label;
                const char** Items;
                int ItemCount;
                int* CurrentItem;
                std::function<void(int)> OnChange;
            };

            struct SliderInt : BaseWiget {
                const char* Label;
                int* Value;
                int Min;
                int Max;
                std::function<void(int)> OnChange;
            };

            struct SliderFloat : BaseWiget {
                const char* Label;
                float* Value;
                float Min;
                float Max;
                std::function<void(float)> OnChange;
            };

            struct InputText : BaseWiget {
                const char* Label;
                char* Buffer;
                size_t BufferSize;
                std::function<void(const char*)> OnChange;
            };

            struct InputInt : BaseWiget {
                const char* Label;
                int* Value;
                std::function<void(int)> OnChange;
            };

            struct InputFloat : BaseWiget {
                const char* Label;
                float* Value;
                std::function<void(float)> OnChange;
            };

            struct ColorPicker : BaseWiget {
                const char* Label;
                float Color[4];
                std::function<void(float[4])> OnChange;
            };

            struct Tooltip {
                const char* Text;
            };

            struct ProgressBar {
                float Fraction;
                const char* Overlay;
            };

            struct Separator {};

            struct SameLine {};

            struct Spacing {
                int Count;
            };

            struct Indent {
                float Width;
            };

            struct Unindent {
                float Width;
            };

            struct ChildWindow {
                const char* Title;
                ImVec2 Size;
                bool Border;
                ImGuiWindowFlags Flags;
            };

            struct Group {};

            struct TabBar {
                const char* Id;
            };

            struct Tab {
                const char* Label;
            };

            struct CollapsingHeader {
                const char* Label;
                bool* Open;
            };

            struct TreeNode {
                const char* Label;
                bool* Open;
            };

            struct MenuBar {};

            struct Menu {
                const char* Label;
            };

            struct MenuItem {
                const char* Label;
                bool* Selected;
            };

            struct ContextMenu {
                const char* Label;
            };

            struct PlotLines {
                const char* Label;
                const float* Values;
                int ValueCount;
                int Offset = 0;
                const char* OverlayText = nullptr;
                float ScaleMin = FLT_MAX;
                float ScaleMax = FLT_MAX;
                ImVec2 GraphSize = ImVec2(0, 0);
            };

            struct PlotHistogram {
                const char* Label;
                const float* Values;
                int Value_count;
                int Offset = 0;
                const char* OverlayText = nullptr;
                float ScaleMin = FLT_MAX;
                float ScaleMax = FLT_MAX;
                ImVec2 GraphSize = ImVec2(0, 0);
            };

            struct Columns {
                int Count;
                const char* Id;
                bool Border = true;
            };

            struct NextColumn {};

            struct Table {
                const char* Id;
                int Columns;
                ImGuiTableFlags Flags = 0;
            };

            struct DragSource {
                const char* PayloadId;
            };

            struct DragTarget {
                const char* PayloadId;
            };

            struct Tree {
                const char* Label;
            };

            struct Tables {
                const char* Id;
                int Columns;
                ImGuiTableFlags Flags = 0;
            };

            struct Image {
                void* TextureId;
                float Width;
                float Height;
            };

            struct ImageButton {
                void* TextureId;
                float Width;
                float Height;
            };

            struct Selectable {
                const char* Label;
                bool* Selected;
            };

            struct Bullet {};

            struct Hyperlink {
                const char* Url;
            };
        }

        
       
        WindowManager::WindowManager(std::string& Title, unsigned int Width, unsigned int Height, bool FullScreen) {
            // Initialize SDL
            SDL_Init(SDL_INIT_VIDEO);
            SDL_version _ver;
            SDL_GetVersion(&_ver);
            std::string ver = std::to_string(_ver.major) + "." + std::to_string(_ver.minor) + "." + std::to_string(_ver.patch);
            DebugInfo("Initialized SDL2 v" + ver);

            // Create SDL window and renderer
            Uint32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE;
            if (FullScreen) flags |= SDL_WINDOW_FULLSCREEN;
            DebugInfo("SDL Window Created FullScreen: " + (FullScreen ? std::string("True") : std::string("False")));


            this->GuiWindowContext = SDL_CreateWindow(Title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Width, Height, flags);
            this->GuiRenderer = SDL_CreateRenderer(this->GuiWindowContext, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
            // Setup ImGui context
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGui_ImplSDL2_InitForSDLRenderer(this->GuiWindowContext, this->GuiRenderer);
            this->Running = true;
        }

            void WindowManager::Render(std::vector<Widgets::BaseWiget*> Ui) {
                SDL_Event event;
                while (SDL_PollEvent(&event)) {
                    ImGui_ImplSDL2_ProcessEvent(&event);
                    if (event.type == SDL_QUIT) this->Running = false;
                }

                ImGui_ImplSDL2_NewFrame();
                ImGui::NewFrame();

                // TODO:  Render GUI elements

                ImGui::Render();
                SDL_SetRenderDrawColor(GuiRenderer, 0, 0, 0, 255);
                SDL_RenderClear(GuiRenderer);
                ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), this->GuiRenderer);
                SDL_RenderPresent(this->GuiRenderer);
            }

            bool WindowManager::IsRunning() const {
                return this->Running;
            }

            WindowManager::~WindowManager() {
                // Cleanup
                ImGui_ImplSDL2_Shutdown();
                ImGui::DestroyContext();
                SDL_DestroyRenderer(GuiRenderer);
                SDL_DestroyWindow(GuiWindowContext);
                SDL_Quit();
            }
    }

} // namespace Plasma