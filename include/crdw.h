#pragma once

#include <SKSE/SKSE.h>
#include <RE/Skyrim.h>

#include <ID.h>
#include <log.h>
#include <ini.h>
#include <hook.h>
#include <cache.h>
#include <overwrite.h>

/* Figure out where to put this */
template<typename... T>
concept WStringVARG = (std::same_as<T, std::wstring_view> && ...);

template<WStringVARG... T>
inline size_t unitans(char* buffer, const size_t length, T... args)
{
    size_t n = 0;

    (
        (
            n += WideCharToMultiByte(
                CP_ACP,
                0,
                args.data(),
                args.size(),
                buffer + n,
                length - n,
                nullptr,
                nullptr
            )
        ),
        ...
    );

    buffer[n] = '\0';
    return n;
}

class CRDW
{
    enum class Option
    { 
        CACHE,
        ACCELERATE,
        LOAD,
        NATIVE
    };
public:
    inline static CRDW* crdw = nullptr;

    INI<"Data/SKSE/Plugins/CRDW.ini"> ini;
    Logger<PROJECT_NAME, PROJECT_VERSION> logger;
    uint8_t* iobuffer = nullptr;
    Hook<WalkerID> hook;

    template<auto f, typename... Args>
    void Fallback(Args&&... args)
    {
        std::span<uint8_t> binary = hook.Binary();
        std::vector<uint8_t> copy(binary.begin(), binary.end());
        auto native = hook.Native<f>();
        native(std::forward<Args>(args)...);
        hook.Install(copy);
    }

    inline std::string GetCachePath(std::string name)
    {
        std::replace(name.begin(), name.end(), '\\', '_');

        for(char& ch : name) { ch = std::tolower(ch); }
        
        std::string directory(ini["General|CacheDirectory"]);
        if(directory.back() != '/') { directory.push_back('/'); }

        return directory + name + ".cache";
    }

    CRDW();
    ~CRDW();

    static void CacheLoad(char* buffer, const char* relative, char* cursor, RE::BSResource::Traverser* traverser, RE::BSResource::LooseFileLocation* location);
    static void CacheGenerateA(char* buffer, const char* relative, char* cursor, RE::BSResource::Traverser* traverser, RE::BSResource::LooseFileLocation* location);
    static void CacheGenerateW(char* buffer, const char* relative, char* cursor, RE::BSResource::Traverser* traverser, RE::BSResource::LooseFileLocation* location);
    static void AccelerateA(char* buffer, const char* relative, char* cursor, RE::BSResource::Traverser* traverser, RE::BSResource::LooseFileLocation* location);
    static void AccelerateW(char* buffer, const char* relative, char* cursor, RE::BSResource::Traverser* traverser, RE::BSResource::LooseFileLocation* location);
    static void ProcessName(RE::BSResource::Traverser* a_this, const char* path, RE::BSResource::Location& location);
    static void MessageInterface(SKSE::MessagingInterface::Message* msg);

    template<typename F> 
    static void DirectoryRecursiveWalkA(char* buffer, const char* relative, char* cursor, F&& function);

    template<typename F>
    static void DirectoryRecursiveWalkW(wchar_t* path, const size_t length, const wchar_t* const relative, F&& function);
};

#include <walk.tpp>