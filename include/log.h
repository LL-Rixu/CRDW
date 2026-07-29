#pragma once

#include <FixedString.h>

#include <SKSE/Logger.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/basic_file_sink.h>

template<FixedString name>
class Logger
{
public:
    Logger(bool enabled)
    {
        if(!enabled || instance) { return; }

        instance = init();
        if(!instance) { return; }
        instance->set_pattern("[%g:%#] [%!] [%L] %v");
        instance->flush_on(spdlog::level::info);

        Log<__FILE__, __FUNCTION__, __LINE__, spdlog::level::info>("{}: Enabled Logging", name.c_str());
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
    }

private:
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

#define Log(lvl, ...) Logger<"CRDW">::Log<__FILE__, __FUNCTION__, __LINE__, lvl>(__VA_ARGS__)