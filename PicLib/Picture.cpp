////////////////////////////////////////////////////////////////
// MSDN Magazine -- October 2001
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
// Compiles with Visual C++ 6.0 for Windows 98 and probably Windows 2000 too.
// Set tabsize = 3 in your editor.
//
#include "StdAfx.h"
#include "Picture.h"
#include <afxpriv2.h>

//#include "LoadFromMemStream.h"
//#include "..\Creator\Image\Image\ImageView.h"

/*
#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif
*/


/////////////////////////////////////////////////////////////////////////////
// CArchiveStream

class CArchiveStream : public IStream
{
public:
	CArchiveStream(CArchive* pArchive);

// Implementation
	CArchive* m_pArchive;

	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();
	STDMETHOD(QueryInterface)(REFIID, LPVOID*);

	STDMETHOD(Read)(void*, ULONG, ULONG*);
	STDMETHOD(Write)(const void*, ULONG cb, ULONG*);
	STDMETHOD(Seek)(LARGE_INTEGER, DWORD, ULARGE_INTEGER*);
	STDMETHOD(SetSize)(ULARGE_INTEGER);
	STDMETHOD(CopyTo)(LPSTREAM, ULARGE_INTEGER, ULARGE_INTEGER*,
		ULARGE_INTEGER*);
	STDMETHOD(Commit)(DWORD);
	STDMETHOD(Revert)();
	STDMETHOD(LockRegion)(ULARGE_INTEGER, ULARGE_INTEGER,DWORD);
	STDMETHOD(UnlockRegion)(ULARGE_INTEGER, ULARGE_INTEGER, DWORD);
	STDMETHOD(Stat)(STATSTG*, DWORD);
	STDMETHOD(Clone)(LPSTREAM*);
};


////////////////////////////////////////////////////////////////
// CPicture implementation
//

CPicture::CPicture()
:	m_bIsLoad(false)
   ,m_hInstanceMod(NULL)	//#IR:100301 +
{
}


CPicture::~CPicture()
{
}


bool CPicture::IsLoad()const
{
	return m_bIsLoad;
}


void CPicture::SetUnload()
{
	m_bIsLoad = false;
}


bool CPicture::LoadBinary(UINT nIDRes)
{
	// find resource in resource file
	//(#IR: * : modified to fix bug, the app miniWinVNC, which uses this library, did get crash/exception on line below (commented and replaced now)!
	//HINSTANCE hInst = AfxGetResourceHandle();
	HINSTANCE hInst = NULL;
	if (NULL == m_hInstanceMod) {
		hInst = AfxGetResourceHandle();
	}
	//)
	HRSRC hRsrc = ::FindResource(hInst, MAKEINTRESOURCE(nIDRes), RT_RCDATA);

	if (!hRsrc)
		return false;

	// load resource into memory
	DWORD len = SizeofResource(hInst, hRsrc);
	BYTE* lpRsrc = (BYTE*)LoadResource(hInst, hRsrc);
	if (!lpRsrc)
		return false;

	// create memory file and load it
	CMemFile file(lpRsrc, len);
	bool bRet = Load(file);
	FreeResource(hRsrc);
	return bRet;
}


bool CPicture::Load(UINT nIDRes)
{
	// find resource in resource file
	HINSTANCE hInst = AfxGetResourceHandle();
	HRSRC hRsrc = ::FindResource(hInst, MAKEINTRESOURCE(nIDRes), "IMAGE"); // type

	if (!hRsrc)
		return false;

	// load resource into memory
	DWORD len = SizeofResource(hInst, hRsrc);
	BYTE* lpRsrc = (BYTE*)LoadResource(hInst, hRsrc);
	if (!lpRsrc)
		return false;

	// create memory file and load it
	CMemFile file(lpRsrc, len);
	bool bRet = Load(file);
	FreeResource(hRsrc);
	return bRet;
}


bool CPicture::Load(const char* szPathName)
{
	CFile file;
	if (!file.Open(szPathName, CFile::modeRead|CFile::shareDenyWrite))
		return false;

	bool bRet = Load(file);
	file.Close();
	return bRet;
}


bool CPicture::Load(CFile& file)
{
	CArchive ar(&file, CArchive::load | CArchive::bNoFlushOnDelete);
	return Load(ar);
}


bool CPicture::Load(CArchive& ar)
{
	CArchiveStream arcstream(&ar);
	return Load((IStream*)&arcstream);
}


bool CPicture::Load(IStream* pstm)
{
	m_bIsLoad = false;
	try
	{
		Free();
		HRESULT hr = OleLoadPicture(pstm, 0, FALSE, IID_IPicture, (void**)&m_spIPicture);

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




bool CPicture::Draw(CDC* pDC, CRect rc, const RECT* prcMFBounds) const
{
	if (!m_bIsLoad)
		return m_bIsLoad;

	ASSERT(pDC);

	long hmWidth, hmHeight; // HIMETRIC units
	GetHIMETRICSize(hmWidth, hmHeight);
	m_spIPicture->Render(*pDC, rc.left, rc.top, rc.Width(), rc.Height(),
		0, hmHeight, hmWidth, -hmHeight, prcMFBounds);

	return true;
}




bool CPicture::Draw(
	CDC* pDC, 
	const CRect& rc,			// A position of image in pDC
	OLE_XPOS_HIMETRIC  xSrc,	// Horizontal offset in source picture
	OLE_YPOS_HIMETRIC  ySrc,	// Vertical offset in source picture
	OLE_XSIZE_HIMETRIC cxSrc,	// Amount to copy horizontally in source picture
	OLE_YSIZE_HIMETRIC cySrc	// Amount to copy vertically in source picture
	) const
{
	if (!m_bIsLoad)
		return m_bIsLoad;

	ASSERT(pDC);

	m_spIPicture->Render(*pDC, rc.left, rc.top, rc.Width(), rc.Height(),
		xSrc, ySrc, cxSrc, cySrc, 
		0);// A pointer to position of destination for a metafile hdc 

	return true;
}


bool CPicture::Draw(
	CDC* pDC, 
	const CRect& rc,	// A position of image in pDC
	float fHorzOffset,	// Horizontal offset in source picture [ 0, 1]
	float fVertOffset,	// Vertical offset in source picture [ 0, 1]
	float fHorzSize,	// Amount to copy horizontally in source picture [ 0, 1]
	float fVertSize,	// Amount to copy vertically in source picture [ 0, 1]
	bool  bLeftRightMirror,// = false,
	bool  bTopBottomMirror // = false
) const
{
	if (!m_bIsLoad)
		return m_bIsLoad;

	ASSERT(pDC);

	if (fHorzOffset < 0 || fVertOffset < 0 )
	{
		ASSERT(false); return false;
	}

	if (fHorzSize < 0 || fHorzSize < 0 )
	{
		ASSERT(false); return false;
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
	int iWidth = rc.Width();
	if (bLeftRightMirror)
	{
		iLeft += iWidth;
		iWidth *= -1;
	}

	int iTop = rc.top;
	int iHeight = rc.Height();
	if (bTopBottomMirror)
	{
		iTop += iHeight;
		iHeight *= -1;
	}

	m_spIPicture->Render(*pDC, iLeft, iTop, iWidth, iHeight,
		xSrc, ySrc, cxSrc, -cySrc, 0);

	return true;
}


CSize CPicture::GetImageSize(CDC* pDC) const
{
	if (!m_spIPicture)
		return CSize(0,0);
	
	LONG hmWidth, hmHeight; // HIMETRIC units
	m_spIPicture->get_Width(&hmWidth);
	m_spIPicture->get_Height(&hmHeight);
	CSize sz(hmWidth,hmHeight);
	if (pDC==NULL) {
		CWindowDC dc(NULL);
		dc.HIMETRICtoDP(&sz); // convert to pixels
	} else {
		pDC->HIMETRICtoDP(&sz);
	}
	return sz;
}


void CPicture::GetHIMETRICSize(OLE_XSIZE_HIMETRIC& cx, OLE_YSIZE_HIMETRIC& cy) const 
{
	cx = cy = 0;
	const_cast<CPicture*>(this)->m_hr = m_spIPicture->get_Width(&cx);
	ASSERT(SUCCEEDED(m_hr));
	const_cast<CPicture*>(this)->m_hr = m_spIPicture->get_Height(&cy);
	ASSERT(SUCCEEDED(m_hr));
}


void CPicture::Free() 
{
	if (m_spIPicture) 
	{
		m_spIPicture.Release();
	}
}
