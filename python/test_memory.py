from __future__ import print_function
from pyvptree import VPTree

import gc

class Point:
  def __init__(self, a):
    print('built', a)
    self.a = a

  def __del__(self):
    print('deallocated', self.a)

  def __str__(self):
    return str(self.a)

  def __repr__(self):
    return str(self)

class MyVPTree(VPTree):
  def distance(self, p, q):
    return abs(p.a - q.a)

def test():
    p1 = Point(1)
    p2 = Point(2)

    vp = MyVPTree()
    vp.add(p1)
    vp.add(p2)

    print('len =', len(vp))

    vp.add_many([Point(i) for i in range(4,10)])

    print('len =', len(vp))
    return vp

vp = test()
print('done with test')
#print(vp.points)
print(vp)

print('knn 7.4 =', vp.nearest_neighbors(Point(7.4), 3))

del vp

print('about to finish main')
