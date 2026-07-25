#pragma once

#include <SKSE/SKSE.h>
#include <RE/Skyrim.h>
#include <Windows.h>

struct Entry
{
    char path[MAX_PATH];
    uint64_t size;
};

class FileSizeCache
{
    uint8_t call[5];
    inline static const Entry* entry;

    static bool GetFileSizeCached(const char* path, uint64_t* size) { return entry->size; }
public:
    [[inline]] static void SetEntry(const Entry* a_entry) { entry = a_entry; }

    FileSizeCache();
    ~FileSizeCache();
};

class Cache : public FileSizeCache
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    HANDLE hMap = INVALID_HANDLE_VALUE;
    const Entry* map = nullptr;
    size_t size = 0;

public:
    bool isOpen() const { return hFile != INVALID_HANDLE_VALUE && hMap != INVALID_HANDLE_VALUE && map != nullptr; }

    const Entry* begin() const { return &map[0]; }
    const Entry* end()   const { return &map[size / sizeof(Entry)]; }

    Cache(std::string& path)
    {
        hFile = CreateFileA(
            path.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
            NULL
        );

        if(hFile == INVALID_HANDLE_VALUE) { return; }

        if (!GetFileSizeEx(hFile, reinterpret_cast<LARGE_INTEGER*>(&size))) 
        {
            this->~Cache();
            return;
        }

        hMap = CreateFileMappingA(
            hFile,
            NULL,
            PAGE_READONLY,
            0,
            0,
            NULL
        );

        if (!hMap) 
        {
            this->~Cache();
            return;
        }

        map = reinterpret_cast<const Entry*>(MapViewOfFile(
            hMap,
            FILE_MAP_READ,
            0, 
            0,
            0
        ));

        if(!map)
        {
            this->~Cache();
            return;
        }
    }

    ~Cache()
    {
        size = 0;

        if(map)
        {
            UnmapViewOfFile(map);
            map = nullptr;
        }
        if(hMap)
        {
            CloseHandle(hMap);
            hMap = INVALID_HANDLE_VALUE;
        }
        if(hFile != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hFile);
            hFile = INVALID_HANDLE_VALUE;
        }
    }
};