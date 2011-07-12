#pragma once

#include "../Generic/tools.h"
#include "../Generic/xboxtools.h"
#include "../Debug/Debug.h"
#include "../Generic/KernelExports.h"

#pragma warning(push)
#pragma warning(disable:4200 4201)

#define CONSOLETYPE_DEVKIT  0x0001
#define CONSOLETYPE_RETAIL	0x0002

/* ESRB Ratings */
#define RATING_ESRB						  0x00 // Index
#define RATING_ESRB_EC					  0x00
#define RATING_ESRB_E					  0x02
#define RATING_ESRB_E10Plus				  0x04
#define RATING_ESRB_T					  0x06
#define RATING_ESRB_M					  0x08
#define RATING_ESRB_RP					  0xFF

#define IMAGE_XEX_HEADER_MAGIC			  0x58455832

#define DIRECTORY_KEY_SECTION			  0x000002FF
#define DIRECTORY_KEY_BASEFILE_DESCRIPTOR 0x000003FF
#define DIRECTORY_KEY_EXECUTION_ID		  0x00040006
#define DIRECTORY_KEY_GAME_RATINGS		  0x00040310
#define DIRECTORY_KEY_SYSTEM_FLAGS_EX	  0x00030100

// Current XEX Privilieges as of 12611
#define  XEX_PRIVILEGE_NO_FORCE_REBOOT                             0
#define  XEX_PRIVILEGE_FOREGROUND_TASKS                            1
#define  XEX_PRIVILEGE_NO_ODD_MAPPING                              2
#define  XEX_PRIVILEGE_HANDLE_MCE_INPUT                            3
#define  XEX_PRIVILEGE_RESTRICT_HUD_FEATURES                       4
#define  XEX_PRIVILEGE_HANDLE_GAMEPAD_DISCONNECT                   5
#define  XEX_PRIVILEGE_INSECURE_SOCKETS                            6
#define  XEX_PRIVILEGE_RESERVED_7          /*(Xbox1 XSP interop)*/ 7
#define  XEX_PRIVILEGE_SET_DASH_CONTEXT                            8
#define  XEX_PRIVILEGE_TITLE_USES_GAME_VOICE_CHANNEL               9
#define  XEX_PRIVILEGE_TITLE_PAL50_INCOMPATIBLE                    10
#define  XEX_PRIVILEGE_TITLE_INSECURE_UTILITYDRIVE                 11
#define  XEX_PRIVILEGE_TITLE_XAM_HOOKS                             12
#define  XEX_PRIVILEGE_TITLE_PII                                   13
#define  XEX_PRIVILEGE_CROSSPLATFORM_SYSTEM_LINK                   14
#define  XEX_PRIVILEGE_MULTIDISC_SWAP                              15
#define  XEX_PRIVILEGE_MULTIDISC_INSECURE_MEDIA                    16
#define  XEX_PRIVILEGE_AUTHENTICATION_EX_REQUIRED /*(AP25 Media)*/ 17
#define  XEX_PRIVILEGE_NO_CONFIRM_EXIT                             18
#define  XEX_PRIVILEGE_ALLOW_BACKGROUND_DOWNLOAD                   19
#define  XEX_PRIVILEGE_CREATE_PERSISTABLE_RAMDRIVE                 20
#define  XEX_PRIVILEGE_INHERIT_PERSISTED_RAMDRIVE                  21
#define  XEX_PRIVILEGE_ALLOW_HUD_VIBRATION                         22
#define  XEX_PRIVILEGE_TITLE_BOTH_UTILITY_PARTITIONS               23
#define  XEX_PRIVILEGE_HANDLE_IPTV_INPUT                           24
#define  XEX_PRIVILEGE_PREFER_BIGBUTTON_INPUT                      25
#define  XEX_PRIVILEGE_RESERVED_26                                 26
#define  XEX_PRIVILEGE_MULTIDISC_CROSS_TITLE                       27
#define  XEX_PRIVILEGE_TITLE_INSTALL_INCOMPATIBLE                  28
#define  XEX_PRIVILEGE_ALLOW_AVATAR_GET_METADATA_BY_XUID           29
#define  XEX_PRIVILEGE_ALLOW_CONTROLLER_SWAPPING                   30
#define  XEX_PRIVILEGE_DASH_EXTENSIBILITY_MODULE                   31

#define  XEX_PRIVILEGE_ALLOW_NETWORK_READ_CANCEL                   32
#define  XEX_PRIVILEGE_UNINTERRUPTABLE_READS                       33
#define  XEX_PRIVILEGE_REQUIRE_EXPERIENCE_FULL                     34
#define  XEX_PRIVILEGE_GAMEVOICE_REQUIRED_UI                       35
#define  XEX_PRIVILEGE_TITLE_SET_PRESENCE_STRING                   36
#define  XEX_PRIVILEGE_NATAL_TILT_CONTROL                          37
#define  XEX_PRIVILEGE_TITLE_REQUIRES_SKELETAL_TRACKING            38
#define  XEX_PRIVILEGE_TITLE_SUPPORTS_SKELETAL_TRACKING            39

#define FILE_DATA_FORMAT_NOT_COMPRESSED   0x0001
#define FILE_DATA_FORMAT_COMPRESSED       0x0002
#define FILE_DATA_FORMAT_DELTA_COMPRESSED 0x0003

#define FILE_DATA_FLAGS_ENCRYPTED		  0x0001

#define IMAGE_FLAG_PAGE_SIZE_4KB		  0x10000000 

const BYTE RetailKey[] = { 
	0x20, 0xB1, 0x85, 0xA5, 0x9D, 0x28, 0xFD, 0xC3, 
	0x40, 0x58, 0x3F, 0xBB, 0x08, 0x96, 0xBF, 0x91 };
const BYTE DevkitKey[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
const BYTE XexSalt[] = "XBOX360XEXXBOXREVXEX";

typedef struct _IMAGE_XEX_HEADER {
	DWORD				Magic;
	DWORD				ModuleFlags;
	DWORD				SizeOfHeaders;
	DWORD				SizeOfDiscardableHeaders;
	DWORD				SecurityInfo;
	DWORD				HeaderDirectoryEntryCount;
} IMAGE_XEX_HEADER, *PIMAGE_XEX_HEADER;

typedef struct _IMAGE_XEX_DIRECTORY_ENTRY {
	DWORD	Key;
	DWORD	Value;
} IMAGE_XEX_DIRECTORY_ENTRY, *PIMAGE_XEX_DIRECTORY_ENTRY;

typedef struct _HV_IMAGE_INFO {
	BYTE	Signature[0x100];
	DWORD	InfoSize;
	DWORD	ImageFlags;
	DWORD	LoadAddress;
	BYTE	ImageHash[0x14];
	DWORD	ImportTableCount;
	BYTE	ImportDigest[0x14];
	BYTE	MediaID[0x10];
	BYTE	ImageKey[0x10];
	DWORD	ExportTableAddress;
	BYTE	HeaderHash[0x14];
	DWORD	GameRegion;
} HV_IMAGE_INFO, *PHV_IMAGE_INFO;

typedef struct _XEX_SECURITY_INFO {
	DWORD			Size;
	DWORD			ImageSize;
	HV_IMAGE_INFO	ImageInfo;
	DWORD			AllowedMediaTypes;
	DWORD			PageDescriptorCount;
} XEX_SECURITY_INFO, *PXEX_SECURITY_INFO;

typedef struct _PAGE_DESCIPTION {
	DWORD	Size : 28;
	DWORD	Info : 4;
} PAGE_DESCIPTION, *PPAGE_DESCIPTION;
typedef struct _HV_PAGE_INFO {
	PAGE_DESCIPTION PageDescription;
	BYTE		    DataDigest[0x14];
} HV_PAGE_INFO, *PHV_PAGE_INFO;

typedef struct _XEX_SECTION_HEADER {
	CHAR	SectionName[0x8];
	DWORD	VirtualAddress;
	DWORD	VirtualSize;
} XEX_SECTION_HEADER, *PXEX_SECTION_HEADER;

typedef struct _XEX_SECTION_INFO {
	DWORD			   Size;
	XEX_SECTION_HEADER Section[1];
} XEX_SECTION_INFO, *PXEX_SECTION_INFO;

typedef struct _XEX_TLS_DATA {
	DWORD TlsSlotCount;
	PVOID AddressOfRawData;
	DWORD SizeOfRawData;
	DWORD SizeOfTlsData;
} XEX_TLS_DATA, *PXEX_TLS_DATA;

typedef struct _XEX_GAME_RATINGS {
	BYTE	Ratings[0x40];
} XEX_GAME_RATINGS, *PXEX_GAME_RATINGS;

typedef struct _XEX_LAN_KEY {
	BYTE	Key[0x10];
} XEX_LAN_KEY, *PXEX_LAN_KEY;

typedef struct _XEX_FILE_DATA_DESCRIPTOR {
	DWORD	Size;
	WORD	Flags;
	WORD	Format;
} XEX_FILE_DATA_DESCRIPTOR, *PXEX_FILE_DATA_DESCRIPTOR;

typedef struct _XEX_RAW_DATA_DESCRIPTOR {
	DWORD DataSize;
	DWORD ZeroSize;
} XEX_RAW_DATA_DESCRIPTOR, *PXEX_RAW_DATA_DESCRIPTOR;

typedef struct _XEX_DATA_DESCRIPTOR {
	DWORD	Size;
	BYTE	DataDigest[0x14];
} XEX_DATA_DESCRIPTOR, *PXEX_DATA_DESCRIPTOR;

typedef struct _XEX_COMPRESSED_DATA_DESCRIPTOR {
	DWORD				WindowSize;
	XEX_DATA_DESCRIPTOR	FirstDescriptor;
} XEX_COMPRESSED_DATA_DESCRIPTOR, *PXEX_COMPRESSED_DATA_DESCRIPTOR;

typedef struct _XEX_EXECUTION_ID {
	DWORD		 MediaID;
	DWORD		 Version;
	DWORD		 BaseVersion;
	union {
		struct {
			WORD PublisherID;
			WORD GameID;
		};
		DWORD	 TitleID;
	};
	BYTE		 Platform;
	BYTE		 ExecutableType;
	BYTE		 DiscNum;
	BYTE		 DiscsInSet;
	DWORD		 SaveGameID;
} XEX_EXECUTION_ID, *PXEX_EXECUTION_ID;

class XeXtractor
{
private :	
	
	HRESULT GetOptionalHeader(DWORD OptHeaderKey, PVOID* Data);
	HRESULT CalculateDecryptKey(PBYTE DecryptKey);

public :

	XeXtractor();
	~XeXtractor();
	XeXtractor(const XeXtractor&);
	XeXtractor& operator=(const XeXtractor&);

	static XeXtractor& getInstance(){
		static XeXtractor singleton;
		return singleton;
	}

	FILE*						fHandle;
	BOOL						isOpen;
	BOOL						isRetailXex;

	PBYTE						FullHeader;
	PIMAGE_XEX_HEADER			Header;
	PIMAGE_XEX_DIRECTORY_ENTRY	DirectoryEntries;
	PXEX_SECURITY_INFO			SecurityInfo;
	PHV_PAGE_INFO				PageInfo;

	BOOL IsRetailXex() { return isRetailXex; };
	BOOL IsKinectEnabled();
	HRESULT OpenXex(string XexPath);
	HRESULT CloseXex();
	HRESULT PrintInfo();
	HRESULT GetBaseFile(PBYTE* Basefile, PDWORD Size);
	HRESULT ExtractResource(string ResourceName, PBYTE* Resouce, PDWORD Size);
	HRESULT ExtractTitleSPA(PBYTE* Resource, PDWORD Size);
	HRESULT GetExecutionId(PXEX_EXECUTION_ID ExecutionId);
	HRESULT GetRating(BYTE RatingType, PBYTE Rating);
	BOOL CheckExecutablePrivilege(DWORD Privilege);
};

#pragma warning(pop)