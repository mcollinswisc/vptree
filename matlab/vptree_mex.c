#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include <mex.h>

#include "../include/vptree/vptree.h"

#include "mex_interface.h"
#include "mex_convert.h"

static double vpmex_distance(void *user_data, const void *p1, const void *p2);
static void *vpmex_allocate(void *user_data, size_t s);
static void vpmex_deallocate(void *user_data, void *data);

typedef struct vpmex_elt vpmex_elt;

struct vpmex_elt {
  mxArray *arr;
  vpmex_elt *next;
  int i;

};

typedef struct {
  vptree *vp;
  vpmex_elt *elts, *last_added;
  mxArray *distance_handle;
} vpmex_tree;

typedef struct {
  mxArray *query;
  vptree_incnn *incnn;
} vpmex_incnn;

static mxArray *vpmex_to_handle(vpmex_tree *vp);
static vpmex_tree *vpmex_from_handle(const mxArray *arr);

static mxArray *vpmex_incnn_to_handle(vpmex_incnn *vp);
static vpmex_incnn *vpmex_incnn_from_handle(const mxArray *arr);

static vpmex_tree *vpmex_create(const mxArray *distance_handle);
static void vpmex_destroy(vpmex_tree *mexvp);

static void vpmex_update_tree(vpmex_tree *mexvp);
static void vpmex_append_elt(vpmex_tree *mexvp, const mxArray *arr);

static mxArray *vpmex_neighbors_to_cellarr(int n, const void **nn);

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  mex_assert(nrhs >= 1);
  
  vpmex_tree *mexvp;
  char *cmd = mxArrayToString(prhs[0]);
  const mxArray *query, *distance_handle;
  const void **nn;
  int k, max_nodes;
  double eps;

  vpmex_incnn *incnn;
  const void *incnbr;

  if(!strcmp(cmd, "create")) {
    mex_assert(nrhs == 2);

    distance_handle = prhs[1];
    mexvp = vpmex_create(distance_handle);
    plhs[0] = vpmex_to_handle(mexvp);
  }
  else if(!strcmp(cmd, "destroy")) {
    mex_assert(nrhs == 2);

    mexvp = vpmex_from_handle(prhs[1]);
    vpmex_destroy(mexvp);
  }
  else if(!strcmp(cmd, "add")) {
    mex_assert(nrhs == 3);
    
    mexvp = vpmex_from_handle(prhs[1]);
    vpmex_append_elt(mexvp, prhs[2]);
  }
  else if(!strcmp(cmd, "nearest_neighbor")) {
    mex_assert(nrhs == 4);

    mexvp = vpmex_from_handle(prhs[1]);
    query = prhs[2];
    k = mxArrayToIntScalar(prhs[3]);

    vpmex_update_tree(mexvp);
    if(k > vptree_npoints(mexvp->vp)) {
      k = vptree_npoints(mexvp->vp);
    }

    nn = (const void **)mxMalloc(sizeof(const void *) * k);
    vptree_nearest_neighbor(mexvp->vp, query, k, nn);
    plhs[0] = vpmex_neighbors_to_cellarr(k, nn);
    mxFree(nn);
  }
  else if(!strcmp(cmd, "nearest_neighbor_approx")) {
    mex_assert(nrhs == 5);

    mexvp = vpmex_from_handle(prhs[1]);
    query = prhs[2];
    k = mxArrayToIntScalar(prhs[3]);
    max_nodes = mxArrayToIntScalar(prhs[4]);

    vpmex_update_tree(mexvp);
    if(k > vptree_npoints(mexvp->vp)) {
      k = vptree_npoints(mexvp->vp);
    }

    nn = (const void **)mxMalloc(sizeof(const void *) * k);
    vptree_nearest_neighbor_approx(mexvp->vp, query, k, nn, max_nodes);
    plhs[0] = vpmex_neighbors_to_cellarr(k, nn);
    mxFree(nn);
  }
  else if(!strcmp(cmd, "neighborhood")) {
    mex_assert(nrhs == 4);

    mexvp = vpmex_from_handle(prhs[1]);
    query = prhs[2];
    eps = mxArrayToDoubleScalar(prhs[3]);

    
    vpmex_update_tree(mexvp);
    nn = vptree_neighborhood(mexvp->vp, query, eps, &k);
    plhs[0] = vpmex_neighbors_to_cellarr(k, nn);

    free(nn); /* TODO: replace with mxFree once realloc issue is resolved */
  }
  else if(!strcmp(cmd, "incnn_begin")) {
    mex_assert(nrhs == 3);
    mexvp = vpmex_from_handle(prhs[1]);
    query = prhs[2];

    incnn = (vpmex_incnn *)mxMalloc(sizeof(vpmex_incnn));
    incnn->query = mxDuplicateArray(query);
    mex_assert(incnn->query != NULL);

    incnn->incnn = vptree_incnn_begin(mexvp->vp, incnn->query);
    mex_assert(incnn->incnn != NULL);

    mexMakeArrayPersistent(incnn->query);
    mexMakeMemoryPersistent(incnn);

    plhs[0] = vpmex_incnn_to_handle(incnn);
  }
  else if(!strcmp(cmd, "incnn_next")) {
    mex_assert(nrhs == 3);
    mexvp = vpmex_from_handle(prhs[1]);
    incnn = vpmex_incnn_from_handle(prhs[2]);

    incnbr = vptree_incnn_next(incnn->incnn);
    plhs[0] = mxDuplicateArray((const mxArray *)incnbr);
  }
  else if(!strcmp(cmd, "incnn_end")) {
    mex_assert(nrhs == 3);
    mexvp = vpmex_from_handle(prhs[1]);
    incnn = vpmex_incnn_from_handle(prhs[2]);

    vptree_incnn_end(incnn->incnn);
    mxDestroyArray(incnn->query);
    mxFree(incnn);
  }

  /* Cleanup */
  mxFree(cmd);
}

static double vpmex_distance(void *user_data, const void *p1, const void *p2)
{
  vpmex_tree *mexvp;
  mxArray *plhs[1], *prhs[3];
  int status;

  mexvp = (vpmex_tree *)user_data;
  prhs[0] = mexvp->distance_handle;
  prhs[1] = mxDuplicateArray((const mxArray *)p1);
  prhs[2] = mxDuplicateArray((const mxArray *)p2);

  status = mexCallMATLAB(1, plhs, 3, prhs, "feval");
  if(status != 0) {
    return -1;
  }

  return mxArrayToDoubleScalar(plhs[0]);
}

static void *vpmex_allocate(void *user_data, size_t s)
{
  void *p;
  p = mxMalloc((mwSize)s);
  mexMakeMemoryPersistent(p);
  return p;
}

static void vpmex_deallocate(void *user_data, void *data)
{
  mxFree(data);
}

static mxArray *vpmex_to_handle(vpmex_tree *vp)
{
  mxArray *arr;
  uint64_t ptr_int;

  arr = mxCreateNumericMatrix(1, 1, mxUINT64_CLASS, mxREAL);
  mex_assert(sizeof(ptr_int) >= sizeof(vp));
  ((uint64_t *)mxGetData(arr))[0] = (uint64_t)vp;

  return arr;
}

static vpmex_tree *vpmex_from_handle(const mxArray *arr)
{
  uint64_t *data;
  data = (uint64_t *)mxGetData(arr);
  return (vpmex_tree *)(data[0]);
}

static mxArray *vpmex_incnn_to_handle(vpmex_incnn *vp)
{
  mxArray *arr;
  uint64_t ptr_int;

  arr = mxCreateNumericMatrix(1, 1, mxUINT64_CLASS, mxREAL);
  mex_assert(sizeof(ptr_int) >= sizeof(vp));
  ((uint64_t *)mxGetData(arr))[0] = (uint64_t)vp;

  return arr;
}

static vpmex_incnn *vpmex_incnn_from_handle(const mxArray *arr)
{
  uint64_t *data;
  data = (uint64_t *)mxGetData(arr);
  return (vpmex_incnn *)(data[0]);
}

static vpmex_tree *vpmex_create(const mxArray *distance_handle)
{
  vptree_options opts;
  vpmex_tree *mexvp;

  /* Construct wrapper object */
  mexvp = (vpmex_tree *)mxMalloc(sizeof(vpmex_tree));
  mex_assert(mexvp != NULL);

  mexvp->distance_handle = mxDuplicateArray(distance_handle);

  mexvp->elts = NULL;
  mexvp->last_added = NULL;

  /* Create VP-Tree */
  opts = vptree_default_options;

  opts.user_data = mexvp;
  opts.distance = vpmex_distance;
  opts.allocate = vpmex_allocate;
  opts.deallocate = vpmex_deallocate;

  mexvp->vp = vptree_create(sizeof(opts), &opts);
  if(mexvp->vp == NULL) {
    mxFree(mexvp);
    mex_err("Failed to create VP-tree");
  }

  mexMakeMemoryPersistent(mexvp);
  mexMakeArrayPersistent(mexvp->distance_handle);

  return mexvp;
}

static void vpmex_destroy(vpmex_tree *mexvp)
{
  vpmex_elt *elt, *next;

  vptree_destroy(mexvp->vp);
  
  elt = mexvp->elts;
  while(elt != NULL) {
    next = elt->next;

    mxDestroyArray(elt->arr);
    mxFree(elt);

    elt = next;
  }
  mxFree(mexvp);
}

static void vpmex_update_tree(vpmex_tree *mexvp)
{
  int i, num_to_add;
  vpmex_elt *elt;
  const void **ptrs;
  
  if(mexvp->elts == NULL) {
    return;
  }
  else if(mexvp->last_added == NULL) {
    num_to_add = mexvp->elts->i + 1;
  }
  else {
    num_to_add = mexvp->elts->i - mexvp->last_added->i;
    if(num_to_add == 0) {
      return;
    }
  }

  ptrs = (const void **)mxMalloc(sizeof(const void *) * num_to_add);
  for(i = 0, elt = mexvp->elts; elt != mexvp->last_added; elt = elt->next, ++i) {
    mex_assert(i < num_to_add);
    ptrs[i] = elt->arr;
  }

  mex_assert(i == num_to_add)

  vptree_add_many(mexvp->vp, num_to_add, ptrs);
  mexvp->last_added = mexvp->elts;
  mxFree(ptrs);
}

static void vpmex_append_elt(vpmex_tree *mexvp, const mxArray *arr)
{
  vpmex_elt *elt;

  elt = (vpmex_elt *)mxMalloc(sizeof(vpmex_elt));
  mex_assert(elt != NULL);
  elt->arr = mxDuplicateArray(arr);
  mex_assert(elt->arr != NULL);

  mexMakeMemoryPersistent(elt);  
  mexMakeArrayPersistent(elt->arr);

  elt->next = mexvp->elts;
  if(elt->next == NULL) {
    elt->i = 0;
  }
  else {
    elt->i = elt->next->i + 1;
  }
  
  mexvp->elts = elt;
}

static mxArray *vpmex_neighbors_to_cellarr(int n, const void **nn)
{
  mwSize cell_dims[2];
  mxArray *cellarr;
  int i;

  cell_dims[0] = (mwSize)n;
  cell_dims[1] = 1;

  cellarr = mxCreateCellArray(2, cell_dims);
  
  for(i = 0; i < n; ++i) {
    mxSetCell(cellarr, (mwIndex)i, mxDuplicateArray((const mxArray *)(nn[i])));
  }

  return cellarr;
}
