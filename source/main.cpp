#include <cache.h>

std::tuple<bool, std::string> parseini(void);
void CachingDirectoryRecursive(char* buffer, const char* relative, char* cursor, RE::BSResource::LooseFileLocation* location, std::ofstream& file);

enum class CacheMode { CACHE, LOAD, NATIVE };
CacheMode mode = CacheMode::LOAD;
void (*native)(char*, const char*, char*, RE::BSResource::Traverser*, RE::BSResource::LooseFileLocation*) = nullptr;

std::string GetCachePath(std::string name)
{
    std::replace(name.begin(), name.end(), '\\', '_');
    return "Data/SKSE/Plugins/" + name + ".cache";
}

void CacheGenerate(char* buffer, const char* relative, char* cursor, RE::BSResource::LooseFileLocation* location, RE::BSResource::Traverser* traverser)
{
    std::string path = GetCachePath(relative);

    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    CachingDirectoryRecursive(buffer, relative, cursor, location, file);
    file.close();

    Cache cache(path);

    if(!cache.isOpen())
    {
        spdlog::critical("CacheGenerate: failed to open file {}", path);
        return native(buffer, relative, cursor, traverser, location);
    }

    for(const auto& entry : cache)
    {
        cache.SetEntry(&entry);
        traverser->ProcessName(entry.path, *location);
    }
}

void CacheLoad(char* buffer, const char* relative, char* cursor, RE::BSResource::Traverser* traverser, RE::BSResource::LooseFileLocation* location)
{
    std::string path = GetCachePath(relative);

    Cache cache(path);

    if(!cache.isOpen())
    {
        spdlog::warn("CacheLoad: failed to open file {}", path);
        return native(buffer, relative, cursor, traverser, location);
    }

    for(const auto& entry : cache)
    {
        cache.SetEntry(&entry);
        traverser->ProcessName(entry.path, *location);
    }
}

void Thunk(char* buffer, const char* relative, char* cursor, RE::BSResource::Traverser* traverser, RE::BSResource::LooseFileLocation* location)
{
    switch(mode)
    {
    case CacheMode::CACHE: return CacheGenerate(buffer, relative, cursor, location, traverser);
    case CacheMode::LOAD: return CacheLoad(buffer, relative, cursor, traverser, location);
    default: return native(buffer, relative, cursor, traverser, location);
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
    SKSE::Init(a_skse);

    SKSE::AllocTrampoline(28);
    auto& trampoline = SKSE::GetTrampoline();

    native = reinterpret_cast<void (*)(char*, const char*, char*, RE::BSResource::Traverser*, RE::BSResource::LooseFileLocation*)>(
        trampoline.write_call<5>(REL::Module::get().base() + 0xd103ad, reinterpret_cast<std::uintptr_t>(Thunk))
    );

    auto [show, selected] = parseini();

    if(show)
    {
        int result = MessageBoxA(
            nullptr, 
            "Do you want to generate CRDW cache files?\n\nPress \"yes\" if you have edited anything in Data\\ this will re-generate the cache files. If you haven't done any changes in Data\\ after last generating cache files press \"No\"", 
            "Cache Recursive Directory Walk", 
            MB_YESNO | MB_ICONQUESTION
        );

        switch(result)
        {
        case IDYES: mode = CacheMode::CACHE; break;
        default: mode = CacheMode::LOAD; break;
        }
    }
    else
    {
        if(selected == "CACHE") { mode = CacheMode::CACHE; }
        else if(selected == "NATIVE") { mode = CacheMode::NATIVE; }
        else{ mode = CacheMode::LOAD; }
    }

    SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message* msg) 
    {
        if(msg->type == SKSE::MessagingInterface::kDataLoaded) { mode = CacheMode::NATIVE; }
    });

    return true;
}