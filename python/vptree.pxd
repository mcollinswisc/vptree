cdef extern from "../include/vptree/vptree.h":
  ctypedef double (*distance_funcptr)(void *, const void *, const void *)
  ctypedef void *(*alloc_funcptr)(void *, size_t)
  ctypedef void (*dealloc_funcptr)(void *, void *)

  ctypedef struct vptree_options:
    void *user_data
    distance_funcptr distance
    alloc_funcptr allocate
    dealloc_funcptr deallocate

  vptree_options vptree_default_options

  ctypedef struct vptree:
    pass
  ctypedef void* VPTreePoint

  vptree* vptree_create(size_t opts_size, const vptree_options *opts)
  void vptree_destroy(vptree *vp)

  int vptree_npoints(const vptree *vp)
  int vptree_add(vptree *vp, const void *p)
  int vptree_add_many(vptree *vp, int n, const void * const *p)

  void vptree_nearest_neighbor(const vptree *vp, const void *p, int k, const void *nn)
  void vptree_nearest_neighbor_approx(const vptree *vp, const void *p, int k, const void *nn, int max_nodes)

  const void **vptree_neighborhood(const vptree *vp, const void *p, double distance, int *n)

  ctypedef struct vptree_incnn:
    pass

  vptree_incnn *vptree_incnn_begin(const vptree *vp, const void *p)
  const void *vptree_incnn_next(vptree_incnn *inc)
  void vptree_incnn_end(vptree_incnn *)