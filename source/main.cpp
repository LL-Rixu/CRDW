#include <cache.h>
#include <span>

std::tuple<bool, std::string, size_t> parseini(void);
void CachingDirectoryRecursive(char* buffer, const char* relative, char* cursor, RE::BSResource::LooseFileLocation* location, std::ofstream& file);
void CustomProcessName(RE::BSResource::Traverser* a_this, const char* a_name, RE::BSResource::Location& a_location);

enum class CacheMode { CACHE, LOAD, NATIVE };

CacheMode mode = CacheMode::LOAD;
void (*native)(char*, const char*, char*, RE::BSResource::Traverser*, RE::BSResource::LooseFileLocation*) = nullptr;
std::span<char> iobuffer;

std::string GetCachePath(std::string name)
{
    std::replace(name.begin(), name.end(), '\\', '_');
    return "Data/SKSE/Plugins/" + name + ".cache";
}

void CacheGenerate(char* buffer, const char* relative, char* cursor, RE::BSResource::LooseFileLocation* location, RE::BSResource::Traverser* traverser)
{
    char copy_buffer[MAX_PATH];
    strcpy_s(copy_buffer, buffer);

    uintptr_t reloff = reinterpret_cast<uintptr_t>(relative) - reinterpret_cast<uintptr_t>(buffer);
    uintptr_t curoff = reinterpret_cast<uintptr_t>(cursor) - reinterpret_cast<uintptr_t>(buffer);

    std::string path = GetCachePath(relative);

    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    file.rdbuf()->pubsetbuf(iobuffer.data(), iobuffer.size());
    CachingDirectoryRecursive(copy_buffer, copy_buffer + reloff, copy_buffer + curoff, location, file);
    file.close();

    Cache cache(path);

    if(!cache.isOpen())
    {
        spdlog::critical("CacheGenerate: failed to open file {}", path);
        cache.~Cache();
        return native(buffer, relative, cursor, traverser, location);
    }

    for(const auto& entry : cache)
    {
        cache.SetEntry(&entry);
        CustomProcessName(traverser, entry.path, *location);
    }
}

void CacheLoad(char* buffer, const char* relative, char* cursor, RE::BSResource::Traverser* traverser, RE::BSResource::LooseFileLocation* location)
{
    std::string path = GetCachePath(relative);

    Cache cache(path);

    if(!cache.isOpen())
    {
        spdlog::warn("CacheLoad: failed to open file {}", path);
        cache.~Cache();
        return native(buffer, relative, cursor, traverser, location);
    }

    for(const auto& entry : cache)
    {
        cache.SetEntry(&entry);
        CustomProcessName(traverser, entry.path, *location);
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

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    SKSE::Init(skse);

    SKSE::AllocTrampoline(14);
    auto& trampoline = SKSE::GetTrampoline();

    native = reinterpret_cast<void (*)(char*, const char*, char*, RE::BSResource::Traverser*, RE::BSResource::LooseFileLocation*)>(
        trampoline.write_call<5>(REL::Module::get().base() + 0xd103ad, reinterpret_cast<std::uintptr_t>(Thunk))
    );

    auto [show, selected, size] = parseini();

    iobuffer = std::span<char>(reinterpret_cast<char*>(malloc(size)), size);

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
        if(msg->type == SKSE::MessagingInterface::kDataLoaded) 
        { 
            mode = CacheMode::NATIVE;
            free(iobuffer.data());
        }
    });

    return true;
}