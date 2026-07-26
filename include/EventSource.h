#include <RE/Skyrim.h>

struct BSResourceFileFoundEvent
{
    const char*               name;      // 00
    RE::BSResource::ID*       id;        // 08
    RE::BSResource::Location* location;  // 10
};

namespace RE::BSResource
{
    class EventSources
    {
    public:
        static EventSources* GetSingleton()
        {
            // DAT_143271d78 (AE 1.6.1170)
            static REL::Relocation<EventSources**> singleton{ REL::Offset(0x3271d78) };
            return *singleton;
        }

        BSTEventSource<BSResourceFileFoundEvent>* GetFileFoundEventSource()
        {
            return reinterpret_cast<BSTEventSource<BSResourceFileFoundEvent>*>(reinterpret_cast<uintptr_t>(this) + 0x10);
        }
    };
}

inline static REL::Relocation<const char*(*)(const char* a_name, const char* a_prefix, void* a_lookupTable)> StripPrefix{ REL::Offset(0xd0b030) };
inline static REL::Relocation<void(*)(void* a_mapRoot, RE::BSResource::ID* a_id, RE::BSTSmartPointer<RE::BSResource::Stream>* a_stream)> InsertNoCache{ REL::Offset(0xd03a10) };
inline static REL::Relocation<void(*)(void* a_cacheNode, void* a_mapRoot, RE::BSResource::ID* a_id, RE::BSTSmartPointer<RE::BSResource::Stream>* a_stream)> InsertWithCache{ REL::Offset(0xd08d70) };
inline static REL::Relocation<void(*)(void* a_globalState)> UpdateCache{ REL::Offset(0xd0d890) };

inline static void *const *const CACHE_PTR = reinterpret_cast<void*const *const>(REL::Offset(0x3271d80).address());