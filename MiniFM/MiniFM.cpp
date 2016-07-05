// MiniFM.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"

// Ϊ����ͨ�������õľ���
#if _MSC_VER >= 1200
#pragma warning(push)
// ΢��SDK���ڵľ���
#pragma warning(disable:4820) // �ֽ������������ݳ�Ա��(�ȼ� 4)
#endif

// IFileDialog/IFileOpenDialog/IFileSaveDialog
#include <shobjidl.h>  

#if _MSC_VER >= 1200
#pragma warning(pop)
#endif

#ifndef _DEBUG
#pragma comment(linker, "/ENTRY:EntryPoint") 
#endif 

// Ϊ����ͨ�������õľ���
#if _MSC_VER >= 1200
#pragma warning(push)
#pragma warning(disable:4191) // �ӡ�type of expression������type required���Ĳ���ȫת��(�ȼ� 3)
#endif

// �����Ի���Per-Monitor DPI Aware֧�֣�Win10���ã�
inline int EnablePerMonitorDialogScaling()
{
	typedef int(WINAPI *PFN_EnablePerMonitorDialogScaling)();

	PFN_EnablePerMonitorDialogScaling pEnablePerMonitorDialogScaling =
		(PFN_EnablePerMonitorDialogScaling)GetProcAddress(
			GetModuleHandleW(L"user32.dll"), (LPCSTR)2577);

	if (pEnablePerMonitorDialogScaling) return pEnablePerMonitorDialogScaling();
	return -1;
}

#if _MSC_VER >= 1200
#pragma warning(pop)
#endif

void EntryPoint()
{
	int uExitCode = wWinMain(
		GetModuleHandleW(nullptr),
		nullptr,
		nullptr,
		SW_SHOWNORMAL);
	
	ExitProcess((UINT)uExitCode);
}

int WINAPI wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nShowCmd)
{
	IIntendToIgnoreThisVariable(hInstance);
	IIntendToIgnoreThisVariable(hPrevInstance);
	IIntendToIgnoreThisVariable(lpCmdLine);
	IIntendToIgnoreThisVariable(nShowCmd);

	EnablePerMonitorDialogScaling();
	
	HRESULT hr = S_OK;
	IFileDialog *pFileDialog = nullptr;
	IShellItem *pShellItemFolder = nullptr;
	wchar_t g_AppPath[260];

	// ��ʼ��COM
	hr = CoInitializeEx(
		nullptr,
		COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (FAILED(hr)) goto FuncEnd;

	// ����IFileDialog�ӿڶ���
	hr = CoCreateInstance(
		CLSID_FileOpenDialog,
		nullptr,
		CLSCTX_INPROC_SERVER,
		__uuidof(IFileDialog),
		(void**)&pFileDialog);
	if (FAILED(hr)) goto FuncEnd;

	// ����ѡ��
	hr = pFileDialog->SetOptions(
		FOS_FORCEFILESYSTEM | FOS_ALLOWMULTISELECT | FOS_FORCESHOWHIDDEN);
	if (FAILED(hr)) goto FuncEnd;

	// ��ȡ��ǰ·��
	GetModuleFileNameW(nullptr, g_AppPath, 260);
	wcsrchr(g_AppPath, L'\\')[0] = L'\0';

	// ��ȡ��ǰ·����IShellItem�ӿڶ���
	hr = SHCreateItemFromParsingName(
		g_AppPath,
		nullptr,
		__uuidof(IShellItem),
		(void**)&pShellItemFolder);
	if (FAILED(hr)) goto FuncEnd;

	// ����·��
	pFileDialog->SetFolder(pShellItemFolder);
	if (FAILED(hr)) goto FuncEnd;

	// ��ʾ����
	hr = pFileDialog->Show(nullptr);

FuncEnd:
	if (pShellItemFolder) pShellItemFolder->Release();
	if (pFileDialog) pFileDialog->Release();
	CoUninitialize();

	return 0;
}
