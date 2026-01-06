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

static int aosl_test_hal_atomic()
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

static int aosl_test_hal_errno()
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

static int aosl_test_hal_file()
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

static int aosl_test_hal_iomp()
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

static int aosl_test_hal_socket()
{
  return 0;
}

static int aosl_test_hal_thread()
{
  return 0;
}

static int aosl_test_hal_time()
{
  return 0;
}

static int aosl_test_hal_utils()
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

  ret = aosl_ip_sk_bind_port_only(fd, AOSL_AF_INET, 0);
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