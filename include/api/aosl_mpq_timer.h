/***************************************************************************
 * Module:	Multiplex queue timer header file
 *
 * Copyright © 2025 Agora
 * This file is part of AOSL, an open source project.
 * Licensed under the Apache License, Version 2.0, with certain conditions.
 * Refer to the "LICENSE" file in the root directory for more information.
 ***************************************************************************/

#ifndef __MPQ_TIMER_H__
#define __MPQ_TIMER_H__

#include <api/aosl_types.h>
#include <api/aosl_defs.h>
#include <api/aosl_time.h>
#include <api/aosl_mpq.h>


#ifdef __cplusplus
extern "C" {
#endif


/* The timer object type */
typedef int aosl_timer_t;

#define AOSL_MPQ_TIMER_INVALID ((aosl_timer_t)-1)

#define aosl_mpq_timer_invalid(timer_id) (((int16_t)(timer_id)) < 0)

/* The proto for a timer-callback function. */
typedef void (*aosl_timer_func_t) (aosl_timer_t timer_id, const aosl_ts_t *now_p, uintptr_t argc, uintptr_t argv []);


#define AOSL_INVALID_TIMER_INTERVAL ((uintptr_t)(-1))

extern __aosl_api__ aosl_timer_t aosl_mpq_create_timer (uintptr_t interval, aosl_timer_func_t func, aosl_obj_dtor_t dtor, uintptr_t argc, ...);
extern __aosl_api__ aosl_timer_t aosl_mpq_create_timer_on_q (aosl_mpq_t qid, uintptr_t interval, aosl_timer_func_t func, aosl_obj_dtor_t dtor, uintptr_t argc, ...);

extern __aosl_api__ aosl_timer_t aosl_mpq_create_oneshot_timer (aosl_timer_func_t func, aosl_obj_dtor_t dtor, uintptr_t argc, ...);
extern __aosl_api__ aosl_timer_t aosl_mpq_create_oneshot_timer_on_q (aosl_mpq_t qid, aosl_timer_func_t func, aosl_obj_dtor_t dtor, uintptr_t argc, ...);

extern __aosl_api__ aosl_timer_t aosl_mpq_set_timer (uintptr_t interval, aosl_timer_func_t func, aosl_obj_dtor_t dtor, uintptr_t argc, ...);
extern __aosl_api__ aosl_timer_t aosl_mpq_set_timer_on_q (aosl_mpq_t qid, uintptr_t interval, aosl_timer_func_t func, aosl_obj_dtor_t dtor, uintptr_t argc, ...);

extern __aosl_api__ aosl_timer_t aosl_mpq_set_oneshot_timer (aosl_ts_t expire_time, aosl_timer_func_t func, aosl_obj_dtor_t dtor, uintptr_t argc, ...);
extern __aosl_api__ aosl_timer_t aosl_mpq_set_oneshot_timer_on_q (aosl_mpq_t qid, aosl_ts_t expire_time, aosl_timer_func_t func, aosl_obj_dtor_t dtor, uintptr_t argc, ...);

extern __aosl_api__ int aosl_mpq_timer_interval (aosl_timer_t timer_id, uintptr_t *interval_p);

/* determine whether a timer is active returned in *active_p, 1 for active, 0 for inactive */
extern __aosl_api__ int aosl_mpq_timer_active (aosl_timer_t timer_id, int *active_p);

/* if you do not want to change the timer interval, set it to 'AOSL_INVALID_TIMER_INTERVAL' */
extern __aosl_api__ int aosl_mpq_resched_timer (aosl_timer_t timer_id, uintptr_t interval);
extern __aosl_api__ int aosl_mpq_resched_oneshot_timer (aosl_timer_t timer_id, aosl_ts_t expire_time);

extern __aosl_api__ int aosl_mpq_cancel_timer (aosl_timer_t timer_id);

/**
 * Get the N-th argument of the timer object specified by timer_id.
 * Parameters:
 *  timer_id: the timer id you want to retrieve the arg
 *         n: which argument you want to get, the first arg is 0;
 *     arg_p: the argument variable address to save the argument value;
 * Return value:
 *        <0: error occured, and errno indicates which error;
 *         0: call successful, and '*arg' is value of the N-th argument;
 **/
extern __aosl_api__ int aosl_mpq_timer_arg (aosl_timer_t timer_id, uintptr_t n, uintptr_t *arg_p);

extern __aosl_api__ int aosl_mpq_kill_timer (aosl_timer_t timer_id);



#ifdef __cplusplus
}
#endif


#endif /* __MPQ_TIMER_H__ */
