// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

// Ϊ����ͨ�������õľ���
#if _MSC_VER >= 1200
// Ϊ�˲�ȥ��δʹ�õ��βΣ����ǽ��øþ���
#pragma warning(disable:4100) // δʹ�õ��β�(�ȼ� 4)
// �������Ż����ܳ��ֵľ��棨ȥ��δ���ú������ʵ���һЩ����ʹ��������
#pragma warning(disable:4505) // δ���õı��غ������Ƴ�(�ȼ� 4)
#pragma warning(disable:4710) // ����δ����(�ȼ� 4)
#pragma warning(disable:4711) // Ϊ�Զ�������չѡ���˺���(�ȼ� 1,ֻ����ʾ��Ϣ)
#endif

#ifndef _DEBUG
#define _CRT_SECURE_NO_WARNINGS
#include <_msvcrt.h>
#endif 

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#include <Windows.h>

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
