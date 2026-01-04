/***************************************************************************
 * Module:	AOSL common definitions header file
 *
 * Copyright © 2025 Agora
 * This file is part of AOSL, an open source project.
 * Licensed under the Apache License, Version 2.0, with certain conditions.
 * Refer to the "LICENSE" file in the root directory for more information.
 ***************************************************************************/

#ifndef __AOSL_DEFS_H__
#define __AOSL_DEFS_H__


#define aosl_stringify_1(x) #x
#define aosl_stringify(x) aosl_stringify_1(x)


#ifndef __MAKERCORE_ASSEMBLY__

#ifdef __cplusplus
extern "C" {
#endif



#ifndef container_of
#if defined (__GNUC__)
#define container_of(ptr, type, member) ({ \
	const typeof( ((type *)0)->member ) *__mptr = (void *)(ptr); \
	(type *)( (char *)__mptr - offsetof(type,member) );})
#else
#define container_of(ptr, type, member) ((type *)((char *)(ptr) - offsetof(type,member)))
#endif
#endif


#define aosl_min(x, y) ((x) < (y) ? (x) : (y))
#define aosl_max(x, y) ((x) > (y) ? (x) : (y))
#define aosl_min3(x, y, z) aosl_min(aosl_min(x, y), z)
#define aosl_max3(x, y, z) aosl_max(aosl_max(x, y), z)
#define aosl_clamp(val, lo, hi) aosl_min(aosl_max(val, lo), hi)


/* I think 64 args is big enough */
#define AOSL_VAR_ARGS_MAX 64


#if defined (__GNUC__)
#define __export_in_so__ __attribute__ ((visibility ("default")))
#elif defined (_MSC_VER) && defined (BUILDING_API_IMPL_SOURCE) && defined (BUILD_TARGET_SHARED)
#define __export_in_so__ __declspec (dllexport)
#elif defined (_MSC_VER) && !defined (BUILDING_API_IMPL_SOURCE)
#define __export_in_so__ __declspec (dllimport)
#else
#define __export_in_so__
#endif

#ifndef __aosl_api__
#if defined (_MSC_VER) && defined (BUILDING_API_IMPL_SOURCE) && defined (BUILD_TARGET_SHARED)
#define __aosl_api__ __declspec (dllexport)
#elif defined (_MSC_VER) && !defined (BUILDING_API_IMPL_SOURCE)
#define __aosl_api__ __declspec (dllimport)
#else
#define __aosl_api__
#endif
#endif

#ifdef BUILDING_API_IMPL_SOURCE

#if defined (__GNUC__)
#define __so_api__ __attribute__ ((visibility ("default")))
#elif defined (_MSC_VER)
#define __so_api__ __declspec (dllexport)
#else
#define __so_api__
#endif

#else

#if defined (_MSC_VER)
#define __so_api__ __declspec (dllimport)
#else
#define __so_api__
#endif

#endif

#if defined(__USE_GLOBAL_RODATA__)
#define __GLOBAL_RODATA__ __attribute__((section(".rodata")))
#else
#define __GLOBAL_RODATA__
#endif
#ifdef __cplusplus
}
#endif

#endif /* __MAKERCORE_ASSEMBLY__ */

#endif /* __AOSL_DEFS_H__ */
