#include <RE/Skyrim.h>
#include <ID.h>

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
            static REL::Relocation<EventSources**> singleton{ REL::ID(EventSourcesID) }; 
            return *singleton;
        }

        BSTEventSource<BSResourceFileFoundEvent>* GetFileFoundEventSource()
        {
            return reinterpret_cast<BSTEventSource<BSResourceFileFoundEvent>*>(reinterpret_cast<uintptr_t>(this) + 0x10);
        }
    };
}

inline static REL::Relocation<const char*(*)(const char* a_name, const char* a_prefix, void* a_lookupTable)> StripPrefix{ REL::ID(StripPrefixID) }; 
inline static REL::Relocation<void(*)(void* a_mapRoot, RE::BSResource::ID* a_id, RE::BSTSmartPointer<RE::BSResource::Stream>* a_stream)> InsertNoCache{ REL::ID(InsertNoCacheID) };
inline static REL::Relocation<void(*)(void* a_cacheNode, void* a_mapRoot, RE::BSResource::ID* a_id, RE::BSTSmartPointer<RE::BSResource::Stream>* a_stream)> InsertWithCache{ REL::ID(InsertWithCacheID) };
inline static REL::Relocation<void(*)(void* a_globalState)> UpdateCache{ REL::ID(UpdateCacheID) };

inline static void *const *const CACHE_PTR = reinterpret_cast<void*const *const>(REL::ID(CacheID).address());