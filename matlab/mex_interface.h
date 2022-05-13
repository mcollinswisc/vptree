#ifndef __BITS_MATLAB_INTERFACE_H__
#define __BITS_MATLAB_INTERFACE_H__

#include <mex.h>

/**
 * Output warning using printf formatting.
 */
void mex_warn(const char *format, ...);

/**
 * Output error and exit using printf formatting.
 */
void mex_err(const char *format, ...);

/**
 * Output error if condition is not met.
 */
#ifdef NDEBUG

#define mex_assert(cond)

#else

#define __mex_assert3(cond, file, line)                              \
  if(!(cond)) {                                                      \
    mex_err("Assertion failed: \"" #cond "\" at " file ":%d", line); \
  }

#define __mex_assert2(cond, file, line) __mex_assert3(cond, file, line)

#define mex_assert(cond) __mex_assert2(cond, __FILE__, __LINE__)

#endif

#endif /* #ifndef __BITS_MATLAB_INTERFACE_H__ */
