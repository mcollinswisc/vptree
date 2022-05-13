from __future__ import print_function
import os, os.path

# Set up environment
env = Environment(ENV = os.environ)

env.Tool('warnings')

env.Append(LIBS = ['m'])
env.AppendUnique(CFLAGS = ['-g'], CXXFLAGS = ['-g'], LINKFLAGS = ['-g'])
env.AppendUnique(CFLAGS = ['-O2'], CXXFLAGS = ['-O2'], LINKFLAGS = ['-O2'])
#env.AppendUnique(CFLAGS = ['-pg'], CXXFLAGS = ['-pg'], LINKFLAGS = ['-pg'])
env.AppendUnique(CFLAGS = ['-fPIC'], CXXFLAGS = ['-fPIC'])

# Compile library
core_src = ['pqueue.c', 'vptree.c', 'geom.c', 'vptree_cpp.cc']
core_src = [os.path.join('src', f) for f in core_src]
static_lib = env.StaticLibrary('lib/vptree', core_src)
shared_lib = env.SharedLibrary('lib/vptree', core_src)

# Install files
env.Install('include/vptree', 'src/vptree.h')
env.Install('include/vptree', 'src/vptree.hh')

# Python interface
pyvptree = SConscript('python/SConscript', exports = 'static_lib')
if pyvptree is not None:
    env.Install('examples', pyvptree)

# Matlab/Python interface
mexvptree = SConscript('matlab/SConscript', exports = 'static_lib')
if mexvptree is not None:
    env.Install('examples', mexvptree)
    env.Install('examples', ['matlab/VPTree.m', 'matlab/VPTreeIncNN.m'])

# Test code
common_test_code = ['src/timing.c']
common_test_code += static_lib
env.Program('bin/test-vptree', ['src/test_vptree.c'] + common_test_code)

# Examples
env.Append(CPPPATH = [env.Dir('include')])
env.Program('bin/cities', ['examples/cities.c'] + common_test_code)
env.Program('bin/cities_cpp', ['examples/cities_cpp.cc'] + common_test_code)
