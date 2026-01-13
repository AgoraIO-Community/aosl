#include <lwip/errno.h>

#include <hal/aosl_hal_errno.h>

int aosl_hal_errno_convert(int errnum)
{
  if (0 == errnum) {
    return AOSL_HAL_RET_SUCCESS;
  }

  if (-1 == errnum) {
    return AOSL_HAL_RET_FAILURE;
  }

  if (EAGAIN == errnum || EWOULDBLOCK == errnum) {
    return AOSL_HAL_RET_EAGAIN;
  }

  if (EINTR == errnum) {
    return AOSL_HAL_RET_EINTR;
  }

  return AOSL_HAL_RET_EHAL;
}