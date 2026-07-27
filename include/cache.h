#pragma once

#include <SKSE/SKSE.h>
#include <RE/Skyrim.h>
#include <Windows.h>

struct Entry
{
    char path[MAX_PATH];
    uint64_t size;

    Entry(const char* a_path, const uint64_t a_size): size(a_size) { strcpy_s(path, sizeof(path), a_path); }
};

class SizeCache
{
    uint8_t call[5]; // callsite old value

    inline static const Entry* entry;
    inline static const uintptr_t codecave = REL::Offset(0xD0F8C5).address();
    inline static const uintptr_t callsite = REL::Offset(0xD0F987).address(); 

    void fast_function(void);
    void thunk_function(void);

    static bool GetSizeCache(const char* path, uint64_t* size) { *size = entry->size; return true; }

public:
    inline static bool optimization = false;

    [[inline]] static void SetEntry(const Entry* a_entry) { entry = a_entry; }

    /* install callsite hook */
    SizeCache()
    {
        std::memcpy(call, reinterpret_cast<uint8_t*>(callsite), sizeof(call));

        if(optimization) { fast_function(); }
        else { thunk_function(); }
        
        const int32_t offset = static_cast<int32_t>(codecave - callsite - 5);
        uint8_t hook[5] = { 0xE8, 0x00, 0x00, 0x00, 0x00 };

        std::memcpy(&hook[1], &offset, sizeof(int32_t));

        REL::safe_write(callsite, hook);
    }

    /* Restore callsite */
    ~SizeCache() { REL::safe_write(callsite, call); }
};

class Cache : public SizeCache
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    HANDLE hMap = INVALID_HANDLE_VALUE;
    const Entry* map = nullptr;
    size_t size = 0;

public:
    bool isOpen() const { return hFile != INVALID_HANDLE_VALUE && hMap != INVALID_HANDLE_VALUE && map != nullptr && size != 0; }

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

        if (!GetFileSizeEx(hFile, reinterpret_cast<LARGE_INTEGER*>(&size)))  { return; }

        hMap = CreateFileMappingA(
            hFile,
            NULL,
            PAGE_READONLY,
            0,
            0,
            NULL
        );

        if (!hMap) { return; }

        map = reinterpret_cast<const Entry*>(MapViewOfFile(
            hMap,
            FILE_MAP_READ,
            0, 
            0,
            0
        ));

        if(!map) { return; }
    }

    ~Cache()
    {
        if(map) { UnmapViewOfFile(map); }
        if(hMap) { CloseHandle(hMap); }
        if(hFile != INVALID_HANDLE_VALUE) { CloseHandle(hFile); }
    }
};
