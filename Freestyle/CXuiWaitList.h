#pragma once
//----------------------------------------------------------------------------------
// Name: CXuiWaitList
// Desc: Class used to wait on handles.  When a handle becomes signalled, a 
//       user-specified action (send specified XUI message or call specified 
//       callback function) is taken.
//----------------------------------------------------------------------------------
typedef void (*PFN_WAITCOMPLETION )( void* pvContext );
class CXuiWaitList
{

protected:

    enum
    {
        WAIT_HANDLE_F_NONE      = 0,
        WAIT_HANDLE_F_NOREMOVE  = 1,
    };

    struct WaitEntry
    {
        HXUIOBJ hObj;
        DWORD dwMessageId;
        PFN_WAITCOMPLETION pfnCompletionRoutine;
        void* pvContext;
        DWORD dwFlags;
    };

    int m_nNumWaitHandles;
    HANDLE      m_WaitHandles[ MAXIMUM_WAIT_OBJECTS ];
    WaitEntry   m_WaitEntries[ MAXIMUM_WAIT_OBJECTS ];

    //------------------------------------------------------------------------------
    // Dispatches the registered entry at the specified index.
    //------------------------------------------------------------------------------
    void        DispatchWaitEntry( int nIndex );

    //------------------------------------------------------------------------------
    // Removes the specified index from the wait list.
    //------------------------------------------------------------------------------
    void        RemoveWaitEntry( int nIndex );

public:

    //------------------------------------------------------------------------------
    // Registers a handle for async operations.
    //------------------------------------------------------------------------------
    BOOL        RegisterWaitHandle( HANDLE hWait, HXUIOBJ hObj, DWORD dwMessageId, BOOL bRemoveAfterSignaled );

    //------------------------------------------------------------------------------
    // Registers a callback for when the given event fires.
    //------------------------------------------------------------------------------
    BOOL        RegisterWaitHandleFunc( HANDLE hWait, PFN_WAITCOMPLETION pfnCompletion, void* pvContext,
                                        BOOL bRemoveAfterSignaled );

    //------------------------------------------------------------------------------
    // Removes the specified handle from the wait list.
    //------------------------------------------------------------------------------
    void        UnregisterWaitHandle( HANDLE hWait );

    //------------------------------------------------------------------------------
    // Checks the list of registered wait handles.
    //------------------------------------------------------------------------------
    void        ProcessWaitHandles();

};
