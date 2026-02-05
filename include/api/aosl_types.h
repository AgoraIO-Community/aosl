/***************************************************************************
 * Module:	AOSL POSIX definitions header file
 *
 * Copyright © 2025 Agora
 * This file is part of AOSL, an open source project.
 * Licensed under the Apache License, Version 2.0, with certain conditions.
 * Refer to the "LICENSE" file in the root directory for more information.
 ***************************************************************************/

#ifndef __AOSL_TYPES_H__
#define __AOSL_TYPES_H__

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef intptr_t isize_t;
typedef uintptr_t usize_t;

/* The proto for a general aosl object destructor function. */
typedef void (*aosl_obj_dtor_t) (uintptr_t argc, uintptr_t argv []);

typedef int aosl_fd_t;
#define AOSL_INVALID_FD ((aosl_fd_t)-1)

static __inline__ int aosl_fd_invalid (aosl_fd_t fd)
{
#if defined(__kspreadtrum__)
	return (fd == AOSL_INVALID_FD);
#else
	return (int)(fd < 0);
#endif
}



#ifdef __cplusplus
}
#endif


#endif /* __AOSL_TYPES_H__ */