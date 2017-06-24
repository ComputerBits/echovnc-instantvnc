// CPicLib.h: interface for the CDawPic class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PICLIB_H__1E11C2CE_407B_4046_BB5D_CAADBE9A6F48__INCLUDED_)
#define AFX_PICLIB_H__1E11C2CE_407B_4046_BB5D_CAADBE9A6F48__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CPicture;

class CPicLib  
{
public:
	CPicLib();

	~CPicLib();

	//(#IR:100301 +
	HINSTANCE m_hInstanceMod;
	void SetInstanceMod(HINSTANCE v);
	//)
	
	bool LoadFromRes( UINT uiID);

	bool LoadFromFile( const char* szPath );

	bool IsLoad()const;

	void GetSize( HDC hDC, UINT& rWidth, UINT& rHeight);

	bool Draw( HDC hDC, RECT rc);

protected:
	CPicture* m_pPicture;
};

#endif // !defined(AFX_PICLIB_H__1E11C2CE_407B_4046_BB5D_CAADBE9A6F48__INCLUDED_)
