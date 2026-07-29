#include <hook.h>

std::vector<uint8_t> jmp64(const void* address)
{
    std::vector<uint8_t> payload =
    { 
        0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // jmp qword ptr [rip + 0]
    };

    std::memcpy(&payload[6], &address, sizeof(uintptr_t));

    return payload;
}

std::vector<uint8_t> inlined(const void* address, const uint32_t offset)
{
    std::vector<uint8_t> payload =
    { 
        0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // movabs rax, entry
        0x48, 0x8B, 0x00,                                           // mov rax, [rax]
        0x48, 0x8B, 0x80, 0x00, 0x00, 0x00, 0x00,                   // mov rax, [rax + offsetof(Entry,size)]
        0x48, 0x89, 0x02,                                           // mov [rdx], rax
        0xB0, 0x01,                                                 // mov al, 1
        0xC3                                                        // ret
    };

    std::memcpy(&payload[2],  &address,  sizeof(uintptr_t));
    std::memcpy(&payload[16], &offset,   sizeof(uint32_t));

    return payload;
}