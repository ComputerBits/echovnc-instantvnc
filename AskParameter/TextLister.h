//Copyright (c) 2008 Echogent Systems, Inc., a California  corporation. All rights reserved.
#pragma once
#include "windows.h"
class CTextLister  
{
public:
	CTextLister();
	virtual ~CTextLister();
	int Show(HINSTANCE hInstance, HWND hwndParent);
	HWND _hwnd;
	HWND _hwndParent;
private:
	void OnInitDialog( HWND hDlg );
	static LRESULT CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static void FillStartingData();

};
