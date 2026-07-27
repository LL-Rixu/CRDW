#include <cache.h>

/*
    Micro optimisation. 
    Skip thunk function and write GetFileSizeCached as binary into 26byte codecave
*/
void SizeCache::fast_function(void)
{
    const uint64_t address = reinterpret_cast<uint64_t>(&entry);
    constexpr uint32_t offset = offsetof(Entry, size);

    uint8_t payload[26] =
    { 
        0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // movabs rax, entry
        0x48, 0x8B, 0x00,                                           // mov rax, [rax]
        0x48, 0x8B, 0x80, 0x00, 0x00, 0x00, 0x00,                   // mov rax, [rax + offsetof(Entry,size)]
        0x48, 0x89, 0x02,                                           // mov [rdx], rax
        0xB0, 0x01,                                                 // mov al, 1
        0xC3                                                        // ret
    };

    std::memcpy(&payload[2],  &address,  sizeof(uint64_t));
    std::memcpy(&payload[16], &offset,   sizeof(uint32_t));

    REL::safe_write(codecave, payload);
}

void SizeCache::thunk_function(void)
{
    const uint64_t address = reinterpret_cast<uint64_t>(GetSizeCache);

    uint8_t payload[14] =
    { 
        0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // jmp qword ptr [rip + 0] 
    };

    std::memcpy(&payload[6], &address, sizeof(uint64_t));

    REL::safe_write(codecave, payload);
}