#include "stdafx.h"

#include "M2.Base.hpp"

#include "M2.NSudoLib.h"

// Ϊ����ͨ�������õľ���
#if _MSC_VER >= 1200
#pragma warning(push)
#pragma warning(disable:4820) // �ֽ������������ݳ�Ա��(�ȼ� 4)
#endif

#include <WtsApi32.h>
#pragma comment(lib,"WtsApi32.lib")

#if _MSC_VER >= 1200
#pragma warning(pop)
#endif

namespace M2
{
	namespace NSudo
	{
		/*
		NSudo���������

		// ��ֹNSudo��ʹ�û���δ�ĵ���Windows API
		//#define NSUDO_DISABLE_USING_UNDOCUMENT_WINDOWS_API

		*/



		/*
		SuDuplicateToken����ͨ�����еķ������ƴ���һ�������ƻ�ģ�����ơ�
		The SuDuplicateToken function creates a primary token or an impersonation 
		token via an existing access token.

		�ú�����DuplicateTokenEx API��һ���ȼ�ʵ�֡�
		This function is an equivalent implementation of DuplicateTokenEx API.
		*/
		HRESULT WINAPI SuDuplicateToken(
			_In_ HANDLE hExistingToken,
			_In_ DWORD dwDesiredAccess,
			_In_opt_ LPSECURITY_ATTRIBUTES lpTokenAttributes,
			_In_ SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
			_In_ TOKEN_TYPE TokenType,
			_Outptr_ PHANDLE phNewToken)
		{
#ifndef NSUDO_DISABLE_USING_UNDOCUMENT_WINDOWS_API
			NTSTATUS status = STATUS_SUCCESS;
			OBJECT_ATTRIBUTES ObjectAttributes;
			SECURITY_QUALITY_OF_SERVICE SQOS;

			M2InitObjectAttributes(
				&ObjectAttributes, nullptr, 0, nullptr, nullptr, &SQOS);
			M2InitSecurityQuailtyOfService(
				&SQOS, ImpersonationLevel, FALSE, FALSE);

			if (lpTokenAttributes &&
				lpTokenAttributes->nLength == sizeof(SECURITY_ATTRIBUTES))
			{
				ObjectAttributes.Attributes =
					(ULONG)(lpTokenAttributes->bInheritHandle ? OBJ_INHERIT : 0);
				ObjectAttributes.SecurityDescriptor =
					lpTokenAttributes->lpSecurityDescriptor;
			}

			// �������ƶ���
			status = NtDuplicateToken(
				hExistingToken,
				dwDesiredAccess,
				&ObjectAttributes,
				FALSE,
				TokenType,
				phNewToken);

			// �������н��
			return __HRESULT_FROM_WIN32(RtlNtStatusToDosError(status));
#else
			// �������ƶ���
			DuplicateTokenEx(
				hExistingToken,
				dwDesiredAccess,
				lpTokenAttributes, 
				ImpersonationLevel, 
				TokenType,
				phNewToken);

			// �������н��
			return __HRESULT_FROM_WIN32(GetLastError());
#endif		
		}

		/*
		SuOpenProcess������һ�����ڵı������̶���
		The SuOpenProcess function opens an existing local process object.

		�ú�����OpenProcess API��һ���ȼ�ʵ�֡�
		This function is an equivalent implementation of OpenProcess API.
		*/
		HRESULT WINAPI SuOpenProcess(
			_Out_ PHANDLE phProcess,
			_In_ DWORD dwDesiredAccess,
			_In_ BOOL bInheritHandle,
			_In_ DWORD dwProcessId)
		{
#ifndef NSUDO_DISABLE_USING_UNDOCUMENT_WINDOWS_API
			NTSTATUS status = STATUS_SUCCESS;
			OBJECT_ATTRIBUTES ObjectAttributes;
			CLIENT_ID ClientID;

			// ������ʼ��

			M2InitObjectAttributes(&ObjectAttributes);
			ObjectAttributes.Attributes = 
				(ULONG)(bInheritHandle ? OBJ_INHERIT : 0);

			M2InitClientID(&ClientID, dwProcessId, 0);
			
			// �򿪽��̶���
			status = NtOpenProcess(
				phProcess, dwDesiredAccess, &ObjectAttributes, &ClientID);
		
			// �������н��
			return __HRESULT_FROM_WIN32(RtlNtStatusToDosError(status));
#else
			// �򿪽��̶���
			*phProcess = OpenProcess(
				dwDesiredAccess,
				bInheritHandle,
				dwProcessId);

			// �������н��
			return __HRESULT_FROM_WIN32(GetLastError());
#endif
		}

		/*
		SuOpenProcessToken������һ�����̵Ĺ������ơ�
		The SuOpenProcessToken function opens the access token associated with a 
		process.

		�ú�����OpenProcessToken API��һ���ȼ�ʵ�֡�
		This function is an equivalent implementation of OpenProcessToken API.
		*/
		HRESULT WINAPI SuOpenProcessToken(
			_In_ HANDLE ProcessHandle,
			_In_ DWORD DesiredAccess,
			_Outptr_ PHANDLE TokenHandle)
		{
#ifndef NSUDO_DISABLE_USING_UNDOCUMENT_WINDOWS_API		
			// �򿪽�������
			NTSTATUS status = NtOpenProcessToken(
				ProcessHandle, DesiredAccess, TokenHandle);

			// �������н��
			return __HRESULT_FROM_WIN32(RtlNtStatusToDosError(status));
#else
			// �򿪽�������
			OpenProcessToken(ProcessHandle, DesiredAccess, TokenHandle);

			// �������н��
			return __HRESULT_FROM_WIN32(GetLastError());
#endif			
		}

		/*
		SuOpenProcessToken�������ݽ���ID��һ�����̵Ĺ������ơ�
		The SuOpenProcessToken function opens the access token associated with a
		process via ProcessID.
		*/
		HRESULT WINAPI SuOpenProcessToken(
			_In_ DWORD dwProcessId,
			_In_ DWORD DesiredAccess,
			_Outptr_ PHANDLE TokenHandle)
		{
#ifndef NSUDO_DISABLE_USING_UNDOCUMENT_WINDOWS_API		
			NTSTATUS status = STATUS_SUCCESS;
			HANDLE hProcess = nullptr;
			OBJECT_ATTRIBUTES ObjectAttributes;
			CLIENT_ID ClientID;

			// ������ʼ��

			M2InitObjectAttributes(&ObjectAttributes);
			M2InitClientID(&ClientID, dwProcessId, 0);

			// �򿪽��̶���
			status = NtOpenProcess(
				&hProcess, MAXIMUM_ALLOWED, &ObjectAttributes, &ClientID);
			if (NT_SUCCESS(status))
			{
				// �򿪽�������
				status = NtOpenProcessToken(
					hProcess, DesiredAccess, TokenHandle);
				
				// �رս��̾��
				NtClose(hProcess);
			}

			// �������н��
			return __HRESULT_FROM_WIN32(RtlNtStatusToDosError(status));
#else
			// �򿪽��̶���
			HANDLE hProcess = OpenProcess(MAXIMUM_ALLOWED, FALSE, dwProcessId);
			if (hProcess != nullptr)
			{
				// �򿪽�������
				OpenProcessToken(ProcessHandle, DesiredAccess, TokenHandle);

				// �رս��̾��
				CloseHandle(hProcess);
			}

			// �������н��
			return __HRESULT_FROM_WIN32(GetLastError());
#endif		
		}

		/*
		SuQuerySessionToken���������ѵ�½���û��ĻỰID��ȡ���������ơ�����Ҫ��
		LocalSystem�˻��ҿ���SE_TCB_NAME��Ȩ�ķ��������������µ��øú�����
		The SuQuerySessionToken function obtains the primary access token of 
		the logged-on user specified by the session ID. To call this function 
		successfully, the calling application must be running within the 
		context of the LocalSystem account and have the SE_TCB_NAME privilege.

		�ú�����WTSQueryUserToken API��һ���ȼ�ʵ�֡�
		This function is an equivalent implementation of WTSQueryUserToken API.
		*/
		HRESULT WINAPI SuQuerySessionToken(
			_In_ ULONG SessionId,
			_Out_ PHANDLE phToken)
		{
#ifndef NSUDO_DISABLE_USING_UNDOCUMENT_WINDOWS_API		
			WINSTATIONUSERTOKEN WSUT = { 0 };
			DWORD ReturnLength = 0;

			// ��ȡ�Ự����
			WinStationQueryInformationW(
				SERVERNAME_CURRENT,
				SessionId,
				WinStationUserToken,
				&WSUT,
				sizeof(WINSTATIONUSERTOKEN),
				&ReturnLength);
			*phToken = WSUT.UserToken;

			// �������н��
			return __HRESULT_FROM_WIN32(M2GetLastError());
#else
			// ��ȡ�Ự����
			WTSQueryUserToken(SessionId, phToken);

			// �������н��
			return __HRESULT_FROM_WIN32(GetLastError());
#endif			
		}

		/*
		SuOpenCurrentProcessToken�����򿪵�ǰ���̵Ĺ������ơ�
		The SuQueryCurrentProcessToken function opens the access token 
		associated with current process.
		*/
		HRESULT WINAPI SuOpenCurrentProcessToken(
			_In_ DWORD DesiredAccess,
			_Outptr_ PHANDLE TokenHandle)
		{
#ifndef NSUDO_DISABLE_USING_UNDOCUMENT_WINDOWS_API		
			// �򿪽�������
			return SuOpenProcessToken(
				NtCurrentProcess(), DesiredAccess, TokenHandle);
#else
			// �򿪽�������
			return SuOpenProcessToken(
				GetCurrentProcess(), DesiredAccess, TokenHandle);
#endif			
		}


	}
	
	// ��������
	NTSTATUS SuDuplicateToken(
		_Out_ PHANDLE phNewToken,
		_In_ HANDLE hExistingToken,
		_In_ DWORD dwDesiredAccess,
		_In_opt_ LPSECURITY_ATTRIBUTES lpTokenAttributes,
		_In_ SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
		_In_ TOKEN_TYPE TokenType)
	{
		// ��������

		OBJECT_ATTRIBUTES ObjAttr;
		SECURITY_QUALITY_OF_SERVICE SQOS;

		// ������ʼ��

		M2InitObjectAttributes(
			&ObjAttr, nullptr, 0, nullptr, nullptr, &SQOS);
		M2InitSecurityQuailtyOfService(
			&SQOS, ImpersonationLevel, FALSE, FALSE);

		if (lpTokenAttributes &&
			lpTokenAttributes->nLength == sizeof(SECURITY_ATTRIBUTES))
		{
			ObjAttr.Attributes =
				(ULONG)(lpTokenAttributes->bInheritHandle ? OBJ_INHERIT : 0);
			ObjAttr.SecurityDescriptor =
				lpTokenAttributes->lpSecurityDescriptor;
		}

		// �������ƶ��󲢷������н��
		return NtDuplicateToken(
			hExistingToken,
			dwDesiredAccess,
			&ObjAttr,
			FALSE,
			TokenType,
			phNewToken);	
	}

	// ͨ������ID��ȡ���̾��
	NTSTATUS SuOpenProcess(
		_Out_ PHANDLE phProcess,
		_In_ DWORD dwProcessID,
		_In_ DWORD DesiredAccess)
	{
		// ��������

		OBJECT_ATTRIBUTES ObjAttr;
		CLIENT_ID ClientID;

		// ������ʼ��

		M2InitObjectAttributes(&ObjAttr);
		M2InitClientID(&ClientID, dwProcessID, 0);

		// ���ݽ���ID��ȡ���̾�����������н��
		return NtOpenProcess(
			phProcess, DesiredAccess, &ObjAttr, &ClientID);
	}

	// ͨ������ID��ȡ�������ƾ��
	NTSTATUS SuQueryProcessToken(
		_Out_ PHANDLE phProcessToken,
		_In_ DWORD dwProcessID,
		_In_ DWORD DesiredAccess)
	{
		// ��������	

		NTSTATUS status = STATUS_SUCCESS;
		HANDLE hProcess = nullptr;

		do
		{
			// ���ݽ���ID��ȡ���̾��
			status = SuOpenProcess(&hProcess, dwProcessID);
			if (!NT_SUCCESS(status)) break;

			// ���ݽ��̾����ȡ�������ƾ��
			status = NtOpenProcessToken(
				hProcess, DesiredAccess, phProcessToken);

		} while (false);

		NtClose(hProcess);

		return status;
	}

	// ͨ���ỰID��ȡ�Ự����
	HRESULT SuQuerySessionToken(
		_Out_ PHANDLE phToken,
		_In_ DWORD dwSessionID)
	{
		// ���弰��ʼ������

		WINSTATIONUSERTOKEN WSUT = { 0 };
		DWORD ccbInfo = 0;

		*phToken = nullptr;

		// ͨ���ỰID��ȡ�Ự���ƣ������ȡʧ���򷵻ش���ֵ
		if (!WinStationQueryInformationW(
			SERVERNAME_CURRENT,
			dwSessionID,
			WinStationUserToken,
			&WSUT,
			sizeof(WINSTATIONUSERTOKEN),
			&ccbInfo))
			return __HRESULT_FROM_WIN32(GetLastError());

		// �����ȡ�ɹ��������÷��صĻỰ���Ʋ��������н��
		*phToken = WSUT.UserToken;
		return S_OK;
	}

	// ��ȡ��ǰ��������
	NTSTATUS SuQueryCurrentProcessToken(
		_Out_ PHANDLE phProcessToken,
		_In_ DWORD DesiredAccess)
	{
		return NtOpenProcessToken(
			NtCurrentProcess(), DesiredAccess, phProcessToken);
	}

	// ����ģ��
	NTSTATUS SuImpersonate(
		_In_ HANDLE hExistingImpersonationToken)
	{
		return NtSetInformationThread(
			NtCurrentThread(),
			ThreadImpersonationToken,
			&hExistingImpersonationToken,
			sizeof(HANDLE));
	}

	// ��������ģ��
	NTSTATUS SuRevertImpersonate()
	{	
		return SuImpersonate(nullptr);
	}

	// ���õ���������Ȩ
	NTSTATUS SuSetTokenPrivilege(
		_In_ HANDLE hExistingToken,
		_In_ TokenPrivilegesList Privilege,
		_In_ bool bEnable)
	{
		// ��������

		TOKEN_PRIVILEGES TP;

		// ������ʼ��

		TP.PrivilegeCount = 1;
		TP.Privileges[0].Luid.LowPart = Privilege;
		TP.Privileges[0].Attributes = (DWORD)(bEnable ? SE_PRIVILEGE_ENABLED : 0);

		// ����������Ȩ�����ؽ��
		return NtAdjustPrivilegesToken(
			hExistingToken, FALSE, &TP, 0, nullptr, nullptr);
	}

	// ��������ȫ����Ȩ
	NTSTATUS SuSetTokenAllPrivileges(
		_In_ HANDLE hExistingToken,
		_In_ DWORD dwAttributes)
	{
		// �������

		NTSTATUS status = STATUS_SUCCESS;
		CPtr<PTOKEN_PRIVILEGES> pTPs;
		DWORD Length = 0;

		do
		{
			// ��ȡ��Ȩ��Ϣ��С
			NtQueryInformationToken(
				hExistingToken,
				TokenPrivileges,
				nullptr,
				0,
				&Length);

			if (!pTPs.Alloc(Length))
			{
				status = STATUS_NO_MEMORY;
				break;
			}

			// ��ȡ��Ȩ��Ϣ
			status = NtQueryInformationToken(
				hExistingToken,
				TokenPrivileges,
				pTPs,
				Length,
				&Length);
			if (!NT_SUCCESS(status)) break;

			// ������Ȩ��Ϣ
			for (DWORD i = 0; i < pTPs->PrivilegeCount; i++)
				pTPs->Privileges[i].Attributes = dwAttributes;

			// ����ȫ����Ȩ
			status = NtAdjustPrivilegesToken(
				hExistingToken, FALSE, pTPs, 0, nullptr, nullptr);

		} while (false);

		return status;
	}

	//�ж��Ƿ�Ϊ��¼SID
	bool SuIsLogonSid(
		_In_ PSID pSid)
	{
		// ��ȡpSid��SID_IDENTIFIER_AUTHORITY�ṹ
		PSID_IDENTIFIER_AUTHORITY pSidAuth = RtlIdentifierAuthoritySid(pSid);

		// ���������SID_IDENTIFIER_AUTHORITY�ṹ���ȣ��򷵻�false
		if (!memcmp(pSidAuth, &SidAuth_NT, SidAuth_Length)) return false;

		// �ж�SID�Ƿ�����Logon SID
		return (*RtlSubAuthorityCountSid(pSid) == SECURITY_LOGON_IDS_RID_COUNT
			&& *RtlSubAuthoritySid(pSid, 0) == SECURITY_LOGON_IDS_RID);
	}
	
	// �����ں˶��������Ա�ǩ
	NTSTATUS SuSetKernelObjectIntegrityLevel(
		_In_ HANDLE Object,
		_In_ IntegrityLevel IL)
	{
		//�������

		const size_t AclLength = 88;
		NTSTATUS status = STATUS_SUCCESS;
		PSID pSID = nullptr;
		PACL pAcl = nullptr;
		SECURITY_DESCRIPTOR SD;
		HANDLE hNewHandle = nullptr;

		// ���ƾ��
		status = NtDuplicateObject(
			NtCurrentProcess(),
			Object,
			NtCurrentProcess(),
			&hNewHandle,
			DIRECTORY_ALL_ACCESS,
			0,
			0);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		//��ʼ��SID
		status = RtlAllocateAndInitializeSid(
			&SIDAuth_IL, 1, IL, 0, 0, 0, 0, 0, 0, 0, &pSID);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		//����ACL�ṹ�ڴ�
		status = M2HeapAlloc(AclLength, pAcl);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		// ����SD
		status = RtlCreateSecurityDescriptor(
			&SD, SECURITY_DESCRIPTOR_REVISION);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		// ����ACL
		status = RtlCreateAcl(pAcl, AclLength, ACL_REVISION);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		// ���������ACE
		status = RtlAddMandatoryAce(
			pAcl, ACL_REVISION, 0, pSID,
			SYSTEM_MANDATORY_LABEL_ACE_TYPE, OBJECT_TYPE_CREATE);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		// ����SACL
		status = RtlSetSaclSecurityDescriptor(&SD, TRUE, pAcl, FALSE);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		// �����ں˶���
		status = NtSetSecurityObject(
			hNewHandle, LABEL_SECURITY_INFORMATION, &SD);

	FuncEnd:
		//�ͷ��ڴ�
		M2HeapFree(pAcl);
		RtlFreeSid(pSID);
		NtClose(hNewHandle);

		return status;
	}

	// �������������Ա�ǩ
	NTSTATUS SuSetTokenIntegrityLevel(
		_In_ HANDLE TokenHandle,
		_In_ IntegrityLevel IL)
	{
		// ��������
		NTSTATUS status = STATUS_SUCCESS;
		TOKEN_MANDATORY_LABEL TML;

		// ��ʼ��SID
		status = RtlAllocateAndInitializeSid(
			&SIDAuth_IL, 1, IL, 0, 0, 0, 0, 0, 0, 0, &TML.Label.Sid);
		if (NT_SUCCESS(status))
		{
			// ��ʼ��TOKEN_MANDATORY_LABEL
			TML.Label.Attributes = SE_GROUP_INTEGRITY;

			// �������ƶ���
			status = NtSetInformationToken(
				TokenHandle, TokenIntegrityLevel, &TML, sizeof(TML));

			// �ͷ�SID
			RtlFreeSid(TML.Label.Sid);
		}

		return status;
	}
	
	// ��������Ȩ���ƴ���һ����ȨΪ��׼�û�������
	NTSTATUS SuCreateLUAToken(
		_Out_ PHANDLE TokenHandle,
		_In_ HANDLE ExistingTokenHandle)
	{
		// ��������

		NTSTATUS status = STATUS_SUCCESS;
		DWORD Length = 0;
		BOOL EnableTokenVirtualization = TRUE;
		TOKEN_OWNER Owner = { 0 };
		TOKEN_DEFAULT_DACL NewTokenDacl = { 0 };
		PTOKEN_USER pTokenUser = nullptr;
		PTOKEN_DEFAULT_DACL pTokenDacl = nullptr;
		PSID pAdminSid = nullptr;
		PACCESS_ALLOWED_ACE pTempAce = nullptr;

		//������������
		status = NtFilterToken(
			ExistingTokenHandle, LUA_TOKEN,
			nullptr, nullptr, nullptr, TokenHandle);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		// ��������������
		status = SuSetTokenIntegrityLevel(
			*TokenHandle, IntegrityLevel::Medium);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		// ��ȡ���ƶ�Ӧ���û��˻�SID
		status = SuQueryInformationToken(
			*TokenHandle, TokenUser, pTokenUser);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		// ��������OwnerΪ��ǰ�û�
		Owner.Owner = pTokenUser->User.Sid;
		status = NtSetInformationToken(
			*TokenHandle, TokenOwner, &Owner, sizeof(TOKEN_OWNER));
		if (!NT_SUCCESS(status)) goto FuncEnd;

		//��ȡ���Ƶ�DACL
		status = SuQueryInformationToken(
			*TokenHandle, TokenDefaultDacl, pTokenDacl);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		// ��ȡ����Ա��SID
		status = RtlAllocateAndInitializeSid(
			&SidAuth_NT, 2,
			SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
			0, 0, 0, 0, 0, 0, &pAdminSid);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		// ������ACL��С
		Length = pTokenDacl->DefaultDacl->AclSize;
		Length += RtlLengthSid(pTokenUser->User.Sid);
		Length += sizeof(ACCESS_ALLOWED_ACE);

		// ����ACL�ṹ�ڴ�
		status = M2HeapAlloc(Length, NewTokenDacl.DefaultDacl);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		// ����ACL
		status = RtlCreateAcl(
			NewTokenDacl.DefaultDacl,
			Length, pTokenDacl->DefaultDacl->AclRevision);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		// ���ACE
		status = RtlAddAccessAllowedAce(
			NewTokenDacl.DefaultDacl,
			pTokenDacl->DefaultDacl->AclRevision,
			GENERIC_ALL,
			pTokenUser->User.Sid);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		// ����ACE
		for (ULONG i = 0;
			NT_SUCCESS(RtlGetAce(pTokenDacl->DefaultDacl, i, (PVOID*)&pTempAce));
			++i)
		{
			if (RtlEqualSid(pAdminSid, &pTempAce->SidStart)) continue;

			RtlAddAce(
				NewTokenDacl.DefaultDacl,
				pTokenDacl->DefaultDacl->AclRevision, 0,
				pTempAce, pTempAce->Header.AceSize);
		}

		// ��������DACL
		Length += sizeof(TOKEN_DEFAULT_DACL);
		status = NtSetInformationToken(
			*TokenHandle, TokenDefaultDacl, &NewTokenDacl, Length);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		// ����LUA���⻯
		status = NtSetInformationToken(
			*TokenHandle,
			TokenVirtualizationEnabled,
			&EnableTokenVirtualization,
			sizeof(BOOL));
		if (!NT_SUCCESS(status)) goto FuncEnd;

	FuncEnd: // ɨβ

		if (NewTokenDacl.DefaultDacl) M2HeapFree(NewTokenDacl.DefaultDacl);
		if (pAdminSid) RtlFreeSid(pAdminSid);
		if (pTokenDacl) M2HeapFree(pTokenDacl);
		if (pTokenUser) M2HeapFree(pTokenUser);
		if (!NT_SUCCESS(status))
		{
			NtClose(*TokenHandle);
			*TokenHandle = INVALID_HANDLE_VALUE;
		}

		return status;
	}

	// ����һ�����񲢷��ط������ID
	DWORD SuStartService(
		_In_ LPCWSTR lpServiceName)
	{
		DWORD dwPID = (DWORD)-1;
		SERVICE_STATUS_PROCESS ssStatus;
		SC_HANDLE schSCManager = nullptr;
		SC_HANDLE schService = nullptr;
		DWORD dwBytesNeeded;
		bool bStarted = false;

		// ��SCM���������
		schSCManager = OpenSCManagerW(nullptr, nullptr, GENERIC_EXECUTE);
		if (!schSCManager) goto FuncEnd;

		// �򿪷�����
		schService = OpenServiceW(
			schSCManager, lpServiceName,
			SERVICE_START | SERVICE_QUERY_STATUS | SERVICE_STOP);
		if (!schService) goto FuncEnd;

		// ��ѯ״̬
		while (QueryServiceStatusEx(
			schService,
			SC_STATUS_PROCESS_INFO,
			(LPBYTE)&ssStatus,
			sizeof(SERVICE_STATUS_PROCESS),
			&dwBytesNeeded))
		{
			// ���������ֹͣ״̬����û�е���StartServiceW
			if (ssStatus.dwCurrentState == SERVICE_STOPPED && !bStarted)
			{
				bStarted = true;
				if (StartServiceW(schService, 0, nullptr)) continue;
			}
			// ��������ڼ��غ�ж�ع����У�����Ҫ�ȴ�
			else if (ssStatus.dwCurrentState == SERVICE_START_PENDING
				|| ssStatus.dwCurrentState == SERVICE_STOP_PENDING)
			{
				Sleep(ssStatus.dwWaitHint);
				continue;
			}
			// �����������break
			break;
		}

		// ����������û��������У��򷵻ط����Ӧ����PID
		if (ssStatus.dwCurrentState != SERVICE_STOPPED &&
			ssStatus.dwCurrentState != SERVICE_STOP_PENDING)
			dwPID = ssStatus.dwProcessId;

	FuncEnd: //�˳�ʱ�رվ��������

		if (schService) CloseServiceHandle(schService);
		if (schSCManager) CloseServiceHandle(schSCManager);
		return dwPID;
	}

	// ��ȡ������Ϣ���ú����Զ�������ڴ���Ҫʹ��M2HeapFree�ͷţ�
	NTSTATUS SuQuerySystemProcessInformation(
		_Out_ PSYSTEM_PROCESS_INFORMATION *ppSystemProcessInformation)
	{
		// �������
		NTSTATUS status = STATUS_SUCCESS;
		DWORD ReturnLength = 0;

		// ��ȡ������Ϣ��С
		status = NtQuerySystemInformation(
			SystemProcessInformation, 
			nullptr,
			0, 
			&ReturnLength);
		if (status == STATUS_INFO_LENGTH_MISMATCH)
		{
			// Ϊ������Ϣ�����ڴ棬���ʧ���򷵻�
			status = M2HeapAlloc(
				ReturnLength,
				*ppSystemProcessInformation);
			if (NT_SUCCESS(status))
			{
				// ��ȡ������Ϣ
				status = NtQuerySystemInformation(
					SystemProcessInformation,
					*ppSystemProcessInformation,
					ReturnLength,
					&ReturnLength);
				if (!NT_SUCCESS(status))
				{
					// �����ȡʧ�����ͷŸոշ�����ڴ�
					M2HeapFree(*ppSystemProcessInformation);
				}
			}
		}

		// �������н��
		return status;
	}

	// ģ��System����
	NTSTATUS SuImpersonateAsSystem()
	{
		// ��������		
		NTSTATUS status = STATUS_SUCCESS;
		DWORD dwWinLogonPID = (DWORD)-1;
		PSYSTEM_PROCESS_INFORMATION pSPI = nullptr;
		HANDLE hProcessToken = nullptr;
		HANDLE hDuplicatedToken = nullptr;

		do
		{
			// ��ʼ�����̱���
			status = SuQuerySystemProcessInformation(&pSPI);
			if (!NT_SUCCESS(status)) break;

			// ��������Ѱ��winlogon���̲���ȡPID
			PSYSTEM_PROCESS_INFORMATION pNextSPI = pSPI;
			do
			{
				if (pNextSPI->SessionId == M2GetCurrentSessionID())
				{
					if (wcscmp(L"winlogon.exe", pNextSPI->ImageName.Buffer) == 0)
					{
						dwWinLogonPID = HandleToUlong(pNextSPI->UniqueProcessId);
						break;
					}
				}

				if (pNextSPI->NextEntryOffset == 0) break;
				((ULONG&)pNextSPI) += pNextSPI->NextEntryOffset;
			} while (true);

			// ���û�ҵ����̣��򷵻ش���
			if (dwWinLogonPID == -1)
			{
				status = STATUS_NOT_FOUND;
				break;
			}

			// ��ȡ��ǰ�Ựwinlogon��������
			status = SuQueryProcessToken(&hProcessToken, dwWinLogonPID);
			if (!NT_SUCCESS(status)) break;

			// ����һ��ģ������
			status = SuDuplicateToken(
				&hDuplicatedToken,
				hProcessToken,
				MAXIMUM_ALLOWED,
				nullptr,
				SecurityImpersonation,
				TokenImpersonation);
			if (!NT_SUCCESS(status)) break;

			// ��������ȫ����Ȩ
			status = SuSetTokenAllPrivileges(
				hDuplicatedToken,
				SE_PRIVILEGE_ENABLED);
			if (!NT_SUCCESS(status)) break;

			// ģ������
			status = SuImpersonate(hDuplicatedToken);

		} while (false);

		NtClose(hDuplicatedToken);
		NtClose(hProcessToken);
		if (pSPI) M2HeapFree(pSPI);

		return status;
	}
}