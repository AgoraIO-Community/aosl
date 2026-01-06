#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <hal/aosl_hal_socket.h>
#include <hal/aosl_hal_errno.h>
#include <api/aosl_route.h>
#include <api/aosl_log.h>

static inline int conv_domain_to_os(enum aosl_socket_domain domain)
{
	switch (domain) {
		case AOSL_AF_UNSPEC:
			return AF_UNSPEC;
		case AOSL_AF_INET:
			return AF_INET;
		case AOSL_AF_INET6:
			return AF_INET6;
		default:
			return AF_UNSPEC;
	}
}

static inline int conv_type_to_os(enum aosl_socket_type type)
{
	switch (type) {
		case AOSL_SOCK_STREAM:
			return SOCK_STREAM;
		case AOSL_SOCK_DGRAM:
			return SOCK_DGRAM;
		default:
			return -1;
	}
}

static inline int conv_proto_to_os(enum aosl_socket_proto proto)
{
	switch (proto) {
		case AOSL_IPPROTO_TCP:
			return IPPROTO_TCP;
		case AOSL_IPPROTO_UDP:
			return IPPROTO_UDP;
		default:
			return -1;
	}
}

static inline void conv_addr_to_os(const aosl_sockaddr_t *ah_addr, struct sockaddr *os_addr)
{
	switch (ah_addr->sa_family) {
		case AOSL_AF_INET: {
			struct sockaddr_in *v4 = (struct sockaddr_in *)os_addr;
		//*v4 = *(const struct sockaddr_in *)ah_addr;
			v4->sin_family = AF_INET;
			v4->sin_port = ah_addr->sa_port;
			v4->sin_addr.s_addr = ah_addr->sin_addr;
			break;
		}
		case AOSL_AF_INET6: {
			struct sockaddr_in6 *v6 = (struct sockaddr_in6 *)os_addr;
			//*v6 = *(const struct sockaddr_in6 *)ah_addr;
			v6->sin6_family = AF_INET;
			v6->sin6_port = ah_addr->sa_port;
			v6->sin6_flowinfo = ah_addr->sin6_flowinfo;
			v6->sin6_scope_id = ah_addr->sin6_scope_id;
			memcpy(&v6->sin6_addr, &ah_addr->sin6_addr, 16);
			break;
		}
		default:
			return;
	}
}

static inline void conv_addr_to_aosl(const struct sockaddr *os_addr, aosl_sockaddr_t *ah_addr)
{
	if (!os_addr || !ah_addr) {
		return;
	}
	switch (os_addr->sa_family) {
		case AF_INET: {
			const struct sockaddr_in *v4 = (const struct sockaddr_in *)os_addr;
			//*(struct sockaddr_in *)ah_addr = *v4;
			ah_addr->sa_family = AOSL_AF_INET;
			ah_addr->sa_port = v4->sin_port;
			ah_addr->sin_addr = v4->sin_addr.s_addr;
			break;
		}
		case AF_INET6: {
			const struct sockaddr_in6 *v6 = (const struct sockaddr_in6 *)os_addr;
			//*(struct sockaddr_in6 *)os_addr = *v6;
			ah_addr->sa_family = AOSL_AF_INET6;
			ah_addr->sa_port = v6->sin6_port;
			ah_addr->sin6_flowinfo = v6->sin6_flowinfo;
			ah_addr->sin6_scope_id = v6->sin6_scope_id;
			memcpy(&ah_addr->sin6_addr, &v6->sin6_addr, 16);
			break;
		}
		default:
			return;
	}
}

static inline int get_addrlen(int af)
{
	switch (af) {
		case AF_INET:
			return sizeof(struct sockaddr_in);
		case AF_INET6:
			return sizeof(struct sockaddr_in6);
		default:
			return -1;
	}
}

int aosl_hal_sk_socket(enum aosl_socket_domain domain,
											 enum aosl_socket_type type,
											 enum aosl_socket_proto protocol)
{
	int n_domain = conv_domain_to_os(domain);
	int n_type = conv_type_to_os(type);
	int n_proto = conv_proto_to_os(protocol);

	return socket(n_domain, n_type, n_proto);
}

int aosl_hal_sk_bind(int sockfd, const aosl_sockaddr_t* addr)
{
	struct sockaddr_in6 com_addr = {0};
	struct sockaddr *n_addr = (struct sockaddr *)&com_addr;
	conv_addr_to_os(addr, n_addr);
	socklen_t addrlen = get_addrlen(n_addr->sa_family);
	int ret = bind(sockfd, n_addr, addrlen);
	if (ret < 0) {
		return aosl_hal_errno_convert(errno);
	}
	return 0;
}

int aosl_hal_sk_listen(int sockfd, int backlog)
{
	int ret = listen(sockfd, backlog);
	if (ret < 0) {
		return aosl_hal_errno_convert(errno);
	}
	return 0;
}

int aosl_hal_sk_accept(int sockfd, aosl_sockaddr_t *addr)
{
	struct sockaddr_in6 com_addr = {0};
	struct sockaddr *n_addr = (struct sockaddr *)&com_addr;
	socklen_t addrlen = sizeof(com_addr);
	int ret = accept(sockfd, n_addr, &addrlen);
	if (ret < 0) {
		ret = aosl_hal_errno_convert(errno);
	} else {
		conv_addr_to_aosl(n_addr, addr);
	}
	return ret;
}

int aosl_hal_sk_connect(int sockfd, const aosl_sockaddr_t *addr)
{
	struct sockaddr_in6 com_addr = {0};
	struct sockaddr *n_addr = (struct sockaddr *)&com_addr;
	conv_addr_to_os(addr, n_addr);
	socklen_t addrlen = get_addrlen(n_addr->sa_family);
	int ret = connect(sockfd, n_addr, addrlen);
	if (ret < 0) {
		return aosl_hal_errno_convert(errno);
	}
	return 0;
}

int aosl_hal_sk_close(int sockfd)
{
	return close(sockfd);
}

int aosl_hal_sk_send(int sockfd, const void* buf, size_t len, int flags)
{
	int ret = send(sockfd, buf, len, flags);
	if (ret < 0) {
		return aosl_hal_errno_convert(errno);
	}
	return ret;
}

int aosl_hal_sk_recv(int sockfd, void* buf, size_t len, int flags)
{
	int ret = recv(sockfd, buf, len, flags);
	if (ret < 0) {
		return aosl_hal_errno_convert(errno);
	}
	return ret;
}

int aosl_hal_sk_sendto(int sockfd, const void *buffer, size_t length,
												int flags, const aosl_sockaddr_t *dest_addr)
{
	struct sockaddr_in6 com_addr = {0};
	struct sockaddr *n_dest_addr = (struct sockaddr *)&com_addr;
	conv_addr_to_os(dest_addr, n_dest_addr);
	socklen_t addrlen = get_addrlen(n_dest_addr->sa_family);
	int ret = sendto(sockfd, buffer, length, flags, n_dest_addr, addrlen);
	if (ret < 0) {
		return aosl_hal_errno_convert(errno);
	}
	return ret;
}

int aosl_hal_sk_recvfrom(int sockfd, void *buffer, size_t length,
													int flags, aosl_sockaddr_t *src_addr)
{
	struct sockaddr_in6 com_addr = {0};
	struct sockaddr *n_src_addr = (struct sockaddr *)&com_addr;
	socklen_t addrlen = sizeof(com_addr);
	int ret = recvfrom(sockfd, buffer, length, flags, n_src_addr, &addrlen);
	if (ret < 0) {
		ret = aosl_hal_errno_convert(errno);
	} else {
		conv_addr_to_aosl(n_src_addr, src_addr);
	}
	return ret;
}

int aosl_hal_sk_read(int sockfd, void *buf, size_t count)
{
	int ret = read(sockfd, buf, count);
	if (ret < 0) {
		return aosl_hal_errno_convert(errno);
	}
	return ret;
}

int aosl_hal_sk_write(int sockfd, const void *buf, size_t count)
{
	int ret = write(sockfd, buf, count);
	if (ret < 0) {
		return aosl_hal_errno_convert(errno);
	}
	return ret;
}

int aosl_hal_sk_set_nonblock(int sockfd)
{
	int oflags = fcntl(sockfd, F_GETFL);
	int err = fcntl(sockfd, F_SETFL, O_NONBLOCK | oflags);
	if (err < 0) {
		return -1;
	}
	return 0;
}

int aosl_hal_sk_get_local_ip(aosl_sockaddr_t *addr)
{
	bool found = false;
	const char *ifname = NULL;
	aosl_def_rt_t def_rt = {0};
	struct ifaddrs *if_addrs;

	// Obtain the interface name of the default route with the highest priority
	if (aosl_get_default_rt(&def_rt) == 0 || !aosl_rt_valid(&def_rt.IPv4)) {
		return -1;
	}
	ifname = def_rt.IPv4.netif.if_name;

	// Retrieve all interface information and match routing interfaces
	getifaddrs(&if_addrs);
	for (struct ifaddrs *ifa = if_addrs; (ifa); ifa = ifa->ifa_next) {
		if (!ifa->ifa_addr) {
			continue;
		}
		if (ifa->ifa_addr->sa_family != AF_INET) {
			continue;
		}
		if (strncmp(ifa->ifa_name, ifname, 64)) {
			continue;
		}
		found = true;
		addr->sa_family = AOSL_AF_INET;
		addr->sin_addr = ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr;
		break;
	}
	if (if_addrs) {
		freeifaddrs(if_addrs);
	}

	return found ? 0 : -1;
}

int aosl_hal_sk_get_sockname(int sockfd, aosl_sockaddr_t *addr)
{
	struct sockaddr_in6 com_addr = {0};
	struct sockaddr *n_addr = (struct sockaddr *)&com_addr;
	socklen_t addrlen = sizeof(com_addr);
	int err = getsockname(sockfd, n_addr, &addrlen);
	if (err == 0) {
		conv_addr_to_aosl(n_addr, addr);
	}
	return err;
}

int aosl_hal_gethostbyname(const char *hostname, aosl_sockaddr_t *addrs, int addr_count)
{
	struct addrinfo *res = NULL;
	int count = 0;

	int err = getaddrinfo (hostname, NULL, NULL, &res);
	if (err != 0) {
		return 0;
	}

	struct addrinfo *ai = NULL;
	struct sockaddr *n_addr = NULL;
	for (ai = res; ai != NULL && count < addr_count; ai = ai->ai_next) {
		switch (ai->ai_family) {
		case AF_INET:
			/* fall through */
		case AF_INET6:
			n_addr = ai->ai_addr;
			conv_addr_to_aosl(n_addr, &addrs[count]);
			count++;
			break;
		default:
			break;
		}
	}

	if (res != NULL) {
		freeaddrinfo (res);
	}

	return count;
}