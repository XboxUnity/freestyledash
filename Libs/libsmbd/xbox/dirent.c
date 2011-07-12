/*

    Implementation of POSIX directory browsing functions and types for Win32.

    Author:  Kevlin Henney (kevlin@acm.org, kevlin@curbralan.com)
    History: Created March 1997. Updated June 2003.
    Rights:  See end of file.

*/

#include <dirent.h>
#include <errno.h>
#include <io.h> /* _findfirst and _findnext set errno iff they return -1 */
#include <stdlib.h>
#include <string.h>

#include "includes.h"


#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _XBOXZZZ
int try_open_as_dir (const char *path)
{
	HANDLE osfh;                    /* OS handle of opened file */
	return CreateFile( (LPTSTR)path, 
							 GENERIC_READ,
							 FILE_SHARE_READ | FILE_SHARE_WRITE,
							 NULL,
							 OPEN_EXISTING ,
							 FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS,
							 NULL );
}
#endif


DIR *opendir(const char *name)
{
    DIR *dir = 0;

    if(name && name[0])
    {
        size_t base_length = strlen(name);
#ifdef _XBOX
        const char *all = /* search pattern must end with suitable wildcard */
            strchr("/\\", name[base_length - 1]) ? "*" : "\\*";
#else
        const char *all = /* search pattern must end with suitable wildcard */
            strchr("/\\", name[base_length - 1]) ? "*" : "/*";
#endif
        if((dir = (DIR *) malloc(sizeof *dir)) != 0 &&
           (dir->name = (char *) malloc(base_length + strlen(all) + 1)) != 0)
        {
            strncat(strncpy(dir->name, name, base_length+1), all, base_length+strlen(all)+1);

            if((dir->handle = (long) _findfirst(dir->name, &dir->info)) != -1)
            {
                dir->result.d_name = 0;
            }
            else /* rollback */
            {
				// but is it an empty directory?  this can't happen on unix because of . and ..
				int res = try_open_as_dir(name);
				if (res !=  INVALID_HANDLE_VALUE) {
					// it is a valid dir...
					dir->result.d_name = 0;
					dir->handle = -1;
					CloseHandle(res);
				}
				else {				
		            free(dir->name);
			        free(dir);
				    dir = 0;
				}
            }
        }
        else /* rollback */
        {
            free(dir);
            dir   = 0;
            errno = ENOMEM;
        }
    }
    else
    {
        errno = EINVAL;
    }

    return dir;
}

int closedir(DIR *dir)
{
    int result = -1;

    if(dir)
    {
        if(dir->handle != -1)
        {
            result = _findclose(dir->handle);
        }

        free(dir->name);
        free(dir);
    }

    if(result == -1) /* map all errors to EBADF */
    {
        errno = EBADF;
    }

    return result;
}

struct dirent *readdir(DIR *dir)
{
    struct dirent *result = 0;

    if(dir && dir->handle != -1)
    {
        if(!dir->result.d_name || _findnext(dir->handle, &dir->info) != -1)
        {
            result         = &dir->result;
            result->d_name = dir->info.name;
        }
    }
    else
    {
        errno = EBADF;
    }

    return result;
}

void rewinddir(DIR *dir)
{
    if(dir && dir->handle != -1)
    {
        _findclose(dir->handle);
        dir->handle = (long) _findfirst(dir->name, &dir->info);
        dir->result.d_name = 0;
    }
    else
    {
        errno = EBADF;
    }
}

long telldir(DIR *dir)
{
    return 0;
}

#ifdef __cplusplus
}
#endif

/*

    Copyright Kevlin Henney, 1997, 2003. All rights reserved.

    Permission to use, copy, modify, and distribute this software and its
    documentation for any purpose is hereby granted without fee, provided
    that this copyright and permissions notice appear in all copies and
    derivatives.
    
    This software is supplied "as is" without express or implied warranty.

    But that said, if there are any problems please get in touch.

*/
