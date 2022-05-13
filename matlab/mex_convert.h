#ifndef __BITS_MATLAB_CONVERT_H__
#define __BITS_MATLAB_CONVERT_H__

#include <mex.h>

/*************************** C -> Matlab *************************/

void mxArrayToIntArray(const mxArray *mat, int *out);
void mxArrayToFloatArray(const mxArray *mat, float *out);
void mxArrayToDoubleArray(const mxArray *mat, double *out);

int mxArrayToIntScalar(const mxArray *mat);
float mxArrayToFloatScalar(const mxArray *mat);
double mxArrayToDoubleScalar(const mxArray *mat);


/**************************** Matlab -> C *************************/

mxArray *mxArrayFromIntArray(size_t ndims, size_t *dims, const int *data);
mxArray *mxArrayFromFloatArray(size_t ndims, size_t *dims, const float *data);
mxArray *mxArrayFromDoubleArray(size_t ndims, size_t *dims, const double *data);


mxArray *mxArrayFromIntScalar(int);
mxArray *mxArrayFromFloatScalar(float);
mxArray *mxArrayFromDoubleScalar(double);

/**************************** Struct Handling **********************/

void mxStructUnpack(int nfields, const char * const *field_names, const mxArray *mxstruct, const mxArray **outfields);

#endif /* #ifndef __BITS_MATLAB_CONVERT_H__ */
