#pragma once

#ifdef SKYRIM_SUPPORT_AE

#define USE_RELID true

#define WalkerID 70050
#define GetSizeID 70164

#define EventSourcesID 410419
#define CacheID 410420

#define StripPrefixID 69864
#define InsertNoCacheID 69687
#define InsertWithCacheID 69805
#define UpdateCacheID 69939

#elif SKYRIM_SUPPORTS_VR

#define USE_RELID false

#define WalkerID 0xC92580
#define GetSizeID 0xC96E00

#define EventSourcesID 0x3169BC0
#define CacheID 0x3169BC8

#define StripPrefixID 0xC8ABB0
#define InsertNoCacheID 0xC83390
#define InsertWithCacheID 0xC88E50
#define UpdateCacheID 0xC8D660

#else

#define USE_RELID true

#define WalkerID 68707
#define GetSizeID 68812

#define EventSourcesID 523853
#define CacheID 523854

#define StripPrefixID 68509
#define InsertNoCacheID 68327
#define InsertWithCacheID 68452
#define UpdateCacheID 68597

#endif