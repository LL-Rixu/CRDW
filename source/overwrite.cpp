#include <overwrite.h>
#include <VersionHelpers.h>

void overwrite::init()
{
    if(!IsWindows8OrGreater()) { return; }

    HMODULE hKernel32 = GetModuleHandleW(L"kernel32.dll");
    PrefetchVirtualMemory = reinterpret_cast<BOOL (*)(HANDLE, ULONG_PTR, PWIN32_MEMORY_RANGE_ENTRY, ULONG)>(GetProcAddress(hKernel32, "PrefetchVirtualMemory"));
}