/***************************************************************************
 * Module:	AOSL regular file operations implementation file
 *
 * Copyright © 2025 Agora
 * This file is part of AOSL, an open source project.
 * Licensed under the Apache License, Version 2.0, with certain conditions.
 * Refer to the "LICENSE" file in the root directory for more information.
 ***************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <api/aosl_types.h>
#include <api/aosl_mm.h>
#include <api/aosl_log.h>

#include <kernel/err.h>
#include <kernel/fileobj.h>
#include <kernel/thread.h>

#define UNUSED(expr) (void)(expr)

static k_rwlock_t fds_lock;
static uint32_t __fobj_life_id = 0;

static int max_fd_alloc = -1;
static int max_fd_curr = -1;
static struct file_obj **attached_fds = NULL;
static uintptr_t fds_count = 0;

static __always_inline int expand_fdtable_locked (int nr)
{
	int new_max = max_fd_alloc;
	int inc_step = 16;
	struct file_obj **new_fds;

	while (nr > new_max)
		new_max += inc_step;

	new_fds = aosl_malloc ((new_max + 1) * sizeof (struct file_obj *));
	if (new_fds == NULL)
		return -1;

	if (max_fd_curr >= 0) {
		memcpy (new_fds, attached_fds, (max_fd_curr + 1) * sizeof (struct file_obj *));
		aosl_free (attached_fds);
	}

	memset (&new_fds [max_fd_curr + 1], 0, (new_max - max_fd_curr) * sizeof (struct file_obj *));
	attached_fds = new_fds;
	max_fd_alloc = new_max;
	return 0;
}

int install_fd (int fd, struct file_obj *f)
{
	int err;
	struct file_obj **__fds;

	if (aosl_fd_invalid (fd))
		return -AOSL_EBADF;

	k_rwlock_wrlock (&fds_lock);
	if (fd > max_fd_alloc && expand_fdtable_locked (fd) < 0) {
		err = -AOSL_ENOMEM;
		goto ____out;
	}

	__fds = attached_fds;
	if (__fds [fd] != NULL) {
		err = -AOSL_EBUSY;
		goto ____out;
	}

	f->life_id = __fobj_life_id++;
	__fds [fd] = f;

	if (fd > max_fd_curr)
		max_fd_curr = fd;

	fds_count++;
	err = 0;

____out:
	k_rwlock_wrunlock (&fds_lock);
	return err;
}

static __always_inline void remove_file_locked (struct file_obj *f, int fd)
{
	UNUSED(f);
	struct file_obj **__fds = attached_fds;

	__fds [fd] = NULL;

	if (fd == max_fd_curr) {
		while (max_fd_curr >= 0 && __fds [max_fd_curr] == NULL)
			max_fd_curr--;
	}

	fds_count--;
}

int remove_fd (struct file_obj *f)
{
	int fd = f->fd;
	int err = 0;

	k_rwlock_wrlock (&fds_lock);
	if (aosl_fd_invalid (fd) || fd > max_fd_curr || attached_fds [fd] != f) {
		err = -AOSL_ENOENT;
		goto ____out;
	}

	remove_file_locked (f, fd);

____out:
	k_rwlock_wrunlock (&fds_lock);
	return err;
}

static __always_inline struct file_obj *__fcheck (int fd)
{
	if (fd > max_fd_curr)
		return NULL;

	return attached_fds [fd];
}

struct file_obj *fget (int fd)
{
	if (!aosl_fd_invalid (fd)) {
		struct file_obj *f;

		k_rwlock_rdlock (&fds_lock);
		f = __fcheck (fd);
		if (f != NULL)
			__fget (f);
		k_rwlock_rdunlock (&fds_lock);

		return f;
	}

	return NULL;
}

void fput (struct file_obj *f)
{
	if (atomic_dec_and_test (&f->usage)) {
		if (f->dtor != NULL)
			f->dtor (f);

		aosl_free (f);
	}
}

void fileobj_init (void)
{
	k_rwlock_init (&fds_lock);
}

void fileobj_fini (void)
{
	if (attached_fds) {
		for (int i = 0; i < max_fd_alloc; i++) {
			if (attached_fds [i] != NULL) {
				AOSL_LOG_ERR("[dtor] no free");
			}
		}
		aosl_free (attached_fds);
		attached_fds = NULL;
	}
	max_fd_alloc = -1;
	max_fd_curr = -1;
	fds_count = 0;

	k_rwlock_destroy (&fds_lock);
}