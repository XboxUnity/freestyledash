// Stream.cpp : Implementation of CStream

#include "stdafx.h"
#include "Stream.h"

#ifndef RETURN_IF_FAILED
#define RETURN_IF_FAILED( hr ) if( FAILED(hr) ) return hr;
#endif
#define STGM_SIMPLE             0x08000000L

#define STGM_READ               0x00000000L
#ifndef ASSERT
#if defined(_DEBUG) || defined(DEBUG)
#define ASSERT(expr) \
        do { if (!(expr) && \
                (1 == _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, NULL, NULL))) \
             _CrtDbgBreak(); } while (0)
#else
#define ASSERT(expr)
#endif
#endif

/////////////////////////////////////////////////////////////////////////////
// CStream
//
// WARNING: This class is minimally implemented.  You may need to implement
//			more methods for it to work for your use.
//
/////////////////////////////////////////////////////////////////////////////

CStream::CStream( ULONG ulInitialSize, DWORD dwBaseAllocSize, IUnknown *pOuterUnk ) 
	: m_cRef(0), m_pbMem(0), m_ullEnd(0), m_ullPos(0), m_ullSize(0), m_iNumCommittedPages(0), m_pOuterUnk(pOuterUnk),
		m_uNumReservedPages(0), BASE_ALLOC_SIZE(dwBaseAllocSize)
{
	// determine the system page size

	m_dwPageSize = 4096;
	// reserve some memory for this stream

	if( ulInitialSize > 0 )
	{
		if( ulInitialSize > BASE_ALLOC_SIZE )
		{
			m_uNumReservedPages = ((ulInitialSize/BASE_ALLOC_SIZE+1)*BASE_ALLOC_SIZE)/m_dwPageSize;
		}
		else
		{
			m_uNumReservedPages = BASE_ALLOC_SIZE/m_dwPageSize;
		}

		// reserve but don't commit yet
		m_pbMem = (PBYTE) VirtualAlloc( NULL, m_dwPageSize*m_uNumReservedPages, MEM_RESERVE, PAGE_NOACCESS );
		ASSERT(m_pbMem);
	}
	// I don't know why I had this here, but it can't be this way in QueryInterface if an unsupported
	// interface is queried for!
	//if( m_pOuterUnk == NULL )
	//	m_pOuterUnk = this;
}

void CStream::freeMem()
{
	if( m_pbMem )
	{
		// decommit the committed pages
		BOOL b;
		if( m_ullSize > ULONGLONG(0) )
		{
			b = VirtualFree( m_pbMem, (SIZE_T) m_ullSize, MEM_DECOMMIT );
			ASSERT(b);
		}
		// free all pages
		b = VirtualFree( m_pbMem, 0, MEM_RELEASE );
		ASSERT(b);
		m_pbMem = NULL;
	}
	// WARNING: does not reset the buffer vars (size, pos, end)
}

HRESULT CStream::Reset( ULONG uSize, BOOL bKeepMem )
{
	HRESULT hr = 0;
	char bSetSize=TRUE;

	if( bKeepMem )
	{
		m_ullEnd = m_ullPos = 0;
		if( uSize < m_ullSize )
			bSetSize = 0;
	}
	else
	{
		freeMem();
		m_ullEnd = m_ullPos = m_ullSize = 0;
		m_iNumCommittedPages = 0;
		m_uNumReservedPages = 0;
	}
	if( bSetSize )
	{
		ULARGE_INTEGER uli = {0};
		uli.QuadPart = uSize;
		hr = SetSize( uli );
	}

	return hr;
}

STDMETHODIMP_(ULONG) 
CStream::AddRef() 
{
	return InterlockedIncrement( &m_cRef );  
} 

STDMETHODIMP_(ULONG) 
CStream::Release() 
{ 
    ULONG ret = InterlockedDecrement( &m_cRef );
	if( ret == 0) 
	{ 
		delete this; 
		return 0; 
	}
	return ret; 
} 
 
STDMETHODIMP 
CStream::QueryInterface( REFIID iid, void** ppv )  
{    
	*ppv = NULL; 
	IUnknown *pUnk = NULL;
     
	if( IsEqualIID( iid, IID_IStream ) )
	{
		pUnk = this;      
		pUnk->AddRef(); 
		*ppv = pUnk;
		return NOERROR;     
	}
	else if( m_pOuterUnk )
		return m_pOuterUnk->QueryInterface( iid, ppv );
	else
		return E_NOINTERFACE;
} 
 
STDMETHODIMP 
CStream::Read(
    void *pv,
    ULONG cb,
    ULONG *pcbRead )
{
	if( pcbRead )
		*pcbRead = 0;

	if( !pv )
		return STG_E_INVALIDPOINTER;

	// determine the number of bytes that can be read
	if( cb + m_ullPos > m_ullEnd )
	{
		cb = ULONG(m_ullEnd - m_ullPos);
	}

	// check to see if the seek pointer was at or after the end of the stream
	// this can happen via the Seek method
	if( cb <= 0 )
	{
		// no data read - return false
		if( pcbRead )
			pcbRead = 0;
		return S_FALSE;
	}

	// read the data
	CopyMemory( pv, &m_pbMem[m_ullPos], cb );

	m_ullPos+= cb;

	if( pcbRead )
		*pcbRead = cb;

	return S_OK;
}

STDMETHODIMP 
CStream::Write( 
    const void *pv,
    ULONG cb,
    ULONG *pcbWritten )
{
	if( !pv )
		return STG_E_INVALIDPOINTER;

	// expand committed memory if necessary
	if( cb + m_ullPos > m_ullSize )
	{
		// dertermine the size needed 
		ULARGE_INTEGER uli; 
		uli.QuadPart = m_ullPos + cb;
		RETURN_IF_FAILED( SetSize( uli ) );
	}

	// write the data
	CopyMemory( &m_pbMem[m_ullPos], pv, cb );

	if( pcbWritten )
		*pcbWritten = cb;

	// update the cur pos
	m_ullPos+= cb;

	// if we went past the current end, update it
	if( m_ullPos > m_ullEnd )
		m_ullEnd = m_ullPos;

	return S_OK;
}

// IStream methods
STDMETHODIMP 
CStream::Seek( 
    LARGE_INTEGER dlibMove,
    DWORD dwOrigin,
    ULARGE_INTEGER *plibNewPosition )
{
	switch( dwOrigin )
	{
		case STREAM_SEEK_SET:  // seek from the beginning
		{
			// interpret dlibMove as unsigned
			ULONGLONG lNewPos = (ULONGLONG) dlibMove.QuadPart;
			// can't seek past the end
			if( lNewPos > m_ullEnd )
				lNewPos = m_ullEnd;
			m_ullPos = lNewPos;
		}
		break;
		
		case STREAM_SEEK_CUR:	// seek from the current location
		{
			// seek relative to the current position, either pos or neg
			LONGLONG llMove = dlibMove.QuadPart,
					 lNewPos = m_ullPos + llMove,
					 lEndPos = m_ullEnd; // must be signed

			if( lNewPos > lEndPos )
				lNewPos = lEndPos;
			m_ullPos = lNewPos;
			if( (LONGLONG) m_ullPos < 0 )
			{
				// the doc says this is an error, but doesn't say what to do
				// so we'll just put it at zero and move on
				m_ullPos = 0;
			}
		}
		break;
		
		case STREAM_SEEK_END:	// seek from the end
		{
			LONGLONG llMove = dlibMove.QuadPart,
					 lNewPos = m_ullEnd + llMove,
					 lEndPos = m_ullEnd; // must be signed

			if( lNewPos > lEndPos )
				lNewPos = lEndPos;
			m_ullPos = lNewPos;
			if( (LONGLONG) m_ullPos < 0 )
			{
				// the doc says this is an error, but doesn't say what to do
				// so we'll just put it at zero and move on
				m_ullPos = 0;
			}
		}
		break;

		default:
			return STG_E_INVALIDFUNCTION;
		break;
	}

	if( plibNewPosition )
		plibNewPosition->QuadPart = m_ullPos;

	return S_OK;
}

STDMETHODIMP 
CStream::SetSize( ULARGE_INTEGER libNewSize )
{
	ULONGLONG ullNewSize = libNewSize.QuadPart;

	// moved from below
	int iPagesNeeded = (ULONG) ullNewSize / m_dwPageSize;
	if( ullNewSize > 0 )
		if( ullNewSize % m_dwPageSize != 0 ) iPagesNeeded++;

	if( m_iNumCommittedPages == iPagesNeeded ) return 0;

	// are we growing or shrinking?
	if( ullNewSize < m_ullSize )
	{
		// shrinking - dertermine the number of pages needed
		int iNumToUncommit = m_iNumCommittedPages - iPagesNeeded;

		ASSERT( m_iNumCommittedPages >= iNumToUncommit );
		// unexpected, but allow it after fix up
		if( iNumToUncommit > m_iNumCommittedPages )
			iNumToUncommit = m_iNumCommittedPages;

		// now we need to uncommit the pages above our new size
		if( iNumToUncommit > 0 )
		{
			BOOL b = VirtualFree(	&m_pbMem[iPagesNeeded*m_dwPageSize], 
									iNumToUncommit*m_dwPageSize, 
									MEM_DECOMMIT );
			ASSERT( b );
			if( !b )
				return E_FAIL;
	
			m_iNumCommittedPages-= iNumToUncommit;
			m_ullSize = m_iNumCommittedPages*m_dwPageSize;

			// Note: At this point, the actual new size of the stream buff may be larger
			// than the requested size (m_ullSize >= ullNewSize) since we are allocating
			// on page boundaries

			// set the stream end pointer to the new size if needed
			if( m_ullEnd > ullNewSize )
				m_ullEnd = ullNewSize;
		}
	}
	else if( ullNewSize > m_ullSize )
	{
		ULONG ulSize = m_dwPageSize * iPagesNeeded;

		if( iPagesNeeded > (int) m_uNumReservedPages )
		{
			// must allocate a new block and copy the data

			UINT uNumReservedPages = ((ulSize/BASE_ALLOC_SIZE+1)*BASE_ALLOC_SIZE)/m_dwPageSize;

			PBYTE pNewMem = (PBYTE) VirtualAlloc( NULL, m_dwPageSize*uNumReservedPages, MEM_RESERVE, PAGE_NOACCESS );
			ASSERT(pNewMem);
			if( !pNewMem )
				return STG_E_MEDIUMFULL;

			pNewMem = (PBYTE) VirtualAlloc( pNewMem, ulSize, MEM_COMMIT, PAGE_READWRITE );
			ASSERT(pNewMem);
			if( !pNewMem )
				return STG_E_MEDIUMFULL;

			if( m_pbMem )
				memcpy( pNewMem, m_pbMem, (size_t) m_ullEnd );

			freeMem();

			m_pbMem = pNewMem;
			m_uNumReservedPages = uNumReservedPages;
		}
        else
        {
		    m_pbMem = (PBYTE) VirtualAlloc( m_pbMem, ulSize, MEM_COMMIT, PAGE_READWRITE );
		    ASSERT(m_pbMem);

            if( !m_pbMem )
			    return STG_E_MEDIUMFULL;
        }

		m_iNumCommittedPages = iPagesNeeded;
		m_ullSize = ulSize;
	}

	return S_OK;
}

STDMETHODIMP 
CStream::CopyTo( 
    IStream *pstm,
    ULARGE_INTEGER cb,
    ULARGE_INTEGER *pcbRead,
    ULARGE_INTEGER *pcbWritten )
{
	return E_NOTIMPL;
}

STDMETHODIMP 
CStream::Commit( DWORD grfCommitFlags )
{
	return E_NOTIMPL;
}

STDMETHODIMP 
CStream::Revert()
{
	return E_NOTIMPL;
}

STDMETHODIMP 
CStream::LockRegion( 
    ULARGE_INTEGER libOffset,
    ULARGE_INTEGER cb,
    DWORD dwLockType )
{
	return E_NOTIMPL;
}

STDMETHODIMP 
CStream::UnlockRegion( 
    ULARGE_INTEGER libOffset,
    ULARGE_INTEGER cb,
    DWORD dwLockType )
{
	return E_NOTIMPL;
}

STDMETHODIMP 
CStream::Stat( 
    STATSTG *pstatstg,
    DWORD grfStatFlag )
{
	#if 0		
	if( grfStatFlag & STATFLAG_DEFAULT ) // wants the stream name using CoMemTaskAlloc
	{
		pstatstg->pwcsName = (PTCHAR) CoTaskMemAlloc( 5 ); //  STATFLAG_NONAME
		_tcscpy( pstatstg->pwcsName, _T("Test") );
	}
	#endif
	memset( pstatstg, 0, sizeof(STATSTG) );
	pstatstg->type = STGTY_STREAM;
	pstatstg->cbSize.QuadPart = m_ullEnd;
	pstatstg->grfMode = STGM_SIMPLE|STGM_READ; //0x80000000;
	pstatstg->clsid = IID_IStream;

	// ??? don't know what to do here...
	/*FILETIME ft;
	CoFileTimeNow( &ft );
	pstatstg->mtime = ft;
	pstatstg->ctime = ft;
	pstatstg->atime = ft;*/

	return S_OK;
}

STDMETHODIMP 
CStream::Clone( IStream **ppstm )
{
	return E_NOTIMPL;
}

/////////////////////////////
// these versions for internal use and speed to write a single char
//
HRESULT CStream::Write( BYTE ch )
{
	// expand committed memory if necessary
	if( 1 + m_ullPos > m_ullSize )
	{
		// dertermine the size needed 
		ULARGE_INTEGER uli; 
		uli.QuadPart = m_ullPos + 1;
		RETURN_IF_FAILED( SetSize( uli ) );
	}

	// write the data
	m_pbMem[m_ullPos++] = ch;

	// if we went past the current end, update it
	if( m_ullPos > m_ullEnd )
		m_ullEnd = m_ullPos;

	return S_OK;
}

HRESULT CStream::Write( WCHAR ch )
{
	// expand committed memory if necessary
	if( 2 + m_ullPos > m_ullSize )
	{
		// dertermine the size needed 
		ULARGE_INTEGER uli; 
		uli.QuadPart = m_ullPos + 2;
		RETURN_IF_FAILED( SetSize( uli ) );
	}

	// write the data
	CopyMemory( &m_pbMem[m_ullPos], &ch, 2 );
	m_ullPos+= 2;

	// if we went past the current end, update it
	if( m_ullPos > m_ullEnd )
		m_ullEnd = m_ullPos;

	return S_OK;
}

