#pragma once

#include <ID.h>
#include <log.h>
#include <mmap.h>
#include <hook.h>

class Entry
{
public:
    char path[MAX_PATH];
    uint64_t size;

    Entry(const char* a_path, const uint64_t a_size): size(a_size) { strcpy_s(path, sizeof(path), a_path); }
};

class Cache: public MMAP<Entry>
{
public:
    Cache(std::string& path, bool optimize = false): MMAP(path)
    {
        if(optimize) { hook.Install(inlined(&entry, offsetof(Entry, size))); }
        else{ hook.Install(jmp64(GetSize)); }
    }

   inline void SetEntry(const Entry* a_entry) { entry = a_entry; }
private:
    static bool GetSize(const char* path, uint64_t* size) 
    {
        size_t plen = strlen(path);
        size_t elen = strlen(entry->path);

        size_t offset = plen - elen;

        if(plen < elen) { Log(spdlog::level::critical, "{} > {}", path, entry->path); }
        else if(strcmp(path + offset, entry->path))
        {
            Log(spdlog::level::critical, "{} != {}", path, entry->path);
        }

        *size = entry->size;
        return true;
    }

    inline static const Entry* entry;
    Hook<GetSizeID> hook;
};

