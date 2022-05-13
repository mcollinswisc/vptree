#include <stdint.h>

#include "mex_convert.h"
#include "mex_interface.h"

/************************** C -> Matlab ***********************/

#define CONVERSION_CASE(CLASS, SRC_TYPE, DST_TYPE) \
  case CLASS: {                                    \
    SRC_TYPE *in;                                  \
                                                   \
    in = (SRC_TYPE *)data;                         \
                                                   \
    for(i = 0; i < n; i++) {                       \
      out[i] = (DST_TYPE)(in[i]);                  \
    }                                              \
  } break;

#define CONVERSION(FUNC, DST_TYPE)                                      \
  void FUNC(const mxArray *mat, DST_TYPE *out) {                        \
    void *data;                                                         \
    mxClassID class_id;                                                 \
    int i, n;                                                           \
                                                                        \
    data = mxGetData(mat);                                              \
    n = mxGetNumberOfElements(mat);                                     \
    class_id = mxGetClassID(mat);                                       \
                                                                        \
    switch(class_id) {                                                  \
      CONVERSION_CASE(mxCHAR_CLASS, char, DST_TYPE);                    \
      CONVERSION_CASE(mxDOUBLE_CLASS, double, DST_TYPE);                \
      CONVERSION_CASE(mxSINGLE_CLASS, float, DST_TYPE);                 \
      CONVERSION_CASE(mxINT8_CLASS, int8_t, DST_TYPE);                  \
      CONVERSION_CASE(mxUINT8_CLASS, uint8_t, DST_TYPE);                \
      CONVERSION_CASE(mxINT16_CLASS, int16_t, DST_TYPE);                \
      CONVERSION_CASE(mxUINT16_CLASS, uint16_t, DST_TYPE);              \
      CONVERSION_CASE(mxINT32_CLASS, int32_t, DST_TYPE);                \
      CONVERSION_CASE(mxUINT32_CLASS, uint32_t, DST_TYPE);              \
      CONVERSION_CASE(mxINT64_CLASS, int64_t, DST_TYPE);                \
      CONVERSION_CASE(mxUINT64_CLASS, uint64_t, DST_TYPE);              \
    default:                                                            \
      mex_err("Unhandled class id: %d\n", (int)class_id);               \
    }                                                                   \
  }

CONVERSION(mxArrayToIntArray, int);
CONVERSION(mxArrayToFloatArray, float);
CONVERSION(mxArrayToDoubleArray, double);

#undef CONVERSION
#undef CONVERSION_CASE

#define SCALAR(FUNC, DST_TYPE, ARRAY_FUNC)                      \
  DST_TYPE FUNC(const mxArray *mat)                             \
  {                                                             \
    DST_TYPE s;                                                 \
    int n;                                                      \
                                                                \
    n = mxGetNumberOfElements(mat);                             \
                                                                \
    if(n != 1) {                                                \
      mex_err("Expected scalar argument, has %d elements", n);  \
    }                                                           \
                                                                \
    ARRAY_FUNC(mat, &s);                                        \
    return s;                                                   \
  }


SCALAR(mxArrayToIntScalar, int, mxArrayToIntArray);
SCALAR(mxArrayToFloatScalar, float, mxArrayToFloatArray);
SCALAR(mxArrayToDoubleScalar, double, mxArrayToDoubleArray);

#undef SCALAR

/***************************** Matlab -> C ************************/


#define CONVERSION(FUNC, SRC_TYPE, DST_TYPE, CLASS)               \
  mxArray *FUNC(size_t ndims, size_t *dims, const SRC_TYPE *src)  \
  {                                                               \
    mxArray *A;                                                   \
    size_t i, n;                                                  \
    mwSize mwndim, mwdims[ndims];                                 \
    DST_TYPE *dst;                                                \
                                                                  \
    mwndim = (mwSize)ndims;                                       \
    for(i = 0; i < ndims; i++) {                                  \
      mwdims[i] = (mwSize)dims[i];                                \
    }                                                             \
                                                                  \
    A = mxCreateNumericArray(mwndim, mwdims, CLASS, mxREAL);      \
                                                                  \
    n = 1;                                                        \
    for(i = 0; i < ndims; i++) {                                  \
      n *= dims[i];                                               \
    }                                                             \
                                                                  \
    dst = (DST_TYPE *)mxGetData(A);                               \
                                                                  \
    for(i = 0; i < n; i++) {                                      \
      dst[i] = (DST_TYPE)src[i];                                  \
    }                                                             \
                                                                  \
    return A;                                                     \
  }

CONVERSION(mxArrayFromIntArray, int, int32_t, mxINT32_CLASS);
CONVERSION(mxArrayFromFloatArray, float, float, mxSINGLE_CLASS);
CONVERSION(mxArrayFromDoubleArray, double, double, mxDOUBLE_CLASS);

#undef CONVERSION

#define SCALAR(FUNC, SRC_TYPE, DST_TYPE, CLASS)       \
  mxArray *FUNC(SRC_TYPE src)                         \
  {                                                   \
    mxArray *A;                                       \
    mwSize dim;                                       \
    DST_TYPE *dst;                                    \
                                                      \
    dim = 1;                                          \
    A = mxCreateNumericArray(1, &dim, CLASS, mxREAL); \
                                                      \
    dst = (DST_TYPE *)mxGetData(A);                   \
    *dst = (DST_TYPE)src;                             \
                                                      \
    return A;                                         \
  }

SCALAR(mxArrayFromIntScalar, int, int32_t, mxINT32_CLASS);
SCALAR(mxArrayFromFloatScalar, float, float, mxSINGLE_CLASS);
SCALAR(mxArrayFromDoubleScalar, double, double, mxDOUBLE_CLASS);

#undef SCALAR

/************************ Struct Handling *************************/

void mxStructUnpack(int noutfields, const char * const *outfieldnames, const mxArray *mxstruct, const mxArray **outfields)
{
  int i, j, nfields;
  const char *fieldname;

  /* Initialize output to NULL */
  for(i = 0; i < noutfields; i++) {
    outfields[i] = NULL;
  }

  if(!mxIsStruct(mxstruct)) {
    return;
  }

  /* Search for names */
  nfields = mxGetNumberOfFields(mxstruct);
  
  for(i = 0; i < nfields; i++) {
    fieldname = mxGetFieldNameByNumber(mxstruct, i);

    for(j = 0; j < noutfields; j++) {
      if(strcmp(fieldname, outfieldnames[j]) == 0) {
        outfields[j] = mxGetFieldByNumber(mxstruct, 0, i);
      }
    }
  }
}
