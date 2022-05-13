#include <stdarg.h>

#include "mex_interface.h"

void mex_warn(const char *format, ...)
{
  va_list args;
  char err[1024];

  va_start(args, format);
  vsnprintf(err, 1024, format, args);
  va_end(args);

  mexWarnMsgTxt(err);
}

void mex_err(const char *format, ...)
{
  va_list args;
  char err[1024];

  va_start(args, format);
  vsnprintf(err, 1024, format, args);
  va_end(args);

  mexErrMsgTxt(err);
}
