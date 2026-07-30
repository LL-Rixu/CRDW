#pragma once

#include <FixedString.h>

#include <SKSE/Logger.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/basic_file_sink.h>

template<FixedString name, FixedString version>
class Logger
{
public:
    Logger(bool enabled, int log_level, bool a_debug = false)
    {
        if(!enabled || instance) { return; }

        debug = a_debug;

        instance = init();
        if(!instance) { return; }
        instance->set_pattern("[%g:%#] [%!] [%t] [%L] %v");
        instance->flush_on(static_cast<spdlog::level::level_enum>(log_level));

        const char* debug_print = debug ? "Debug Enabled" : "";
        Log<__FILE__, __FUNCTION__, __LINE__>("{} {} {}", name.c_str(), version.c_str(), debug_print);
    }

    ~Logger()
    {
        if(!instance) { return; }
        instance->flush();

        delete instance;
        instance = nullptr;
    }

    Logger(const Logger&) = delete;

    template<FixedString file, FixedString function, int64_t line, spdlog::level::level_enum lvl = spdlog::level::info, typename... Args>
    static void Log(spdlog::format_string_t<Args...> fmt, Args&&... args)
    {
        if(!instance) { return; }

        instance->log({file, line, function}, lvl, fmt, std::forward<Args>(args)...);
        if(debug){ instance->flush(); }
    }

private:
    inline static bool debug = false;
    inline static spdlog::logger* instance = nullptr;

    static spdlog::logger* init()
    {
        std::optional<std::filesystem::path> path = SKSE::log::log_directory();
        if(!path) { return nullptr; }

        *path /= std::format("{}.log", name.c_str());

        std::vector<spdlog::sink_ptr> sinks{
            std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true),
            std::make_shared<spdlog::sinks::msvc_sink_mt>()
        };

        return new spdlog::logger("global", sinks.begin(), sinks.end());
    }
};

#define Log(lvl, ...) Logger<PROJECT_NAME, PROJECT_VERSION>::Log<__FILE__, __FUNCTION__, __LINE__, lvl>(__VA_ARGS__)