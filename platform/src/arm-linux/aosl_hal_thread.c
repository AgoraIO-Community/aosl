#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include <sys/prctl.h>
#include <sys/errno.h>
#include <api/aosl_mm.h>
#include <api/aosl_log.h>
#include <api/aosl_defs.h>
#include <hal/aosl_hal_thread.h>

int aosl_hal_thread_create(aosl_thread_t *thread, aosl_thread_param_t *param,
													 void *(*entry)(void *), void *arg)
{
	assert(sizeof(pthread_t) <= sizeof(aosl_thread_t));

	pthread_t n_td;
	int err = pthread_create (&n_td, NULL, entry, arg);
	if (err < 0) {
		goto __tag_failed;
	}

	*thread = (aosl_thread_t)n_td;

	return 0;

__tag_failed:
	AOSL_LOG_ERR("create failed, err=%d", err);
	return -1;
}

void aosl_hal_thread_destroy(aosl_thread_t thread)
{
}

void aosl_hal_thread_exit(void *retval)
{
	pthread_exit(retval);
}

aosl_thread_t aosl_hal_thread_self()
{
	pthread_t n_td = pthread_self();
	return (aosl_thread_t)n_td;
}

int aosl_hal_thread_set_name(const char *name)
{
	return prctl(PR_SET_NAME, name);
}

static int __get_os_priority (aosl_thread_proiority_e aosl_pri, int *os_pri)
{
	int min_prio = sched_get_priority_min (SCHED_RR);
	int max_prio = sched_get_priority_max (SCHED_RR);
	int top_prio;
	int low_prio;

	if (min_prio == EINVAL || max_prio == EINVAL || max_prio - min_prio <= 2)
		return -1;

	top_prio = max_prio - 1;
	low_prio = min_prio + 1;

	switch (aosl_pri) {
	case AOSL_THRD_PRI_LOW:
		*os_pri = low_prio;
		break;
	case AOSL_THRD_PRI_NORMAL:
		*os_pri = (low_prio + top_prio - 1) / 2;
		break;
	case AOSL_THRD_PRI_HIGH:
		*os_pri = aosl_max (top_prio - 2, low_prio);
		break;
	case AOSL_THRD_PRI_HIGHEST:
		*os_pri = aosl_max (top_prio - 1, low_prio);
		break;
	case AOSL_THRD_PRI_RT:
		*os_pri = top_prio;
		break;
	default:
		return -1;
	}

	return 0;
}

int aosl_hal_thread_set_priority(aosl_thread_proiority_e priority)
{
	int os_pri;
	int err;

	if (__get_os_priority(priority, &os_pri) != 0) {
		return -1;
	}

	struct sched_param param;
	param.sched_priority = os_pri;
	err = pthread_setschedparam(pthread_self(), SCHED_RR, &param);
	if (err < 0) {
		AOSL_LOG_ERR("set priotiry failed, err=%d", err);
		return err;
	}
	return 0;
}

int aosl_hal_thread_join(aosl_thread_t thread, void **retval)
{
	pthread_t n_td = (pthread_t)thread;
	return pthread_join(n_td, retval);
}

void aosl_hal_thread_detach(aosl_thread_t thread)
{
	pthread_t n_td = (pthread_t)thread;
	pthread_detach(n_td);
}

aosl_mutex_t aosl_hal_mutex_create()
{
	int err;
	pthread_mutex_t *n_mutex = aosl_calloc(1, sizeof(pthread_mutex_t));
	if (!n_mutex) {
		return NULL;
	}
	err = pthread_mutex_init(n_mutex, NULL);
	if (err != 0) {
		AOSL_LOG_ERR("mutex create failed, err=%d", err);
		aosl_free(n_mutex);
		return NULL;
	}

	return (aosl_mutex_t)n_mutex;
}

void aosl_hal_mutex_destroy(aosl_mutex_t mutex)
{
	pthread_mutex_t *n_mutex = (pthread_mutex_t *)mutex;
	if (!n_mutex) {
		return;
	}
	pthread_mutex_destroy(n_mutex);
	aosl_free(n_mutex);
}

int aosl_hal_mutex_lock(aosl_mutex_t mutex)
{
	return pthread_mutex_lock((pthread_mutex_t *)mutex);
}

int aosl_hal_mutex_trylock(aosl_mutex_t mutex)
{
	return pthread_mutex_trylock((pthread_mutex_t *)mutex);
}

int aosl_hal_mutex_unlock(aosl_mutex_t mutex)
{
	return pthread_mutex_unlock((pthread_mutex_t *)mutex);
}

aosl_cond_t aosl_hal_cond_create(void)
{
	pthread_cond_t *n_cond = aosl_calloc(1, sizeof(pthread_cond_t));
	if (!n_cond) {
		return NULL;
	}

	int err = pthread_cond_init(n_cond, NULL);
	if (err != 0) {
		AOSL_LOG_ERR("cond create failed, err=%d", err);
		aosl_free(n_cond);
		return NULL;
	}

	return (aosl_cond_t)n_cond;
}

void aosl_hal_cond_destroy(aosl_cond_t cond)
{
	pthread_cond_t *n_cond = (pthread_cond_t *)cond;
	if (!n_cond) {
		return;
	}

	pthread_cond_destroy(n_cond);
	aosl_free(n_cond);
}

int aosl_hal_cond_signal(aosl_cond_t cond)
{
	pthread_cond_t *n_cond = (pthread_cond_t *)cond;
	if (!n_cond) {
		return -1;
	}

	return pthread_cond_signal(n_cond);
}

int aosl_hal_cond_broadcast(aosl_cond_t cond)
{
	pthread_cond_t *n_cond = (pthread_cond_t *)cond;
	if (!n_cond) {
		return -1;
	}

	return pthread_cond_broadcast(n_cond);
}

int aosl_hal_cond_wait(aosl_cond_t cond, aosl_mutex_t mutex)
{
	pthread_cond_t *n_cond = (pthread_cond_t *)cond;
	pthread_mutex_t *n_mutex = (pthread_mutex_t *)mutex;

	if (!n_cond || !n_mutex) {
		return -1;
	}

	return pthread_cond_wait(n_cond, n_mutex);
}

int aosl_hal_cond_timedwait(aosl_cond_t cond, aosl_mutex_t mutex, intptr_t timeout_ms)
{
	pthread_cond_t *n_cond = (pthread_cond_t *)cond;
	pthread_mutex_t *n_mutex = (pthread_mutex_t *)mutex;

	if (!n_cond || !n_mutex) {
		return -1;
	}

	struct timespec timeo;
	struct timespec now;
	clock_gettime (CLOCK_MONOTONIC, &now);
	timeo.tv_sec = now.tv_sec + timeout_ms / 1000;
	timeo.tv_nsec = now.tv_nsec + (timeout_ms % 1000) * 1000000;
	while (timeo.tv_nsec >= 1000000000) {
		timeo.tv_nsec -= 1000000000;
		timeo.tv_sec++;
	}

	return pthread_cond_timedwait(n_cond, n_mutex, &timeo);
}

aosl_sem_t aosl_hal_sem_create(void)
{
	int ret;
	sem_t *sem = aosl_malloc(sizeof(sem_t));
	if (!sem) {
		return NULL;
	}

	ret = sem_init(sem, 0, 1);
	if (ret != 0) {
		aosl_free(sem);
		return NULL;
	}

	return (aosl_sem_t)sem;
}

void aosl_hal_sem_destroy(aosl_sem_t sem)
{
	if (!sem) {
		return;
	}

	sem_t *n_sem = (sem_t *)sem;
	sem_destroy(n_sem);
	aosl_free(n_sem);
}

int aosl_hal_sem_post(aosl_sem_t sem)
{
	if (!sem) {
		return -1;
	}
	return sem_post((sem_t*)sem);
}

int aosl_hal_sem_wait(aosl_sem_t sem)
{
	if (!sem) {
		return -1;
	}
	return sem_wait((sem_t *)sem);
}

int aosl_hal_sem_timedwait(aosl_sem_t sem, intptr_t timeout_ms)
{
	if (!sem) {
		return -1;
	}

	struct timespec timeo;
	struct timespec now;
	clock_gettime (CLOCK_MONOTONIC, &now);
	timeo.tv_sec = now.tv_sec + timeout_ms / 1000;
	timeo.tv_nsec = now.tv_nsec + (timeout_ms % 1000) * 1000000;
	while (timeo.tv_nsec >= 1000000000) {
		timeo.tv_nsec -= 1000000000;
		timeo.tv_sec++;
	}
	return sem_timedwait((sem_t *)sem, &timeo);
}