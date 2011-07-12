#include "embedhttp.h"

int handleFiles( ehttp &obj, void * cookie );
int handleIndex( ehttp &obj, void * cookie );
int handleLogin( ehttp &obj, void * cookie );
int handlePaths( ehttp &obj, void * cookie );
int handleGames( ehttp &obj, void * cookie );
int handleGameDetails( ehttp &obj, void * cookie );
int handleGameScreenshots( ehttp &obj, void * cookie );
int handleMemoryImage( ehttp &obj, void * cookie );
int handleGameAssets( ehttp &obj, void * cookie );

int handleAssetUpload( ehttp &obj, void * cookie );
int handleScreenshotUpload( ehttp &obj, void * cookie );
string getEntryContent(ehttp &obj, int nChunk);