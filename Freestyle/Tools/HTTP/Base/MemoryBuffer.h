//--------------------------------------------------------------------------------------
// MemoryBuffer.h
//
// XNA Developer Connection.
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once
#ifndef MemoryBuffer_H
#define MemoryBuffer_H

#include "stdafx.h"

//-----------------------------------------------------------------------------
// Internal buffer length
//-----------------------------------------------------------------------------
#define INIT_BUFFER_SIZE            512

//--------------------------------------------------------------------------------------
// Name: class MemoryBuffer
// Desc: Memory buffer, automatically expands as needed to hold more data
//--------------------------------------------------------------------------------------
class MemoryBuffer
{
public:

    MemoryBuffer( DWORD dwSize = INIT_BUFFER_SIZE ) {

        m_pBuffer = NULL;
        m_dwDataLength = 0;
        m_dwBufferSize = 0;

        if( ( dwSize < UINT_MAX ) && ( dwSize != 0 ) )
        {
			// Allocate one more char, in case when using string funcions
            m_pBuffer = ( BYTE* )malloc( dwSize + 1 );
            if( m_pBuffer )
            {
                m_dwBufferSize = dwSize;
                m_pBuffer[0] = 0;
            }
        }
    };

    ~MemoryBuffer() {

        if( m_pBuffer)
            free( m_pBuffer );

        m_pBuffer = NULL;
        m_dwDataLength = 0;
        m_dwBufferSize = 0;
    };

    // Add chunk of memory to buffer
    BOOL Add( const void* p, DWORD dwSize ) {

        if( CheckSize( dwSize ) ) {

            memcpy( m_pBuffer + m_dwDataLength, p, dwSize );
            m_dwDataLength += dwSize;
            *( m_pBuffer + m_dwDataLength ) = 0;    // fill end zero
            return TRUE;
        }
        else {

            return FALSE;
        }
    };

    // Get the data in buffer
    BYTE* GetData() const {

        return m_pBuffer;
    };

    // Get the lenght of data in buffer
    DWORD GetDataLength() const {

        return m_dwDataLength;
    };

    // Rewind the data pointer to the begining
    void    Rewind() {

        m_dwDataLength = 0; m_pBuffer[ 0 ] = 0;
    };

private:

    BYTE* m_pBuffer;

    DWORD m_dwDataLength;

    DWORD m_dwBufferSize;

    // Automatically adjust increase buffer size if necessary
    BOOL CheckSize( DWORD dwSize ) {

        if( m_dwBufferSize >= ( m_dwDataLength + dwSize ) )
        {
            return TRUE;    // Enough space
        }
        else
        {
            // Try to double it
            DWORD dwNewSize = max( m_dwDataLength + dwSize, m_dwBufferSize * 2 );
            BYTE* pNewBuffer = ( UCHAR* )realloc( m_pBuffer, dwNewSize + 1 );        // one more char
            if( pNewBuffer )
            {
                m_pBuffer = pNewBuffer;
                m_dwBufferSize = dwNewSize;
                return TRUE;
            }
            else
            {
                // Failed
                return FALSE;
            }
        }
    }
};

#endif