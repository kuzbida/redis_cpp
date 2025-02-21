// functions.h
#ifndef FILES_H  // Include guard to prevent multiple inclusion
#define FILES_H

#include <sys/_types/_size_t.h>
#include <sys/_types/_ssize_t.h>

static int32_t read_full(int fd, char *buf, size_t n); 
static int32_t write_all(int fd, const char *buf, size_t n);

#endif
