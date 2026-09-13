// Minimal spdlog compatibility shim (vendored).
// Provides the small subset of the spdlog API used by the NLT C++ ports:
//   spdlog::set_level / get_level, spdlog::info / warn / error
// with simple "{}" placeholder formatting. Logs go to stderr and can be
// silenced by setting SPDLOG_SHIM_QUIET. Replace with the real spdlog
// dependency when the full toolchain is available.
#pragma once

#include <cstdio>
#include <cstring>
#include <sstream>
#include <string>

namespace spdlog {

namespace level {
enum level_enum { trace = 0, debug = 1, info = 2, warn = 3, err = 4, off = 5 };
} // namespace level

inline level::level_enum& current_level() {
    static level::level_enum lvl = level::info;
    return lvl;
}

inline void set_level(level::level_enum lvl) { current_level() = lvl; }
inline level::level_enum get_level() { return current_level(); }

namespace detail {

inline void format_impl(std::ostringstream& os, const char* fmt) {
    os << fmt;
}

template <typename T, typename... Rest>
void format_impl(std::ostringstream& os, const char* fmt, T&& value, Rest&&... rest) {
    const char* brace = std::strstr(fmt, "{}");
    if (brace == nullptr) {
        os << fmt;
        return;
    }
    os.write(fmt, static_cast<std::streamsize>(brace - fmt));
    os << value;
    format_impl(os, brace + 2, std::forward<Rest>(rest)...);
}

template <typename... Args>
void log(level::level_enum lvl, const char* fmt, Args&&... args) {
    if (lvl < current_level() || current_level() == level::off) return;
#ifdef SPDLOG_SHIM_QUIET
    (void)sizeof...(args);
    return;
#else
    std::ostringstream os;
    format_impl(os, fmt, std::forward<Args>(args)...);
    std::fprintf(stderr, "[%s] %s\n",
                 lvl == level::info ? "info" : (lvl == level::warn ? "warning" : "error"),
                 os.str().c_str());
#endif
}

} // namespace detail

template <typename... Args>
void info(const char* fmt, Args&&... args) {
    detail::log(level::info, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void warn(const char* fmt, Args&&... args) {
    detail::log(level::warn, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void error(const char* fmt, Args&&... args) {
    detail::log(level::err, fmt, std::forward<Args>(args)...);
}

} // namespace spdlog
