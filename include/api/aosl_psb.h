/***************************************************************************
 * Module:	Packet Segment Buffer relatives header file
 *
 * Copyright © 2025 Agora
 * This file is part of AOSL, an open source project.
 * Licensed under the Apache License, Version 2.0, with certain conditions.
 * Refer to the "LICENSE" file in the root directory for more information.
 ***************************************************************************/

#ifndef __AOSL_PSB_H__
#define __AOSL_PSB_H__


#include <api/aosl_types.h>
#include <api/aosl_defs.h>


#ifdef __cplusplus
extern "C" {
#endif



#define DFLT_MAX_PSB_PKT (4 << 10) /* default max packet size is 4K */
#define MAX_PSB_PKT_SIZE (2 << 20) /* max packet size is limited to 2M */

/* psb size is 2 times of max packet, this is default */
#define DEFAULT_PSB_SIZE (DFLT_MAX_PSB_PKT << 1)

/* AOSL Packet Segment Buffer */
typedef struct _____psb {
	void *data;
	size_t len;
	struct _____psb *next;
} aosl_psb_t;


extern __aosl_api__ aosl_psb_t *aosl_alloc_user_psb (void *buf, size_t bufsz);
extern __aosl_api__ aosl_psb_t *aosl_alloc_psb (size_t size);
extern __aosl_api__ void aosl_psb_attach_buf (aosl_psb_t *psb, void *buf, size_t bufsz);
extern __aosl_api__ void aosl_psb_detach_buf (aosl_psb_t *psb);
extern __aosl_api__ size_t aosl_psb_headroom (const aosl_psb_t *psb);
extern __aosl_api__ size_t aosl_psb_tailroom (const aosl_psb_t *psb);
extern __aosl_api__ int aosl_psb_reserve (aosl_psb_t *psb, size_t len);
extern __aosl_api__ void *aosl_psb_data (const aosl_psb_t *psb);
extern __aosl_api__ size_t aosl_psb_len (const aosl_psb_t *psb);
extern __aosl_api__ size_t aosl_psb_total_len (const aosl_psb_t *psb);
extern __aosl_api__ void *aosl_psb_put (aosl_psb_t *psb, size_t len);
extern __aosl_api__ void *aosl_psb_get (aosl_psb_t *psb, size_t len);
extern __aosl_api__ void *aosl_psb_peek (const aosl_psb_t *psb, size_t len);
extern __aosl_api__ void *aosl_psb_push (aosl_psb_t *psb, size_t len);
extern __aosl_api__ void *aosl_psb_pull (aosl_psb_t *psb, size_t len);
extern __aosl_api__ int aosl_psb_single (const aosl_psb_t *psb);
extern __aosl_api__ void aosl_psb_reset (aosl_psb_t *psb);
extern __aosl_api__ void aosl_free_psb_list (aosl_psb_t *psb);



#ifdef __cplusplus
}
#endif



#endif /* __AOSL_PSB_H__ */
