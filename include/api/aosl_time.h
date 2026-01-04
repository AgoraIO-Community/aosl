/***************************************************************************
 * Module:	Time relative utilities header file
 *
 * Copyright © 2025 Agora
 * This file is part of AOSL, an open source project.
 * Licensed under the Apache License, Version 2.0, with certain conditions.
 * Refer to the "LICENSE" file in the root directory for more information.
 ***************************************************************************/

#ifndef __AOSL_API_TIME_H__
#define __AOSL_API_TIME_H__

#include <api/aosl_types.h>
#include <api/aosl_defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The AOSL timestamp type */
typedef uint64_t aosl_ts_t;


extern __aosl_api__ aosl_ts_t aosl_tick_now (void);
extern __aosl_api__ aosl_ts_t aosl_tick_ms (void);
extern __aosl_api__ aosl_ts_t aosl_tick_us (void);

extern __aosl_api__ aosl_ts_t aosl_time_sec (void);
extern __aosl_api__ aosl_ts_t aosl_time_ms (void);

extern __aosl_api__ void aosl_msleep (uint64_t ms);

extern __aosl_api__ int aosl_time_str(char *buf, int len);

#ifdef __cplusplus
}
#endif


#endif /* __AOSL_API_TIME_H__ */
