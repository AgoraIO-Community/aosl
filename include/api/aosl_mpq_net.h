/***************************************************************************
 * Module:	Socket helper utils header file
 *
 * Copyright © 2025 Agora
 * This file is part of AOSL, an open source project.
 * Licensed under the Apache License, Version 2.0, with certain conditions.
 * Refer to the "LICENSE" file in the root directory for more information.
 ***************************************************************************/

#ifndef __AOSL_MPQ_NET_H__
#define __AOSL_MPQ_NET_H__

#include <api/aosl_types.h>
#include <api/aosl_socket.h>
#include <api/aosl_defs.h>
#include <api/aosl_mpq.h>
#include <api/aosl_mpq_fd.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef aosl_fd_t aosl_sk_t;
#define AOSL_INVALID_SK AOSL_INVALID_FD

#define AOSL_IPHDR_LEN 20
#define AOSL_UDPHDR_LEN 8
#define AOSL_TCPHDR_LEN 20
#define AOSL_IP_UDP_HDR_LEN (AOSL_IPHDR_LEN + AOSL_UDPHDR_LEN)
#define AOSL_IP_TCP_HDR_LEN (AOSL_IPHDR_LEN + AOSL_TCPHDR_LEN)


typedef union {
	aosl_sockaddr_t sa;
	aosl_sockaddr_in_t in;
	aosl_sockaddr_in6_t in6;
} aosl_sk_addr_t;

typedef struct {
	aosl_fd_t newsk;
	aosl_sk_addr_t addr;
} aosl_accept_data_t;

/**
 * The listen state socket readable callback function type
 * Parameters:
 *    data: the data buffer holding the packet
 *     len: the data length in bytes in the buffer
 *    argc: the args count when adding the fd
 *    argv: the args vector when adding the fd
 *    addr: the socket address data received from
 * Return value:
 *    None.
 **/
typedef void (*aosl_sk_accepted_t) (aosl_accept_data_t *accept_data, size_t len, uintptr_t argc, uintptr_t argv []);

/**
 * The dgram socket received data callback function type
 * Parameters:
 *    data: the data buffer holding the packet
 *     len: the data length in bytes in the buffer
 *    argc: the args count when adding the fd
 *    argv: the args vector when adding the fd
 *    addr: the socket address data received from
 * Return value:
 *    None.
 **/
typedef void (*aosl_dgram_sk_data_t) (void *data, size_t len, uintptr_t argc, uintptr_t argv [], const aosl_sk_addr_t *addr);

extern __aosl_api__ aosl_fd_t aosl_socket (int domain, int type, int protocol);
extern __aosl_api__ int aosl_bind (aosl_fd_t sockfd, const aosl_sockaddr_t *addr);
//extern __aosl_api__ int aosl_getsockname (aosl_fd_t sockfd, aosl_sockaddr_t *addr);
//extern __aosl_api__ int aosl_getpeername (aosl_fd_t sockfd, aosl_sockaddr_t *addr);
//extern __aosl_api__ int aosl_getsockopt (aosl_fd_t sockfd, int level, int optname, void *optval, int *optlen);
//extern __aosl_api__ int aosl_setsockopt (aosl_fd_t sockfd, int level, int optname, const void *optval, int optlen);
extern __aosl_api__ int aosl_get_sockaddr(int sockfd, aosl_sockaddr_t *addr);

extern __aosl_api__ int aosl_mpq_connect (aosl_fd_t fd, const aosl_sockaddr_t *dest_addr,
												int timeo, size_t max_pkt_size, aosl_check_packet_t chk_pkt_f,
											aosl_fd_data_t data_f, aosl_fd_event_t event_f, uintptr_t argc, ...);

extern __aosl_api__ int aosl_mpq_connect_on_q (aosl_mpq_t qid, aosl_fd_t fd, const aosl_sockaddr_t *dest_addr,
																		int timeo, size_t max_pkt_size, aosl_check_packet_t chk_pkt_f,
																aosl_fd_data_t data_f, aosl_fd_event_t event_f, uintptr_t argc, ...);

extern __aosl_api__ int aosl_mpq_listen (aosl_fd_t fd, int backlog, aosl_sk_accepted_t accepted_f, aosl_fd_event_t event_f, uintptr_t argc, ...);

extern __aosl_api__ int aosl_mpq_listen_on_q (aosl_mpq_t qid, aosl_fd_t fd, int backlog,
				aosl_sk_accepted_t accepted_f, aosl_fd_event_t event_f, uintptr_t argc, ...);

extern __aosl_api__ int aosl_mpq_add_dgram_socket (aosl_fd_t fd, size_t max_pkt_size, aosl_dgram_sk_data_t data_f, aosl_fd_event_t event_f, uintptr_t argc, ...);

extern __aosl_api__ int aosl_mpq_add_dgram_socket_on_q (aosl_mpq_t qid, aosl_fd_t fd, size_t max_pkt_size,
									aosl_dgram_sk_data_t data_f, aosl_fd_event_t event_f, uintptr_t argc, ...);

extern __aosl_api__ int aosl_mpq_add_stream_socket (aosl_fd_t fd, size_t max_pkt_size, aosl_check_packet_t chk_pkt_f,
													aosl_fd_data_t data_f, aosl_fd_event_t event_f, uintptr_t argc, ...);

extern __aosl_api__ int aosl_mpq_add_stream_socket_on_q (aosl_mpq_t qid, aosl_fd_t fd, size_t max_pkt_size, aosl_check_packet_t chk_pkt_f,
																	aosl_dgram_sk_data_t data_f, aosl_fd_event_t event_f, uintptr_t argc, ...);

extern __aosl_api__ ssize_t aosl_send (aosl_fd_t sockfd, const void *buf, size_t len, int flags);
extern __aosl_api__ ssize_t aosl_sendto (aosl_fd_t sockfd, const void *buf, size_t len, int flags, const aosl_sockaddr_t *dest_addr);

extern __aosl_api__ int aosl_ip_sk_bind_port_only (aosl_fd_t sk, uint16_t af, unsigned short port);


typedef struct {
	aosl_fd_t v4;
	aosl_fd_t v6;
} aosl_ip_sk_t;

static __inline__ void aosl_ip_sk_init (aosl_ip_sk_t *sk)
{
	sk->v4 = AOSL_INVALID_FD;
	sk->v6 = AOSL_INVALID_FD;
}

extern __aosl_api__ int aosl_ip_sk_create (aosl_ip_sk_t *sk, int type, int protocol);

typedef struct {
	aosl_sockaddr_in_t v4;
	aosl_sockaddr_in6_t v6;
} aosl_ip_addr_t;

extern __aosl_api__ void aosl_ip_addr_init (aosl_ip_addr_t *addr);
extern __aosl_api__ int aosl_ip_sk_bind (const aosl_ip_sk_t *sk, const aosl_ip_addr_t *addr);

extern __aosl_api__ int aosl_mpq_ip_sk_connect (const aosl_ip_sk_t *sk, const aosl_sockaddr_t *dest_addr,
												int timeo, size_t max_pkt_size, aosl_check_packet_t chk_pkt_f,
										aosl_fd_data_t data_f, aosl_fd_event_t event_f, uintptr_t argc, ...);

extern __aosl_api__ int aosl_mpq_ip_sk_connect_on_q (aosl_mpq_t qid, const aosl_ip_sk_t *sk,
							const aosl_sockaddr_t *dest_addr, int timeo, size_t max_pkt_size,
				aosl_check_packet_t chk_pkt_f, aosl_fd_data_t data_f, aosl_fd_event_t event_f, uintptr_t argc, ...);

extern __aosl_api__ ssize_t aosl_ip_sk_sendto (const aosl_ip_sk_t *sk, const void *buf, size_t len, int flags, const aosl_sockaddr_t *dest_addr);
extern __aosl_api__ void aosl_ip_sk_close (aosl_ip_sk_t *sk);

extern __aosl_api__ int aosl_ipv6_addr_v4_mapped (const aosl_in6_addr_t *a6);
extern __aosl_api__ int aosl_ipv6_addr_nat64 (const aosl_in6_addr_t *a6);
extern __aosl_api__ int aosl_ipv6_addr_v4_compatible (const aosl_in6_addr_t *a6);

extern __aosl_api__ int aosl_ipv6_sk_addr_from_ipv4 (aosl_sockaddr_t *sk_addr_v6, const aosl_sockaddr_t *sk_addr_v4);
extern __aosl_api__ int aosl_ipv6_sk_addr_to_ipv4 (aosl_sockaddr_t *sk_addr_v6, const aosl_sockaddr_t *sk_addr_v4);


extern __aosl_api__ int aosl_ip_sk_addr_init_with_port (aosl_sk_addr_t *sk_addr, uint16_t af, unsigned short port);

/* Structure for describing a resolved sock address information */
typedef struct {
	uint16_t sk_af;
	int sk_type;
	int sk_prot;
	aosl_sk_addr_t sk_addr;
} aosl_sk_addrinfo_t;


extern __aosl_api__ int aosl_sk_addr_ip_equal (const aosl_sockaddr_t *addr1, const aosl_sockaddr_t *addr2);

extern __aosl_api__ const char *aosl_inet_addr_str (int af, const void *addr, char *addr_buf, size_t buf_len);
extern __aosl_api__ const char *aosl_ip_sk_addr_str (const aosl_sk_addr_t *addr, char *addr_buf, size_t buf_len);
extern __aosl_api__ unsigned short aosl_ip_sk_addr_port (const aosl_sk_addr_t *addr);

extern __aosl_api__ aosl_socklen_t aosl_inet_addr_from_string (void *addr, const char *str_addr);
extern __aosl_api__ aosl_socklen_t aosl_ip_sk_addr_from_string (aosl_sk_addr_t *sk_addr, const char *str_addr, uint16_t port);

extern __aosl_api__ const aosl_in6_addr_t *aosl_mpq_get_ipv6_prefix (void);
extern __aosl_api__ int aosl_mpq_set_ipv6_prefix_on_q (aosl_mpq_t qid, const aosl_in6_addr_t *a6);


/**
 * Resolve host name asynchronously relative functions, these functions would queue back a function call
 * specified by f to the mpq object specified by q with the specified args.
 **/
extern __aosl_api__ int aosl_tcp_resolve_host_async (const char *hostname, unsigned short port, aosl_sk_addrinfo_t *addrs, size_t addr_count, aosl_mpq_t q, aosl_mpq_func_argv_t f, uintptr_t argc, ...);
extern __aosl_api__ int aosl_tcp_resolve_host_asyncv (const char *hostname, unsigned short port, aosl_sk_addrinfo_t *addrs, size_t addr_count, aosl_mpq_t q, aosl_mpq_func_argv_t f, uintptr_t argc, va_list args);
extern __aosl_api__ int aosl_udp_resolve_host_async (const char *hostname, unsigned short port, aosl_sk_addrinfo_t *addrs, size_t addr_count, aosl_mpq_t q, aosl_mpq_func_argv_t f, uintptr_t argc, ...);
extern __aosl_api__ int aosl_udp_resolve_host_asyncv (const char *hostname, unsigned short port, aosl_sk_addrinfo_t *addrs, size_t addr_count, aosl_mpq_t q, aosl_mpq_func_argv_t f, uintptr_t argc, va_list args);



#ifdef __cplusplus
}
#endif


#endif /* __AOSL_MPQ_NET_H__ */