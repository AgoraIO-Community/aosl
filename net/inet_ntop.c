/***************************************************************************
 * Module:	AOSL inet operations implementation file
 *
 * Copyright © 2025 Agora
 * This file is part of AOSL, an open source project.
 * Licensed under the Apache License, Version 2.0, with certain conditions.
 * Refer to the "LICENSE" file in the root directory for more information.
 ***************************************************************************/

 #include <stdio.h>
#include <string.h>

#include <api/aosl_types.h>
#include <api/aosl_socket.h>
#include <kernel/err.h>

#define __NS_IN6ADDRSZ 16 /*%< IPv6 T_AAAA */
#define __NS_INT16SZ 2 /*%< #/bytes of data in a uint16_t */

static const char *inet_ntop4 (const unsigned char *src, char *dst, aosl_socklen_t size)
{
	char tmp [sizeof "255.255.255.255"];

	if (sprintf (tmp, "%u.%u.%u.%u", src[0], src[1], src[2], src[3]) >= (int)size) {
		aosl_errno = AOSL_ENOSPC;
		return NULL;
	}

	return strcpy (dst, tmp);
}

static const char *inet_ntop6 (const unsigned char *src, char *dst, aosl_socklen_t size)
{
	char tmp [sizeof "ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"], *tp;
	struct { int base, len; } best, cur;
	unsigned int words[__NS_IN6ADDRSZ / __NS_INT16SZ];
	int i;

	memset(words, '\0', sizeof words);
	for (i = 0; i < __NS_IN6ADDRSZ; i += 2)
		words[i / 2] = (src[i] << 8) | src[i + 1];
	best.base = -1;
	cur.base = -1;
	best.len = 0;
	cur.len = 0;
	for (i = 0; i < (__NS_IN6ADDRSZ / __NS_INT16SZ); i++) {
		if (words[i] == 0) {
			if (cur.base == -1)
				cur.base = i, cur.len = 1;
			else
				cur.len++;
		} else {
			if (cur.base != -1) {
				if (best.base == -1 || cur.len > best.len)
					best = cur;
				cur.base = -1;
			}
		}
	}
	if (cur.base != -1) {
		if (best.base == -1 || cur.len > best.len)
			best = cur;
	}
	if (best.base != -1 && best.len < 2)
		best.base = -1;

	tp = tmp;
	for (i = 0; i < (__NS_IN6ADDRSZ / __NS_INT16SZ); i++) {
		/* Are we inside the best run of 0x00's? */
		if (best.base != -1 && i >= best.base &&
		    i < (best.base + best.len)) {
			if (i == best.base)
				*tp++ = ':';
			continue;
		}
		/* Are we following an initial run of 0x00s or any real hex? */
		if (i != 0)
			*tp++ = ':';
		/* Is this address an encapsulated IPv4? */
		if (i == 6 && best.base == 0 &&
		    (best.len == 6 || (best.len == 5 && words[5] == 0xffff))) {
			if (!inet_ntop4(src+12, tp, sizeof tmp - (tp - tmp)))
				return (NULL);
			tp += strlen(tp);
			break;
		}
		tp += sprintf (tp, "%x", words [i]);
	}
	/* Was it a trailing run of 0x00's? */
	if (best.base != -1 && (best.base + best.len) ==
	    (__NS_IN6ADDRSZ / __NS_INT16SZ))
		*tp++ = ':';
	*tp++ = '\0';

	if ((aosl_socklen_t)(tp - tmp) > size) {
		aosl_errno = AOSL_ENOSPC;
		return NULL;
	}
	return strcpy (dst, tmp);
}

const char *k_inet_ntop (int af, const void *src, char *dst, aosl_socklen_t size)
{
	switch (af) {
	case AOSL_AF_INET:
		return (inet_ntop4(src, dst, size));
#ifdef CONFIG_AOSL_IPV6
	case AOSL_AF_INET6:
		return (inet_ntop6(src, dst, size));
#endif
	default:
		aosl_errno = AOSL_EAFNOSUPPORT;
		return NULL;
	}
}