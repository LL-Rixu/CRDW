#include <crdw.h>

void CRDW::CacheGenerate(char* buffer, const char* relative, char* cursor, RE::BSResource::Traverser* traverser, RE::BSResource::LooseFileLocation* location)
{
    char copy_buffer[MAX_PATH];
    strcpy_s(copy_buffer, buffer);

    uintptr_t reloff = reinterpret_cast<uintptr_t>(relative) - reinterpret_cast<uintptr_t>(buffer);
    uintptr_t curoff = reinterpret_cast<uintptr_t>(cursor) - reinterpret_cast<uintptr_t>(buffer);

    std::string path = GetCachePath<"Data/SKSE/Plugins/">(relative);

    std::span<char>& iobuffer = crdw->iobuffer;

    std::ofstream file;
    file.rdbuf()->pubsetbuf(iobuffer.data(), iobuffer.size());
    file.open(path, std::ios::binary | std::ios::trunc);

    if(!file.is_open())
    {
        Log(spdlog::level::critical, "failed to open/create cache {}", path);
        return crdw->Fallback<CacheGenerate>(buffer, relative, cursor, traverser, location);
    }

    DirectoryRecursiveWalk(copy_buffer, copy_buffer + reloff, copy_buffer + curoff, location, file);
    file.close();

    Cache cache(path, crdw->ini["Optimization|Experimental"]);

    if(!cache.Open())
    {
        cache.~Cache();
        Log(spdlog::level::critical, "failed to open cache {}", path);
        return crdw->Fallback<CacheGenerate>(buffer, relative, cursor, traverser, location);
    }

    for(const auto& entry : cache)
    {
        cache.SetEntry(&entry);
        ProcessName(traverser, entry.path, *location);
    }
}

void CRDW::CacheLoad(char* buffer, const char* relative, char* cursor, RE::BSResource::Traverser* traverser, RE::BSResource::LooseFileLocation* location)
{
    std::string path = GetCachePath<"Data/SKSE/Plugins/">(relative);

    Cache cache(path, crdw->ini["Optimization|Experimental"]);

    if(!cache.Open())
    {
        cache.~Cache();
        Log(spdlog::level::warn, "failed to open cache {}", path);
        return crdw->Fallback<CacheLoad>(buffer, relative, cursor, traverser, location);
    }

    for(const auto& entry : cache)
    {
        cache.SetEntry(&entry);
        ProcessName(traverser, entry.path, *location);
    }
}

void CRDW::MessageInterface(SKSE::MessagingInterface::Message* msg)
{
    if(msg->type != SKSE::MessagingInterface::kDataLoaded) { return; }

    crdw->~CRDW();
}

CRDW::CRDW(): ini(), iobuffer(new char[ini["Optimization|IOBuffer"]], ini["Optimization|IOBuffer"]), logger(ini["General|Logging"], ini["General|LogFlush"])
{
    crdw = this;

    Option mode = Option::LOAD;

    bool show = ini["General|ShowPopUp"];
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
        case IDYES: mode = Option::CACHE; break;
        default: mode = Option::LOAD; break;
        }
    }
    else
    {
        std::string_view option = ini["General|Mode"];

        if(option == "CACHE") { mode = Option::CACHE; }
        else if(option == "NATIVE") { mode = Option::NATIVE; }
        else{ mode = Option::LOAD; }
    }

    switch(mode)
    {
    case Option::CACHE: hook.Install(jmp64(CacheGenerate)); break;
    case Option::LOAD: hook.Install(jmp64(CacheLoad)); break;
    default: break;
    }

    SKSE::GetMessagingInterface()->RegisterListener(MessageInterface);

    Log(spdlog::level::info, "Loaded CRDW");
}

CRDW::~CRDW()
{
    delete[] iobuffer.data();

    Log(spdlog::level::info, "Unloaded CRDW");
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    SKSE::Init(skse, false);

    static CRDW crdw;

    return true;
}