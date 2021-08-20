#include "Peb.h"

typedef void(__stdcall *CODE) ();

DWORD _getKernelBase()
{
	DWORD dwPEB;
	DWORD dwLDR;
	DWORD dwInitList;
	DWORD dwDllBase;//��ǰ��ַ
	PIMAGE_DOS_HEADER pImageDosHeader;//ָ��DOSͷ��ָ��
	PIMAGE_NT_HEADERS pImageNtHeaders;//ָ��NTͷ��ָ��
	DWORD dwVirtualAddress;//������ƫ�Ƶ�ַ
	PIMAGE_EXPORT_DIRECTORY pImageExportDirectory;//ָ�򵼳����ָ��
	PCHAR lpName;								  //ָ��dll���ֵ�ָ��
	CHAR szKernel32[] = "KERNEL32.dll";
	__asm
	{
		mov eax, FS: [0x30]//fs:[0x30]��ȡPEB���ڵ�ַ
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		mov dwPEB, eax // eax ���Ƹ�dwPEB
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
	}
	dwLDR = *(PDWORD)(dwPEB + 0xc);//��ȡPEB_LDR_DATA �ṹָ��
	dwInitList = *(PDWORD)(dwLDR + 0x1c);//��ȡInInitializationOrderModuleList  ����ͷָ��
	//��һ��LDR_MODULE�ڵ�InInitializationOrderModuleList��Ա��ָ��
	for (;
		dwDllBase = *(PDWORD)(dwInitList + 8);//�ṹƫ��0x8�����ģ���ַ
		dwInitList = *(PDWORD)dwInitList//�ṹƫ��0�������һģ��ṹ��ָ��
		)
	{
		pImageDosHeader = (PIMAGE_DOS_HEADER)dwDllBase;
		pImageNtHeaders = (PIMAGE_NT_HEADERS)(dwDllBase + pImageDosHeader->e_lfanew);
		dwVirtualAddress = pImageNtHeaders->OptionalHeader.DataDirectory[0].VirtualAddress;//������ƫ��
		pImageExportDirectory = (PIMAGE_EXPORT_DIRECTORY)(dwDllBase + dwVirtualAddress);//�������ַ
		lpName = (PCHAR)(dwDllBase + pImageExportDirectory->Name);//dll����
		if (strlen(lpName) == 0xc && !strcmp(lpName, szKernel32))//�ж��Ƿ�Ϊ��KERNEL32.dll��
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
	PIMAGE_DOS_HEADER pImageDosHeader;//ָ��DOSͷ��ָ��
	PIMAGE_NT_HEADERS pImageNtHeaders;//ָ��NTͷ��ָ��
	DWORD dwVirtualAddress;//������ƫ�Ƶ�ַ
	PIMAGE_EXPORT_DIRECTORY pImageExportDirectory;//ָ�򵼳����ָ��
	CHAR** lpAddressOfNames;
	PWORD lpAddressOfNameOrdinals;//����API�ַ����ĳ���
	for (i = 0; _lpApi[i]; ++i);
		dwLen = i;
	pImageDosHeader = (PIMAGE_DOS_HEADER)_hModule;
	pImageNtHeaders = (PIMAGE_NT_HEADERS)(_hModule + pImageDosHeader->e_lfanew);
	dwVirtualAddress = pImageNtHeaders->OptionalHeader.DataDirectory[0].VirtualAddress;     //������ƫ��
	pImageExportDirectory = (PIMAGE_EXPORT_DIRECTORY)(_hModule + dwVirtualAddress);         //�������ַ
	lpAddressOfNames = (PCHAR*)(_hModule + pImageExportDirectory->AddressOfNames);         //�����ֵ��������б�

	//����������ĺ������������жϣ�Ȼ�󷵻�ָ���������ĺ�����ַ
	for (i = 0; _hModule + lpAddressOfNames[i]; i++)
	{
		if (strlen(_hModule + lpAddressOfNames[i]) == dwLen && !strcmp(_hModule + lpAddressOfNames[i], _lpApi))//�ж��Ƿ�Ϊ_lpApi
		{
			lpAddressOfNameOrdinals = (PWORD)(_hModule + pImageExportDirectory->AddressOfNameOrdinals);//�����ֵ������������б�
			return _hModule + ((PDWORD)(_hModule + pImageExportDirectory->AddressOfFunctions))[lpAddressOfNameOrdinals[i]];//���ݺ��������ҵ�������ַ
		}
	}
	return 0;
	
}