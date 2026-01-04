/***************************************************************************
 * Module:	Network route relative functionals header file
 *
 * Copyright © 2025 Agora
 * This file is part of AOSL, an open source project.
 * Licensed under the Apache License, Version 2.0, with certain conditions.
 * Refer to the "LICENSE" file in the root directory for more information.
 ***************************************************************************/

#ifndef __AOSL_NET_API_H__
#define __AOSL_NET_API_H__

#include <api/aosl_types.h>
#include <api/aosl_defs.h>
#include <api/aosl_mpq_net.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
	AOSL_NET_EV_NONE = 0,
	AOSL_NET_EV_NET_DOWN,
	AOSL_NET_EV_NET_UP,
	AOSL_NET_EV_NET_UP_CHANGED,
	AOSL_NET_EV_NET_SWITCH,
	AOSL_NET_EV_RT_CNT_CHANGED,
} aosl_net_ev_t;

typedef struct {
	int if_index;
	int if_type;
	char if_name [64];
} aosl_netif_t;

typedef struct {
	aosl_netif_t netif;
	int if_cellnet;
	int if_wireless;
	int def_rt_cnt;
	aosl_sk_addr_t gw;
} aosl_rt_t;

typedef struct {
	aosl_rt_t IPv4;
	aosl_rt_t IPv6;
} aosl_def_rt_t;

extern __aosl_api__ void aosl_init_def_rt (aosl_def_rt_t *def_rt);

extern __aosl_api__ void aosl_invalidate_rt (aosl_rt_t *rt);
extern __aosl_api__ void aosl_invalidate_def_rt (aosl_def_rt_t *def_rt);

extern __aosl_api__ int aosl_rt_valid (const aosl_rt_t *rt);
extern __aosl_api__ int aosl_def_rt_valid (const aosl_def_rt_t *def_rt);

extern __aosl_api__ int aosl_network_is_down (void);

extern __aosl_api__ int aosl_same_rt (const aosl_rt_t *rt1, const aosl_rt_t *rt2);
extern __aosl_api__ int aosl_same_def_rt (const aosl_def_rt_t *def_rt1, const aosl_def_rt_t *def_rt2);

extern __aosl_api__ const char *aosl_rt_str (const aosl_rt_t *rt, char buf [], size_t buf_size);
extern __aosl_api__ const char *aosl_def_rt_str (const aosl_def_rt_t *def_rt, char buf [], size_t buf_size);


typedef void (*aosl_net_ev_func_t) (aosl_net_ev_t ev, void *arg, ...);

/**
 * Subscribe/Unsubscribe the low level network events.
 * If f is not NULL, then subscribe the low level network events, and the callback
 * function specified by f will be invoked in the calling thread context. Please
 * note that only one subscriber allowed in the whole process.
 * If f is NULL, then unsubscribe the low level network events.
 * Parameters:
 *       f: the callback function
 *     arg: the parameter which will be passed when invoking the f callback
 * Return values:
 *     <0: failed with errno set
 *      0: successful
 **/
extern __aosl_api__ int aosl_subscribe_net_events (aosl_net_ev_func_t f, void *arg);

/**
 * Return value is the default route count both for IPv4 and IPv6 we retrieved.
 *      0: no default route
 *      1: only one of IPv4 or IPv6 has default route
 *      2: both IPv4 and IPv6 have default route
 **/
extern __aosl_api__ int aosl_get_default_rt (aosl_def_rt_t *def_rt);


extern __aosl_api__ int aosl_is_mobile_net (uint16_t af);


#ifdef __cplusplus
}
#endif


#endif /* __AOSL_NET_API_H__ */
