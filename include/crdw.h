#pragma once

#include <SKSE/SKSE.h>
#include <RE/Skyrim.h>

#include <log.h>
#include <ini.h>
#include <hook.h>
#include <cache.h>

#include <span>

class CRDW
{
    enum class Option
    { 
        CACHE,
        LOAD,
        NATIVE 
    };
public:
    inline static CRDW* crdw = nullptr;

    INI<"Data/SKSE/Plugins/CRDW.ini"> ini;
    std::span<char> iobuffer;
    Hook<70050> hook;
    Logger<"CRDW"> logger;

    template<auto f, typename... Args>
    void Fallback(Args&&... args)
    {
        std::span<uint8_t> binary = hook.Binary();
        std::vector<uint8_t> copy(binary.begin(), binary.end());
        auto native = hook.Native<f>();
        native(std::forward<Args>(args)...);
        hook.Install(copy);
    }

    template<FixedString path>
    static inline std::string GetCachePath(std::string name)
    {
        std::replace(name.begin(), name.end(), '\\', '_');

        for(char& ch : name) { ch = std::tolower(ch); }
        
        return path.c_str() + name + ".cache";
    }

    CRDW();
    ~CRDW();

    static void CacheLoad(char* buffer, const char* relative, char* cursor, RE::BSResource::Traverser* traverser, RE::BSResource::LooseFileLocation* location);
    static void CacheGenerate(char* buffer, const char* relative, char* cursor, RE::BSResource::Traverser* traverser, RE::BSResource::LooseFileLocation* location);
    static void ProcessName(RE::BSResource::Traverser* a_this, const char* path, RE::BSResource::Location& location);
    static void DirectoryRecursiveWalk(char* buffer, const char* relative, char* cursor, RE::BSResource::LooseFileLocation* location, std::ofstream& file);
    static void MessageInterface(SKSE::MessagingInterface::Message* msg);
};