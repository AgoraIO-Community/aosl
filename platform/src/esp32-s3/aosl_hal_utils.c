#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <hal/aosl_hal_time.h>

int aosl_hal_get_uuid (char buf [], int buf_sz)
{
	uint64_t ts_now = (uint64_t) aosl_hal_get_tick_ms ();
	unsigned int s, r1, r2;

	s = (unsigned int)ts_now;
	while (s > RAND_MAX) {
		/* s *= 7 */
		s = (s << 2) + (s << 1) + s;
	}

	srand (s);
	r1 = (unsigned int)rand ();
	r2 = (unsigned int)rand ();
	snprintf (buf, buf_sz, "%016llx%08x%08x", (unsigned long long)ts_now, r1, r2);
	return 0;
}

int aosl_hal_os_version (char buf [], int buf_sz)
{
	snprintf(buf, buf_sz, "%s", "esp32-s3");
	return 0;
}