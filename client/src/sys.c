#include <errno.h>
#undef errno
extern int errno;
int _close(int fd);
int _close(int fd){
  (void) fd;
  errno = ENOSYS;
  return -1;
}

int _lseek(int fd, int ptr, int dir);
int _lseek(int fd, int ptr, int dir){
  (void) fd;
  (void) ptr;
  (void) dir;
  errno = ENOSYS;
	return -1;
}

int _read(int fd, char* ptr, int len);
int _read(int fd, char* ptr, int len){
  (void) fd;
  (void) ptr;
  (void) len;
  errno = ENOSYS;
	return -1;
}

int _write(int fd, const char* ptr, int len);
int _write(int fd, const char* ptr, int len){
  (void) fd;
  (void) ptr;
  (void) len;
  errno = ENOSYS;
	return -1;
}