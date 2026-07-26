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

    FileSizeCache()
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

    ~FileSizeCache()
    {
        const uintptr_t callsite = REL::Offset(0xD0F987).address();
        REL::safe_write(callsite, call);
    }
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