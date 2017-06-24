// Serialize.h: interface for the CSerialize class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERIALIZE_H__F189677F_6CB6_46BD_BF03_4CE2B3866FA7__INCLUDED_)
#define AFX_SERIALIZE_H__F189677F_6CB6_46BD_BF03_4CE2B3866FA7__INCLUDED_



class CSerialize
{
public:
	CSerialize( BYTE* pBuf, int iSize)
	:	m_pBuf(pBuf),
		m_iSize(iSize),
		m_iInitSize(iSize),
		m_iOffset(0)
	{
	}

	template <class DATA> 
	static UINT GetSize( const DATA& rData)
	{
		return sizeof(rData);
	}

	static UINT GetSize( const std::string& rStr)
	{
		return (strlen(rStr.c_str()) + 1 );
	}

	template <class DATA> 
	bool LoadFromBuffer(DATA& rData)
	{
		if ( m_iSize >= sizeof(rData))
		{
			memcpy( &rData, m_pBuf + m_iOffset, sizeof rData );
			m_iSize -= sizeof rData;
			m_iOffset += sizeof rData;
			return true;
		}
		else
			return false;
	}

	bool LoadFromBuffer(std::string& rStr)
	{
		int iLen = strlen((const char *)m_pBuf + m_iOffset) + 1;
		if ( m_iSize >= iLen)
		{
			rStr = std::string( (const char *)m_pBuf + m_iOffset);
			m_iSize -= iLen;
			m_iOffset += iLen;
			return true;
		}
		else
			return false;
	}

	
	template <class DATA> 
	bool SaveToBuffer(const DATA& rData)
	{
		if ( m_iSize >= sizeof(rData))
		{
			memcpy( m_pBuf + m_iOffset, &rData, sizeof rData );
			m_iSize -= sizeof rData;
			m_iOffset += sizeof rData;
			return true;
		}
		else
			return false;
	}

  
	bool SaveToBuffer(const std::string& rStr)
	{
		int iLen = strlen( rStr.c_str() ) + 1;
		if ( m_iSize >= iLen)
		{
			memcpy( m_pBuf + m_iOffset, rStr.c_str(), iLen );
			m_iSize -= iLen;
			m_iOffset += iLen;
			return true;
		}
		else
			return false;
	}

	void Code()
	{
		for( int n = 0; n < m_iInitSize; n++)
		{
			m_pBuf[n] = m_pBuf[n] + 77;
		}
	}

	void Decode()
	{
		for( int n = 0; n < m_iInitSize; n++)
		{
			m_pBuf[n] = m_pBuf[n] - 77;
		}
	}
	

protected:
	BYTE* m_pBuf;
	int m_iSize, m_iInitSize;
	int m_iOffset;
};


#endif // !defined(AFX_SERIALIZE_H__F189677F_6CB6_46BD_BF03_4CE2B3866FA7__INCLUDED_)
