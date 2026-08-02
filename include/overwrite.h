#pragma once

#include <Windows.h>

namespace overwrite
{
    inline BOOL (__stdcall *PrefetchVirtualMemory)(HANDLE hProcess, ULONG_PTR NumberOfEntries, PWIN32_MEMORY_RANGE_ENTRY VirtualAddresses, ULONG Flags) = nullptr;

    void init();
}
