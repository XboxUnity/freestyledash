/*
  CStream class written by Brett Goodman, EzTools Software (www.eztools-software.com)
  Donated to the programming community via CodeProject.com on 9 Oct 2009
*/

#pragma once


#if !defined(ASSERT) 
#if defined(_ASSERTE)
#define ASSERT	_ASSERTE
#else
#define ASSERT
#endif
#endif

#ifndef SEEK_BEG
#define SEEK_BEG	STREAM_SEEK_SET
#endif
#ifndef SEEK_CUR
#define SEEK_CUR	STREAM_SEEK_CUR
#endif
#ifndef SEEK_END
#define SEEK_END	STREAM_SEEK_END
#endif

#ifndef EOF
#define EOF     (-1)
#endif

//////////////////////////////////////////////////////////////////////////////
// CStream
//
////////////////////////////////////////////////////////////////////////////// 

class CStream : public IStream
{
	long		m_cRef;
	PBYTE		m_pbMem;
	ULONGLONG	m_ullPos,	// the current stream position
				m_ullEnd,	// the end of the stream (or current stream size)
				m_ullSize;	// the current buffer size
	DWORD		m_dwPageSize,
				BASE_ALLOC_SIZE;
	int			m_iNumCommittedPages;
	IUnknown	*m_pOuterUnk;

	UINT		m_uNumReservedPages;

	

	public:
	void freeMem();
	~CStream() { freeMem(); }

	CStream( ULONG ulInitialSize=0, DWORD dwBaseAllocSize=4096, IUnknown *pOuterUnk=NULL );

	// non-virtual helper/access methods

    // Get the pointer to the internal buffer
	PBYTE GetBuf() { return m_pbMem; }

    // Read a BYTE, optionally advancing the current position
	BYTE GetByte( bool bAdvance=true )
	{
		if( m_ullPos < m_ullEnd )
		{
			BYTE byte = m_pbMem[m_ullPos];
			if( bAdvance ) 
				m_ullPos++;
			return byte;
		}
		throw EOF;
	}

	// gets the current stream position
	ULONG GetCurPos()
	{
		ASSERT( m_ullPos < ULONG(-1L) );
		return ULONG( m_ullPos );
	}

	// gets the current end of stream (the stream length)
	ULONG GetEndPos()
	{
		ASSERT( m_ullEnd < ULONG(-1L) );
		return ULONG( m_ullEnd );
	}

    // sets the current end position, resizing the stream if necessary
	void SetEndPos( ULONG ulNewPos )
	{
		if( ulNewPos >= m_ullSize )
		{
			//ulNewPos = static_cast<ULONG>(m_ullSize-1);
			ULARGE_INTEGER uli={0};
			uli.QuadPart = ulNewPos;
			// if we're on the boundary, we must add one
			if( ulNewPos == m_ullSize )
				uli.QuadPart+= 1;
			SetSize( uli );
		}
		m_ullEnd = ulNewPos;
	}

    // set current position to the beginning
	void SeekToBegin() { m_ullPos = 0; }

    // set current position to the end
	void SeekToEnd() { m_ullPos = m_ullEnd; }

    // gets the current size as a ULONG vs. LONGLONG
	ULONG GetSize() { return ULONG(m_ullSize); }

	// alternate Seek method avoids having to use LARGE_INTEGER
	void Seek( LONG ulLoc, UINT uOrigin, ULONGLONG* pullNewLoc=NULL )
	{
		LARGE_INTEGER liSeekPos = {0};
		ULARGE_INTEGER ulNewPos = {0};
		liSeekPos.QuadPart = ulLoc;

		Seek( liSeekPos, uOrigin, &ulNewPos );
		if( pullNewLoc )
			*pullNewLoc = ulNewPos.QuadPart;
	}

    // returns True if at the end of stream
	BOOL eos() { return m_ullPos >= m_ullEnd; }

    // Resets the stream, optionally setting the size and whether to keep the memory in tact
    // if applicable
	HRESULT Reset( ULONG uSize=0, BOOL bKeepMem=0 );

    // Write a Byte
	HRESULT Write( BYTE ch );

    // Write a WCHAR
    HRESULT Write( WCHAR ch );

    // Write a UINT
    HRESULT Write( UINT u )
	{
		return Write( &u, sizeof(UINT), 0 );
	}

    // Write a ULONG
	HRESULT Write( ULONG l )
	{
		return Write( &l, sizeof(ULONG), 0 );
	}

    // Read a BYTE
	HRESULT Read( PBYTE pch )
	{
		return Read( pch, sizeof(BYTE), 0 );
	}

    // Read a WCHAR
	HRESULT Read( PWCHAR pch )
	{
		return Read( pch, sizeof(WCHAR), 0 );
	}

    // Read a ULONG
	HRESULT Read( ULONG *pl )
	{
		return Read( pl, sizeof(ULONG), 0 );
	}

    // set the size the desired new size
    HRESULT SetSize( ULONGLONG ullNewSize )
    {
        ULARGE_INTEGER uli = {0};
        uli.QuadPart = ullNewSize;
        return SetSize( uli );
    }
    
	// IUnknown Interface
	STDMETHOD_(ULONG,	AddRef)();
	STDMETHOD_(ULONG,	Release)();
	STDMETHOD(QueryInterface)(REFIID iid, void **ppv);

	// ISequentialStream Interface
    STDMETHOD( Read )( 
        void *pv,
        ULONG cb,
        ULONG *pcbRead);
    
    STDMETHOD( Write )( 
        const void *pv,
        ULONG cb,
        ULONG *pcbWritten);

	// IStream methods
    STDMETHOD( Seek )( 
        LARGE_INTEGER dlibMove,
        DWORD dwOrigin,
        ULARGE_INTEGER *plibNewPosition);
    
    STDMETHOD( SetSize )( 
        ULARGE_INTEGER libNewSize);
    
    STDMETHOD( CopyTo )( 
        IStream *pstm,
        ULARGE_INTEGER cb,
        ULARGE_INTEGER *pcbRead,
        ULARGE_INTEGER *pcbWritten);
    
    STDMETHOD( Commit )( 
        DWORD grfCommitFlags);
    
    STDMETHOD( Revert )( void);
    
    STDMETHOD( LockRegion )( 
        ULARGE_INTEGER libOffset,
        ULARGE_INTEGER cb,
        DWORD dwLockType);
    
    STDMETHOD( UnlockRegion )( 
        ULARGE_INTEGER libOffset,
        ULARGE_INTEGER cb,
        DWORD dwLockType);
    
    STDMETHOD( Stat )( 
        STATSTG *pstatstg,
        DWORD grfStatFlag);
    
    STDMETHOD( Clone )( 
        IStream **ppstm);
        
};

inline CStream& operator<<( CStream& strm, PCSTR pstr )
{
	while( *pstr )
	{
		strm.Write( BYTE(*pstr) );
		pstr++;
	}
	return strm;
}

inline CStream& operator<<( CStream& strm, PCWSTR pstr )
{
	while( *pstr )
	{
		strm.Write( *pstr );
		pstr++;
	}
	return strm;
}

inline CStream& operator<<( CStream& strm, long lVal )
{
	strm.Write( &lVal, sizeof(long), NULL );
	return strm;
}

inline CStream& operator<<( CStream& strm, UINT nVal )
{
	strm.Write( &nVal, sizeof(UINT), NULL );
	return strm;
}

inline CStream& operator<<( CStream& strm, BYTE cVal )
{
	strm.Write( cVal );
	return strm;
}

inline CStream& operator<<( CStream& strm, char cVal )
{
	strm.Write( (BYTE) cVal );
	return strm;
}

inline CStream& operator<<( CStream& strm, WCHAR wcVal )
{
	strm.Write( wcVal );
	return strm;
}


