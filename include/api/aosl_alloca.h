/***************************************************************************
 * Module		:		AOSL alloca definition header file
 *
 * Copyright © 2025 Agora
 * This file is part of AOSL, an open source project.
 * Licensed under the Apache License, Version 2.0, with certain conditions.
 * Refer to the "LICENSE" file in the root directory for more information.
 ***************************************************************************/

#ifndef __AOSL_ALLOCA_H__
#define __AOSL_ALLOCA_H__

#if defined (__GNUC__)
#include <alloca.h>
#elif defined (_MSC_VER)
#include <malloc.h>
#endif

#define aosl_alloca(size)  alloca((size))

#endif /* __AOSL_ALLOCA_H__ */