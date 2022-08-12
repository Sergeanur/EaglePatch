# patcher
a lightweight win32 patcher that can also hook constructors, destructors and virtual functions

largely adapted from Silent's MemoryMgr.h.

Just include "patcher.h" and compile "patcher.cpp" into your dll/exe to use it. patcher.h does not include Windows.h for faster compilation speeds, but only works on Win32 anyway.

## info

### StaticPatcher

at the bottom of a source file, use the `STARTPATCHES` and `ENDPATCHES` macros to include a list of functions between them that is to be executed automatically a exe/dll startup. Handy for InjectHook, Patch etc.

Usage example:

    STARTPATCHES
      InjectHook(addr, &struct::func);
    ENDPATCHES

### macros

There are some macros on the top of the file. They should be fairly self explanatory, but some should be futher expanded upon.

`ASM(function_name)` will create a naked function which can include inline assembly, so you can inject a jmp to it. You can use `RET(addr)` to return to the proper location.

There are two ways to call a function from the original binary with this patcher. Either declare it `WRAPPER prototype { EAXJMP(addr); }`, which messes up the actual function declaration, or use the `XCALL` macro as such: `prototype { XCALL(addr); }`. These both work for any calling convention, and inside classes.

### functions

`Patch(0xBAADF00D, "string");` patches a string at the memory location. Works with all types, beware that you need to cast uchars and shorts, otherwise it will overwrite 4 bytes.

`PatchBytes` and `ReadBytes` should be self explanatory, you can patch an array of bytes at a specific location.

`SetBytes` fills bytes at a location. `Nop` nops count bytes at the location. `NopTo` nops a range of bytes (addr to addr).

`InjectHook` injects a hook to a function to the specified location. Either by keeping the original operation (call or jmp) in place (`PATCH_EXISTING`), or by replacing the bytes with a call (`PATCH_CALL`) or jmp (`PATCH_JUMP`) instruction. You can take function pointers of most class methods, but if there are overloaded functions, you'll need to cast them rather horribly. Check PatcherTest.cpp for an example.

`PatchJump` adds a jump from addr 1 to addr2.

`ExtractCall` extracts the actual call addr, see `InterceptCall` for example.

`InterceptVmethod` places your own function on a vtable and gets you the original function.

### constructors, destructors and virtual functions

And now the most disgusting thing I have ever written:

You know how you can't use the address of your own Constructor/Destructor/Virtual Functions to place them into the memory of the exe you're injecting to? Well, now you can.

Instead of me explaining it all here complicatedly, just check Patcher.cpp for `InjectHook_Constructor_Init` and the corresponding `InjectHook_Constructor`, `InjectHook_Destructor_Init` its corresponding `InjectHook_Destructor`, and the same for `InjectHook_VirtualMethod_Init` and `InjectHook_VirtualMethod`. Even works for overloaded functions.

Use the `HOOK_CALL` define to place a call at the address, and `HOOK_JUMP` to place a jump at the address.

You're welcome.
