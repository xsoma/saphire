// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "Memory.h"
#include "..\utils\crypt_str.h"

template <typename T>
static constexpr auto relativeToAbsolute(int* address) noexcept
{
    return reinterpret_cast<T>(reinterpret_cast<char*>(address + 1) + *address);
}

#define FIND_PATTERN(type, ...) \
reinterpret_cast<type>(findPattern(__VA_ARGS__))

void Memory::initialize() noexcept
{
	auto temp = FIND_PATTERN(std::uintptr_t*, crypt_str("client"), crypt_str("\xB9????\xE8????\x8B\x5D\x08"), 1);
	hud = *temp;
	findHudElement = relativeToAbsolute<decltype(findHudElement)>(reinterpret_cast<int*>(reinterpret_cast<char*>(temp) + 5));
	clearHudWeapon = FIND_PATTERN(decltype(clearHudWeapon), crypt_str("client"), crypt_str("\x55\x8B\xEC\x51\x53\x56\x8B\x75\x08\x8B\xD9\x57\x6B\xFE\x2C"));
	itemSchema = relativeToAbsolute<decltype(itemSchema)>(FIND_PATTERN(int*, crypt_str("client"), crypt_str("\xE8????\x0F\xB7\x0F"), 1));
	equipWearable = FIND_PATTERN(decltype(equipWearable), crypt_str("client"), crypt_str("\x55\x8B\xEC\x83\xEC\x10\x53\x8B\x5D\x08\x57\x8B\xF9"));
	//keyValuesFromString = relativeToAbsolute<decltype(keyValuesFromString)>(reinterpret_cast<int*>(reinterpret_cast<char*>(findPattern("client", "\xE8????\x83\xC4\x04\x89\x45\xD8")) + 1));
	//keyValuesFindKey = relativeToAbsolute<decltype(keyValuesFindKey)>(reinterpret_cast<int*>(reinterpret_cast<char*>(findPattern("client", "\xE8????\xF7\x45")) + 1));
	//keyValuesSetString = relativeToAbsolute<decltype(keyValuesSetString)>(reinterpret_cast<int*>(reinterpret_cast<char*>(findPattern("client", "\xE8????\x89\x77\x38")) + 1));
}