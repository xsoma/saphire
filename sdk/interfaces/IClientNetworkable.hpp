#pragma once

class IClientUnknown;
class ClientClass;
class bf_read;

class IClientNetworkable
{
public:
    virtual IClientUnknown*  GetIClientUnknown() = 0;
    virtual void             Release() = 0;
    virtual ClientClass*     GetClientClass() = 0;
    virtual void             NotifyShouldTransmit(int state) = 0;
    virtual void             OnPreDataChanged(int updateType) = 0;
    virtual void             OnDataChanged(int updateType) = 0;
    virtual void             PreDataUpdate(int updateType) = 0;
    virtual void             PostDataUpdate(int updateType) = 0;
    virtual void             __unkn(void) = 0;
    virtual bool             IsDormant(void) = 0;
    virtual int              EntIndex(void) const = 0;
    virtual void             ReceiveMessage(int classID, bf_read& msg) = 0;
    virtual void*            GetDataTableBasePtr() = 0;
    virtual void             SetDestroyedOnRecreateEntities(void) = 0;
};

// classes, definitions
#include <cstddef>

namespace VirtualMethod
{
    template <typename T, std::size_t Idx, typename ...Args>
    constexpr auto call(void* classBase, Args... args) noexcept
    {
        return ((*reinterpret_cast<T(__thiscall***)(void*, Args...)>(classBase))[Idx])(classBase, args...);
    }
}

#define VIRTUAL_METHOD(returnType, name, idx, args, argsRaw) \
constexpr auto name args noexcept \
{ \
    return VirtualMethod::call<returnType, idx>argsRaw; \
}

class NetworkStringTable {
public:
    VIRTUAL_METHOD(int, addString, 8, (bool isServer, const char* value, int length = -1, const void* userdata = nullptr), (this, isServer, value, length, userdata))
};

class NetworkStringTableContainer {
public:
    VIRTUAL_METHOD(NetworkStringTable*, findTable, 3, (const char* name), (this, name))
};