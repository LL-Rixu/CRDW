#include <RE/Skyrim.h>
#include <cache.h>

class DirectorySearch
{
    bool             done;

    HANDLE           handle;
    WIN32_FIND_DATAA data;
    char             path[MAX_PATH];

public:
    DirectorySearch(char* directory): handle(nullptr), done(false)
    {
        if (directory == nullptr) 
        {
            done = true;

            return;
        }
    
        strcpy_s(path, sizeof(path), directory);
        strcat_s(path, sizeof(path), "\\*");

        Next();
    }

    ~DirectorySearch()
    {
        if(handle == INVALID_HANDLE_VALUE) { return; }

        FindClose(handle);
    }

    bool Done() const { return done; }

    void Next()
    {
        if(handle == nullptr)
        {
            handle = FindFirstFileA(path, &data);
            done = handle == INVALID_HANDLE_VALUE ? true : false;
            return;
        }

        if(handle == INVALID_HANDLE_VALUE) 
        { 
            done = true;
            return;
        }

        done = !FindNextFileA(handle, &data);
    }

    const bool isDirectory() const { return (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0; }
    const bool isDot() const
    {
        const char* filename = FileName();

        if(filename[0] == '.')
        {
            if(filename[1] == '\0' || (filename[1] == '.' && filename[2] == '\0'))
            {
                return true;
            }
        }

        return false;
    }
    const bool isFile() const { return (data.dwFileAttributes & (FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_NORMAL)) != 0; }
    const char* FileName() const { return data.cFileName; }
};

static inline bool CheckArchiveOverwrite(RE::BSResource::LooseFileLocation* location, const char* path, uint64_t* filesize)
{
    RE::BSResource::Info info;
    RE::BSResource::Location* tmp = nullptr;
    RE::BSTSmartPointer<RE::BSResource::Stream> stream;

    const auto result = location->DoCreateStream(path, stream, tmp, false);
    if(result != RE::BSResource::ErrorCode::kNone) { return false; }

    stream.get()->DoGetInfo(info);
    *filesize = info.fileSize;

    return true;
}

void CachingDirectoryRecursive(char* buffer, const char* relative, char* cursor, RE::BSResource::LooseFileLocation* location, std::ofstream& file)
{
    for(DirectorySearch search(buffer); !search.Done(); search.Next())
    {
        const size_t remaining = static_cast<size_t>(buffer - cursor + MAX_PATH);

        if(search.isDirectory())
        {
            if(search.isDot()) { continue; }

            strcpy_s(cursor, remaining, search.FileName());
            strcat_s(cursor, remaining, "\\");

            CachingDirectoryRecursive(
                buffer,
                relative, 
                cursor + strlen(search.FileName()) + 1,
                location,
                file
            );
        }
        else if(search.isFile())
        {
            uint64_t filesize;
            strcpy_s(cursor, remaining, search.FileName());
            if(CheckArchiveOverwrite(location, relative, &filesize))
            {
                Entry cache;
                cache.size = filesize;
                strcpy_s(cache.path, sizeof(cache.path), relative);
                file.write(reinterpret_cast<char*>(&cache), sizeof(cache));
            }
        }
    }
}