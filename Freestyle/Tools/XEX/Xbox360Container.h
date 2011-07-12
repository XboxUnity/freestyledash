#pragma once

#include "../Generic/tools.h"
#include "../Generic/xboxtools.h"
#include "../Debug/Debug.h"
#include "XeXtractor.h"

#pragma warning(push)
#pragma warning(disable:4200 4201)

typedef unsigned __int64 u64;
typedef unsigned __int64 UINT64;
typedef unsigned int	 u32;
typedef unsigned short	 u16;

#define NxeImageSlot						0x0
#define NxeImageBackground					0x1
#define NxeImageBanner						0x2
#define NxeImageBoxart						0x3
#define NxeImageScreenshot1					0x4
#define NxeImageScreenshot2					0x5

#define LanguageEnglish						0
#define LanguageJapanese					1
#define LanguageGerman						2
#define LanguageFrench						3
#define LanguageSpanish						4
#define LanguageItalian						5
#define LanguageKorean						6
#define LanguageChinese						7
#define LanguageProtuguese					8
#define LanguageRussian						9
#define LanguagePolish						10

#pragma warning(push)
#pragma warning(disable:4005) // Get rid of the redefinition warning
#define XCONTENTTYPE_SAVEDGAME               0x1
#define XCONTENTTYPE_MARKETPLACE             0x2
#define XCONTENTTYPE_PUBLISHER               0x3
#define XCONTENTTYPE_IPTV_DVR                0x1000
#define XCONTENTTYPE_INSTALLED_XBOX360TITLE  0x4000
#define XCONTENTTYPE_XBOXTITLE               0x5000
#define XCONTENTTYPE_SOCIALTITLE             0x6000
#define XCONTENTTYPE_XBOX360TITLE            0x7000
#define XCONTENTTYPE_SU_STORAGEPACK          0x8000
#define XCONTENTTYPE_AVATAR_ASSET            0x9000
#define XCONTENTTYPE_PROFILE                 0x10000
#define XCONTENTTYPE_GAMERPICTURE            0x20000
#define XCONTENTTYPE_THEMATICSKIN            0x30000
#define XCONTENTTYPE_CACHE                   0x40000
#define XCONTENTTYPE_STORAGEDOWNLOAD         0x50000
#define XCONTENTTYPE_XBOXSAVEDGAME           0x60000
#define XCONTENTTYPE_XBOXDOWNLOAD            0x70000
#define XCONTENTTYPE_GAMEDEMO                0x80000
#define XCONTENTTYPE_VIDEO                   0x90000
#define XCONTENTTYPE_GAMETITLE               0xA0000
#define XCONTENTTYPE_INSTALLER               0xB0000
#define XCONTENTTYPE_GAMETRAILER             0xC0000
#define XCONTENTTYPE_ARCADE                  0xD0000
#define XCONTENTTYPE_XNA                     0xE0000
#define XCONTENTTYPE_LICENSESTORE            0xF0000
#define XCONTENTTYPE_MOVIE                   0x100000
#define XCONTENTTYPE_TV                      0x200000
#define XCONTENTTYPE_MUSICVIDEO              0x300000
#define XCONTENTTYPE_PROMOTIONAL             0x400000
#define XCONTENTTYPE_PODCASTVIDEO            0x500000
#define XCONTENTTYPE_VIRALVIDEO              0x600000
#pragma warning(pop)

/////////////////////////////////////////////////////////////////////////
// XCONTENT_HEADER
/////////////////////////////////////////////////////////////////////////
#pragma pack(1)
typedef enum _XCONTENT_SIGNATURE_TYPE {
	CONSOLE_SIGNED = 0x434F4E20,
	LIVE_SIGNED    = 0x4C495645,
	PIRS_SIGNED    = 0x50495253,
} XCONTENT_SIGNATURE_TYPE;

typedef struct _XE_CONSOLE_SIGNATURE {
	XE_CONSOLE_CERTIFICATE Cert;
	BYTE				   Signature[0x80];
} XE_CONSOLE_SIGNATURE, *PXE_CONSOLE_SIGNATURE;

typedef struct _XCONTENT_SIGNATURE {
	BYTE Signature[0x100];
	BYTE Reserved[0x128];
} XCONTENT_SIGNATURE, *PXCONTENT_SIGNATURE;

typedef struct _LICENSEE_BITS {
	UINT64 Type : 16;
	UINT64 Data : 48;
} LICENSEE_BITS, *PLICENSEE_BITS;

typedef struct _XCONTENT_LICENSE {
	union {
		LICENSEE_BITS Bits;
		ULONGLONG	  AsULONGLONG;
	}				  LicenseeId;
	DWORD			  LicenseBits;
	DWORD			  LicenseFlags;
} XCONTENT_LICENSE, *PXCONTENT_LICENSE;

typedef struct _XCONTENT_HEADER {
	XCONTENT_SIGNATURE_TYPE	 SignatureType;
	union {
		XE_CONSOLE_SIGNATURE ConsoleSignature;
		XCONTENT_SIGNATURE   ContentSignature;
	}						 Signature;
	XCONTENT_LICENSE		 LicenseDescriptors[0x10];
	BYTE					 ContentId[0x14];
	DWORD					 SizeOfHeaders;
} XCONTENT_HEADER, *PXCONTENT_HEADER;
#pragma pack()
/////////////////////////////////////////////////////////////////////////
// XCONTENT_METADATA
/////////////////////////////////////////////////////////////////////////

// Volume types
typedef enum _XCONTENT_VOLUME_TYPE {
	STFS_VOLUME = 0x00,
	SVOD_VOLUME = 0x01,
} XCONTENT_VOLUME_TYPE;

// STFS_VOLUME_DESCIPTOR
#pragma warning(disable:4201)
typedef struct _STF_VOLUME_DESCRIPTOR {
	BYTE	DescriptorLength;
	BYTE	Version;
	struct {
	  BYTE  ReadOnlyFormat         : 1;
	  BYTE  RootActiveIndex        : 1;
	  BYTE  DirectoryOverAllocated : 1;
	  BYTE  IndexBoundsValid       : 1;
	};
	BYTE	DirectoryAllocationBlocks0;
	BYTE	DirectoryAllocationBlocks1;
	BYTE	DirectoryFirstBlockNumber0;
	BYTE	DirectoryFirstBlockNumber1;
	BYTE	DirectoryFirstBlockNumber2;
	BYTE	RootHash[20];
	DWORD	NumberOfTotalBlocks;
	DWORD	NumberOfFreeBlocks;
} STFS_VOLUME_DESCIPTOR, *PSTFS_VOLUME_DESCRIPTOR;
#pragma warning(default:4201)

// SVOD_DEVICE_DESCRIPTOR
typedef struct _SVOD_HASH_ENTRY {
	BYTE Hash[0x14];
} SVOD_HASH_ENTRY, *PSVOD_HASH_ENTRY;

typedef struct _SVOD_DEVICE_FEATURES {
	BYTE MustBeZeroForFutureUsage		 : 6;
	BYTE HasEnhancedGDFLayout			 : 1;
	BYTE ShouldBeZeroForDownlevelClients : 1;
} SVOD_DEVICE_FEATURES, *PSVOD_DEVICE_FEATURES; 

typedef struct _SVOD_DEVICE_DESCRIPTOR {
	BYTE				 DescriptorLength;
	BYTE				 BlockCacheElementCount;
	BYTE				 WorkerThreadProcessor;
	BYTE				 WorkerThreadPriority;
	SVOD_HASH_ENTRY		 FirstFragmentHashEntry;
	SVOD_DEVICE_FEATURES Features;
	BYTE				 NumberOfDataBlocks2;
	BYTE				 NumberOfDataBlocks1;
	BYTE				 NumberOfDataBlocks0;
	BYTE				 StartingDataBlock0;
	BYTE				 StartingDataBlock1;
	BYTE				 StartingDataBlock2;
	BYTE				 Reserved[0x05];
} SVOD_DEVICE_DESCRIPTOR, *PSVOD_DEVICE_DESCRIPTOR;

// Meta data structures
typedef struct _XCONTENT_METADATA_MEDIA_DATA {
	BYTE SeriesId[0x10];
	BYTE SeasonId[0x10];
	WORD SeasonNumber;
	WORD EpisodeNumber;
} XCONTENT_METADATA_MEDIA_DATA, *PXCONTENT_METADATA_MEDIA_DATA;

typedef struct _XCONTENT_METADATA_AVATAR_ASSET_DATA {
	DWORD	SubCategory;
	INT		Colorizable;
	BYTE	AssetId[0x10];
	BYTE	SkeletonVersionMask;
	BYTE	Reserved[0xB];
} XCONTENT_METADATA_AVATAR_ASSET_DATA, *PXCONTENT_METADATA_AVATAR_ASSET_DATA;

typedef struct _XCONTENT_TRANSFERFLAGS {
	BYTE ProfileTransfer  : 1;
	BYTE DeviceTransfer	  : 1;
	BYTE MoveOnlyTransfer : 1;
	BYTE Reserved         : 5;
} XCONTENT_TRANSFERFLAGS, *PXCONTENT_TRANSFERFLAGS;

// XCONTENT_METADATA
#pragma pack(1)
typedef union _XCONTENT_METADATA_FLAGS {
	XCONTENT_TRANSFERFLAGS	Bits;
	BYTE					FlagsAsBYTE;
} XCONTENT_METADATA_FLAGS, *PXCONTENT_METADATA_FLAGS;

// XCONTENT_METADATA
typedef struct _XCONTENT_METADATA {
	DWORD						ContentType;
	DWORD						ContentMetadataVersion;
	LARGE_INTEGER				ContentSize;
	XEX_EXECUTION_ID			ExecutionId;
	BYTE						ConsoleId[0x05];
	XUID						Creator;
	union {
		STFS_VOLUME_DESCIPTOR	StfsVolumeDescriptor;
		SVOD_DEVICE_DESCRIPTOR	SvodVolumeDescriptor;
	};
	DWORD						DataFiles;
	UINT64						DataFilesSize;
	XCONTENT_VOLUME_TYPE		VolumeType;
	BYTE						Reserved2[0x2C];
	union {
		XCONTENT_METADATA_MEDIA_DATA		MediaData;
		XCONTENT_METADATA_AVATAR_ASSET_DATA AvatarAssetData;
	};
	BYTE						DeviceId[0x14];
	WCHAR						DisplayName[0x80][0x09];
	WCHAR						Description[0x80][0x09];
	WCHAR						Publisher[0x40];
	WCHAR						TitleName[0x40];
	XCONTENT_METADATA_FLAGS		Flags;		
	DWORD						ThumbnailSize;
	DWORD						TitleThumbnailSize;
	BYTE						Thumbnail[0x3D00];
	WCHAR						DisplayNameEx[0x80][0x03];
	BYTE						TitleThumbnail[0x3D00];
	WCHAR						DescriptionEx[0x80][0x03];
} XCONTENT_METADATA, *PXCONTENT_METADATA;
#pragma pack()

class Xbox360Container
{
private:

	BOOL isRetailCon;
public:

	BOOL IsRetailCon() { return isRetailCon; };
	
	string			   containerPath;
	string			   mountPath;
	FILE*			   fHandle;
	XCONTENT_HEADER*   pContainerHeader;
	XCONTENT_METADATA* pContainerMetaData;
	BOOL			   isMounted;

	BOOL IsMounted(){ return isMounted; }
	string MountPath(){ return mountPath; }

	Xbox360Container();
	~Xbox360Container();
	HRESULT OpenContainer(string ContainerPath);
	HRESULT CloseContainer();
	HRESULT GetDisplayName(string* DisplayName, int* Length, int Language = LanguageEnglish);
	wstring GetDisplayName(int Language = LanguageEnglish);
	HRESULT GetDescription(string* Description, int* Length, int Language = LanguageEnglish);
	wstring GetDescription(int Language = LanguageEnglish);
	HRESULT GetTitleName(string* TitleName, int* Length);
	wstring GetTitleName();
	HRESULT GetPublisher(string* Publisher, int* Length);
	wstring GetPublisher();
	HRESULT GetTitleId(DWORD* TitleId);
	HRESULT GetMediaId(u32* MediaId);
	HRESULT GetDiscNumbers(byte* CurrentDisc, byte* TotalDiscs);
	HRESULT GetContentType(DWORD* ContentType);
	HRESULT GetThumbnail(void** Data, int* Size);
	HRESULT GetTitleThumbnail(void** Data, int* Size);
	HRESULT Mount(string MountPath = "con");
	HRESULT UnMount(bool bSaveChanges = false);
	HRESULT ExtractNXE(string destFilePath, DWORD nxeImage);

	HRESULT WriteDWORDToOffset(DWORD Value, fstream::pos_type Offset);
	HRESULT ReHash();
	HRESULT ConvertNXE2GOD(string& ItemDirectory);

	void LaunchGame();
};

#pragma warning(pop)