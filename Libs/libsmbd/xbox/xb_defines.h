#ifndef XBOX_SMB__DEFINES_H
#define XBOX_SMB__DEFINES_H

#define SHLIBEXT "so"											// not used on xbox
#define LIBDIR "GAME:"												// not used on xbox
#define CONFIGFILE "GAME:\\smb.conf"
#define LMHOSTSFILE "GAME:\\lmhosts"
#define LOGFILEBASE "GAME:"
#define SWATDIR "GAME:\\web\\swat"						// not used on xbox
#define BINDIR "GAME:"												// not used on xbox
#define SBINDIR "GAME:"											// not used on xbox
#define SMB_PASSWD_FILE "GAME:\\passwd"
#define PRIVATE_DIR "GAME:"
#define LOCKDIR "GAME:"
#define PIDDIR "GAME:"
 /*
typedef struct SMB_SYSTEMTIME {
    WORD year;
    WORD month;
    WORD dayofweek;
    WORD day;
    WORD hour;
    WORD minute;
    WORD second;
    WORD milliseconds;
} SMB_SYSTEMTIME;
*/

/* Globally Unique ID *
#define GUID_SIZE 16
typedef struct guid_info
{
	uint32 info[GUID_SIZE];
} uuid;
*/

#define uint64_t unsigned __int64
#define uint32_t unsigned __int32
#define uint16_t unsigned __int16
#define uint8_t unsigned __int8
#define uint_t unsigned int

#define uint64 unsigned __int64
#define uint32 unsigned __int32
#define uint16 unsigned __int16
#define uint8 unsigned __int8
#define uint unsigned int

struct group {
	char *gr_name;
	char *gr_passwd;
	int gr_gid;
	char **gr_mem;
};

struct hostent {
        char    FAR * h_name;           /* official name of host */
        char    FAR * FAR * h_aliases;  /* alias list */
        short   h_addrtype;             /* host address type */
        short   h_length;               /* length of address */
        char    FAR * FAR * h_addr_list; /* list of addresses */
#define h_addr  h_addr_list[0]          /* address, for backward compat */
};

typedef struct servent {
		char FAR* s_name;
		char FAR  FAR** s_aliases;
		short s_port;
		char FAR* s_proto;
} servent;

typedef struct protoent {
		char FAR* p_name;
		char FAR  FAR** p_aliases;
		short p_proto;
} protoent;

// defines for fcntl.
#define F_UNLCK				0x0002
#define F_RDLCK				0x0004
#define F_WRLCK				0x0008

#define O_NONBLOCK		0x0010

// ? #define F_SETLK				0x0008
#define F_SETLK 13
#define F_SETLKW 14

#define SO_KEEPALIVE	0x0008

#define WNOHANG				0x00000001 

//!!!!!!!!!
#define SIGCHLD 20
#define SIGUSR1 30
#define SIGALRM 40

//#define EISCONN WSAEISCONN
//#define EINPROGRESS WSAEINPROGRESS
////#define EALREADY WSAEALREADY
//#define EADDRINUSE WSAEADDRINUSE
//#define ENOBUFS WSAENOBUFS
//#define ENETUNREACH WSAENETUNREACH
//#define EHOSTUNREACH WSAEHOSTUNREACH
//#define ECONNABORTED WSAECONNABORTED
//#define ECONNREFUSED WSAECONNREFUSED
#define ESHUTDOWN WSAESHUTDOWN
//#define ENETRESET WSAENETRESET
//#define EMSGSIZE WSAEMSGSIZE
//#define ENOPROTOOPT WSAENOPROTOOPT
//#define ETIMEDOUT WSAETIMEDOUT
//#define ENOTSUP WSAEOPNOTSUPP

#define stat64(fname, sbuf) _stat64(fname, sbuf)
#define fstat64(fd, sbuf) _fstat64(fd, sbuf)
#define fseek64(fp, offset, whence) _fseek64(fp, offset, whence)
#define lseek64(fp, offset, whence) _lseeki64(fp, offset, whence)

#define LOG_USER 0
#define LOG_LOCAL0 1
#define LOG_LOCAL1 2
#define LOG_LOCAL2 3
#define LOG_LOCAL3 4
#define LOG_LOCAL4 5
#define LOG_LOCAL5 6
#define LOG_LOCAL6 7
#define LOG_LOCAL7 8
#define LOG_PID 9

#endif //XBOX_SMB__DEFINES_H