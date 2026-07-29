#pragma once

#include <RE/Skyrim.h>
#include <log.h>

template<uint64_t id>
class Hook
{
    bool loaded = false;
    std::vector<uint8_t> bytes;

public:
    void Install(std::vector<uint8_t> payload)
    {
        const uintptr_t address = REL::ID(id).address();

        bytes.reserve(payload.size());
        std::copy_n(reinterpret_cast<uint8_t*>(address), payload.size(), std::back_inserter(bytes));
        REL::safe_write(address, payload);

        loaded = true;

        Log(spdlog::level::info, "Install");
    }

    void Remove()
    {
        if(!loaded) { return; }

        const uintptr_t address = REL::ID(id).address();
        REL::safe_write(address, bytes);

        loaded = false;

        Log(spdlog::level::info, "Remove");
    }

    ~Hook() { Remove(); }

    std::span<uint8_t> Binary() const { return { reinterpret_cast<uint8_t*>(REL::ID(id).address()), bytes.size() }; }

    template<auto function>
    auto Native()
    {
        if(loaded) { Remove(); }
        return reinterpret_cast<decltype(function)>(REL::ID(id).address()); 
    }

    template<typename T>
    auto Native()
    {
        if(loaded) { Remove(); }
        return reinterpret_cast<T>(REL::ID(id).address()); 
    }
};

std::vector<uint8_t> jmp64(const void* address);
std::vector<uint8_t> inlined(const void* address, const uint32_t offset);