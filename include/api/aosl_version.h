/***************************************************************************
 * Module:	AOSL version definitions.
 *
 * Copyright © 2025 Agora
 * This file is part of AOSL, an open source project.
 * Licensed under the Apache License, Version 2.0, with certain conditions.
 * Refer to the "LICENSE" file in the root directory for more information.
 ***************************************************************************/

#ifndef __AOSL_VERSION_H__
#define __AOSL_VERSION_H__

#include <api/aosl_defs.h>

#ifdef __cplusplus
extern "C" {
#endif



extern __aosl_api__ const char *aosl_get_git_branch (void);
extern __aosl_api__ const char *aosl_get_git_commit (void);



#ifdef __cplusplus
}
#endif


#endif /* __AOSL_VERSION_H__ */