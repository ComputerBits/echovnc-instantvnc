////////////////////////////////////////////////////////////////
// MSDN Magazine -- October 2001
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
// Compiles with Visual C++ 6.0 for Windows 98 and probably Windows 2000 too.
// Set tabsize = 3 in your editor.
//
#include "StdAfx.h"
#include "Picture.h"

//#include <afxpriv2.h> //#IR:-

//#include "LoadFromMemStream.h"
//#include "..\Creator\Image\Image\ImageView.h"

/*
#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif
*/


//#IR
//IMPORTANT NOTE:
//I've marked some methods and modifications with #IR+, but it's just to distinct these modifications!
//Most of the added code was extracted from article "Loading JPEG and GIF pictures"
//See http://www.arstdesign.com/articles/picloader.html

#define HIMETRIC_INCH   2540    // HIMETRIC units per inch


/////////////////////////////////////////////////////////////////////////////
// CArchiveStream

//#IR:-
//class CArchiveStream : public IStream
//{
//public:
//	CArchiveStream(CArchive* pArchive);
//
//// Implementation
//	CArchive* m_pArchive;
//
//	STDMETHOD_(ULONG, AddRef)();
//	STDMETHOD_(ULONG, Release)();
//	STDMETHOD(QueryInterface)(REFIID, LPVOID*);
//
//	STDMETHOD(Read)(void*, ULONG, ULONG*);
//	STDMETHOD(Write)(const void*, ULONG cb, ULONG*);
//	STDMETHOD(Seek)(LARGE_INTEGER, DWORD, ULARGE_INTEGER*);
//	STDMETHOD(SetSize)(ULARGE_INTEGER);
//	STDMETHOD(CopyTo)(LPSTREAM, ULARGE_INTEGER, ULARGE_INTEGER*,
//		ULARGE_INTEGER*);
//	STDMETHOD(Commit)(DWORD);
//	STDMETHOD(Revert)();
//	STDMETHOD(LockRegion)(ULARGE_INTEGER, ULARGE_INTEGER,DWORD);
//	STDMETHOD(UnlockRegion)(ULARGE_INTEGER, ULARGE_INTEGER, DWORD);
//	STDMETHOD(Stat)(STATSTG*, DWORD);
//	STDMETHOD(Clone)(LPSTREAM*);
//};


////////////////////////////////////////////////////////////////
// CPicture implementation
//

CPicture::CPicture()
:	m_bIsLoad(false)
   ,m_hInstanceMod(NULL)	//#IR:100301 +
   ,m_spIPicture(NULL)		//#updated:IR:100614: +
{
	
}


CPicture::~CPicture()
{
	Free();		//#updated:IR:100614: +
}


bool CPicture::IsLoad()const
{
	return m_bIsLoad;
}


void CPicture::SetUnload()
{
	m_bIsLoad = false;
}


#pragma region " Load "

//#IR+
bool CPicture::Load(HINSTANCE hInst, HRSRC hRsrc)
{
	// load resource into memory
	DWORD len = ::SizeofResource(hInst, hRsrc);
	HANDLE hResData = ::LoadResource(hInst, hRsrc);
	if (!hResData)
		return false;

	HGLOBAL hGlobal = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_NODISCARD, len);
	if ( !hGlobal ) {
		::FreeResource(hResData);
		return false;
	}
	char* pDest = reinterpret_cast<char*> ( ::GlobalLock(hGlobal) );
	char* pSrc = reinterpret_cast<char*> ( ::LockResource(hResData) );
	if (!pSrc || !pDest) {
		::GlobalFree(hGlobal);
		::FreeResource(hResData);
		return false;
	}
	::CopyMemory(pDest,pSrc,len);
	::FreeResource(hResData);
	::GlobalUnlock(hGlobal);

	IStream* pStream = NULL;
	if (S_OK != ::CreateStreamOnHGlobal(hGlobal, FALSE, &pStream)) {  //the 'FALSE' is to don't delete memory on object's release
		::GlobalFree(hGlobal);
		return false;
	}

	// create memory file and load it
	bool bRet = Load(pStream);

	::GlobalFree(hGlobal);
	
	return bRet;
}


bool CPicture::LoadBinary(UINT nIDRes)
{
	// find resource in resource file
	//(#IR: * : modified to fix bug, the app miniWinVNC, which uses this library, did get crash/exception on line below (commented and replaced now)!
	//HINSTANCE hInst = AfxGetResourceHandle();
	HINSTANCE hInst = NULL;
	if (NULL == m_hInstanceMod) {
		//hInst = AfxGetResourceHandle();	//#IR-
		hInst = ::GetModuleHandle(NULL);	//#IR +
	}
	//)
	HRSRC hRsrc = ::FindResource(hInst, MAKEINTRESOURCE(nIDRes), RT_RCDATA);
	if (!hRsrc)
		return false;
	
	return Load(hInst, hRsrc);
}


bool CPicture::Load(UINT nIDRes)
{
	// find resource in resource file
	HINSTANCE hInst = NULL;
	if (NULL == m_hInstanceMod) {
		hInst = ::GetModuleHandle(NULL);
	}
	//)
	HRSRC hRsrc = ::FindResource(hInst, MAKEINTRESOURCE(nIDRes), "IMAGE"); // type
	if (!hRsrc)
		return false;

	// load resource into memory
	return Load(hInst, hRsrc);
}


//bool CPicture::Load(const char* szPathName)
//{
//	CFile file;
//	if (!file.Open(szPathName, CFile::modeRead|CFile::shareDenyWrite))
//		return false;
//
//	bool bRet = Load(file);
//	file.Close();
//	return bRet;
//}


bool CPicture::Load(LPCTSTR pszPathName)
{
	HANDLE hFile = ::CreateFile(pszPathName, 
								FILE_READ_DATA,
								FILE_SHARE_READ,
								NULL, 
								OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL,
								NULL);
	if ( !hFile )
		return false;

	DWORD len = ::GetFileSize( hFile, NULL); // only 32-bit of the actual file size is retained
	if (len == 0)
		return false;

	HGLOBAL hGlobal = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_NODISCARD, len);
	if ( !hGlobal ) {
		::CloseHandle(hFile);
		return false;
	}

	char* lpBuffer = reinterpret_cast<char*> ( ::GlobalLock(hGlobal) );
	DWORD dwBytesRead = 0;

	while ( ::ReadFile(hFile, lpBuffer, 4096, &dwBytesRead, NULL) )
	{
		lpBuffer += dwBytesRead;
		if (dwBytesRead == 0)
			break;
		dwBytesRead = 0;
	}

	::CloseHandle(hFile);

	
	::GlobalUnlock(hGlobal);


	// don't delete memory on object's release
	IStream* pStream = NULL;
	if ( ::CreateStreamOnHGlobal(hGlobal,FALSE,&pStream) != S_OK )
	{
		::GlobalFree(hGlobal);
		return false;
	}

	// create memory file and load it
	bool bRet = Load(pStream);

	::GlobalFree(hGlobal);

	return bRet;
}



//#IR : -
//(

//bool CPicture::Load(CFile& file)
//{
//	CArchive ar(&file, CArchive::load | CArchive::bNoFlushOnDelete);
//	return Load(ar);
//}
//
//
//bool CPicture::Load(CArchive& ar)
//{
//	CArchiveStream arcstream(&ar);
//	return Load((IStream*)&arcstream);
//}

//)


bool CPicture::Load(IStream* pstm)
{
	m_bIsLoad = false;
	try
	{
		Free();
		HRESULT hr = ::OleLoadPicture(pstm, 0, FALSE, IID_IPicture, (void**)&m_spIPicture);

		if (SUCCEEDED(hr) && m_spIPicture)
			m_bIsLoad = SUCCEEDED(hr) && m_spIPicture;
		else
			m_bIsLoad = false;
	}
	catch(...)
	{
	}

	return m_bIsLoad;
}

#pragma endregion //\ Load


#pragma region " Draw "

bool CPicture::Draw(HDC hDC, RECT* rc, const RECT* prcMFBounds) const
{
	if (!m_bIsLoad || NULL == hDC)
		return false;

	long hmWidth, hmHeight; // HIMETRIC units
	GetHIMETRICSize(hmWidth, hmHeight);

	RECT rcDefault = {0};
	if (NULL == rc) {
		rc = &rcDefault;
	}
	
	m_spIPicture->Render(hDC, 
				rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top,
				0, hmHeight, hmWidth, -hmHeight, prcMFBounds);

	return true;
}


bool CPicture::Draw(
	HDC  hDC, 
	const RECT& rc,				// A position of image in pDC
	OLE_XPOS_HIMETRIC  xSrc,	// Horizontal offset in source picture
	OLE_YPOS_HIMETRIC  ySrc,	// Vertical offset in source picture
	OLE_XSIZE_HIMETRIC cxSrc,	// Amount to copy horizontally in source picture
	OLE_YSIZE_HIMETRIC cySrc	// Amount to copy vertically in source picture
	) const
{
	if (!m_bIsLoad || NULL == hDC)
		return false;

	m_spIPicture->Render(hDC, 
				rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
				xSrc, ySrc, cxSrc, cySrc, 
				0);// A pointer to position of destination for a metafile hdc 

	return true;
}


bool CPicture::Draw(
	HDC		hDC, 
	const RECT& rc,	// A position of image in pDC
	float fHorzOffset,	// Horizontal offset in source picture [ 0, 1]
	float fVertOffset,	// Vertical offset in source picture [ 0, 1]
	float fHorzSize,	// Amount to copy horizontally in source picture [ 0, 1]
	float fVertSize,	// Amount to copy vertically in source picture [ 0, 1]
	bool  bLeftRightMirror,// = false,
	bool  bTopBottomMirror // = false
) const
{
	if (!m_bIsLoad || NULL == hDC)
		return false;

	if (fHorzOffset < 0 || fVertOffset < 0 ) {
		return false;
	}

	if (fHorzSize < 0 || fHorzSize < 0 ) {
		return false;
	}

	long hmWidth, hmHeight; // HIMETRIC units
	GetHIMETRICSize(hmWidth, hmHeight);

	// Horizontal offset in source picture
	OLE_XPOS_HIMETRIC xSrc = (OLE_XPOS_HIMETRIC)(fHorzOffset * (float)hmWidth);

	// Amount to copy horizontally in source picture
	OLE_XSIZE_HIMETRIC cxSrc = (OLE_XPOS_HIMETRIC)(fHorzSize * (float)hmWidth);

	// Vertical offset in source picture
	OLE_YPOS_HIMETRIC ySrc = (OLE_XPOS_HIMETRIC)(( 1.0 - fVertOffset ) * (float)hmHeight);

	// Amount to copy vertically in source picture
	OLE_YSIZE_HIMETRIC cySrc = (OLE_XPOS_HIMETRIC)(fVertSize * (float)hmHeight);

	int iLeft = rc.left;
	int iWidth = rc.right - rc.left;
	if (bLeftRightMirror)
	{
		iLeft += iWidth;
		iWidth *= -1;
	}

	int iTop = rc.top;
	int iHeight = abs(rc.bottom - rc.top);
	if (bTopBottomMirror)
	{
		iTop += iHeight;
		iHeight *= -1;
	}

	m_spIPicture->Render(hDC, 
						iLeft, iTop, iWidth, iHeight,
						xSrc, ySrc, cxSrc, -cySrc, 0);

	return true;
}

#pragma endregion //\ Draw


SIZE CPicture::GetImageSize(HDC hDC) const
{
	SIZE sz = {0,0};
	
	if (!m_spIPicture)
		return sz;
	
	LONG hmWidth, hmHeight; // HIMETRIC units
	m_spIPicture->get_Width(&hmWidth);
	m_spIPicture->get_Height(&hmHeight);

	sz.cx = hmWidth; 
	sz.cy = hmHeight;

	if (hDC==NULL) {
		hDC = ::GetWindowDC(NULL);
		SetHIMETRICtoDP(hDC, &sz); // convert to pixels
	} 
	else {
		SetHIMETRICtoDP(hDC, &sz);
	}
	return sz;
}

//#IR + : just added code from this article
//See http://www.arstdesign.com/articles/picloader.html

void CPicture::SetHIMETRICtoDP(HDC hdc, SIZE* sz) const
{
	int nMapMode;
	if ( (nMapMode = ::GetMapMode(hdc)) < MM_ISOTROPIC && nMapMode != MM_TEXT)
	{
		// when using a constrained map mode, map against physical inch
		::SetMapMode(hdc,MM_HIMETRIC);
		POINT pt;
		pt.x = sz->cx;
		pt.y = sz->cy;
		::LPtoDP(hdc,&pt,1);
		sz->cx = pt.x;
		sz->cy = pt.y;
		::SetMapMode(hdc, nMapMode);
	}
	else
	{
		// map against logical inch for non-constrained mapping modes
		int cxPerInch, cyPerInch;
		cxPerInch = ::GetDeviceCaps(hdc,LOGPIXELSX);
		cyPerInch = ::GetDeviceCaps(hdc,LOGPIXELSY);
		sz->cx = MulDiv(sz->cx, cxPerInch, HIMETRIC_INCH);
		sz->cy = MulDiv(sz->cy, cyPerInch, HIMETRIC_INCH);
	}

	POINT pt;
	pt.x = sz->cx;
	pt.y = sz->cy;
	::DPtoLP(hdc,&pt,1);
	sz->cx = pt.x;
	sz->cy = pt.y;
}


void CPicture::GetHIMETRICSize(OLE_XSIZE_HIMETRIC& cx, OLE_YSIZE_HIMETRIC& cy) const 
{
	cx = cy = 0;
	const_cast<CPicture*>(this)->m_hr = m_spIPicture->get_Width(&cx);
	const_cast<CPicture*>(this)->m_hr = m_spIPicture->get_Height(&cy);
}


void CPicture::Free() 
{
	//if (m_spIPicture) 
	//{
	//	m_spIPicture.Release();
	//}
//#updated:IR:100614: +
	if (NULL != m_spIPicture) 
	{
		m_spIPicture->Release();
		m_spIPicture = NULL;
	}
}
