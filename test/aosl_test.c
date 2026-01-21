/***************************************************************************
 * Module:	aosl test
 *
 * Copyright © 2025 Agora
 * This file is part of AOSL, an open source project.
 * Licensed under the Apache License, Version 2.0, with certain conditions.
 * Refer to the "LICENSE" file in the root directory for more information.
 ***************************************************************************/
#include <stdio.h>

#include "hal/aosl_hal_atomic.h"
#include "hal/aosl_hal_errno.h"
#include "hal/aosl_hal_file.h"
#include "hal/aosl_hal_iomp.h"
#include "hal/aosl_hal_log.h"
#include "hal/aosl_hal_memory.h"
#include "hal/aosl_hal_socket.h"
#include "hal/aosl_hal_thread.h"
#include "hal/aosl_hal_time.h"
#include "hal/aosl_hal_utils.h"

#include "api/aosl.h"
#include "api/aosl_log.h"
#include "api/aosl_mpq.h"
#include "api/aosl_socket.h"
#include "api/aosl_mpq_net.h"

#define UNUSED(expr) (void)(expr)
#define CAST_UINT64(val) ((unsigned long long)val)
#define LOG_FMT(fmt, ...) aosl_printf(fmt "  [%s:%u]" "\n", ##__VA_ARGS__, __FUNCTION__, __LINE__);

// expect
#define EXPECT_EQ(val, expect)                                                                                         \
  if ((val) != (expect)) {                                                                                             \
    LOG_FMT("expect_eq failed, v1=%llu v2=%llu", CAST_UINT64(val), CAST_UINT64(expect));                               \
    return -1;                                                                                                         \
  }
#define EXPECT_NE(val, expect)                                                                                         \
  if ((val) == (expect)) {                                                                                             \
    LOG_FMT("expect_ne failed, v1=%llu v2=%llu", CAST_UINT64(val), CAST_UINT64(expect));                               \
    return -1;                                                                                                         \
  }
#define EXPECT_LT(val, expect)                                                                                         \
  if ((val) >= (expect)) {                                                                                             \
    LOG_FMT("expect_lt failed, v1=%llu v2=%llu", CAST_UINT64(val), CAST_UINT64(expect));                               \
    return -1;                                                                                                         \
  }
#define EXPECT_LE(val, expect)                                                                                         \
  if ((val) > (expect)) {                                                                                              \
    LOG_FMT("expect_le failed, v1=%llu v2=%llu", CAST_UINT64(val), CAST_UINT64(expect));                               \
    return -1;                                                                                                         \
  }
#define EXPECT_GT(val, expect)                                                                                         \
  if ((val) <= (expect)) {                                                                                             \
    LOG_FMT("expect_gt failed, v1=%llu v2=%llu", CAST_UINT64(val), CAST_UINT64(expect));                               \
    return -1;                                                                                                         \
  }
#define EXPECT_GE(val, expect)                                                                                         \
  if ((val) < (expect)) {                                                                                              \
    LOG_FMT("expect_ge failed, v1=%llu v2=%llu", CAST_UINT64(val), CAST_UINT64(expect));                               \
    return -1;                                                                                                         \
  }

// check
#define CHECK(cond)                                                                                                    \
  if (!(cond)) {                                                                                                         \
    LOG_FMT("check %s failed.", #cond);                                                                              \
    return -1;                                                                                                         \
  }

#define CHECK_FMT(cond, fmt, ...)                                                                                      \
  if (!(cond)) {                                                                                                         \
    LOG_FMT("check %s failed. " fmt, #cond, ##__VA_ARGS__);                                                       \
    return -1;                                                                                                         \
  }

static const char *server_ip = "127.0.0.1";
static const uint16_t server_port = 9527;

static int aosl_test_hal_atomic(void)
{
  intptr_t a = 0;

  CHECK(aosl_hal_atomic_read(&a) == 0);
  aosl_hal_atomic_set(&a, 2);
  CHECK(aosl_hal_atomic_read(&a) == 2);
  aosl_hal_atomic_add(3, &a);
  CHECK(aosl_hal_atomic_read(&a) == 5);
  aosl_hal_atomic_sub(1, &a);
  CHECK(aosl_hal_atomic_read(&a) == 4);
  aosl_hal_atomic_inc(&a);
  CHECK(aosl_hal_atomic_read(&a) == 5);
  aosl_hal_atomic_dec(&a);
  CHECK(aosl_hal_atomic_read(&a) == 4);
  CHECK(aosl_hal_atomic_add(10, &a) == 14);
  CHECK(aosl_hal_atomic_sub(5, &a) == 9);
  CHECK(aosl_hal_atomic_cmpxchg(&a, 10, 100) == 9);
  CHECK(aosl_hal_atomic_cmpxchg(&a, 9, 100) == 9);
  CHECK(aosl_hal_atomic_cmpxchg(&a, 9, 100) == 100);
  CHECK(aosl_hal_atomic_xchg(&a, 50) == 100);
  CHECK(aosl_hal_atomic_read(&a) == 50);

  LOG_FMT("test success");
  return 0;
}

static int aosl_test_hal_errno(void)
{
  bool have_EAGAIN = 0;
  bool have_EINTR = 0;
  int ret;
  for (int i = 0; i < 1024; i++) {
    ret = aosl_hal_errno_convert(i);
    if (ret == AOSL_HAL_RET_EAGAIN)
      have_EAGAIN = true;
    else if (ret == AOSL_HAL_RET_EINTR)
      have_EINTR = true;
  }

  CHECK(have_EAGAIN == true);
  CHECK(have_EINTR == true);
  LOG_FMT("test success");
  return 0;
}

static int aosl_test_hal_file(void)
{
  // ignore
  return 0;
}

#if defined(AOSL_HAL_HAVE_EPOLL) && (AOSL_HAL_HAVE_EPOLL == 1)
static int aosl_test_hal_iomp_epoll(int server_fd, int client_fd, const aosl_sockaddr_t *server_addr)
{
  int ret;
  aosl_poll_event_t event = {0};
  char snd_buf[100] = {0};
  char rcv_buf[100] = {0};
  int epfd = aosl_hal_epoll_create();
  CHECK(epfd >= 0);

  event.fd = server_fd;
  event.events = AOSL_POLLIN;
  ret = aosl_hal_epoll_ctl(epfd, AOSL_POLL_CTL_ADD, server_fd, &event);
  if (ret != 0) {
    LOG_FMT("epoll ctl add failed");
    goto __tag_out;
  }

  for (int i = 0; i < 10; i++) {
    sprintf(snd_buf, "iomp test msg [%d]", i);
    ret = aosl_hal_sk_sendto(client_fd, snd_buf, sizeof(snd_buf), 0, server_addr);
    if (ret != sizeof(snd_buf)) {
      LOG_FMT("[%d] send failed, ret=%d", i, ret);
      ret = -1;
      goto __tag_out;
    }

    memset(&event, 0, sizeof(event));
    ret = aosl_hal_epoll_wait(epfd, &event, 1, 1000);
    if (ret <= 0) {
      LOG_FMT("[%d] epoll failed, ret=%d", i, ret);
      ret = -1;
      goto __tag_out;
    }

    if (!(event.events & AOSL_POLLIN)) {
      LOG_FMT("[%d] fd check failed", i);
      ret = -1;
      goto __tag_out;
    }

    ret = aosl_hal_sk_recvfrom(server_fd, rcv_buf, sizeof(rcv_buf), 0, NULL);
    if (ret != sizeof(rcv_buf)) {
      LOG_FMT("[%d] recvfrom failed, ret=%d", i, ret);
      ret = -1;
      goto __tag_out;
    }
    LOG_FMT("rcv_msg='%s'", rcv_buf);
  }

  ret = 0;

__tag_out:
  aosl_hal_epoll_destroy(epfd);
  return ret;
}
#endif

#if defined(AOSL_HAL_HAVE_POLL) && (AOSL_HAL_HAVE_POLL == 1)
static int aosl_test_hal_iomp_poll(int server_fd, int client_fd, const aosl_sockaddr_t *server_addr)
{
  int ret;
  aosl_poll_event_t event = {0};
  char snd_buf[100] = {0};
  char rcv_buf[100] = {0};

  for (int i = 0; i < 10; i++) {
    sprintf(snd_buf, "iomp test msg [%d]", i);
    ret = aosl_hal_sk_sendto(client_fd, snd_buf, sizeof(snd_buf), 0, server_addr);
    if (ret != sizeof(snd_buf)) {
      LOG_FMT("[%d] send failed, ret=%d", i, ret);
      ret = -1;
      goto __tag_out;
    }

    memset(&event, 0, sizeof(event));
    event.fd = server_fd;
    event.events = AOSL_POLLIN;
    ret = aosl_hal_poll(&event, 1, 1000);
    if (ret <= 0) {
      LOG_FMT("[%d] epoll failed, ret=%d", i, ret);
      ret = -1;
      goto __tag_out;
    }

    if (!(event.revents & AOSL_POLLIN)) {
      LOG_FMT("[%d] fd check failed", i);
      ret = -1;
      goto __tag_out;
    }

    ret = aosl_hal_sk_recvfrom(server_fd, rcv_buf, sizeof(rcv_buf), 0, NULL);
    if (ret != sizeof(rcv_buf)) {
      LOG_FMT("[%d] recvfrom failed, ret=%d", i, ret);
      ret = -1;
      goto __tag_out;
    }
    LOG_FMT("rcv_msg='%s'", rcv_buf);
  }

  ret = 0;

__tag_out:
  return ret;
}
#endif

#if defined(AOSL_HAL_HAVE_SELECT) && (AOSL_HAL_HAVE_SELECT == 1)
static int aosl_test_hal_iomp_select(int server_fd, int client_fd, const aosl_sockaddr_t *server_addr)
{
  int ret;
  char snd_buf[100] = {0};
  char rcv_buf[100] = {0};
  fd_set_t *fdset = aosl_hal_fdset_create();
  CHECK(fdset != NULL);

  for (int i = 0; i < 10; i++) {
    sprintf(snd_buf, "iomp test msg [%d]", i);
    ret = aosl_hal_sk_sendto(client_fd, snd_buf, sizeof(snd_buf), 0, server_addr);
    if (ret != sizeof(snd_buf)) {
      LOG_FMT("[%d] send failed, ret=%d", i, ret);
      ret = -1;
      goto __tag_out;
    }

    aosl_hal_fdset_zero(fdset);
    aosl_hal_fdset_set(fdset, server_fd);
    ret = aosl_hal_select(server_fd + 1, fdset, NULL, NULL, 1000);
    if (ret <= 0) {
      LOG_FMT("[%d] select failed, ret=%d", i, ret);
      ret = -1;
      goto __tag_out;
    }
    ret = aosl_hal_fdset_isset(fdset, server_fd);
    if (ret != 1) {
      LOG_FMT("[%d] fdset check failed", i);
      ret = -1;
      goto __tag_out;
    }

    ret = aosl_hal_sk_recvfrom(server_fd, rcv_buf, sizeof(rcv_buf), 0, NULL);
    if (ret != sizeof(rcv_buf)) {
      LOG_FMT("[%d] recvfrom failed, ret=%d", i, ret);
      ret = -1;
      goto __tag_out;
    }
    LOG_FMT("rcv_msg='%s'", rcv_buf);
  }

  ret = 0;

__tag_out:
  aosl_hal_fdset_destroy(fdset);
  return ret;
}
#endif

static int aosl_test_hal_iomp(void)
{
  int ret;
  int client_fd = -1;
  int server_fd = -1;

  // server
  server_fd = aosl_socket(AOSL_AF_INET, AOSL_SOCK_DGRAM, AOSL_IPPROTO_UDP);
  if (server_fd < 0) {
    LOG_FMT("get server socket failed, fd=%d", server_fd);
    return -1;
  }

  aosl_sockaddr_t server_addr = {0};
  server_addr.sa_family = AOSL_AF_INET;
  server_addr.sa_port = aosl_htons(server_port);
  aosl_inet_addr_from_string(&server_addr.sin_addr, server_ip);
  ret = aosl_hal_sk_bind(server_fd, &server_addr);
  if (ret != 0) {
    LOG_FMT("bind failed, ret=%d", ret);
    goto __tag_out;
  }
  ret = aosl_hal_sk_set_nonblock(server_fd);
  if (ret != 0) {
    LOG_FMT("set nonblock failed, ret=%d", ret);
    goto __tag_out;
  }

  // client
  client_fd = aosl_socket(AOSL_AF_INET, AOSL_SOCK_DGRAM, AOSL_IPPROTO_UDP);
  if (client_fd < 0) {
    LOG_FMT("get client socket failed, fd=%d", client_fd);
    ret = -1;
    goto __tag_out;
  }

  // test iomp
#if defined(AOSL_HAL_HAVE_EPOLL) && (AOSL_HAL_HAVE_EPOLL == 1)
  ret = aosl_test_hal_iomp_epoll(server_fd, client_fd, &server_addr);
#elif defined(AOSL_HAL_HAVE_POLL) && (AOSL_HAL_HAVE_POLL == 1)
  ret = aosl_test_hal_iomp_poll(server_fd, client_fd, &server_addr);
#elif defined(AOSL_HAL_HAVE_SELECT) && (AOSL_HAL_HAVE_SELECT == 1)
  ret = aosl_test_hal_iomp_select(server_fd, client_fd, &server_addr);
#else
  ret = -1;
  LOG_FMT(0, "not impl iomp");
#endif

__tag_out:
  if (server_fd >= 0) {
    aosl_hal_sk_close(server_fd);
  }
  if (client_fd >= 0) {
    aosl_hal_sk_close(client_fd);
  }

  CHECK(ret == 0);

  LOG_FMT("test success");
  return 0;
}


static int aosl_test_hal_socket_trans_udp(char *server_ip)
{
  aosl_sockaddr_t addr = {0};
  const char *dns_server = "8.8.8.8";
  addr.sa_family = AOSL_AF_INET;
  addr.sa_port = aosl_htons(53);
  aosl_inet_addr_from_string(&addr.sin_addr, dns_server);

  int fd = aosl_socket(AOSL_AF_INET, AOSL_SOCK_DGRAM, AOSL_IPPROTO_UDP);
  if (aosl_fd_invalid(fd)) {
    LOG_FMT("create socket failed, fd=%d", fd);
    return -1;
  }

  // Construct a simple DNS query (A record for ipinfo.io)
  char dns_query[512];
  int query_len = 0;
  unsigned char query[] = {
      0xAA, 0xAA,  // random ID
      0x01, 0x00,  // standard query
      0x00, 0x01,  // 1 question
      0x00, 0x00,  // 0 answers
      0x00, 0x00,  // 0 authority
      0x00, 0x00,  // 0 additional
      // query ipinfo.io
      0x06, 'i', 'p', 'i', 'n', 'f', 'o',
      0x02, 'i', 'o',
      0x00,        // termination
      0x00, 0x01,  // query type A
      0x00, 0x01   // query class IN
  };
  memcpy(dns_query, query, sizeof(query));
  query_len = sizeof(query);

  int sent = aosl_hal_sk_sendto(fd, dns_query, query_len, 0, &addr);
  if (sent < 0) {
    LOG_FMT("sendto failed, ret=%d", sent);
    goto __tag_failed;
  }
  char buffer[1024];
  int received = aosl_hal_sk_recvfrom(fd, buffer, sizeof(buffer), 0, NULL);
  if (received <= 0) {
    LOG_FMT("recvfrom failed, ret=%d", received);
    goto __tag_failed;
  }

  // Parse DNS response and print IPv4 A records
  if (received >= 12) {
    unsigned char *p = (unsigned char *)buffer;
    int offset = 0;
    if (received < 12) {
      LOG_FMT("dns response too short, len=%d", received);
      goto __tag_failed;
    }
    uint16_t qdcount = (p[4] << 8) | p[5];
    uint16_t ancount = (p[6] << 8) | p[7];
    offset = 12;

    // Skip question section
    for (uint16_t qi = 0; qi < qdcount; qi++) {
      if (offset >= received) break;
      // Skip name label chain
      while (offset < received && p[offset] != 0) {
        // If it's a pointer (compressed form)
        if ((p[offset] & 0xC0) == 0xC0) {
          offset += 2;
          break;
        }
        unsigned int labellen = p[offset];
        offset += 1 + labellen;
      }
      if (offset < received && p[offset] == 0) offset++;
      // Skip qtype(2) + qclass(2)
      offset += 4;
    }

    // Parse answer section
    for (uint16_t ai = 0; ai < ancount; ai++) {
      if (offset + 10 > received) break; // Need at least type/class/ttl/rdlength

      // Skip name (may be pointer or label chain)
      if ((p[offset] & 0xC0) == 0xC0) {
        offset += 2;
      } else {
        while (offset < received && p[offset] != 0) {
          if ((p[offset] & 0xC0) == 0xC0) { offset += 2; break; }
          unsigned int labellen = p[offset];
          offset += 1 + labellen;
        }
        if (offset < received && p[offset] == 0) offset++;
      }

      if (offset + 10 > received) break;
      uint16_t type = (p[offset] << 8) | p[offset + 1];
      uint16_t clas = (p[offset + 2] << 8) | p[offset + 3];
      (void)clas;
      // ttl 4 bytes
      uint16_t rdlen = (p[offset + 8] << 8) | p[offset + 9];
      offset += 10;
      if (offset + rdlen > received) break;

      // A record
      if (type == 1 && rdlen == 4) {
        snprintf(server_ip, 16, "%u.%u.%u.%u", p[offset], p[offset + 1], p[offset + 2], p[offset + 3]);
        LOG_FMT("got ipinfo.io : A record: %u.%u.%u.%u", p[offset], p[offset + 1], p[offset + 2], p[offset + 3]);
      }

      offset += rdlen;
    }
  } else {
    LOG_FMT("dns response too short, len=%d", received);
    goto __tag_failed;
  }

  aosl_hal_sk_close(fd);
  LOG_FMT("test success");
  return 0;

__tag_failed:
  aosl_hal_sk_close(fd);
  return -1;
}

static int aosl_test_hal_socket_trans_tcp(char *server_ip)
{
  aosl_sockaddr_t addr = {0};
  addr.sa_family = AOSL_AF_INET;
  addr.sa_port = aosl_htons(80);
  aosl_inet_addr_from_string(&addr.sin_addr, server_ip);

  int fd = aosl_socket(AOSL_AF_INET, AOSL_SOCK_STREAM, AOSL_IPPROTO_TCP);
  if (aosl_fd_invalid(fd)) {
    LOG_FMT("create tcp socket failed, fd=%d", fd);
    return -1;
  }

  const char *host = "ipinfo.io";
  if (aosl_hal_sk_connect(fd, &addr) != 0) {
    LOG_FMT("connect to %s (IP %s):80 failed", host, server_ip);
    aosl_hal_sk_close(fd);
    return -1;
  }

  char req[256];
  int req_len = snprintf(req, sizeof(req), "GET /ip HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n", host);
  if (req_len <= 0 || req_len >= (int)sizeof(req)) {
    LOG_FMT("build http request failed");
    aosl_hal_sk_close(fd);
    return -1;
  }

  int sent = aosl_hal_sk_send(fd, req, req_len, 0);
  if (sent < 0) {
    LOG_FMT("send http request failed, ret=%d", sent);
    aosl_hal_sk_close(fd);
    return -1;
  }

  char resp[1024];
  int total = 0;
  while (total < (int)sizeof(resp) - 1) {
    int r = aosl_hal_sk_recv(fd, resp + total, sizeof(resp) - 1 - total, 0);
    if (r < 0) {
      LOG_FMT("recv failed, ret=%d", r);
      aosl_hal_sk_close(fd);
      return -1;
    }
    if (r == 0) break; // remote closed
    total += r;
  }
  resp[total] = '\0';

  // find header-body separator
  char *body = NULL;
  char *sep = NULL;
  sep = strstr(resp, "\r\n\r\n");
  if (sep) body = sep + 4;
  else {
    // maybe only LF
    sep = strstr(resp, "\n\n");
    if (sep) body = sep + 2;
  }

  if (!body) {
    LOG_FMT("invalid http response, no header/body separator");
    aosl_hal_sk_close(fd);
    return -1;
  }

  LOG_FMT("http response body: '%s'", body);

  // trim leading/trailing whitespace from body
  while (*body == '\r' || *body == '\n' || *body == ' ' || *body == '\t') body++;
  char *end = body + strlen(body) - 1;
  while (end > body && (*end == '\r' || *end == '\n' || *end == ' ' || *end == '\t')) { *end = '\0'; end--; }

  // validate IPv4 dotted format
  unsigned int a, b, c, d;
  int n = sscanf(body, "%u.%u.%u.%u", &a, &b, &c, &d);
  if (n == 4 && a <= 255 && b <= 255 && c <= 255 && d <= 255) {
    snprintf(server_ip, 16, "%u.%u.%u.%u", a, b, c, d);
    LOG_FMT("got wan ip: %s", server_ip);
    LOG_FMT("test success");
    aosl_hal_sk_close(fd);
    return 0;
  }

  LOG_FMT("invalid ip format in response: '%s'", body);
  aosl_hal_sk_close(fd);
  return -1;
}

static int aosl_test_hal_socket_trans(void)
{
  char server_ip[16] = {0};
  CHECK(aosl_test_hal_socket_trans_udp(server_ip) == 0);
  CHECK(aosl_test_hal_socket_trans_tcp(server_ip) == 0);
  LOG_FMT("test success");
  return 0;
}

static int aosl_test_hal_socket_maxcnt(void)
{
  int fds[20];
  int cnt = 0;

  for (;;) {
    int fd = aosl_hal_sk_socket(AOSL_AF_INET, AOSL_SOCK_DGRAM, AOSL_IPPROTO_UDP);
    if (fd < 0) {
      break;
    }

    if (cnt >= 20) {
      // reached configured cap, close this fd and stop
      aosl_hal_sk_close(fd);
      break;
    }
    fds[cnt++] = fd;
  }

  LOG_FMT("max sockets opened: %d", cnt);

  // close all opened sockets
  for (int i = 0; i < cnt; i++) {
    aosl_hal_sk_close(fds[i]);
  }

  return (cnt >= 10) ? 0 : -1;
}

static int aosl_test_hal_socket_dns(void)
{
  const char *server_name = "ap1.agora.io";
  aosl_sockaddr_t addrs[100] = {0};
  int ret = aosl_hal_gethostbyname(server_name, addrs, 100);
  if (ret < 0) {
    LOG_FMT("dns resolve %s failed, ret=%d", server_name, ret);
    return -1;
  }
  LOG_FMT("dns resolve %s success, ret=%d, result:", server_name, ret);

  for (int i = 0; i < ret; i++) {
    char ip_str[64] = {0};
    aosl_sockaddr_str(&addrs[i], ip_str, sizeof(ip_str));
    LOG_FMT("  addr[%d]: %s", i, ip_str);
  }

  LOG_FMT("test success");
  return 0;
}

static int aosl_test_hal_socket(void)
{
  CHECK(aosl_test_hal_socket_trans() == 0);
  CHECK(aosl_test_hal_socket_maxcnt() == 0);
  CHECK(aosl_test_hal_socket_dns() == 0);
  LOG_FMT("test success");
  return 0;
}

static int aosl_test_hal_thread(void)
{
  
  return 0;
}

static int aosl_test_hal_time(void)
{
  return 0;
}

static int aosl_test_hal_utils(void)
{
  return 0;
}

static int aosl_test_hal(void)
{
  CHECK(aosl_test_hal_atomic() == 0);
  CHECK(aosl_test_hal_errno() == 0);
  CHECK(aosl_test_hal_file() == 0);
  CHECK(aosl_test_hal_iomp() == 0);
  CHECK(aosl_test_hal_socket() == 0);
  CHECK(aosl_test_hal_thread() == 0);
  CHECK(aosl_test_hal_time() == 0);
  CHECK(aosl_test_hal_utils() == 0);
  LOG_FMT("test success");
  return 0;
}

struct test_mpq_server_res {
  int sk;
  int recv_cnt;
};

struct test_mpq_client_res {
  int sk;
  int sent_cnt;
  aosl_sockaddr_t server_addr;
};


static struct test_mpq_server_res mpq_server_res = { 0 };
static struct test_mpq_client_res mpq_client_res = { 0 };

static void mpq_server_on_data(void *data, size_t len, uintptr_t argc, uintptr_t argv[], const aosl_sk_addr_t *addr)
{
  UNUSED(argc);
  UNUSED(addr);
  struct test_mpq_server_res *server_res = (struct test_mpq_server_res *)argv[0];
  char *msg = (char *)data;
  server_res->recv_cnt++;
  if (server_res->recv_cnt % 500 == 1) {
    LOG_FMT("len=%d recv msg %s", (int)len, msg);
  }
}

static void mpq_server_on_event(aosl_fd_t fd, int event, uintptr_t argc, uintptr_t argv[])
{
  UNUSED(argc);
  UNUSED(argv);
  if (event >= 0) {
    return;
  }
  LOG_FMT("fd=%d event=%d\n", fd, event);
}

static void mpq_client_on_data(void *data, size_t len, uintptr_t argc, uintptr_t argv[], const aosl_sk_addr_t *addr)
{
  UNUSED(data);
  UNUSED(len);
  UNUSED(argc);
  UNUSED(argv);
  UNUSED(addr);
}

static void mpq_client_on_event(aosl_fd_t fd, int event, uintptr_t argc, uintptr_t argv[])
{
  UNUSED(argc);
  UNUSED(argv);
  if (event >= 0) {
    return;
  }
  LOG_FMT("fd=%d event=%d\n", fd, event);
}

static int test_mpq_server_init(void *arg)
{
  UNUSED(arg);
  int ret;
  mpq_server_res.recv_cnt = 0;
  mpq_server_res.sk = -1;
  int fd = aosl_socket(AOSL_AF_INET, AOSL_SOCK_DGRAM, AOSL_IPPROTO_UDP);
  CHECK(!aosl_fd_invalid(fd));

  aosl_sockaddr_t addr = { 0 };
  addr.sa_family = AOSL_AF_INET;
  addr.sa_port = aosl_htons(server_port);
  aosl_inet_addr_from_string(&addr.sin_addr, server_ip);
  ret = aosl_bind(fd, &addr);
  EXPECT_EQ(ret, 0);
  ret = aosl_mpq_add_dgram_socket(fd, 1400, mpq_server_on_data, mpq_server_on_event, 1, &mpq_server_res);
  EXPECT_EQ(ret, 0);
  mpq_server_res.sk = fd;
  return 0;
}

static void test_mpq_server_fini(void *arg)
{
  UNUSED(arg);
  aosl_hal_sk_close(mpq_server_res.sk);
}

static int test_mpq_client_init(void *arg)
{
  UNUSED(arg);
  int ret;
  mpq_client_res.sent_cnt = 0;
  mpq_client_res.sk = -1;
  int fd = aosl_socket(AOSL_AF_INET, AOSL_SOCK_DGRAM, AOSL_IPPROTO_UDP);
  CHECK(!aosl_fd_invalid(fd));

  aosl_sockaddr_t addr = { 0 };
  addr.sa_family = AOSL_AF_INET;
  addr.sa_port = aosl_htons(server_port);
  aosl_inet_addr_from_string(&addr.sin_addr, server_ip);
  mpq_client_res.server_addr = addr;

  ret = aosl_bind_port_only(fd, AOSL_AF_INET, 0);
  EXPECT_EQ(ret, 0);
  ret = aosl_mpq_add_dgram_socket(fd, 1400, mpq_client_on_data, mpq_client_on_event, 1, &mpq_client_res);
  EXPECT_EQ(ret, 0);
  mpq_client_res.sk = fd;
  return 0;
}

static void test_mpq_client_fini(void *arg)
{
  UNUSED(arg);
  aosl_hal_sk_close(mpq_client_res.sk);
}

static void test_mpq_client_send_func(const aosl_ts_t *queued_ts_p, aosl_refobj_t robj, uintptr_t argc,
                                      uintptr_t argv[])
{
  UNUSED(queued_ts_p);
  UNUSED(robj);
  UNUSED(argc);
  struct test_mpq_client_res *client_res = (struct test_mpq_client_res *)argv[0];
  char msg[1024] = { 0 };
  sprintf(msg, "this is the %d cnt client sent msg!", client_res->sent_cnt++);
  int ret = aosl_sendto(client_res->sk, msg, sizeof(msg), 0, &client_res->server_addr);
  if (client_res->sent_cnt % 500 == 1) {
    LOG_FMT("ret=%d send msg %s", ret, msg);
  }
}

static int aosl_test_mpq(void)
{
  int priority = AOSL_THRD_PRI_DEFAULT; // default
  int stack_size = 0; // default
  int max_func_size = 10000;
  aosl_mpq_t q_server = aosl_mpq_create(priority, stack_size, max_func_size, "udp-server", test_mpq_server_init,
                                        test_mpq_server_fini, NULL);
  CHECK(!aosl_mpq_invalid(q_server));

  aosl_mpq_t q_client = aosl_mpq_create(priority, stack_size, max_func_size, "udp-client", test_mpq_client_init,
                                        test_mpq_client_fini, NULL);
  CHECK(!aosl_mpq_invalid(q_client));

  // client async send msg
  int cnt_cycs = 20;
  int cnt_pers = 50;
  int cnt_alls = cnt_cycs * cnt_pers * 2;
  for (int i = 0; i < cnt_cycs; i++) {
    for (int j = 0; j < cnt_pers; j++) {
      aosl_mpq_queue(q_client, AOSL_MPQ_INVALID, AOSL_REF_INVALID, "test_mpq_client_send_func",
                     test_mpq_client_send_func, 1, &mpq_client_res);
    }
    aosl_msleep(100);
  }

  // client sync send msg
  for (int i = 0; i < cnt_cycs; i++) {
    for (int j = 0; j < cnt_pers; j++) {
      aosl_mpq_call(q_client, AOSL_REF_INVALID, "test_mpq_client_send_func", test_mpq_client_send_func, 1,
                    &mpq_client_res);
    }
    aosl_msleep(100);
  }

  // check cnts
  aosl_ts_t start_ts = aosl_tick_ms();
  while (mpq_server_res.recv_cnt < cnt_alls && (aosl_tick_ms() - start_ts) < 5000) {
    aosl_msleep(100);
  }
  aosl_mpq_destroy_wait(q_server);
  aosl_mpq_destroy_wait(q_client);
  EXPECT_EQ(mpq_server_res.recv_cnt, cnt_alls);
  EXPECT_EQ(mpq_client_res.sent_cnt, cnt_alls);
  LOG_FMT("test success");
  return 0;
}

__export_in_so__ void aosl_test(void)
{
  aosl_ctor();

  aosl_test_hal();
  aosl_test_mpq();

  aosl_dtor();

  return;
}