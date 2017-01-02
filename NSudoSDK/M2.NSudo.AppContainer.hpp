/**************************************************************************
������NSudo AppContainer��
ά���ߣ�Mouri_Naruto (M2-Team)
�汾��1.0 (2017-01-01)
������Ŀ����
Э�飺The MIT License
�÷���ֱ��Include��ͷ�ļ�����
�����Windows SDK�汾��10.0.10586���Ժ�
***************************************************************************/

#pragma once

#ifndef M2_NSUDO_APPCONTAINER
#define M2_NSUDO_APPCONTAINER

#include <strsafe.h>

namespace M2
{
	// AppContainer�����б�
	const enum SuAppContainerHandleList
	{
		RootDirectory, // ��Ŀ¼����
		RpcDirectory,  // RPCĿ¼����
		GlobalSymbolicLink, // Global��������
		LocalSymbolicLink, // Local��������
		SessionSymbolicLink, // Session��������
		NamedPipe //�����ܵ�
	};

	// ��AppContainer�ں˶���Ŀ¼�´�����������
	static NTSTATUS SuCreateAppContainerSymbolicLinks(
		_In_ HANDLE RootDirectory,
		_In_ PUNICODE_STRING LinkTarget,
		_In_ PVOID SecurityDescriptor,
		_Out_ PHANDLE GlobalSymbolicLink,
		_Out_ PHANDLE LocalSymbolicLink, 
		_Out_ PHANDLE SessionSymbolicLink)
	{
		// �������

		UNICODE_STRING usGlobal = RTL_CONSTANT_STRING(L"Global");
		UNICODE_STRING usLocal = RTL_CONSTANT_STRING(L"Local");
		UNICODE_STRING usSession = RTL_CONSTANT_STRING(L"Session");	
		UNICODE_STRING usBNO = RTL_CONSTANT_STRING(L"\\BaseNamedObjects");

		NTSTATUS status = STATUS_SUCCESS;
		OBJECT_ATTRIBUTES ObjectAttributes;

		// ��ʼ������Global�������ӵ�OBJECT_ATTRIBUTES�ṹ
		M2InitObjectAttributes(
			&ObjectAttributes,
			&usGlobal, 
			OBJ_INHERIT | OBJ_OPENIF,
			RootDirectory, 
			SecurityDescriptor, 
			nullptr);

		// ����Global��������
		status = NtCreateSymbolicLinkObject(
			GlobalSymbolicLink,
			SYMBOLIC_LINK_ALL_ACCESS,
			&ObjectAttributes,
			&usBNO);
		if (!NT_SUCCESS(status)) goto Error;
		
		// ��ʼ������Local�������ӵ�OBJECT_ATTRIBUTES�ṹ
		M2InitObjectAttributes(
			&ObjectAttributes,
			&usLocal,
			OBJ_INHERIT | OBJ_OPENIF,
			RootDirectory,
			SecurityDescriptor,
			nullptr);

		// ����Local��������
		status = NtCreateSymbolicLinkObject(
			LocalSymbolicLink,
			SYMBOLIC_LINK_ALL_ACCESS,
			&ObjectAttributes,
			LinkTarget);
		if (!NT_SUCCESS(status)) goto Error;

		// ��ʼ������Session�������ӵ�OBJECT_ATTRIBUTES�ṹ
		M2InitObjectAttributes(
			&ObjectAttributes,
			&usSession,
			OBJ_INHERIT | OBJ_OPENIF,
			RootDirectory,
			SecurityDescriptor,
			nullptr);

		// ����Session��������
		status = NtCreateSymbolicLinkObject(
			SessionSymbolicLink,
			SYMBOLIC_LINK_ALL_ACCESS,
			&ObjectAttributes,
			LinkTarget);
		if (!NT_SUCCESS(status)) goto Error;

		return status;

	Error: // ������
		
		if (INVALID_HANDLE_VALUE != SessionSymbolicLink)
		{
			NtClose(*SessionSymbolicLink);
			*SessionSymbolicLink = INVALID_HANDLE_VALUE;
		}

		if (INVALID_HANDLE_VALUE != LocalSymbolicLink)
		{
			NtClose(*LocalSymbolicLink);
			*LocalSymbolicLink = INVALID_HANDLE_VALUE;
		}
		
		if (INVALID_HANDLE_VALUE != GlobalSymbolicLink)
		{
			NtClose(*GlobalSymbolicLink);
			*GlobalSymbolicLink = INVALID_HANDLE_VALUE;		
		}

		return status;
	}	

	//����ɳ�а�ȫ��ʶ��
	static NTSTATUS WINAPI SuBuildAppContainerSecurityDescriptor(
		_In_ PSECURITY_DESCRIPTOR ExistingSecurityDescriptor,
		_In_ PSID SandBoxSid,
		_In_ PSID UserSid,
		_In_ bool IsRpcControl,
		_Out_ PSECURITY_DESCRIPTOR *NewSecurityDescriptor)
	{
		//�������
		NTSTATUS status = STATUS_SUCCESS;
		DWORD ReturnLength = 0;
		BOOLEAN DaclPresent = FALSE;
		BOOLEAN DaclDefaulted = FALSE;
		PACL pAcl = nullptr;
		PACL pNewAcl = nullptr;
		PSID AdminSid = nullptr;
		PSID RestrictedSid = nullptr;
		PSID WorldSid = nullptr;
		bool bUserSidExist = false;
		PACCESS_ALLOWED_ACE pTempAce = nullptr;

		//����������SID�ṹ
		status = RtlAllocateAndInitializeSid(
			&SidAuth_NT, 1, SECURITY_RESTRICTED_CODE_RID,
			0, 0, 0, 0, 0, 0, 0, &RestrictedSid);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		//���ɹ���Ա��SID�ṹ
		status = RtlAllocateAndInitializeSid(
			&SidAuth_NT, 2, SECURITY_BUILTIN_DOMAIN_RID,
			DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdminSid);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		//����Everyone��SID�ṹ
		status = RtlAllocateAndInitializeSid(
			&SidAuth_World, 1, SECURITY_WORLD_RID,
			0, 0, 0, 0, 0, 0, 0, &WorldSid);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		//��ȡ���ж����ACL
		status = RtlGetDaclSecurityDescriptor(
			ExistingSecurityDescriptor, &DaclPresent, &pAcl, &DaclDefaulted);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		//������ACL��С
		ReturnLength = pAcl->AclSize;
		ReturnLength += RtlLengthSid(SandBoxSid) * 2;
		ReturnLength += RtlLengthSid(UserSid) * 2;
		ReturnLength += RtlLengthSid(RestrictedSid);
		ReturnLength += RtlLengthSid(AdminSid);
		ReturnLength += RtlLengthSid(WorldSid);
		ReturnLength += sizeof(ACCESS_ALLOWED_ACE) * 7;

		//����ACL�ṹ�ڴ�
		status = M2HeapAlloc(ReturnLength, pNewAcl);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		//����ACL
		status = RtlCreateAcl(pNewAcl, ReturnLength, pAcl->AclRevision);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		//����ACE
		for (ULONG i = 0; NT_SUCCESS(RtlGetAce(pAcl, i, (PVOID*)&pTempAce)); i++)
		{
			//����½SID����Ȩ�������޸�
			if (SuIsLogonSid(&pTempAce->SidStart)
				&& !(pTempAce->Header.AceFlags & INHERIT_ONLY_ACE))
			{
				pTempAce->Mask = DIRECTORY_ALL_ACCESS;
			}

			//���������rpc�������������Ա��Everyone��SID���
			if (!IsRpcControl
				&& (RtlEqualSid(&pTempAce->SidStart, AdminSid)
					|| RtlEqualSid(&pTempAce->SidStart, RestrictedSid)
					|| RtlEqualSid(&pTempAce->SidStart, WorldSid))) continue;

			//������û�SID��������
			if (RtlEqualSid(&pTempAce->SidStart, UserSid))
				bUserSidExist = true;

			//���ACE
			RtlAddAce(pNewAcl, pAcl->AclRevision, 0,
				pTempAce, pTempAce->Header.AceSize);
		}

		//���ACE�����⣩ - ɳ��SID
		status = RtlAddAccessAllowedAce(
			pNewAcl,
			pAcl->AclRevision,
			DIRECTORY_ALL_ACCESS,
			SandBoxSid);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		//���ACE��InheritNone�� - ɳ��SID
		status = RtlAddAccessAllowedAceEx(
			pNewAcl,
			pAcl->AclRevision,
			OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE,
			GENERIC_ALL,
			SandBoxSid);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		if (!bUserSidExist)
		{
			//���ACE�����⣩ - �û�SID
			status = RtlAddAccessAllowedAce(
				pNewAcl,
				pAcl->AclRevision,
				DIRECTORY_ALL_ACCESS,
				UserSid);
			if (!NT_SUCCESS(status)) goto FuncEnd;

			//���ACE��InheritNone�� - �û�SID
			status = RtlAddAccessAllowedAceEx(
				pNewAcl,
				pAcl->AclRevision,
				OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE,
				0x5A4D,
				UserSid);
			if (!NT_SUCCESS(status)) goto FuncEnd;
		}

		if (IsRpcControl)
		{
			//���ACE��InheritNone�� - ����ԱSID
			status = RtlAddAccessAllowedAceEx(
				pNewAcl,
				pAcl->AclRevision,
				OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE,
				GENERIC_ALL,
				AdminSid);
			if (!NT_SUCCESS(status)) goto FuncEnd;

			//���ACE��InheritNone�� - ����SID
			status = RtlAddAccessAllowedAceEx(
				pNewAcl,
				pAcl->AclRevision,
				OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE,
				0xA0000000,
				RestrictedSid);
			if (!NT_SUCCESS(status)) goto FuncEnd;

			//���ACE��InheritNone�� - Everyone SID
			status = RtlAddAccessAllowedAceEx(
				pNewAcl,
				pAcl->AclRevision,
				OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE,
				0xA0000000,
				WorldSid);
			if (!NT_SUCCESS(status)) goto FuncEnd;
		}

		//����SD�ṹ�ڴ�
		status = M2HeapAlloc(
			sizeof(SECURITY_DESCRIPTOR), *NewSecurityDescriptor);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		//����SD
		status = RtlCreateSecurityDescriptor(
			*NewSecurityDescriptor, SECURITY_DESCRIPTOR_REVISION);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		//����SD
		status = RtlSetDaclSecurityDescriptor(
			*NewSecurityDescriptor, DaclPresent, pNewAcl, DaclDefaulted);
		if (!NT_SUCCESS(status)) goto FuncEnd;

	FuncEnd:
		RtlFreeSid(WorldSid);
		RtlFreeSid(AdminSid);
		RtlFreeSid(RestrictedSid);
		return status;
	}

	// ����AppContainer����
	static NTSTATUS WINAPI SuCreateAppContainerToken(
		_Out_ PHANDLE TokenHandle,
		_In_ HANDLE ExistingTokenHandle,
		_In_ PSECURITY_CAPABILITIES SecurityCapabilities)
	{
		// �����ʼ��

		NTSTATUS status = STATUS_SUCCESS;
		DWORD ReturnLength = 0;
		DWORD TokenSessionID = 0;
		wchar_t Buffer[MAX_PATH];	
		UNICODE_STRING usBNO = RTL_CONSTANT_STRING(L"\\BaseNamedObjects");
		OBJECT_ATTRIBUTES ObjectAttributes;			
		UNICODE_STRING usACNO = { 0 };	
		UNICODE_STRING usRpcControl = RTL_CONSTANT_STRING(L"\\RPC Control");		
		UNICODE_STRING usRpcControl2 = RTL_CONSTANT_STRING(L"RPC Control");
		UNICODE_STRING usRootDirectory = { 0 };
		PACCESS_ALLOWED_ACE pTempAce = nullptr;
		UNICODE_STRING usNamedPipe = {0};
		IO_STATUS_BLOCK IoStatusBlock;
		UNICODE_STRING usAppContainerSID = { 0 };
		HANDLE hBaseNamedObjects = nullptr;
		PSECURITY_DESCRIPTOR pSD = nullptr;
		PTOKEN_USER pTokenUser = nullptr;
		PSECURITY_DESCRIPTOR pDirectorySD = nullptr;
		PSECURITY_DESCRIPTOR pRpcControlSD = nullptr;
		HANDLE hAppContainerNamedObjects = nullptr;
		HANDLE hRpcControl = nullptr;
		HANDLE HandleList[6] = { nullptr };

		// ��ȡ���ƻỰID
		status = NtQueryInformationToken(
			ExistingTokenHandle,
			TokenSessionId,
			&TokenSessionID,
			sizeof(DWORD),
			&ReturnLength);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		// ��SIDת��ΪUnicode�ַ���
		status = RtlConvertSidToUnicodeString(
			&usAppContainerSID, SecurityCapabilities->AppContainerSid, TRUE);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		// ���SessionID��Ϊ0�������ɶ�Ӧ�Ự��·��
		if (TokenSessionID)
		{
			StringCbPrintfW(
				Buffer, sizeof(Buffer),
				L"\\Sessions\\%ld\\BaseNamedObjects", TokenSessionID);

			RtlInitUnicodeString(&usBNO, Buffer);
		}

		// ��ʼ����BaseNamedObjectsĿ¼��OBJECT_ATTRIBUTES�ṹ
		M2InitObjectAttributes(
			&ObjectAttributes, &usBNO, 0, nullptr, nullptr, nullptr);

		// ��BaseNamedObjectsĿ¼
		status = NtOpenDirectoryObject(
			&hBaseNamedObjects,
			READ_CONTROL | DIRECTORY_QUERY | DIRECTORY_TRAVERSE,
			&ObjectAttributes);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		// ��ȡBaseNamedObjectsĿ¼��ȫ��ʶ����Ϣ��С
		NtQuerySecurityObject(
			hBaseNamedObjects, DACL_SECURITY_INFORMATION,
			nullptr, 0, &ReturnLength);

		// Ϊ��ȫ��ʶ�������ڴ�
		status = M2HeapAlloc(ReturnLength, pSD);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		// ��ȡBaseNamedObjectsĿ¼��ȫ��ʶ����Ϣ
		status = NtQuerySecurityObject(
			hBaseNamedObjects, DACL_SECURITY_INFORMATION,
			pSD, ReturnLength, &ReturnLength);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		// ��ȡ�����û���Ϣ��С
		NtQueryInformationToken(
			ExistingTokenHandle, TokenUser,
			nullptr, 0, &ReturnLength);

		// Ϊ�����û���Ϣ�����ڴ�
		status = M2HeapAlloc(ReturnLength, pTokenUser);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		// ��ȡ�����û���Ϣ
		status = NtQueryInformationToken(
			ExistingTokenHandle, TokenUser,
			pTokenUser, ReturnLength, &ReturnLength);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		// ����AppContainer����Ŀ¼��ȫ��ʶ��
		status = SuBuildAppContainerSecurityDescriptor(
			pSD, 
			SecurityCapabilities->AppContainerSid,
			pTokenUser->User.Sid,
			false,
			&pDirectorySD);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		// ����AppContainer RPC����Ŀ¼��ȫ��ʶ��
		status = SuBuildAppContainerSecurityDescriptor(
			pSD,
			SecurityCapabilities->AppContainerSid, 
			pTokenUser->User.Sid, 
			true,
			&pRpcControlSD);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		// ��ʼ��AppContainerNamedObjects����Ŀ¼·���ַ���
		StringCbPrintfW(
			Buffer, sizeof(Buffer),
			L"\\Sessions\\%ld\\AppContainerNamedObjects", TokenSessionID);

		// ��ʼ��AppContainerNamedObjects����Ŀ¼·��UNICODE_STRING�ṹ
		RtlInitUnicodeString(&usACNO, Buffer);

		// ��ʼ����AppContainerNamedObjectsĿ¼��OBJECT_ATTRIBUTES�ṹ
		M2InitObjectAttributes(
			&ObjectAttributes, &usACNO, 0, nullptr, nullptr, nullptr);

		// ��AppContainerNamedObjectsĿ¼
		status = NtOpenDirectoryObject(
			&hAppContainerNamedObjects,
			DIRECTORY_QUERY | DIRECTORY_TRAVERSE |
			DIRECTORY_CREATE_OBJECT | DIRECTORY_CREATE_SUBDIRECTORY,
			&ObjectAttributes);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		// ��ʼ������AppContainer����Ŀ¼��OBJECT_ATTRIBUTES�ṹ
		M2InitObjectAttributes(
			&ObjectAttributes,
			&usAppContainerSID,
			OBJ_INHERIT | OBJ_OPENIF,
			hAppContainerNamedObjects,
			pDirectorySD,
			nullptr);

		// ����AppContainerĿ¼����
		status = NtCreateDirectoryObjectEx(
			&HandleList[SuAppContainerHandleList::RootDirectory],
			DIRECTORY_QUERY | DIRECTORY_TRAVERSE |
			DIRECTORY_CREATE_OBJECT | DIRECTORY_CREATE_SUBDIRECTORY,
			&ObjectAttributes,
			hBaseNamedObjects,
			1);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		// ����AppContainerĿ¼���������Ա�ǩΪ��
		status = SuSetKernelObjectIL(
			HandleList[SuAppContainerHandleList::RootDirectory],
			IntegrityLevel::Low);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		// ��ʼ����RPC ControlĿ¼��OBJECT_ATTRIBUTES�ṹ
		M2InitObjectAttributes(
			&ObjectAttributes, &usRpcControl, 0, nullptr, nullptr, nullptr);

		// ��RPC ControlĿ¼
		status = NtOpenDirectoryObject(
			&hRpcControl,
			DIRECTORY_QUERY | DIRECTORY_TRAVERSE,
			&ObjectAttributes);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		// ��ʼ������AppContainer RPC ControlĿ¼�����OBJECT_ATTRIBUTES�ṹ
		M2InitObjectAttributes(
			&ObjectAttributes,
			&usRpcControl2,
			OBJ_INHERIT | OBJ_OPENIF,
			HandleList[SuAppContainerHandleList::RootDirectory],
			pRpcControlSD,
			nullptr);

		// ����AppContainer RPC ControlĿ¼����
		status = NtCreateDirectoryObjectEx(
			&HandleList[SuAppContainerHandleList::RpcDirectory],
			DIRECTORY_QUERY | DIRECTORY_TRAVERSE |
			DIRECTORY_CREATE_OBJECT | DIRECTORY_CREATE_SUBDIRECTORY,
			&ObjectAttributes,
			hBaseNamedObjects,
			1);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		// ����AppContainer RPC ControlĿ¼���������Ա�ǩΪ��
		status = SuSetKernelObjectIL(
			HandleList[SuAppContainerHandleList::RpcDirectory],
			IntegrityLevel::Low);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		// ��ʼ��AppContainerĿ¼�����ַ���
		StringCbPrintfW(
			Buffer, sizeof(Buffer),
			L"\\Sessions\\%d\\AppContainerNamedObjects\\%ws",
			TokenSessionID,
			usAppContainerSID.Buffer, usAppContainerSID.Length);

		// ��ʼ��AppContainerĿ¼�����UNICODE_STRING�ṹ
		RtlInitUnicodeString(&usRootDirectory, Buffer);

		// ��AppContainerĿ¼�����´�������ķ�������
		status = SuCreateAppContainerSymbolicLinks(
			HandleList[SuAppContainerHandleList::RootDirectory],
			&usRootDirectory,
			pDirectorySD,
			&HandleList[SuAppContainerHandleList::GlobalSymbolicLink],
			&HandleList[SuAppContainerHandleList::LocalSymbolicLink],
			&HandleList[SuAppContainerHandleList::SessionSymbolicLink]);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		// ��ʼ��AppContainer�����ܵ�·���ַ���
		StringCbPrintfW(
			Buffer, sizeof(Buffer),
			L"\\Device\\NamedPipe\\Sessions\\%d\\AppContainerNamedObjects\\%ws",
			TokenSessionID,
			usAppContainerSID.Buffer, usAppContainerSID.Length);

		for (ULONG i = 0;
			NT_SUCCESS(RtlGetAce(
			((SECURITY_DESCRIPTOR*)pDirectorySD)->Dacl,
				i,
				(PVOID*)&pTempAce));
			++i)
		{
			DWORD LowMask = LOWORD(pTempAce->Mask);

			// ����pTempAce->Mask��16λ

			pTempAce->Mask >>= 16;
			pTempAce->Mask <<= 16;

			if (FILE_CREATE_PIPE_INSTANCE == (LowMask & FILE_CREATE_PIPE_INSTANCE))
				pTempAce->Mask |= SYNCHRONIZE | FILE_WRITE_DATA;

			if (FILE_READ_EA == (LowMask & FILE_READ_EA))
				pTempAce->Mask |= SYNCHRONIZE | FILE_CREATE_PIPE_INSTANCE;
		}

		// ��ʼ��AppContainer�����ܵ�UNICODE_STRING�ṹ
		RtlInitUnicodeString(&usNamedPipe, Buffer);

		// ��ʼ������AppContainer�����ܵ���OBJECT_ATTRIBUTES�ṹ
		M2InitObjectAttributes(
			&ObjectAttributes,
			&usNamedPipe,
			OBJ_INHERIT | OBJ_CASE_INSENSITIVE,
			nullptr,
			pDirectorySD,
			nullptr);

		// ����AppContainer�����ܵ�
		status = NtCreateFile(
			&HandleList[SuAppContainerHandleList::NamedPipe],
			SYNCHRONIZE | STANDARD_RIGHTS_REQUIRED |
			FILE_WRITE_DATA | FILE_CREATE_PIPE_INSTANCE,
			&ObjectAttributes,
			&IoStatusBlock,
			nullptr,
			FILE_ATTRIBUTE_NORMAL,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			FILE_OPEN_IF,
			FILE_DIRECTORY_FILE,
			nullptr,
			0);
		if (!NT_SUCCESS(status)) goto FuncEnd;

		// ����AppContainer������
		status = NtCreateLowBoxToken(
			TokenHandle,
			ExistingTokenHandle,
			MAXIMUM_ALLOWED,
			NULL,
			SecurityCapabilities->AppContainerSid,
			SecurityCapabilities->CapabilityCount,
			SecurityCapabilities->Capabilities,
			6,
			HandleList);

	FuncEnd: // ��������
		
		for (size_t i = 0; i < 6; ++i) NtClose(HandleList[i]);	
		NtClose(hRpcControl);
		NtClose(hAppContainerNamedObjects);
		M2HeapFree(pRpcControlSD);
		M2HeapFree(pDirectorySD);
		M2HeapFree(pTokenUser);
		M2HeapFree(pSD);
		NtClose(hBaseNamedObjects);
		RtlFreeUnicodeString(&usAppContainerSID);

		return status;
	}

	// �������һ��AppContainer SID
	static void WINAPI SuGenerateRandomAppContainerSid(
		_Out_ PSID *RandomAppContainerSid)
	{
		// �������
		LARGE_INTEGER PerfCounter, PerfFrequency;

		// ��ȡ���ܼ�������ֵ
		NtQueryPerformanceCounter(&PerfCounter, &PerfFrequency);

		//��������
		ULONG seed = (ULONG)(PerfCounter.QuadPart - PerfFrequency.QuadPart);

		RtlAllocateAndInitializeSid(
			&SidAuth_App,
			SECURITY_APP_PACKAGE_RID_COUNT,
			SECURITY_APP_PACKAGE_BASE_RID,
			(DWORD)RtlRandomEx(&seed),
			(DWORD)RtlRandomEx(&seed),
			(DWORD)RtlRandomEx(&seed),
			(DWORD)RtlRandomEx(&seed),
			(DWORD)RtlRandomEx(&seed),
			(DWORD)RtlRandomEx(&seed),
			(DWORD)RtlRandomEx(&seed),
			RandomAppContainerSid);
	}

	// ����AppContainer�����б�
	static NTSTATUS WINAPI SuGenerateAppContainerCapabilitiy(
		_Out_ PSID_AND_ATTRIBUTES *Capabilities,
		_Out_ PDWORD CapabilityCount)
	{
		//�������
		NTSTATUS status = STATUS_SUCCESS;
		SID_IDENTIFIER_AUTHORITY SIDAuthority = SECURITY_APP_PACKAGE_AUTHORITY;
		PSID_AND_ATTRIBUTES CapabilitiesList = nullptr;
		const DWORD CapabilitiyTypeRID[] =
		{
			SECURITY_CAPABILITY_INTERNET_CLIENT,
			SECURITY_CAPABILITY_PRIVATE_NETWORK_CLIENT_SERVER,
			SECURITY_CAPABILITY_SHARED_USER_CERTIFICATES,
			SECURITY_CAPABILITY_ENTERPRISE_AUTHENTICATION,
		};

		//���ò����������ڴ�
		*CapabilityCount = 4;
		CapabilitiesList = (PSID_AND_ATTRIBUTES)M2HeapAlloc(
			*CapabilityCount * sizeof(SID_AND_ATTRIBUTES));
		if (!CapabilitiesList) return STATUS_NO_MEMORY;

		//��ȡ����SID
		for (DWORD i = 0; i < *CapabilityCount; i++)
		{
			CapabilitiesList[i].Attributes = SE_GROUP_ENABLED;
			status = RtlAllocateAndInitializeSid(
				&SIDAuthority,
				SECURITY_BUILTIN_CAPABILITY_RID_COUNT,
				SECURITY_CAPABILITY_BASE_RID, CapabilitiyTypeRID[i],
				NULL, NULL, NULL, NULL, NULL, NULL,
				&CapabilitiesList[i].Sid);
			if (!NT_SUCCESS(status)) goto FuncEnd;
		}
		*Capabilities = CapabilitiesList;

	FuncEnd:
		return status;
	}

	//����ɳ��Job����
	static NTSTATUS WINAPI SuCreateSandBoxJobObject(
		_Out_ PHANDLE JobObject)
	{
		//�������
		NTSTATUS status = NULL;
		OBJECT_ATTRIBUTES ObjectAttributes =
		{
			sizeof(OBJECT_ATTRIBUTES), // Length
			NULL, // RootDirectory
			NULL, // ObjectName
			OBJ_OPENIF, // Attributes
			NULL, // SecurityDescriptor
			NULL // SecurityQualityOfService
		};
		JOBOBJECT_EXTENDED_LIMIT_INFORMATION JELI = { 0 };

		//����Job����
		status = NtCreateJobObject(JobObject, MAXIMUM_ALLOWED, &ObjectAttributes);
		if (!NT_SUCCESS(status)) return status;

		//������Ʋ�����
		JELI.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
		return NtSetInformationJobObject(
			*JobObject, JobObjectExtendedLimitInformation, &JELI, sizeof(JELI));
	}

}

#endif