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

#define CHECK(cond, ...) \
  if (cond) { \
    aosl_printf("check %s success.\n", #cond); \
  } else { \
    aosl_printf("check %s failed.\n", #cond); \
  }

#define CHECK_FMT(cond, fmt, ...) \
  if (cond) { \
    aosl_printf("check %s success." fmt "\n", #cond, ##__VA_ARGS__); \
  } else { \
    aosl_printf("check %s failed. " fmt "\n", #cond, ##__VA_ARGS__); \
  }

void aosl_test_hal(void)
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
  return;
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

const char *server_ip = "127.0.0.1";
const uint16_t server_port = 9527;
static struct test_mpq_server_res mpq_server_res = {0};
static struct test_mpq_client_res mpq_client_res = {0};

static void mpq_server_on_data(void *data, size_t len, uintptr_t argc, uintptr_t argv[], const aosl_sk_addr_t *addr)
{
  UNUSED(argc);
  UNUSED(addr);
  struct test_mpq_server_res *server_res = (struct test_mpq_server_res *)argv[0];
  char *msg = (char*)data;
  aosl_printf("len=%d recv msg %s\n", (int)len, msg);
  server_res->recv_cnt++;
}

static void mpq_server_on_event(aosl_fd_t fd, int event, uintptr_t argc, uintptr_t argv[])
{
  UNUSED(argc);
  UNUSED(argv);
  if (event >= 0) {
    return;
  }
  aosl_printf("server_on_event: fd=%d event=%d\n", fd, event);
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
  aosl_printf("client_on_event: fd=%d event=%d\n", fd, event);
}

static int test_mpq_server_init (void *arg)
{
  UNUSED(arg);
  int ret;
  mpq_server_res.recv_cnt = 0;
  mpq_server_res.sk = -1;
  int fd = aosl_socket(AOSL_AF_INET, AOSL_SOCK_DGRAM, AOSL_IPPROTO_UDP);
  CHECK(!aosl_fd_invalid(fd));

  aosl_sockaddr_t addr = {0};
  addr.sa_family = AOSL_AF_INET;
  addr.sa_port = aosl_htons(server_port);
  aosl_inet_addr_from_string(&addr.sin_addr, server_ip);
  ret = aosl_bind(fd, &addr);
  CHECK_FMT(ret == 0, "aosl_ip_sk_bind");
  ret = aosl_mpq_add_dgram_socket(fd, 1400, mpq_server_on_data, mpq_server_on_event, 1, &mpq_server_res);
  CHECK_FMT(ret == 0, "aosl_mpq_add_dgram_socket");
  mpq_server_res.sk = fd;
  return 0;
}

static void test_mpq_server_fini (void *arg)
{
  UNUSED(arg);
  aosl_close(mpq_server_res.sk);
}

static int test_mpq_client_init(void *arg)
{
  UNUSED(arg);
  int ret;
  mpq_client_res.sent_cnt = 0;
  mpq_client_res.sk = -1;
  int fd = aosl_socket(AOSL_AF_INET, AOSL_SOCK_DGRAM, AOSL_IPPROTO_UDP);
  CHECK(!aosl_fd_invalid(fd));

  aosl_sockaddr_t addr = {0};
  addr.sa_family = AOSL_AF_INET;
  addr.sa_port = aosl_htons(server_port);
  aosl_inet_addr_from_string(&addr.sin_addr, server_ip);
  mpq_client_res.server_addr = addr;

  ret = aosl_ip_sk_bind_port_only(fd, AOSL_AF_INET, 0);
  CHECK_FMT(ret == 0, "aosl_ip_sk_bind_port_only");
  ret = aosl_mpq_add_dgram_socket(fd, 1400, mpq_client_on_data, mpq_client_on_event, 1, &mpq_client_res);
  CHECK_FMT(ret == 0, "aosl_mpq_add_dgram_socket");
  mpq_client_res.sk = fd;
  return 0;
}

static void test_mpq_client_fini (void *arg)
{
  UNUSED(arg);
  aosl_close(mpq_client_res.sk);
}

static void test_mpq_client_send_func(const aosl_ts_t *queued_ts_p, aosl_refobj_t robj, uintptr_t argc, uintptr_t argv [])
{
  UNUSED(queued_ts_p);
  UNUSED(robj);
  UNUSED(argc);
  struct test_mpq_client_res *client_res = (struct test_mpq_client_res *)argv[0];
  char msg[1024] = {0};
  sprintf(msg, "this is the %d cnt client sent msg!", client_res->sent_cnt++);
  int ret = aosl_sendto(client_res->sk, msg, sizeof(msg), 0, &client_res->server_addr);
  aosl_printf("ret=%d send msg %s\n", ret, msg);
}

static void aosl_test_mpq(void)
{
  int priority = AOSL_THRD_PRI_DEFAULT;
  aosl_mpq_t q_server = aosl_mpq_create(priority, 0, 10000, "udp-server",
      test_mpq_server_init, test_mpq_server_fini, NULL);
  CHECK(!aosl_mpq_invalid(q_server));

  aosl_mpq_t q_client = aosl_mpq_create(priority, 0, 10000, "udp-client",
      test_mpq_client_init, test_mpq_client_fini, NULL);
  CHECK(!aosl_mpq_invalid(q_client));

  // client send msg
  int cnt_cycs = 20;
  int cnt_pers = 50;
  int cnt_alls = cnt_cycs * cnt_pers;
  for (int i = 0; i < cnt_cycs; i++) {
    for (int j = 0; j < cnt_pers; j++) {
      aosl_mpq_queue(q_client, AOSL_MPQ_INVALID, AOSL_REF_INVALID, "test_mpq_client_send_func",
          test_mpq_client_send_func, 1, &mpq_client_res);
    }
    aosl_msleep(500);
  }


  // check cnts
  aosl_ts_t start_ts = aosl_tick_ms();
  while (mpq_server_res.recv_cnt < cnt_alls && (aosl_tick_ms() - start_ts) < 5000) {
    aosl_msleep(100);
  }
  aosl_mpq_destroy_wait(q_server);
  aosl_mpq_destroy_wait(q_client);
  aosl_printf("mpq_server_res.recv_cnt=%d\n", mpq_server_res.recv_cnt);
  aosl_printf("mpq_client_res.sent_cnt=%d\n", mpq_client_res.sent_cnt);
  CHECK(mpq_server_res.recv_cnt == cnt_alls);
  CHECK(mpq_client_res.sent_cnt == cnt_alls);
}

__export_in_so__ void aosl_test(void)
{
  aosl_ctor();

  aosl_test_hal();
  aosl_test_mpq();

  aosl_dtor();

  return;
}