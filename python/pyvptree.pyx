import cython
cimport vptree
from cpython.mem cimport PyMem_Malloc, PyMem_Free
import sys

cdef void* pyalloc(void *user_data, size_t sz):
  return PyMem_Malloc(sz)

cdef void pyfree(void *user_data, void *ptr):
  PyMem_Free(ptr)

cdef double pydistance(void *user_data, const void *p1, const void *p2):
  self = <object>user_data
  py1 = <object>p1
  py2 = <object>p2

  try:
    return self.distance(py1, py2)
  except:
    self.excepts = sys.exc_info()[0]
    return -1

cdef class VPTree:
  cdef vptree.vptree* _c_vp

  def __cinit__(self):
    opts = vptree.vptree_default_options
    opts.user_data = <void *>self
    opts.distance = pydistance
    opts.allocate = pyalloc
    opts.deallocate = pyfree
    self._c_vp = vptree.vptree_create(cython.sizeof(opts), &opts)
    if self._c_vp is NULL:
      raise MemoryError()

    self.points = []
    self.excepts = None

  def __dealloc__(self):
    if self._c_vp is not NULL:
        vptree.vptree_destroy(self._c_vp)

  def __len__(self):
    return vptree.vptree_npoints(self._c_vp)

  def distance(self, point1, point2):
    raise NotImplementedError()

  def add(self, p):
    self.points.append(p)
    s = vptree.vptree_add(self._c_vp, <void *>(self.points[-1]))
    if self.excepts is not None:
      raise self.excepts

  def add_many(self, point_seq):
    prev_points = len(self.points)
    self.points.extend(point_seq)

    cdef int npts = len(self.points) - prev_points
    cdef const void **ptrs = <const void **>PyMem_Malloc(npts * sizeof(void *))
    for i, pt in enumerate(self.points[prev_points:]):
      ptrs[i] = <void *>(self.points[prev_points + i])

    vptree.vptree_add_many(self._c_vp, npts, ptrs)

    PyMem_Free(ptrs)

    if self.excepts is not None:
      raise self.excepts

  def nearest_neighbors(self, query, k = 1, max_nodes = None):
    cdef const void **ptrs = <const void **>PyMem_Malloc(k * sizeof(void *))

    if max_nodes is None:
      vptree.vptree_nearest_neighbor(self._c_vp, <const void *>query, k, ptrs)
    else:
      vptree.vptree_nearest_neighbor_approx(self._c_vp, <const void *>query, k, ptrs, max_nodes)

    if self.excepts is not None:
      PyMem_Free(ptrs)
      raise self.excepts

    nn = []
    for i in range(k):
      nn.append(<object>ptrs[i])

    PyMem_Free(ptrs)

    return nn

  def neighborhood(self, query, distance):
    cdef const void **ptrs
    cdef int npoints

    ptrs = vptree.vptree_neighborhood(self._c_vp, <const void *>query, distance, &npoints)

    if self.excepts is not None:
      PyMem_Free(ptrs)
      raise self.excepts

    nn = []
    for i in range(npoints):
      nn.append(<object>ptrs[i])

    PyMem_Free(ptrs)

    return nn

  def incremental_knn(self, query):
    cdef vptree.vptree_incnn *incnn
    cdef const void *next

    incnn = vptree.vptree_incnn_begin(self._c_vp, <const void *>query)

    next = vptree.vptree_incnn_next(incnn)
    while next is not NULL:
      yield <object>next
      next = vptree.vptree_incnn_next(incnn)
      
    vptree.vptree_incnn_end(incnn)