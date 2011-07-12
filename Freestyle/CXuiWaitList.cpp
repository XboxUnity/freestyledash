#include "stdafx.h"

#include "CXuiWaitList.h"


//----------------------------------------------------------------------------------
// Name: RegisterWaitHandle
// Desc: Registers a handle for async operations.
//       When the specified handle is signaled, the application will send the 
//       specified message to hObj.  If bRemoveAfterSignaled, the handle is 
//       unregistered before the message is sent.  If the caller needs the handle 
//       to persist, then it must be registered again.
//----------------------------------------------------------------------------------
BOOL CXuiWaitList::RegisterWaitHandle( HANDLE hWait, HXUIOBJ hObj, DWORD dwMessageId, BOOL bRemoveAfterSignaled )
{
    ASSERT( m_nNumWaitHandles < MAXIMUM_WAIT_OBJECTS );
    if( m_nNumWaitHandles >= MAXIMUM_WAIT_OBJECTS )
        return FALSE;

    // Append to the wait handle array.
    m_WaitHandles[ m_nNumWaitHandles ] = hWait;

    memset( &m_WaitEntries[ m_nNumWaitHandles ], 0x00, sizeof( m_WaitEntries[ 0 ] ) );
    m_WaitEntries[ m_nNumWaitHandles ].hObj = hObj;
    m_WaitEntries[ m_nNumWaitHandles ].dwMessageId = dwMessageId;
    if( bRemoveAfterSignaled )
    {
        m_WaitEntries[ m_nNumWaitHandles ].dwFlags = WAIT_HANDLE_F_NONE;
    }
    else
    {
        m_WaitEntries[ m_nNumWaitHandles ].dwFlags = WAIT_HANDLE_F_NOREMOVE;
    }

    ++m_nNumWaitHandles;
    return TRUE;
}


//----------------------------------------------------------------------------------
// Name: RegisterWaitHandleFunc
// Desc: Registers a callback for when the given event fires.
//----------------------------------------------------------------------------------
BOOL CXuiWaitList::RegisterWaitHandleFunc( HANDLE hWait, PFN_WAITCOMPLETION pfnCompletion, void* pvContext,
                                           BOOL bRemoveAfterSignaled )
{
    ASSERT( m_nNumWaitHandles < MAXIMUM_WAIT_OBJECTS );
    if( m_nNumWaitHandles >= MAXIMUM_WAIT_OBJECTS )
        return FALSE;

    // Append to the wait handle array.
    m_WaitHandles[ m_nNumWaitHandles ] = hWait;

    memset( &m_WaitEntries[ m_nNumWaitHandles ], 0x00, sizeof( m_WaitEntries[ 0 ] ) );
    m_WaitEntries[ m_nNumWaitHandles ].pfnCompletionRoutine = pfnCompletion;
    m_WaitEntries[ m_nNumWaitHandles ].pvContext = pvContext;
    if( bRemoveAfterSignaled )
    {
        m_WaitEntries[ m_nNumWaitHandles ].dwFlags = WAIT_HANDLE_F_NONE;
    }
    else
    {
        m_WaitEntries[ m_nNumWaitHandles ].dwFlags = WAIT_HANDLE_F_NOREMOVE;
    }

    ++m_nNumWaitHandles;
    return TRUE;
}


//----------------------------------------------------------------------------------
// Name: ProcessWaitHandles
// Desc: Checks the list of registered wait handles.  If a handle is signaled, the 
//       message that was passed to RegisterWaitHandle is sent to the associated 
//       object.
//----------------------------------------------------------------------------------
void CXuiWaitList::ProcessWaitHandles()
{
    if( m_nNumWaitHandles < 1 )
        return;

    DWORD dwRet = WaitForMultipleObjects( m_nNumWaitHandles, m_WaitHandles, FALSE, 0 );
    if( dwRet >= WAIT_OBJECT_0 && dwRet <= WAIT_OBJECT_0 + ( DWORD )m_nNumWaitHandles )
    {
        DispatchWaitEntry( dwRet - WAIT_OBJECT_0 );
    }
    else if( dwRet >= WAIT_ABANDONED_0 && dwRet <= WAIT_ABANDONED_0 + ( DWORD )m_nNumWaitHandles )
    {
        DispatchWaitEntry( dwRet - WAIT_ABANDONED_0 );
    }
    else
    {
        ASSERT( dwRet == WAIT_TIMEOUT );
    }
}


//----------------------------------------------------------------------------------
// Name: DispatchWaitEntry
// Desc: Dispatches the registered entry at the specified index.
//       This will send the message and remove the handle from the list.
//----------------------------------------------------------------------------------
void CXuiWaitList::DispatchWaitEntry( int nIndex )
{
    ASSERT( nIndex >= 0 && nIndex < m_nNumWaitHandles );
    if( nIndex < 0 || nIndex >= m_nNumWaitHandles )
        return;

    HXUIOBJ hObj = m_WaitEntries[ nIndex ].hObj;
    DWORD dwMessage = m_WaitEntries[ nIndex ].dwMessageId;

    // Now remove the entry from the wait arrays.
    if( ( m_WaitEntries[ nIndex ].dwFlags & WAIT_HANDLE_F_NOREMOVE ) == 0 )
        RemoveWaitEntry( nIndex );

    if( m_WaitEntries[ nIndex ].hObj != NULL )
    {
        // Now send the message.
        XUIMessage msg;
        XuiMessage( &msg, dwMessage );
        XuiSendMessage( hObj, &msg );
    }
    else
    {
        m_WaitEntries[ nIndex ].pfnCompletionRoutine( m_WaitEntries[ nIndex ].pvContext );
    }
}


//----------------------------------------------------------------------------------
// Name: RemoveWaitEntry
// Desc: Removes the specified index from the wait list.
//----------------------------------------------------------------------------------
void CXuiWaitList::RemoveWaitEntry( int nIndex )
{
    if( nIndex < m_nNumWaitHandles - 1 )
    {
        memmove( &m_WaitHandles[ nIndex ], &m_WaitHandles[ nIndex + 1 ], sizeof( HANDLE ) *
                 ( m_nNumWaitHandles - nIndex - 1 ) );
        memmove( &m_WaitEntries[ nIndex ], &m_WaitEntries[ nIndex + 1 ], sizeof( WaitEntry ) *
                 ( m_nNumWaitHandles - nIndex - 1 ) );
    }
    --m_nNumWaitHandles;
}


//----------------------------------------------------------------------------------
// Name: UnregisterWaitHandle
// Desc: Removes the specified handle from the wait list.
//----------------------------------------------------------------------------------
void CXuiWaitList::UnregisterWaitHandle( HANDLE hWait )
{
    int nEntryIndex = -1;
    for( int i = 0; i < m_nNumWaitHandles; ++i )
    {
        if( m_WaitHandles[ i ] == hWait )
        {
            nEntryIndex = i;
            break;
        }
    }
    ASSERT( nEntryIndex >= 0 );
    if( nEntryIndex < 0 )
        return;

    RemoveWaitEntry( nEntryIndex );
}
