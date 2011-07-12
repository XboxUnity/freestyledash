////////////////////////////////////////////////////////////////////
//
// FreestyleDash.spa.h
//
// Auto-generated on Tuesday, 20 July 2010 at 10:25:15
// XLAST project version 1.0.34.0
// SPA Compiler version 2.0.9328.0
//
////////////////////////////////////////////////////////////////////

#ifndef __FREESTYLE_DASH_V2_0_SPA_H__
#define __FREESTYLE_DASH_V2_0_SPA_H__

#ifdef __cplusplus
extern "C" {
#endif

//
// Title info
//

#define TITLEID_FREESTYLE_DASH_V2_0                 0xF5D20000

//
// Context ids
//
// These values are passed as the dwContextId to XUserSetContext.
//


//
// Context values
//
// These values are passed as the dwContextValue to XUserSetContext.
//

// Values for X_CONTEXT_PRESENCE

#define CONTEXT_PRESENCE_FSD                        0

// Values for X_CONTEXT_GAME_MODE

#define CONTEXT_GAME_MODE_EASY                      0

//
// Property ids
//
// These values are passed as the dwPropertyId value to XUserSetProperty
// and as the dwPropertyId value in the XUSER_PROPERTY structure.
//


//
// Achievement ids
//
// These values are used in the dwAchievementId member of the
// XUSER_ACHIEVEMENT structure that is used with
// XUserWriteAchievements and XUserCreateAchievementEnumerator.
//


//
// AvatarAssetAward ids
//


//
// Stats view ids
//
// These are used in the dwViewId member of the XUSER_STATS_SPEC structure
// passed to the XUserReadStats* and XUserCreateStatsEnumerator* functions.
//

// Skill leaderboards for ranked game modes

#define STATS_VIEW_SKILL_RANKED_EASY                0xFFFF0000

// Skill leaderboards for unranked (standard) game modes

#define STATS_VIEW_SKILL_STANDARD_EASY              0xFFFE0000

// Title defined leaderboards


//
// Stats view column ids
//
// These ids are used to read columns of stats views.  They are specified in
// the rgwColumnIds array of the XUSER_STATS_SPEC structure.  Rank, rating
// and gamertag are not retrieved as custom columns and so are not included
// in the following definitions.  They can be retrieved from each row's
// header (e.g., pStatsResults->pViews[x].pRows[y].dwRank, etc.).
//

//
// Matchmaking queries
//
// These values are passed as the dwProcedureIndex parameter to
// XSessionSearch to indicate which matchmaking query to run.
//


//
// Gamer pictures
//
// These ids are passed as the dwPictureId parameter to XUserAwardGamerTile.
//



#ifdef __cplusplus
}
#endif

#endif // __FREESTYLE_DASH_V2_0_SPA_H__


