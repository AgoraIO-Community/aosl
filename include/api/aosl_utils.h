/***************************************************************************
 * Module:	AOSL utilities definition file.
 *
 * Copyright © 2025 Agora
 * This file is part of AOSL, an open source project.
 * Licensed under the Apache License, Version 2.0, with certain conditions.
 * Refer to the "LICENSE" file in the root directory for more information.
 ***************************************************************************/

#ifndef __AOSL_UTILS_H__
#define __AOSL_UTILS_H__

#include <api/aosl_types.h>
#include <api/aosl_defs.h>
#include <api/aosl_mm.h>


#ifdef __cplusplus
extern "C" {
#endif

extern __aosl_api__ int aosl_get_uuid (char buf [], size_t buf_sz);
extern __aosl_api__ int aosl_os_version (char buf [], size_t buf_sz);

#ifdef __cplusplus
}
#endif


#endif /* __AOSL_UTILS_H__ */