import os, os.path, subprocess
import re

from SCons.Script import *

mexre = re.compile('.*mex\s*=\s*(.*)')

def mexext(env):
    """Get extension for mex files."""

    matlab_bin = env.subst('$MATLAB')

    # Run Matlab
    cmd = [matlab_bin, '-nodisplay', '-nojvm']
    devnull = file(os.devnull
)
    proc = subprocess.Popen(cmd, stdout = subprocess.PIPE, stdin = subprocess.PIPE, stderr = devnull)

    # Issue mexext command
    proc.stdin.write("fprintf(1, 'mex = %s\\n', mexext)\nexit\n")

    # Parse output
    for line in proc.stdout:
        m = mexre.match(line)
        if m:
            return m.groups()[0]


def mexGenerator(source, target, env, for_signature):
    return '$MEX $MEXFLAGS $_CPPDEFFLAGS $_CPPINCFLAGS -output $TARGET $SOURCES $_LIBDIRFLAGS $_LIBFLAGS'

def generate(env):
    env.SetDefault(MEX = 'mex')
    env.SetDefault(MATLAB = 'matlab')
    env.SetDefault(MATLABDIR = os.path.dirname(env.WhereIs(env['MATLAB'])))

    if not env.Detect(env['MATLAB']):
        del env['MEX']
        del env['MATLAB']
        return

    env.SetDefault(MEXSUFFIX = '.' + mexext(env))
    env.SetDefault(MEXINCPATH = '$MATLABDIR/extern/include')

    if env['MEXSUFFIX'] in ['.mexa64', '.mexmaci64', '.mexw64']:
        env.PrependUnique(MEXFLAGS = ['-largeArrayDims'])

    env.AppendUnique(CPPPATH = ['$MEXINCPATH'])

    #print env.Dump()
    #Exit(0)


    # Create builder
    bld = env.Builder(generator = mexGenerator,
                      suffix = env['MEXSUFFIX'],
                      source_scanner = CScanner)
    env.Append(BUILDERS = {'Mex': bld})

def exists(env):
    return True
