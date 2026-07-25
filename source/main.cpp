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

FileSizeCache::FileSizeCache()
{
    const uintptr_t callsite = REL::Offset(0xD0F987).address();
    const uintptr_t codecave = REL::Offset(0xD0F8C5).address();
    const uintptr_t hook     = reinterpret_cast<uintptr_t>(GetFileSizeCached);

    std::memcpy(call, reinterpret_cast<uint8_t*>(callsite), sizeof(call));

    uintptr_t jmpFromCaveAddr = codecave + 12;
    int32_t caveJmpOffset = static_cast<int32_t>((callsite + 5) - (jmpFromCaveAddr + 5));

    uint8_t payload[17] = { 0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xD0, 0xE9, 0x00, 0x00, 0x00, 0x00 };
    std::memcpy(&payload[2], &hook, sizeof(uint64_t));
    std::memcpy(&payload[13], &caveJmpOffset, sizeof(int32_t));

    uint8_t jump[5] = { 0xE9 };
    int32_t hookJmpOffset = static_cast<int32_t>(codecave - (callsite + 5));    
    std::memcpy(&jump[1], &hookJmpOffset, sizeof(int32_t));

    REL::safe_write(codecave, payload);
    REL::safe_write(callsite, jump);
}

FileSizeCache::~FileSizeCache()
{
    const uintptr_t callsite = REL::Offset(0xD0F987).address();
    REL::safe_write(callsite, call);
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