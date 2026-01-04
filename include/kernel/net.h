/***************************************************************************
 * Module:	Internal used net relative functionals header file
 *
 * Copyright © 2025 Agora
 * This file is part of AOSL, an open source project.
 * Licensed under the Apache License, Version 2.0, with certain conditions.
 * Refer to the "LICENSE" file in the root directory for more information.
 ***************************************************************************/

#ifndef __KERNEL_NET_H__
#define __KERNEL_NET_H__


#include <api/aosl_types.h>
#include <api/aosl_route.h>
#include <kernel/byteorder/generic.h>

const char *k_inet_ntop (int af, const void *src, char *dst, aosl_socklen_t size);
int k_inet_pton (int af, const char *src, void *dst);

//#define k_inet_ntop(af, src, dst, size) inet_ntop (af, src, dst, size)
//#define k_inet_pton(af, src, dst) inet_pton (af, src, dst)


extern void __invalidate_rt (aosl_rt_t *rt);
extern void __invalidate_def_rt (aosl_def_rt_t *def_rt);

extern void netifs_hash_init (void);
extern void netifs_hash_fini (void);

extern aosl_netif_t *netif_by_index (int idx);
extern int update_netifs (int del, int ifindex, ...);

extern void check_report_def_rt_change_event (aosl_net_ev_func_t f, void *arg);

#if defined (__linux__)
extern int os_get_def_rt (aosl_def_rt_t *def_rt);

#else
static inline int os_get_def_rt (aosl_def_rt_t *def_rt) {(void)def_rt; return -1;}
#endif

#endif /* __KERNEL_NET_H__ */
