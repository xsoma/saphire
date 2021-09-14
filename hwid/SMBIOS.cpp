
#include "SMBIOS.h"

void DumpSMBIOSStruct(void* Addr, UINT Len, wchar_t* outData) {
	LPBYTE p = (LPBYTE)(Addr);
	const DWORD lastAddress = ((DWORD)p) + Len;
	PSMBIOSHEADER pHeader;
	pHeader = (PSMBIOSHEADER)p;
	PBIOSInfo pBIOS = (PBIOSInfo)p;
	const char* str = toPointString(p);
	wchar_t* biosVendor = (wchar_t*)LocateStringW(str, pBIOS->Vendor);
	wcscpy(outData, biosVendor);
	wchar_t* biosVersion = (wchar_t*)LocateStringW(str, pBIOS->Version);
	wcscpy(outData, (L" v."));
	wcscpy(outData, biosVersion);
}
const char* toPointString(void* p) {
	return (char*)p + ((PSMBIOSHEADER)p)->Length;
}
const wchar_t* LocateStringW(const char* str, UINT i) {
	static wchar_t buff[2048];
	const char* pStr = LocateStringA(str, i);
	SecureZeroMemory(buff, sizeof(buff));
	MultiByteToWideChar(CP_UTF8, 0, pStr, strlen(pStr), buff, sizeof(buff));
	return buff;
}
const char* LocateStringA(const char* str, UINT i) {
	static const char strNull[] = "Null String";
	if (0 == i || 0 == *str) return strNull;
	while (--i) {
		str += strlen((char*)str) + 1;
	}
	return str;
}