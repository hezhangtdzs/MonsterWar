#pragma once

#include <exception>
#include <future>
#include <utility>

namespace engine::utils {

template <typename Future, typename SuccessHandler, typename ExceptionHandler>
bool consumeFuture(Future& future, SuccessHandler&& on_success, ExceptionHandler&& on_exception) {
	if (!future.valid()) {
		return true;
	}

	try {
		return static_cast<bool>(std::forward<SuccessHandler>(on_success)(future.get()));
	}
	catch (const std::exception& e) {
		return static_cast<bool>(std::forward<ExceptionHandler>(on_exception)(e));
	}
}

template <typename ExceptionHandler>
bool consumeFuture(std::future<void>& future, ExceptionHandler&& on_exception) {
	if (!future.valid()) {
		return true;
	}

	try {
		future.get();
		return true;
	}
	catch (const std::exception& e) {
		return static_cast<bool>(std::forward<ExceptionHandler>(on_exception)(e));
	}
}

} // namespace engine::utils
