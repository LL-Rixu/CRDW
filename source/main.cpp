#include <crdw.h>

void CRDW::AccelerateA(char* buffer, const char* relative, char* cursor, RE::BSResource::Traverser* traverser, RE::BSResource::LooseFileLocation* location)
{
    Log(spdlog::level::info, "Opening {}", relative);

    auto& ini = crdw->ini;
    Accelerate accelerate(ini["Optimization|Experimental"], ini["General|Debug"]);

    DirectoryRecursiveWalkA(
        buffer,
        relative,
        cursor,
        [&accelerate, traverser, location] (const Entry* entry)
        {
            accelerate.SetEntry(entry);
            ProcessName(traverser, entry->path, *location);
        }
    );
}

void CRDW::AccelerateW(char* buffer, const char* relative, char* cursor, RE::BSResource::Traverser* traverser, RE::BSResource::LooseFileLocation* location)
{
    Log(spdlog::level::info, "Opening {}", relative);

    auto& ini = crdw->ini;
    Accelerate accelerate(ini["Optimization|Experimental"], ini["General|Debug"]);

    wchar_t copy_buffer[MAX_PATH];
    size_t length = MultiByteToWideChar(
        CP_ACP,
        0,
        buffer,
        -1,
        copy_buffer,
        MAX_PATH
    );

    size_t offset = length - MultiByteToWideChar(CP_ACP, 0, relative, -1, nullptr, 0);

    DirectoryRecursiveWalkW(
        copy_buffer,
        length - 1,
        copy_buffer + offset,
        [&accelerate, traverser, location] (const Entry* entry)
        {
            accelerate.SetEntry(entry);
            ProcessName(traverser, entry->path, *location);
        }
    );
}

void CRDW::CacheGenerateA(char* buffer, const char* relative, char* cursor, RE::BSResource::Traverser* traverser, RE::BSResource::LooseFileLocation* location)
{
    Log(spdlog::level::info, "Opening {}", relative);

    char copy_buffer[MAX_PATH];
    strcpy_s(copy_buffer, buffer);

    size_t reloff = relative - buffer;
    size_t curoff = cursor - buffer;

    const std::string path = crdw->GetCachePath(relative);

    auto& ini = crdw->ini;
    const size_t size = ini["Optimization|IOBuffer"];
    uint8_t* iobuffer = crdw->iobuffer;
    size_t head = 0;

    HANDLE hFile = CreateFileA(
        path.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
        nullptr
    );

    if(hFile == INVALID_HANDLE_VALUE)
    {
        LPVOID message;
        DWORD error = GetLastError();

        if(FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&message,
            0, 
            nullptr
        ) != 0)
        {
            Log(spdlog::level::critical, "{} {}", path, reinterpret_cast<char*>(message));
            LocalFree(message);
        }

        return crdw->Fallback<CacheGenerateA>(buffer, relative, cursor, traverser, location);
    }

    DirectoryRecursiveWalkA(
        copy_buffer,
        copy_buffer + reloff,
        copy_buffer + curoff,
        [hFile, &iobuffer, size, &head] (const Entry* entry)
        {
            [[unlikely]] if(head + sizeof(*entry) > size)
            {
                const size_t overwrite = head + sizeof(*entry) - size;
                const size_t cut = sizeof(*entry) - overwrite;

                std::memcpy(iobuffer + head, entry, cut);
                
                WriteFile(hFile, iobuffer, size, nullptr, nullptr);

                head = sizeof(*entry) - cut;
                std::memcpy(iobuffer, reinterpret_cast<const char*>(entry) + cut, head);
            }
            else
            {
                std::memcpy(iobuffer + head, entry, sizeof(*entry));
                head += sizeof(*entry);
            }
        }
    );

    WriteFile(hFile, iobuffer, head, nullptr, nullptr);
    CloseHandle(hFile);

    CacheLoad(buffer, relative, cursor, traverser, location);
}

void CRDW::CacheGenerateW(char* buffer, const char* relative, char* cursor, RE::BSResource::Traverser* traverser, RE::BSResource::LooseFileLocation* location)
{
    Log(spdlog::level::info, "Opening {}", relative);

    const std::string path = crdw->GetCachePath(relative);

    wchar_t copy_buffer[MAX_PATH];
    size_t length = MultiByteToWideChar(
        CP_ACP,
        0,
        buffer,
        -1,
        copy_buffer,
        MAX_PATH
    );

    size_t offset = length - MultiByteToWideChar(CP_ACP, 0, relative, -1, nullptr, 0);
    const size_t size = crdw->ini["Optimization|IOBuffer"];
    uint8_t* iobuffer = crdw->iobuffer;
    size_t head = 0;

    HANDLE hFile = CreateFileA(
        path.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
        nullptr
    );

    if(hFile == INVALID_HANDLE_VALUE)
    {
        LPVOID message;
        DWORD error = GetLastError();

        if(FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&message,
            0, 
            nullptr
        ) != 0)
        {
            Log(spdlog::level::critical, "{} {}", path, reinterpret_cast<char*>(message));
            LocalFree(message);
        }

        return crdw->Fallback<CacheGenerateW>(buffer, relative, cursor, traverser, location);
    }

    DirectoryRecursiveWalkW(
        copy_buffer,
        length - 1,
        copy_buffer + offset,
        [hFile, &iobuffer, size, &head] (const Entry* entry)
        {
            [[unlikely]] if(head + sizeof(*entry) > size)
            {
                const size_t overwrite = head + sizeof(*entry) - size;
                const size_t cut = sizeof(*entry) - overwrite;

                std::memcpy(iobuffer + head, entry, cut);
                
                WriteFile(hFile, iobuffer, size, nullptr, nullptr);

                head = sizeof(*entry) - cut;
                std::memcpy(iobuffer, reinterpret_cast<const char*>(entry) + cut, head);
            }
            else
            {
                std::memcpy(iobuffer + head, entry, sizeof(*entry));
                head += sizeof(*entry);
            }
        }
    );

    WriteFile(hFile, iobuffer, head, nullptr, nullptr);
    CloseHandle(hFile);

    CacheLoad(buffer, relative, cursor, traverser, location);
}

void CRDW::CacheLoad(char* buffer, const char* relative, char* cursor, RE::BSResource::Traverser* traverser, RE::BSResource::LooseFileLocation* location)
{
    const std::string path = crdw->GetCachePath(relative);

    auto& ini = crdw->ini;
    Cache cache(path, ini["Optimization|Experimental"], ini["General|Debug"]);

    if(!cache.Open())
    {
        cache.UnHook();
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
    if(
        (
            msg->type != SKSE::MessagingInterface::kPreLoadGame && 
            msg->type != SKSE::MessagingInterface::kNewGame
        ) || 
        !crdw
    )
    { return; }

    delete crdw;
    crdw = nullptr;
}

CRDW::CRDW(): ini(), logger(ini["General|Logging"], ini["General|LogFlush"], ini["General|Debug"])
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
        else if(option == "ACCELERATE") { mode = Option::ACCELERATE; }
        else{ mode = Option::LOAD; }
    }

    constexpr void* const table[4] = 
    {
        CacheGenerateA, CacheGenerateW,
        AccelerateA, AccelerateW
    };

    switch(mode)
    {
    case Option::CACHE: 
    iobuffer = new uint8_t[ini["Optimization|IOBuffer"]];
    case Option::ACCELERATE:
    hook.Install(
        jmp64(
            table[
                (static_cast<int64_t>(mode) << 1) + 
                static_cast<int64_t>(ini["Optimization|Unicode"])
            ]
        )
    );
    break;
    case Option::LOAD: hook.Install(jmp64(CacheLoad)); break;
    default: break;
    }

    SKSE::GetMessagingInterface()->RegisterListener(MessageInterface);
}

CRDW::~CRDW()
{
    if(!iobuffer) { return; }

    delete[] iobuffer;
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    overwrite::init();

    SKSE::Init(skse, false);

    new CRDW();

    return true;
}