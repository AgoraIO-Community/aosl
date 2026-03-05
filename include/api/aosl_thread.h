/***************************************************************************
 * Module:	AOSL thread relative definitions
 *
 * Copyright © 2025 Agora
 * This file is part of AOSL, an open source project.
 * Licensed under the Apache License, Version 2.0, with certain conditions.
 * Refer to the "LICENSE" file in the root directory for more information.
 ***************************************************************************/

#ifndef __AOSL_THREAD_H__
#define __AOSL_THREAD_H__

#include <api/aosl_types.h>
#include <api/aosl_defs.h>
#include <hal/aosl_hal_thread.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef uintptr_t aosl_tls_key_t;

extern __aosl_api__ int aosl_tls_key_create (aosl_tls_key_t *key);
extern __aosl_api__ void *aosl_tls_key_get (aosl_tls_key_t key);
extern __aosl_api__ int aosl_tls_key_set (aosl_tls_key_t key, void *value);
extern __aosl_api__ int aosl_tls_key_delete (aosl_tls_key_t key);


typedef void *aosl_lock_t;

extern __aosl_api__ aosl_lock_t aosl_lock_create (void);
extern __aosl_api__ void aosl_lock_lock (aosl_lock_t lock);
extern __aosl_api__ int aosl_lock_trylock (aosl_lock_t lock);
extern __aosl_api__ void aosl_lock_unlock (aosl_lock_t lock);
extern __aosl_api__ void aosl_lock_destroy (aosl_lock_t lock);


/**
 * @brief Static lock type for static declaration
 * This type can be statically declared and initialized at compile time.
 */
typedef struct {
	aosl_static_mutex_t hal_mutex;  /**< HAL layer static mutex */
	intptr_t state;                 /**< Atomic initialization state */
} aosl_static_lock_t;

/**
 * @brief Static lock initializer macro
 * Use this macro to initialize a static lock at compile time:
 * static aosl_static_lock_t my_lock = AOSL_STATIC_LOCK_INIT;
 */
#define AOSL_STATIC_LOCK_INIT { \
	.hal_mutex = AOSL_STATIC_MUTEX_INIT, \
	.state = 0 \
}

extern __aosl_api__ int aosl_static_lock_init (aosl_static_lock_t *lock);
extern __aosl_api__ int aosl_static_lock_lock (aosl_static_lock_t *lock);
extern __aosl_api__ int aosl_static_lock_trylock (aosl_static_lock_t *lock);
extern __aosl_api__ int aosl_static_lock_unlock (aosl_static_lock_t *lock);


typedef void *aosl_rwlock_t;

extern __aosl_api__ aosl_rwlock_t aosl_rwlock_create (void);
extern __aosl_api__ void aosl_rwlock_rdlock (aosl_rwlock_t rwlock);
extern __aosl_api__ int aosl_rwlock_tryrdlock (aosl_rwlock_t rwlock);
extern __aosl_api__ void aosl_rwlock_wrlock (aosl_rwlock_t rwlock);
extern __aosl_api__ int aosl_rwlock_trywrlock (aosl_rwlock_t rwlock);
extern __aosl_api__ void aosl_rwlock_rdunlock (aosl_rwlock_t rwlock);
extern __aosl_api__ void aosl_rwlock_wrunlock (aosl_rwlock_t rwlock);
extern __aosl_api__ void aosl_rwlock_rd2wrlock (aosl_rwlock_t rwlock);
extern __aosl_api__ void aosl_rwlock_wr2rdlock (aosl_rwlock_t rwlock);
extern __aosl_api__ void aosl_rwlock_destroy (aosl_rwlock_t rwlock);


//typedef void *aosl_cond_t;

extern __aosl_api__ aosl_cond_t aosl_cond_create (void);
extern __aosl_api__ void aosl_cond_signal (aosl_cond_t cond_var);
extern __aosl_api__ void aosl_cond_broadcast (aosl_cond_t cond_var);
extern __aosl_api__ void aosl_cond_wait (aosl_cond_t cond_var, aosl_lock_t lock);
extern __aosl_api__ int aosl_cond_timedwait (aosl_cond_t cond_var, aosl_lock_t lock, intptr_t timeo);
extern __aosl_api__ void aosl_cond_destroy (aosl_cond_t cond_var);

typedef void *aosl_event_t;

extern __aosl_api__ aosl_event_t aosl_event_create (void);
extern __aosl_api__ void aosl_event_set (aosl_event_t event_var);
extern __aosl_api__ void aosl_event_pulse (aosl_event_t event_var);
extern __aosl_api__ void aosl_event_wait (aosl_event_t event_var);
extern __aosl_api__ int aosl_event_timedwait (aosl_event_t event_var, intptr_t timeo);
extern __aosl_api__ void aosl_event_reset (aosl_event_t event_var);
extern __aosl_api__ void aosl_event_destroy (aosl_event_t event_var);


#ifdef __cplusplus
}
#endif

#endif /* __AOSL_THREAD_H__ */