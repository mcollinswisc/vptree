# libvptree

Implements vantage-point (VP) trees for nearest-neighbor lookup. The vp-tree
provides a general way to perform nearest neighbor search in arbitrary metric
spaces. The data structure was developed by Yianilos and independently by
Uhlmann as "Metric Trees." vp-trees were shown by Kumar et. al. to provide
superior performance on common Computer Vision nearest neighbor computations.
In addition to the flexibility of performing queries in any arbitrary metric
space (for example the metric derived from using the Pyramid Match Kernel as
an inner product), even in Euclidean spaces the vp-tree may be a good way to
handle very high-dimensional nearest neighbor problems. 

The code provided in this package allows for the following types of queries:

- k-nearest neighbor
- ε-neighbor
- Incremental nearest neighbor: Returns an arbitrary number of neighbors, one
  at a time, in order from closest to farthest
- Approximate k-NN: Visits a fixed number of nodes in order defined by a
  priority queue (like done in [ANN](https://www.cs.umd.edu/~mount/ANN/) by
  Mount & Arya)
     

Language bindings are provided for C++, Python, and Matlab. Example code in
each language shows how to build and query a VP-tree using the provided
interface.

## Dependencies

- scons (<http://www.scons.org/>)
- cython (<http://cython.org/>), needed for Python bindings
- Octave (<http://www.gnu.org/software/octave/>) or Matlab needed for Matlab
  bindings

## Building

libvptree can be build by running the `scons` command in the root directory of
this distribution.

## Running

A test program will be built in the `bin/` directory.  It takes no arguments.

Example code to show how to use the package within each language is given in the
`examples/` directory.  Examples in C and C++ are compiled into the `bin/`
directory.  Assuming all language bindings can be built, each example can be
run with the commands:

```sh
./bin/cities
./bin/cities_cpp
python examples/cities.py
octave -q --path examples --eval cities_octave
cd examples; matlab -r cities_matlab
```

TODO: Matlab script?

## Linking

Static and shared libraries will be built in the `lib/` directory.

Necessary include files will be placed in `include/vptree/`

## References

- <https://pages.cs.wisc.edu/~mcollins/software/vptree.html>
- Yianilos, P.N. [Data structures and algorithms for nearest neighbor search in general metric spaces.](https://dl.acm.org/doi/10.5555/313559.313789) Symposium on Discrete Algorithms (SoDA), January 1993
- J. K. Uhlmann. [Satisfying general proximity/similarity queries with metric trees.](https://www.sciencedirect.com/science/article/abs/pii/002001909190074R) Information Processing Letters, November 1991.
- N. Kumar, L. Zhang, and S. K. Nayar. [What is a Good Nearest Neighbors Algorithm for Finding Similar Patches in Images?](https://cave.cs.columbia.edu/projects/categories/project?cid=Visual%20Recognition&pid=Nearest%20Neighbor%20Algorithms%20for%20Finding%20Similar%20Image%20Patches) European Conference on Computer Vision (ECCV), October 2008.
