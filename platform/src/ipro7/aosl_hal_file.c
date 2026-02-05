#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <hal/aosl_hal_file.h>

ssize_t aosl_hal_write(int fd, const void *buf, size_t count)
{
  return write(fd, buf, count);
}

ssize_t aosl_hal_read(int fd, void *buf, size_t count)
{
  return read(fd, buf, count);
}

int aosl_hal_close(int fd)
{
  return close(fd);
}
