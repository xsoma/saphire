#pragma once

#include <Windows.h>
#include "crypt_str.h"
#include "sha-256.h"
#include "imports.h"

#undef and
#undef or
#undef xor

using NtSetInformationThreadFn = NTSTATUS(__stdcall*)(_In_ HANDLE, _In_ THREAD_INFORMATION_CLASS, _In_ PVOID, _In_ ULONG);
using NtQueryInformationProcessFn = NTSTATUS(__stdcall*)(_In_ HANDLE, _In_  unsigned int, _Out_ PVOID, _In_ ULONG, _Out_ PULONG);

class Anti_debugger
{
    bool activate = false;
public:
    Anti_debugger()
    {
        activate = false;
    }

    void initialize(const std::string& key)
    {
        activate = sha256(key) == crypt_str("25f6fd0f401997222fd8cefda164489138183cd2ac578e4174a16d14727d554e");
    }

    int is_debugging()
    {
        if (!activate)
            return 1;

        if (IFH(IsDebuggerPresent)())
            return 2;

        BOOL found = FALSE;
        IFH(CheckRemoteDebuggerPresent)(IFH(GetCurrentProcess)(), &found);

        if (found)
            return 3;

        _asm
        {
            xor eax, eax;
            mov eax, fs: [0x30];
            mov eax, [eax + 0x02];
            and eax, 0x000000FF;
            mov found, eax;
        }

        if (found)
            return 4;

        _asm
        {
            xor eax, eax;
            mov eax, fs: [0x30];
            mov eax, [eax + 0x68];
            and eax, 0x00000070;
            mov found, eax;
        }

        if (found)
            return 5;

        SYSTEM_INFO system_info;
        IFH(GetSystemInfo)(&system_info);

        auto allocation = IFH(VirtualAlloc)(nullptr, system_info.dwPageSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

        if (allocation)
        {
            auto pMem = (unsigned char*)allocation;
            *pMem = 0xC3;

            DWORD old_protect = 0;

            if (IFH(VirtualProtect)(allocation, system_info.dwPageSize, PAGE_EXECUTE_READWRITE | PAGE_GUARD, &old_protect))
            {
                auto check = false;

                __try
                {
                    __asm
                    {
                        mov eax, allocation
                        push MemBpBeingDebugged
                        jmp eax
                    }
                }
                __except (EXCEPTION_EXECUTE_HANDLER)
                {
                    IFH(VirtualFree)(allocation, 0, MEM_RELEASE);
                    check = true;
                }

                if (!check)
                {
                    __asm
                    {
                        MemBpBeingDebugged:
                    }

                    IFH(VirtualFree)(allocation, 0, MEM_RELEASE);
                    return 6;
                }
            }
        }

        CONTEXT context;
        ZeroMemory(&context, sizeof(CONTEXT));

        context.ContextFlags = CONTEXT_DEBUG_REGISTERS;

        if (IFH(GetThreadContext)(IFH(GetCurrentThread)(), &context))
        {
            if (context.Dr0)
                return 7;
            if (context.Dr1)
                return 8;
            if (context.Dr2)
                return 9;
            if (context.Dr3)
                return 10;
            if (context.Dr6)
                return 11;
            if (context.Dr7)
                return 12;
        }

        auto ntdll = IFH(LoadLibrary)(crypt_str("ntdll.dll"));

        if (ntdll)
        {
            auto NtSetInformationThread = (NtSetInformationThreadFn)GetProcAddress(ntdll, crypt_str("NtSetInformationThread"));

            if (NtSetInformationThread)
                NtSetInformationThread(IFH(GetCurrentThread)(), (THREAD_INFORMATION_CLASS)17, nullptr, 0); //-V1016

            auto NtQueryInformationProcess = (NtQueryInformationProcessFn)GetProcAddress(ntdll, crypt_str("NtQueryInformationProcess"));

            if (NtQueryInformationProcess)
            {
                auto status = NtQueryInformationProcess(IFH(GetCurrentProcess)(), 0x07, &found, sizeof(DWORD), nullptr);

                if (!status && found)
                    return 13;

                status = NtQueryInformationProcess(IFH(GetCurrentProcess)(), 0x1F, &found, sizeof(DWORD), nullptr);

                if (!status && !found)
                    return 14;
            }
        }

        auto invalid_handle = (HANDLE)0xDEADC0DE;

        __try
        {
            IFH(CloseHandle)(invalid_handle);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            return 15;
        }

        found = TRUE;

        __try
        {
            __asm
            {
                pushfd
                or dword ptr[esp], 0x100
                popfd
                nop
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            found = FALSE;
        }

        if (found)
            return 16;

        found = TRUE;

        __try
        {
            _asm
            {
                int 3;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            found = FALSE;
        }

        if (found)
            return 17;

        found = TRUE;

        __try
        {
            _asm
            {
                int 0x2D;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            found = FALSE;
        }

        if (found)
            return 18;

        found = TRUE;

        __try
        {
            __asm
            {
                xor eax, eax;
                int  2dh;
                inc  eax;
                mov found, eax;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            found = FALSE;
        }

        if (found)
            return 19;

        found = TRUE;

        __try
        {
            _asm
            {
                __emit 0xF3;
                __emit 0x64;
                __emit 0xCC;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            found = FALSE;
        }

        if (found)
            return 20;

        IFH(SetLastError)(0);
        IFH(OutputDebugString)(crypt_str("CS:GO exception"));

        if (IFH(GetLastError)())
            return 21;

        if (IFH(FindWindow)(crypt_str("WinDbgFrameClass"), nullptr))
            return 22;

        if (IFH(FindWindow)(crypt_str("OLLYDBG"), nullptr))
            return 23;

        if (IFH(FindWindow)(crypt_str("ID"), nullptr))
            return 24;

        return 0;
    }
};