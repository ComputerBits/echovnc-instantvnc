#ifndef _inc_picture_h
#define _inc_picture_h

////////////////////////////////////////////////////////////////
// MSDN Magazine -- October 2001
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
// Compiles with Visual C++ 6.0 for Windows 98 and probably Windows 2000 too.

#pragma once
//#include <vector>

//(#updated:IR:100614: -
//#include <atlbase.h>

//(#updated:IR:100614: +
//#include <atlcomcli.h>	// for CComQIPtr
#include <ocidl.h>		// for IPicture, OLE_XPOS_HIMETRIC etc
#include <olectl.h>		// for OleLoadPicture() etc
//)

//////////////////
// Picture object--encapsulates IPicture
//
class CPicture 
{
public:
	CPicture();
	~CPicture();

	//(#IR:100301 +
	HINSTANCE m_hInstanceMod;
	void SetInstanceMod(HINSTANCE v) { m_hInstanceMod = v; }
	//)

	// Load frm various sosurces:

	// Load from binary resource. Looks for RCDATA type:
	// IDR_IMAGE RCDATA  DISCARDABLE "RES\\BlueFlower.gif"
	bool LoadBinary(UINT nIDRes);

	// Load from resource. Looks for "IMAGE" type.
	bool Load(UINT nIDRes);

	// Load from path name
	//bool Load(const char* szPathName);
	bool Load(LPCTSTR pszPathName);

private:
	bool Load(HINSTANCE hInst, HRSRC hRsrc);	//#IR:+
	void SetHIMETRICtoDP(HDC hdc, SIZE* sz) const;

public:
//#IR:-
	//// Load from CFile
	//bool Load(CFile& file);

//#IR:-
	//// Load from archive - create stream as CArchiveStream and load from it.
	//bool Load(CArchive& ar);

	// Load from stream (IStream). 
	// This is the one that really does it: call OleLoadPicture to do the work.
	bool Load(IStream* pstm);

	bool IsLoad()const;

	void SetUnload();

//(#IR:-
	//// Draw to device context. Covert to HIMETRIC for IPicture.
	//bool Draw(CDC* pDC, CRect rc=CRect(0,0,0,0), const RECT* prcMFBounds=NULL) const;

	//bool Draw(
	//		CDC* pDC, 
	//		const CRect& rc,			// A position of image in pDC
	//		OLE_XPOS_HIMETRIC  xSrc,	// Horizontal offset in source picture
	//		OLE_YPOS_HIMETRIC  ySrc,	// Vertical offset in source picture
	//		OLE_XSIZE_HIMETRIC cxSrc,	// Amount to copy horizontally in source picture
	//		OLE_YSIZE_HIMETRIC cySrc	// Amount to copy vertically in source picture
	//	) const;

	//bool Draw(
	//		CDC* pDC, 
	//		const CRect& rc,	// A position of image in pDC
	//		float fHorzOffset,	// Horizontal offset in source picture [ 0, 1]
	//		float fVertOffset,	// Vertical offset in source picture [ 0, 1]
	//		float fHorzSize,	// Amount to copy horizontally in source picture [ 0, 1]
	//		float fVertSize,	// Amount to copy vertically in source picture [ 0, 1],
	//		bool  bLeftRightMirror = false,
	//		bool  bTopBottomMirror = false
	//	) const;

	//// Get image size in pixels. Converts from HIMETRIC to device coords.
	//CSize GetImageSize(CDC* pDC=NULL) const;
//)


//(#IR:+
	// Draw to device context. Covert to HIMETRIC for IPicture.
	// prcMFBounds : NULL if dc is not a metafile dc
	bool Draw(HDC hDC, RECT* rc=NULL, const RECT* prcMFBounds=NULL) const;

	bool Draw(
			HDC hDC, 
			const RECT& rc,				// A position of image in pDC
			OLE_XPOS_HIMETRIC  xSrc,	// Horizontal offset in source picture
			OLE_YPOS_HIMETRIC  ySrc,	// Vertical offset in source picture
			OLE_XSIZE_HIMETRIC cxSrc,	// Amount to copy horizontally in source picture
			OLE_YSIZE_HIMETRIC cySrc	// Amount to copy vertically in source picture
		) const;

	bool Draw(
			HDC hDC, 
			const RECT& rc,				// A position of image in pDC
			float fHorzOffset,	// Horizontal offset in source picture [ 0, 1]
			float fVertOffset,	// Vertical offset in source picture [ 0, 1]
			float fHorzSize,	// Amount to copy horizontally in source picture [ 0, 1]
			float fVertSize,	// Amount to copy vertically in source picture [ 0, 1],
			bool  bLeftRightMirror = false,
			bool  bTopBottomMirror = false
		) const;

	// Get image size in pixels. Converts from HIMETRIC to device coords.
	SIZE GetImageSize(HDC hDC=NULL) const;
//)

	operator IPicture*(){ return m_spIPicture; }

	void GetHIMETRICSize(OLE_XSIZE_HIMETRIC& cx, OLE_YSIZE_HIMETRIC& cy) const; 

protected:
	void Free(); 

protected:
//	CComQIPtr<IPicture>m_spIPicture;	// ATL smart pointer to IPicture
	IPicture* m_spIPicture;
	HRESULT m_hr;						// last error code
	bool m_bIsLoad;
};


#endif	//_inc_picture_h