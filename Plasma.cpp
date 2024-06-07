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



	} // namespace Concurrency


    namespace Ui {
        namespace Widgets {
            struct BaseWiget {
                ~BaseWiget() {}
            };

            struct Text : BaseWiget {
                const char* label;
            };

            struct Button : BaseWiget {
                const char* label;
                std::function<void()> onClick;
            };

            struct Checkbox : BaseWiget {
                const char* label;
                bool* value;
                std::function<void(bool)> onChange;
            };

            struct RadioButton : BaseWiget {
                const char* label;
                int* value;
                int button_id;
                std::function<void(int)> onChange;
            };

            struct ComboBox : BaseWiget {
                const char* label;
                const char** items;
                int item_count;
                int* current_item;
                std::function<void(int)> onChange;
            };

            struct ListBox : BaseWiget {
                const char* label;
                const char** items;
                int item_count;
                int* current_item;
                std::function<void(int)> onChange;
            };

            struct SliderInt : BaseWiget {
                const char* label;
                int* value;
                int min;
                int max;
                std::function<void(int)> onChange;
            };

            struct SliderFloat : BaseWiget {
                const char* label;
                float* value;
                float min;
                float max;
                std::function<void(float)> onChange;
            };

            struct InputText : BaseWiget {
                const char* label;
                char* buffer;
                size_t buffer_size;
                std::function<void(const char*)> onChange;
            };

            struct InputInt : BaseWiget {
                const char* label;
                int* value;
                std::function<void(int)> onChange;
            };

            struct InputFloat : BaseWiget {
                const char* label;
                float* value;
                std::function<void(float)> onChange;
            };

            struct ColorPicker : BaseWiget {
                const char* label;
                float color[4];
                std::function<void(float[4])> onChange;
            };

            struct Tooltip {
                const char* text;
            };

            struct ProgressBar {
                float fraction;
                const char* overlay;
            };

            struct Separator {};

            struct SameLine {};

            struct Spacing {
                int count;
            };

            struct Indent {
                float width;
            };

            struct Unindent {
                float width;
            };

            struct ChildWindow {
                const char* title;
                ImVec2 size;
                bool border;
                ImGuiWindowFlags flags;
            };

            struct Group {};

            struct TabBar {
                const char* id;
            };

            struct Tab {
                const char* label;
            };

            struct CollapsingHeader {
                const char* label;
                bool* open;
            };

            struct TreeNode {
                const char* label;
                bool* open;
            };

            struct MenuBar {};

            struct Menu {
                const char* label;
            };

            struct MenuItem {
                const char* label;
                bool* selected;
            };

            struct ContextMenu {
                const char* label;
            };

            struct PlotLines {
                const char* label;
                const float* values;
                int value_count;
                int offset = 0;
                const char* overlay_text = nullptr;
                float scale_min = FLT_MAX;
                float scale_max = FLT_MAX;
                ImVec2 graph_size = ImVec2(0, 0);
            };

            struct PlotHistogram {
                const char* label;
                const float* values;
                int value_count;
                int offset = 0;
                const char* overlay_text = nullptr;
                float scale_min = FLT_MAX;
                float scale_max = FLT_MAX;
                ImVec2 graph_size = ImVec2(0, 0);
            };

            struct Columns {
                int count;
                const char* id;
                bool border = true;
            };

            struct NextColumn {};

            struct Table {
                const char* id;
                int columns;
                ImGuiTableFlags flags = 0;
            };

            struct DragSource {
                const char* payload_id;
            };

            struct DragTarget {
                const char* payload_id;
            };

            struct Tree {
                const char* label;
            };

            struct Tables {
                const char* id;
                int columns;
                ImGuiTableFlags flags = 0;
            };

            struct Image {
                void* texture_id;
                float width;
                float height;
            };

            struct ImageButton {
                void* texture_id;
                float width;
                float height;
            };

            struct Selectable {
                const char* label;
                bool* selected;
            };

            struct Bullet {};

            struct Hyperlink {
                const char* url;
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

            void WindowManager::Render() {
                SDL_Event event;
                while (SDL_PollEvent(&event)) {
                    ImGui_ImplSDL2_ProcessEvent(&event);
                    if (event.type == SDL_QUIT) this->Running = false;
                }

                ImGui_ImplSDL2_NewFrame();
                ImGui::NewFrame();

                // Render GUI elements here

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