/***************************************************************************
 * Module:	AOSL inet operations implementation file
 *
 * Copyright © 2025 Agora
 * This file is part of AOSL, an open source project.
 * Licensed under the Apache License, Version 2.0, with certain conditions.
 * Refer to the "LICENSE" file in the root directory for more information.
 ***************************************************************************/

#include <string.h>

#include <api/aosl_types.h>
#include <api/aosl_socket.h>
#include <kernel/err.h>

#define __NS_IN6ADDRSZ 16 /*%< IPv6 T_AAAA */
#define __NS_INADDRSZ 4 /* IPv4 T_A */
#define __NS_INT16SZ 2 /*%< #/bytes of data in a uint16_t */

/* Like inet_aton but without all the hexadecimal, octal and shorthand
   (and trailing garbage is not ignored).  Return 1 if SRC is a valid
   dotted quad, else 0.  This function does not touch DST unless it's
   returning 1.
   Author: Paul Vixie, 1996.  */
static int inet_pton4 (const char *src, const char *end, unsigned char *dst)
{
  int saw_digit, octets, ch;
  unsigned char tmp[__NS_INADDRSZ], *tp;

  saw_digit = 0;
  octets = 0;
  *(tp = tmp) = 0;
  while (src < end)
    {
      ch = *src++;
      if (ch >= '0' && ch <= '9')
        {
          unsigned int new = *tp * 10 + (ch - '0');

          if (saw_digit && *tp == 0)
            return 0;
          if (new > 255)
            return 0;
          *tp = new;
          if (! saw_digit)
            {
              if (++octets > 4)
                return 0;
              saw_digit = 1;
            }
        }
      else if (ch == '.' && saw_digit)
        {
          if (octets == 4)
            return 0;
          *++tp = 0;
          saw_digit = 0;
        }
      else
        return 0;
    }
  if (octets < 4)
    return 0;
  memcpy (dst, tmp, __NS_INADDRSZ);
  return 1;
}

/* Return the value of CH as a hexademical digit, or -1 if it is a
   different type of character.  */
static int hex_digit_value (char ch)
{
  if ('0' <= ch && ch <= '9')
    return ch - '0';
  if ('a' <= ch && ch <= 'f')
    return ch - 'a' + 10;
  if ('A' <= ch && ch <= 'F')
    return ch - 'A' + 10;
  return -1;
}

/* Convert presentation-level IPv6 address to network order binary
   form.  Return 1 if SRC is a valid [RFC1884 2.2] address, else 0.
   This function does not touch DST unless it's returning 1.
   Author: Paul Vixie, 1996.  Inspired by Mark Andrews.  */
static int inet_pton6 (const char *src, const char *src_endp, unsigned char *dst)
{
  unsigned char tmp[__NS_IN6ADDRSZ], *tp, *endp, *colonp;
  const char *curtok;
  int ch;
  size_t xdigits_seen;	/* Number of hex digits since colon.  */
  unsigned int val;

  tp = memset (tmp, '\0', __NS_IN6ADDRSZ);
  endp = tp + __NS_IN6ADDRSZ;
  colonp = NULL;

  /* Leading :: requires some special handling.  */
  if (src == src_endp)
    return 0;
  if (*src == ':')
    {
      ++src;
      if (src == src_endp || *src != ':')
        return 0;
    }

  curtok = src;
  xdigits_seen = 0;
  val = 0;
  while (src < src_endp)
    {
      ch = *src++;
      int digit = hex_digit_value (ch);
      if (digit >= 0)
	{
	  if (xdigits_seen == 4)
	    return 0;
	  val <<= 4;
	  val |= digit;
	  if (val > 0xffff)
	    return 0;
	  ++xdigits_seen;
	  continue;
	}
      if (ch == ':')
	{
	  curtok = src;
	  if (xdigits_seen == 0)
	    {
	      if (colonp)
		return 0;
	      colonp = tp;
	      continue;
	    }
	  else if (src == src_endp)
            return 0;
	  if (tp + __NS_INT16SZ > endp)
	    return 0;
	  *tp++ = (unsigned char) (val >> 8) & 0xff;
	  *tp++ = (unsigned char) val & 0xff;
	  xdigits_seen = 0;
	  val = 0;
	  continue;
	}
      if (ch == '.' && ((tp + __NS_INADDRSZ) <= endp)
          && inet_pton4 (curtok, src_endp, tp) > 0)
	{
	  tp += __NS_INADDRSZ;
	  xdigits_seen = 0;
	  break;  /* '\0' was seen by inet_pton4.  */
	}
      return 0;
    }
  if (xdigits_seen > 0)
    {
      if (tp + __NS_INT16SZ > endp)
	return 0;
      *tp++ = (unsigned char) (val >> 8) & 0xff;
      *tp++ = (unsigned char) val & 0xff;
    }
  if (colonp != NULL)
    {
      /* Replace :: with zeros.  */
      if (tp == endp)
        /* :: would expand to a zero-width field.  */
        return 0;
      size_t n = tp - colonp;
      memmove (endp - n, colonp, n);
      memset (colonp, 0, endp - n - colonp);
      tp = endp;
    }
  if (tp != endp)
    return 0;
  memcpy (dst, tmp, __NS_IN6ADDRSZ);
  return 1;
}

static int __inet_pton_length (int af, const char *src, size_t srclen, void *dst)
{
  switch (af)
  {
  case AOSL_AF_INET:
    return inet_pton4 (src, src + srclen, dst);
#if defined(CONFIG_AOSL_IPV6)
  case AOSL_AF_INET6:
    return inet_pton6 (src, src + srclen, dst);
#endif
  default:
    aosl_errno = AOSL_EAFNOSUPPORT;
    return -1;
  }
}

/* Like k_inet_pton_length, but use strlen (SRC) as the length of
   SRC.  */
int k_inet_pton (int af, const char *src, void *dst)
{
  return __inet_pton_length (af, src, strlen (src), dst);
}