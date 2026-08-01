#include <crdw.h>
#include <EventSource.h>

void CRDW::ProcessName(RE::BSResource::Traverser* a_this, const char* path, RE::BSResource::Location& location)
{
    const char* prefix = reinterpret_cast<const char*>(a_this->unkC8);
    void* lookup = reinterpret_cast<void*>(a_this->unkE8);

    const char* stripped = StripPrefix(path, prefix, lookup);

    RE::BSResource::ID id;
    id.GenerateFromPath(stripped);

    RE::BSResource::ID* lastID = reinterpret_cast<RE::BSResource::ID*>(&a_this->unkD0);

    if(lastID->file != id.file)
    {
        a_this->unkE0 = 0;
        *lastID = id;
        UpdateCache(*CACHE_PTR, id.file, &a_this->unkE0);
    }

    RE::BSTSmartPointer<RE::BSResource::Stream> stream;
    RE::BSResource::Location* locationOut = nullptr;

    if(location.DoCreateStream(path, stream, locationOut, false) == RE::BSResource::ErrorCode::kNone)
    {
        void* map = &a_this->unk08;
        
        if (a_this->unkE0 == 0) 
        {
            InsertNoCache(map, &id, &stream);
        }
        else
        {
            void* node = reinterpret_cast<void*>(a_this->unkE0);
            InsertWithCache(node, map, &id, &stream);
        }
    }
    else { Log(spdlog::level::warn, "DoCreateStream failed {}", path); }

    BSResourceFileFoundEvent event{ stripped, &id, &location };

    auto* eventSources = RE::BSResource::EventSources::GetSingleton();
    auto* source = eventSources->GetFileFoundEventSource();
    source->SendEvent(&event);
}