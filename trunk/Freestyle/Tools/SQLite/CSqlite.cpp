#include <xtl.h>
#include <assert.h>
#include <stdio.h>

#include "sqlite3.h"
#include "sqlite3ext.h"
#define bool int

#define UNUSED_PARAMETER(x) (void)(x)
#define UNUSED_PARAMETER2(x,y) UNUSED_PARAMETER(x),UNUSED_PARAMETER(y)
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)

#define MX_DELETION_ATTEMPTS 5
#define MX_CLOSE_ATTEMPT 3

#define SimulateIOError
/*
** The winFile structure is a subclass of sqlite3_file* specific to the win32
** portability layer.
*/

struct cFile {
	const sqlite3_io_methods *pMethod;/* Must be first */
	HANDLE h;               /* Handle for accessing the file */
	unsigned char locktype; /* Type of lock currently held on this file */
	short sharedLockByte;   /* Randomly chosen byte used as a shared lock */
	DWORD lastErrno;        /* The Windows errno from the last I/O error */
	DWORD sectorSize;       /* Sector size of the device file is on */
};

typedef struct cFile cFile;

static int CSectorSize(sqlite3_file *id){
	return SQLITE_DEFAULT_SECTOR_SIZE;
}

static int CClose(sqlite3_file *id){
	int rc, cnt = 0;
	cFile *pFile = (cFile*)id;

//	DebugMsg("CSqlite.cpp", "CLOSE file handle %d\n", pFile->h);
	do{
		rc = CloseHandle(pFile->h);
	}
	while( rc==0 && ++cnt < MX_CLOSE_ATTEMPT && (Sleep(100), 1) );

	return SQLITE_OK;
}

static int CRead(
  sqlite3_file *id,          /* File to read from */
  void *pBuf,                /* Write content into this buffer */
  int amt,                   /* Number of bytes to read */
  sqlite3_int64 offset       /* Begin reading at this offset */
)
{
	LONG upperBits = (LONG)((offset>>32) & 0x7fffffff);
	LONG lowerBits = (LONG)(offset & 0xffffffff);
	DWORD rc;
	cFile *pFile = (cFile*)id;
	DWORD error;
	DWORD got;

	assert( id!=0 );
	//SimulateIOError(return SQLITE_IOERR_READ);
//	DebugMsg("Sqlite.cpp","READ %d lock=%d\n", pFile->h, pFile->locktype);
	rc = SetFilePointer(pFile->h, lowerBits, &upperBits, FILE_BEGIN);
	if( rc==INVALID_SET_FILE_POINTER && (error=GetLastError())!=NO_ERROR ){
//		DebugMsg("Sqlite.cpp","READ 1");
		pFile->lastErrno = error;
		return SQLITE_FULL;
	}
	if( !ReadFile(pFile->h, pBuf, amt, &got, 0) ){
//		DebugMsg("Sqlite.cpp","READ 2");
		pFile->lastErrno = GetLastError();
		return SQLITE_IOERR_READ;
	}
	if( got==(DWORD)amt ){
//		DebugMsg("Sqlite.cpp","READ 3 (okay)");
		return SQLITE_OK;
	}else{
//		DebugMsg("Sqlite.cpp","READ 4");
		/* Unread parts of the buffer must be zero-filled */
		memset(&((char*)pBuf)[got], 0, amt-got);
		return SQLITE_IOERR_SHORT_READ;
	}
}

static int CWrite(
  sqlite3_file *id,         /* File to write into */
  const void *pBuf,         /* The bytes to be written */
  int amt,                  /* Number of bytes to write */
  sqlite3_int64 offset      /* Offset into the file to begin writing at */
){
	LONG upperBits = (LONG)((offset>>32) & 0x7fffffff);
	LONG lowerBits = (LONG)(offset & 0xffffffff);
	DWORD rc;
	cFile *pFile = (cFile*)id;
	DWORD error;
	DWORD wrote = 0;

//	DebugMsg("Sqlite.cpp","WRITE %d lock=%d\n", pFile->h, pFile->locktype);
	rc = SetFilePointer(pFile->h, lowerBits, &upperBits, FILE_BEGIN);
	if( rc==INVALID_SET_FILE_POINTER && (error=GetLastError())!=NO_ERROR ){
		pFile->lastErrno = error;
		return SQLITE_FULL;
	}
	assert( amt>0 );
	while(
		amt>0
		&& (rc = WriteFile(pFile->h, pBuf, amt, &wrote, 0))!=0
		&& wrote>0
	)
	{
		amt -= wrote;
		pBuf = &((char*)pBuf)[wrote];
	}
	if( !rc || amt>(int)wrote ){
		pFile->lastErrno = GetLastError();
		return SQLITE_FULL;
	}
	return SQLITE_OK;
}

/*
** Truncate an open file to a specified size
*/
static int CTruncate(sqlite3_file *id, sqlite3_int64 nByte){
	LONG upperBits = (LONG)((nByte>>32) & 0x7fffffff);
	LONG lowerBits = (LONG)(nByte & 0xffffffff);
	DWORD rc;
	cFile *pFile = (cFile*)id;
	DWORD error;

//	DebugMsg("Sqlite.cpp","TRUNCATE %d %lld\n", pFile->h, nByte);
	rc = SetFilePointer(pFile->h, lowerBits, &upperBits, FILE_BEGIN);

	if( rc==INVALID_SET_FILE_POINTER && (error=GetLastError())!=NO_ERROR ){
		pFile->lastErrno = error;
		return SQLITE_IOERR_TRUNCATE;
	}

	/* SetEndOfFile will fail if nByte is negative */
	if( !SetEndOfFile(pFile->h) ){
		pFile->lastErrno = GetLastError();
		return SQLITE_IOERR_TRUNCATE;
	}
	return SQLITE_OK;
}

static int CSync(sqlite3_file *id, int flags){
	cFile *pFile = (cFile*)id;
//	DebugMsg("Sqlite.cpp","SYNC %d lock=%d\n", pFile->h, pFile->locktype);
	return SQLITE_OK;
}

/*
** Determine the current size of a file in bytes
*/
static int CFileSize(sqlite3_file *id, sqlite3_int64 *pSize){
	DWORD upperBits;
	DWORD lowerBits;
	cFile *pFile = (cFile*)id;
	DWORD error;

	assert( id!=0 );
	lowerBits = GetFileSize(pFile->h, &upperBits);
	if(   (lowerBits == INVALID_FILE_SIZE)
			&& ((error = GetLastError()) != NO_ERROR) )
	{
		pFile->lastErrno = error;
		return SQLITE_IOERR_FSTAT;
	}
	*pSize = (((sqlite3_int64)upperBits)<<32) + lowerBits;
	return SQLITE_OK;
}

/*
** Return a vector of device characteristics.
*/
static int CDeviceCharacteristics(sqlite3_file *id){
  UNUSED_PARAMETER(id);
  return 0;
}

/*
** Acquire a reader lock.
** Different API routines are called depending on whether or not this
** is Win95 or WinNT.
*/
static int getReadLock(cFile *pFile){
	int res=0;
//	DebugMsg("CSqlite.cpp", "getReadLock() on handle %d", pFile->h);
	return res;
}

/*
** Undo a readlock
*/
static int unlockReadLock(cFile *pFile){
  int res=0;
//  DebugMsg("CSqlite.cpp", "unlockReadLock() on handle %d", pFile->h);
  return res;
}


static int CLock(sqlite3_file *id, int locktype){
	int rc = SQLITE_OK;    /* Return code from subroutines */
	int res = 1;           /* Result of a windows lock call */
	//int newLocktype;       /* Set pFile->locktype to this value before exiting */
	int gotPendingLock = 0;/* True if we acquired a PENDING lock this time */
	cFile *pFile = (cFile*)id;
	DWORD error = NO_ERROR;

	assert( id!=0 );
//	DebugMsg("Sqlite.cpp","LOCK handle=%d new lockType %d was %d (lockbyte %d)\n",
//		  pFile->h, locktype, pFile->locktype, pFile->sharedLockByte);

	/* If there is already a lock of this type or more restrictive on the
	** OsFile, do nothing. Don't use the end_lock: exit path, as
	** sqlite3OsEnterMutex() hasn't been called yet.
	*/
	if( pFile->locktype >= locktype ){
		printf("Unlock ok!");
		return SQLITE_OK;
	}

//	DebugMsg("CSqlite.cpp", "Forcing the CLock to return SQLITE_OK");
	//force le lock
	return SQLITE_OK;
}

static int CCheckReservedLock(sqlite3_file *id, int *pResOut){
  return SQLITE_OK;
}

/*
** Lower the locking level on file descriptor id to locktype.  locktype
** must be either NO_LOCK or SHARED_LOCK.
**
** If the locking level of the file descriptor is already at or below
** the requested locking level, this routine is a no-op.
**
** It is not possible for this routine to fail if the second argument
** is NO_LOCK.  If the second argument is SHARED_LOCK then this routine
** might return SQLITE_IOERR;
*/
static int CUnlock(sqlite3_file *id, int locktype){
  int type;
  cFile *pFile = (cFile*)id;
  int rc = SQLITE_OK;
  assert( pFile!=0 );
  assert( locktype<=SHARED_LOCK );
//  DebugMsg("Sqlite.cpp","UNLOCK handle=%d new lockType %d was %d (lockbyte %d)\n",
//	pFile->h, locktype, pFile->locktype, pFile->sharedLockByte);
  type = pFile->locktype;
  if( type>=EXCLUSIVE_LOCK ){
    //UnlockFile(pFile->h, SHARED_FIRST, 0, SHARED_SIZE, 0);
    if( locktype==SHARED_LOCK && !getReadLock(pFile) ){
      /* This should never happen.  We should always be able to
      ** reacquire the read lock */
      rc = SQLITE_IOERR_UNLOCK;
    }
  }
  if( type>=RESERVED_LOCK ){
  //  UnlockFile(pFile->h, RESERVED_BYTE, 0, 1, 0);
  }
  if( locktype==NO_LOCK && type>=SHARED_LOCK ){
    unlockReadLock(pFile);
  }
  if( type>=PENDING_LOCK ){
   // UnlockFile(pFile->h, PENDING_BYTE, 0, 1, 0);
  }
  pFile->locktype = (u8)locktype;
  
  //return rc;
  return SQLITE_OK;
}

/*
** Control and query of the open file handle.
*/
static int CFileControl(sqlite3_file *id, int op, void *pArg){
  switch( op ){
    case SQLITE_FCNTL_LOCKSTATE: {
      *(int*)pArg = ((cFile*)id)->locktype;
      return SQLITE_OK;
    }
    case SQLITE_LAST_ERRNO: {
      *(int*)pArg = (int)((cFile*)id)->lastErrno;
      return SQLITE_OK;
    }
  }
  return SQLITE_ERROR;
}


static int CDelete(
  sqlite3_vfs *pVfs,          /* Not used on win32 */
  const char *zFilename,      /* Name of file to delete */
  int syncDir                 /* Not used on win32 */
){
	int cnt = 0;
	DWORD rc;
	DWORD error = 0;
	UNUSED_PARAMETER(pVfs);
	UNUSED_PARAMETER(syncDir);

	do{
	DeleteFileA(zFilename);
	}while(   
		(   ((rc = GetFileAttributesA(zFilename)) != INVALID_FILE_ATTRIBUTES)
		|| ((error = GetLastError()) == ERROR_ACCESS_DENIED))
		&& (++cnt < MX_DELETION_ATTEMPTS)
		&& (Sleep(100), 1) 
	);

//	DebugMsg("Sqlite.cpp","DELETE \"%s\"\n", zFilename);
	return (   (rc == INVALID_FILE_ATTRIBUTES) && (error == ERROR_FILE_NOT_FOUND)) ? SQLITE_OK : SQLITE_IOERR_DELETE;
}




static const sqlite3_io_methods CIoMethod = {
	1,                        /* iVersion */
	CClose,
	CRead,
	CWrite,
	CTruncate,
	CSync,
	CFileSize,
	CLock,
	CUnlock,//,
	CCheckReservedLock,//winCheckReservedLock,
	CFileControl,//winFileControl,
	CSectorSize,
	CDeviceCharacteristics
};


/*
** Get the sector size of the device used to store
** file.
*/
static int getSectorSize(
    sqlite3_vfs *pVfs,
    const char *zRelative     /* UTF-8 file name */
){
  DWORD bytesPerSector = SQLITE_DEFAULT_SECTOR_SIZE;

  return (int) bytesPerSector; 
}

/*
** Open a file.
*/
static int COpen(
  sqlite3_vfs *pVfs,        /* Not used */
  const char *zName,        /* Name of the file (UTF-8) */
  sqlite3_file *id,         /* Write the SQLite file handle here */
  int flags,                /* Open mode flags */
  int *pOutFlags            /* Status return flags */
)
{
	HANDLE h;
	DWORD dwDesiredAccess;
	DWORD dwShareMode=0;
	DWORD dwCreationDisposition=0;
	DWORD dwFlagsAndAttributes = flags;
	DWORD err;

	cFile *pFile = (cFile*)id;

	assert( id!=0 );
	UNUSED_PARAMETER(pVfs);

//	DebugMsg("CSqlite.cpp", "COpen for [%s]", zName);

	//creation du bon flags
	dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;

	if( flags & SQLITE_OPEN_READWRITE )
	{
		dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
	}
	else
	{
		dwDesiredAccess = GENERIC_READ;
	}
	if( flags & SQLITE_OPEN_EXCLUSIVE )
	{
		printf("CREATE_NEW\n");
		dwCreationDisposition = CREATE_NEW;
	}
	else if( flags & SQLITE_OPEN_CREATE )
	{
		printf("OPEN_ALWAYS\n");
		dwCreationDisposition = OPEN_ALWAYS;
	}
	else
	{
		printf("OPEN_EXISTING\n");
		dwCreationDisposition = OPEN_EXISTING;
	}

	if( flags & SQLITE_OPEN_DELETEONCLOSE ){
		//rien
	}
	else
	{
		dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
	}

	//setmethode
	pFile->pMethod = &CIoMethod;
	

	h = CreateFileA(
		zName,
		dwDesiredAccess,
		dwShareMode,
		NULL,
		dwCreationDisposition,
		dwFlagsAndAttributes,
		NULL
	);

	err = GetLastError();
	printf("CreateFile: %08x - %08x -%08x ",h,dwCreationDisposition,dwFlagsAndAttributes);

	if( h==INVALID_HANDLE_VALUE ){
		printf("Error opening file [%X]", err);
		if( flags & SQLITE_OPEN_READWRITE )
		{
			return COpen(pVfs, zName, id, 
				((flags|SQLITE_OPEN_READONLY)&~SQLITE_OPEN_READWRITE), pOutFlags);
		}
		else
		{
			printf("SQLITE_CANTOPEN_BKPT");
			return SQLITE_CANTOPEN_BKPT;
		}
	}

	//pfile
	memset(pFile, 0, sizeof(*pFile));
	pFile->pMethod = &CIoMethod;
	pFile->h = h;
	pFile->lastErrno = NO_ERROR;
	pFile->sectorSize = getSectorSize(pVfs, zName);
//	DebugMsg("Sqlite.cpp", "COpen assigned handle %d", h);
	return SQLITE_OK;
}

/*
** Turn a relative pathname into a full pathname.  Write the full
** pathname into zOut[].  zOut[] will be at least pVfs->mxPathname
** bytes in size.
*/
static int CFullPathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  const char *zRelative,        /* Possibly relative input path */
  int nFull,                    /* Size of output buffer in bytes */
  char *zFull                   /* Output buffer */
)
{
	sqlite3_snprintf(pVfs->mxPathname, zFull, "%s", zRelative);
	return SQLITE_OK;
}


/*
** Write up to nBuf bytes of randomness into zBuf.
*/
static int CRandomness(sqlite3_vfs *pVfs, int nBuf, char *zBuf){
	int n = 0;

	memset(zBuf, 0, nBuf);

	if( sizeof(SYSTEMTIME)<=nBuf-n ){
		SYSTEMTIME x;
		GetSystemTime(&x);
		memcpy(&zBuf[n], &x, sizeof(x));
		n += sizeof(x);
	}
	if( sizeof(DWORD)<=nBuf-n ){
		//DWORD pid = GetCurrentProcessId();
		DWORD pid = 145;
		memcpy(&zBuf[n], &pid, sizeof(pid));
		n += sizeof(pid);
	}
	if( sizeof(DWORD)<=nBuf-n ){
		DWORD cnt = GetTickCount();
		memcpy(&zBuf[n], &cnt, sizeof(cnt));
		n += sizeof(cnt);
	}
	if( sizeof(LARGE_INTEGER)<=nBuf-n ){
		LARGE_INTEGER i;
		QueryPerformanceCounter(&i);
		memcpy(&zBuf[n], &i, sizeof(i));
		n += sizeof(i);
	}
	return n;
}

/*
** Check the existance and status of a file.
*/
static int CAccess(
  sqlite3_vfs *pVfs,         /* Not used on win32 */
  const char *zFilename,     /* Name of file to check */
  int flags,                 /* Type of test to make on this file */
  int *pResOut               /* OUT: Result */
){
	DWORD attr;
	int rc = 0;
	UNUSED_PARAMETER(pVfs);

	attr = GetFileAttributesA((char*)zFilename);

	switch( flags ){
		case SQLITE_ACCESS_READ:
		case SQLITE_ACCESS_EXISTS:
			rc = attr!=INVALID_FILE_ATTRIBUTES;
			break;
		case SQLITE_ACCESS_READWRITE:
			rc = (attr & FILE_ATTRIBUTE_READONLY)==0;
			break;
		default:
			printf("Invalid flags argument");
	}
	*pResOut = rc;
	return SQLITE_OK;
}

SQLITE_API int sqlite3_os_init(void)
{
	static sqlite3_vfs CVfs = {
		1,                 /* iVersion */
		sizeof(cFile),		/* szOsFile */
		MAX_PATH,          /* mxPathname */
		0,                 /* pNext */
		"xbox",           /* zName */
		0,                 /* pAppData */
		COpen,           /* xOpen */
		CDelete,         /* xDelete */
		CAccess,         /* xAccess */
		CFullPathname,   /* xFullPathname */
		0,				/* xDlOpen */
		0,				/* xDlError */
		0,				 /* xDlSym */
		0,				/* xDlClose */
		CRandomness,     /* xRandomness */
		0,				/* xSleep */
		0,				/* xCurrentTime */
		0				/* xGetLastError */
	};

	sqlite3_vfs_register(&CVfs, 1);
	return SQLITE_OK; 
};

SQLITE_API int sqlite3_os_end(void)
{
	return SQLITE_OK;	
};

