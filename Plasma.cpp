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
		void Plasma::Concurrency::ThreadManager<ReturnType, Args...>::JoinThread() {
			if (this->MutexedThread.joinable()) {
				this->MutexedThread.join();
			}
		}



	} // namespace Concurrency
} // namespace Plasma
