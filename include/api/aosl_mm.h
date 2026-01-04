/***************************************************************************
 * Module:	Memory Management relative utilities header file
 *
 * Copyright © 2025 Agora
 * This file is part of AOSL, an open source project.
 * Licensed under the Apache License, Version 2.0, with certain conditions.
 * Refer to the "LICENSE" file in the root directory for more information.
 ***************************************************************************/

#ifndef __AOSL_MM_H__
#define __AOSL_MM_H__

#include <api/aosl_types.h>
#include <api/aosl_defs.h>


#ifdef __cplusplus
extern "C" {
#endif

extern __aosl_api__ void *aosl_malloc_impl (size_t size);
extern __aosl_api__ void *aosl_calloc_impl (size_t nmemb, size_t size);
extern __aosl_api__ void *aosl_realloc_impl (void *ptr, size_t size);
extern __aosl_api__ char *aosl_strdup_impl (const char *s);
extern __aosl_api__ void aosl_free_impl (void *ptr);

#if defined(CONFIG_AOSL_MEM_STAT) && defined(CONFIG_AOSL_MEM_DUMP)
extern __aosl_api__ void *aosl_malloc_impl_dbg (size_t size, const char *func, int line);
extern __aosl_api__ void *aosl_calloc_impl_dbg (size_t nmemb, size_t size, const char *func, int line);
extern __aosl_api__ void *aosl_realloc_impl_dbg (void *ptr, size_t size, const char *func, int line);
extern __aosl_api__ char *aosl_strdup_impl_dbg (const char *s, const char *func, int line);

#define aosl_malloc(size)         aosl_malloc_impl_dbg(size, __FUNCTION__, __LINE__)
#define aosl_calloc(nmemb, size)  aosl_calloc_impl_dbg(nmemb, size, __FUNCTION__, __LINE__)
#define aosl_realloc(ptr, size)   aosl_realloc_impl_dbg(ptr, size, __FUNCTION__, __LINE__)
#define aosl_strdup(s)            aosl_strdup_impl_dbg(s, __FUNCTION__, __LINE__)
#define aosl_free(ptr)            aosl_free_impl(ptr)
#else
#define aosl_malloc(size)         aosl_malloc_impl(size)
#define aosl_calloc(nmemb, size)  aosl_calloc_impl(nmemb, size)
#define aosl_realloc(ptr, size)   aosl_realloc_impl(ptr, size)
#define aosl_strdup(s)            aosl_strdup_impl(s)
#define aosl_free(ptr)            aosl_free_impl(ptr)
#endif // end CONFIG_AOSL_MEM_DUMP

extern __aosl_api__ size_t aosl_memused(void);
extern __aosl_api__ void   aosl_memdump(void);
extern __aosl_api__ int    aosl_memdump_r(int cnts[2], char *buf, int len);

#ifdef __cplusplus
}
#endif



#endif /* __AOSL_MM_H__ */