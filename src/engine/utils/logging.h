#pragma once

#include <utility>

#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>

namespace engine::utils::log {

template <typename... Args>
inline void trace(const char* file, int line, const char* func, fmt::format_string<Args...> format, Args&&... args) {
    spdlog::trace("[{}:{} {}] {}", file, line, func, fmt::format(format, std::forward<Args>(args)...));
}

template <typename... Args>
inline void debug(const char* file, int line, const char* func, fmt::format_string<Args...> format, Args&&... args) {
    spdlog::debug("[{}:{} {}] {}", file, line, func, fmt::format(format, std::forward<Args>(args)...));
}

template <typename... Args>
inline void info(const char* file, int line, const char* func, fmt::format_string<Args...> format, Args&&... args) {
    spdlog::info("[{}:{} {}] {}", file, line, func, fmt::format(format, std::forward<Args>(args)...));
}

template <typename... Args>
inline void warn(const char* file, int line, const char* func, fmt::format_string<Args...> format, Args&&... args) {
    spdlog::warn("[{}:{} {}] {}", file, line, func, fmt::format(format, std::forward<Args>(args)...));
}

template <typename... Args>
inline void error(const char* file, int line, const char* func, fmt::format_string<Args...> format, Args&&... args) {
    spdlog::error("[{}:{} {}] {}", file, line, func, fmt::format(format, std::forward<Args>(args)...));
}

} // namespace engine::utils::log

#define ENGINE_LOG_TRACE(...) engine::utils::log::trace(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define ENGINE_LOG_DEBUG(...) engine::utils::log::debug(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define ENGINE_LOG_INFO(...) engine::utils::log::info(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define ENGINE_LOG_WARN(...) engine::utils::log::warn(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define ENGINE_LOG_ERROR(...) engine::utils::log::error(__FILE__, __LINE__, __func__, __VA_ARGS__)
