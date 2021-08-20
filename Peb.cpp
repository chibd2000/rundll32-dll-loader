#include "Peb.h"

typedef void(__stdcall *CODE) ();

DWORD _getKernelBase()
{
	DWORD dwPEB;
	DWORD dwLDR;
	DWORD dwInitList;
	DWORD dwDllBase;//当前地址
	PIMAGE_DOS_HEADER pImageDosHeader;//指向DOS头的指针
	PIMAGE_NT_HEADERS pImageNtHeaders;//指向NT头的指针
	DWORD dwVirtualAddress;//导出表偏移地址
	PIMAGE_EXPORT_DIRECTORY pImageExportDirectory;//指向导出表的指针
	PCHAR lpName;								  //指向dll名字的指针
	CHAR szKernel32[] = "KERNEL32.dll";
	__asm
	{
		mov eax, FS: [0x30]//fs:[0x30]获取PEB所在地址
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		mov dwPEB, eax // eax 复制给dwPEB
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
	}
	dwLDR = *(PDWORD)(dwPEB + 0xc);//获取PEB_LDR_DATA 结构指针
	dwInitList = *(PDWORD)(dwLDR + 0x1c);//获取InInitializationOrderModuleList  链表头指针
	//第一个LDR_MODULE节点InInitializationOrderModuleList成员的指针
	for (;
		dwDllBase = *(PDWORD)(dwInitList + 8);//结构偏移0x8处存放模块基址
		dwInitList = *(PDWORD)dwInitList//结构偏移0处存放下一模块结构的指针
		)
	{
		pImageDosHeader = (PIMAGE_DOS_HEADER)dwDllBase;
		pImageNtHeaders = (PIMAGE_NT_HEADERS)(dwDllBase + pImageDosHeader->e_lfanew);
		dwVirtualAddress = pImageNtHeaders->OptionalHeader.DataDirectory[0].VirtualAddress;//导出表偏移
		pImageExportDirectory = (PIMAGE_EXPORT_DIRECTORY)(dwDllBase + dwVirtualAddress);//导出表地址
		lpName = (PCHAR)(dwDllBase + pImageExportDirectory->Name);//dll名字
		if (strlen(lpName) == 0xc && !strcmp(lpName, szKernel32))//判断是否为“KERNEL32.dll”
		{
			return dwDllBase;
		}
	}
	return 0;
}

DWORD _getApi(DWORD _hModule, PCHAR _lpApi) 
{
	DWORD i;
	DWORD dwLen;
	PIMAGE_DOS_HEADER pImageDosHeader;//指向DOS头的指针
	PIMAGE_NT_HEADERS pImageNtHeaders;//指向NT头的指针
	DWORD dwVirtualAddress;//导出表偏移地址
	PIMAGE_EXPORT_DIRECTORY pImageExportDirectory;//指向导出表的指针
	CHAR** lpAddressOfNames;
	PWORD lpAddressOfNameOrdinals;//计算API字符串的长度
	for (i = 0; _lpApi[i]; ++i);
		dwLen = i;
	pImageDosHeader = (PIMAGE_DOS_HEADER)_hModule;
	pImageNtHeaders = (PIMAGE_NT_HEADERS)(_hModule + pImageDosHeader->e_lfanew);
	dwVirtualAddress = pImageNtHeaders->OptionalHeader.DataDirectory[0].VirtualAddress;     //导出表偏移
	pImageExportDirectory = (PIMAGE_EXPORT_DIRECTORY)(_hModule + dwVirtualAddress);         //导出表地址
	lpAddressOfNames = (PCHAR*)(_hModule + pImageExportDirectory->AddressOfNames);         //按名字导出函数列表

	//遍历导出表的函数名来进行判断，然后返回指定函数名的函数地址
	for (i = 0; _hModule + lpAddressOfNames[i]; i++)
	{
		if (strlen(_hModule + lpAddressOfNames[i]) == dwLen && !strcmp(_hModule + lpAddressOfNames[i], _lpApi))//判断是否为_lpApi
		{
			lpAddressOfNameOrdinals = (PWORD)(_hModule + pImageExportDirectory->AddressOfNameOrdinals);//按名字导出函数索引列表
			return _hModule + ((PDWORD)(_hModule + pImageExportDirectory->AddressOfFunctions))[lpAddressOfNameOrdinals[i]];//根据函数索引找到函数地址
		}
	}
	return 0;
	
}