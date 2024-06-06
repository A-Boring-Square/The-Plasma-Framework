#include "Plasma.hpp"


namespace Plasma {

	namespace Concurrency {
        template<typename ReturnType, typename... Args>
        MutexedThreadManager<ReturnType, Args...>::MutexedThreadManager(std::function<ReturnType(Args...)> Func, Args&&... args)
            : Function(std::move(Func)), ArgsTuple(std::forward<Args>(args)...) {}

        template<typename ReturnType, typename... Args>
        void MutexedThreadManager<ReturnType, Args...>::StartThread() {
            MutexedThread = std::thread([this]() {
                std::lock_guard<std::mutex> lock(MutexForThread);
                ReturnValue = std::apply(Function, ArgsTuple);
                });
        }

        template<typename ReturnType, typename... Args>
        void MutexedThreadManager<ReturnType, Args...>::JoinThread() {
            if (MutexedThread.joinable()) {
                MutexedThread.join();
            }
        }

        template<typename ReturnType, typename... Args>
        std::optional<ReturnType> MutexedThreadManager<ReturnType, Args...>::GetReturnValue() {
            std::lock_guard<std::mutex> lock(MutexForThread);
            return ReturnValue;
        }

        template<typename ReturnType, typename... Args>
        MutexedThreadManager<ReturnType, Args...>::~MutexedThreadManager() {
            if (MutexedThread.joinable()) {
                MutexedThread.join();
            }
        }

        template<typename ReturnType, typename... Args>
        ThreadManager<ReturnType, Args...>::ThreadManager(std::function<ReturnType(Args...)> Func, Args&&... args)
            : Function(std::move(Func)), ArgsTuple(std::forward<Args>(args)...) {}

        template<typename ReturnType, typename... Args>
        void ThreadManager<ReturnType, Args...>::StartThread() {
            Thread = std::thread([this]() {
                ReturnValue = std::apply(Function, ArgsTuple);
                });
        }

        template<typename ReturnType, typename... Args>
        void ThreadManager<ReturnType, Args...>::JoinThread() {
            if (Thread.joinable()) {
                Thread.join();
            }
        }


	} // namespace Concurrency


	namespace Ui {

	}

} // namespace Plasma