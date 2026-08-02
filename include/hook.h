#pragma once

#include <ID.h>
#include <RE/Skyrim.h>
#include <log.h>

template<uint64_t id>
class Hook
{
    bool loaded = false;
    std::vector<uint8_t> bytes;

    constexpr uintptr_t GetAddress() const
    {
    #if USE_RELID == true
        return REL::ID(id).address();
    #else
        return REL::Offset(id);
    #endif
    } 

public:
    void Install(std::vector<uint8_t> payload)
    {
        const uintptr_t address = GetAddress();

        bytes.reserve(payload.size());
        std::copy_n(reinterpret_cast<uint8_t*>(address), payload.size(), std::back_inserter(bytes));
        REL::safe_write(address, payload);

        loaded = true;
    }

    void Remove()
    {
        if(!loaded) { return; }

        const uintptr_t address = GetAddress();

        REL::safe_write(address, bytes);

        bytes.clear();
        loaded = false;
    }

    ~Hook() { Remove(); }

    std::span<uint8_t> Binary() const 
    {
        return { reinterpret_cast<uint8_t*>(GetAddress()), bytes.size() }; 
    }

    template<auto function>
    auto Native()
    {
        if(loaded) { Remove(); }

        return reinterpret_cast<decltype(function)>(GetAddress()); 
    }

    template<typename T>
    auto Native()
    {
        if(loaded) { Remove(); }

        return reinterpret_cast<T>(GetAddress()); 
    }
};

std::vector<uint8_t> jmp64(const void* address);
std::vector<uint8_t> inlined(const void* address, const uint32_t offset);