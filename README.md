## RetroHook
RetroHook is an x86/x64, minimalistic header only library for placing hooks on virtual-functions. Ease and stealth is a priority, and future updates are inevitable. The library doesn't change any data of the address, bypassing any data checks, and also doesn't require `VirtualProtect` to be called on the address. RetroHook was compiled with Microsoft Visual Studio 2022, but should be no problem building on any other compiler.

## Requirements
- Default C++ Language Standard.
- Default C Language Standard.

## x86 Use
Usage for x86 (MSVC):
```#include <iostream>
#include <Windows.h>

#include "RetroHook.h"

class GameClass
{
public:
    virtual void DamagePlayer(int forAmount)
    {
        std::cout << "Damaged for " << forAmount << std::endl;
    }
};

using DamagePlayer_t = void(__stdcall*)(int);
DamagePlayer_t oDamagePlayer = nullptr;

void __stdcall DamagePlayerHk(int forAmount)
{
    return oDamagePlayer(forAmount);
}

int main()
{
    GameClass* gameClass = new GameClass();

    // Create new VMT instance.
    RetroHook* vmt1 = new RetroHook(gameClass, 0, DamagePlayerHk);
    
    // Set hook originals.
    oDamagePlayer = (DamagePlayer_t)RetroHook_Util::GetVirtualAddr(gameClass, 0);

    // Set hook.
    if (vmt1->SetHook())
    {
        printf("[+] Hooked\n");
        gameClass->DamagePlayer(123);

        printf("[+] 0x%p\n", vmt1->GetHookedAddr());
    }
    else
    {
        printf("[-] Failed to hook\n");
    }

    std::cin.get();
    return 0;
}```
