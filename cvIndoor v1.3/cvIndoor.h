
// cvIndoor.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CcvIndoorApp:
// �йش����ʵ�֣������ cvIndoor.cpp
//

class CcvIndoorApp : public CWinApp
{
public:
	CcvIndoorApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CcvIndoorApp theApp;