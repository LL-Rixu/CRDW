#include <crdw.h>
#include <string>

class DirectoryA
{
    bool             done;

    HANDLE           handle;
    WIN32_FIND_DATAA data;
    char             path[MAX_PATH];

public:
    DirectoryA(char* directory): handle(nullptr), done(false)
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

    ~DirectoryA()
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
    const uint64_t FileSize() const { return static_cast<uint64_t>(data.nFileSizeHigh) << 32 | static_cast<uint64_t>(data.nFileSizeLow); }
};

template<typename F>
void CRDW::DirectoryRecursiveWalkA(char* buffer, const char* relative, char* cursor, F&& function)
{
    for(DirectoryA search(buffer); !search.Done(); search.Next())
    {
        const size_t remaining = static_cast<size_t>(buffer - cursor + MAX_PATH);

        if(search.isDirectory())
        {
            if(search.isDot()) { continue; }

            strcpy_s(cursor, remaining, search.FileName());
            strcat_s(cursor, remaining, "\\");

            DirectoryRecursiveWalkA(
                buffer,
                relative,
                cursor + strlen(search.FileName()) + 1,
                function
            );
        }
        else if(search.isFile())
        {
            strcpy_s(cursor, remaining, search.FileName());
            const Entry entry(relative, search.FileSize());

            function(&entry);
        }
    }
}

class DirectoryW
{
public:
    class File
    {
    public:
        File(const FILE_ID_BOTH_DIR_INFO& a_info): info(a_info) {}

        const bool isDirectory() const { return (info.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0; }
        const bool isFile() const { return (info.FileAttributes & (FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_NORMAL)) != 0; }
        const size_t Size() const { return info.EndOfFile.QuadPart; }
        const bool isDot() const 
        {
            bool dot = info.FileNameLength == 2 && info.FileName[0] == L'.';
            bool dotdot = info.FileNameLength == 4 && info.FileName[0] == L'.' && info.FileName[1] == L'.';

            return dot || dotdot;
        }

        const std::wstring_view Name() const { return { info.FileName, info.FileNameLength / sizeof(wchar_t) }; }
    private:
        const FILE_ID_BOTH_DIR_INFO& info;
    };

    DirectoryW(const wchar_t* path)
    {
        hDirectory = CreateFileW(
            path,
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS,
            nullptr
        );

        if(hDirectory == INVALID_HANDLE_VALUE)
        {
            done = true;
            return;
        }

        if(!GetFileInformationByHandleEx(
            hDirectory,
            FileIdBothDirectoryRestartInfo,
            buffer,
            sizeof(buffer)
        ))
        {
            done = true;
        }
    }

    ~DirectoryW()
    {
        if(hDirectory == INVALID_HANDLE_VALUE) { return; }

        CloseHandle(hDirectory);
    }

    void Next()
    {
        const FILE_ID_BOTH_DIR_INFO& info = GetFileInfo();

        [[unlikely]] if(info.NextEntryOffset == 0)
        {
            offset = 0;

            if(!GetFileInformationByHandleEx(
                hDirectory,
                FileIdBothDirectoryInfo,
                buffer,
                sizeof(buffer)
            ))
            {
                done = true;
            }
        }
        else { offset += info.NextEntryOffset; }
    }

    const FILE_ID_BOTH_DIR_INFO& GetFileInfo() { return *reinterpret_cast<const PFILE_ID_BOTH_DIR_INFO>(buffer + offset); }
    const bool Done() const { return done; }
private:
    bool done = false;
    HANDLE hDirectory;

    uint8_t buffer[65536];
    size_t offset = 0;
};

template<typename F>
void CRDW::DirectoryRecursiveWalkW(wchar_t* path, const size_t length, const wchar_t* const relative, F&& function)
{
    for(DirectoryW directory(path); !directory.Done(); directory.Next())
    {
        const size_t remaining = MAX_PATH - length;

        DirectoryW::File file(directory.GetFileInfo());
        const std::wstring_view filename(file.Name());
    
        if(file.isDirectory())
        {
            if(file.isDot()) { continue; }

            wmemcpy(path + length, filename.data(), filename.size());
            path[length + filename.size()] = L'\\';
            path[length + filename.size() + 1] = L'\0';

            DirectoryRecursiveWalkW(
                path, 
                length + filename.size() + 1, 
                relative, 
                function
            );
        }
        else if(file.isFile())
        {
            const size_t relsize = length - (relative - path);
            Entry entry;
            unitans(
                entry.path, 
                sizeof(entry.path), 
                std::wstring_view(relative, relsize),
                filename
            );
            entry.size = file.Size();

            function(&entry);
        }
    }
}