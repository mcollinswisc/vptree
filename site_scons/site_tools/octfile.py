from SCons.Script import *


def octfileGenerator(source, target, env, for_signature):
    return 'mkoctfile --mex $_CPPDEFFLAGS $_CPPINCFLAGS --output $TARGET $SOURCES $_LIBDIRFLAGS $_LIBFLAGS'


def generate(env):
    if not env.Detect('mkoctfile'):
        return

    bld = env.Builder(generator=octfileGenerator,
                      suffix='.mex',
                      source_scanner = CScanner)
    env.Append(BUILDERS = {'Mex': bld})
    

def exists(env):
    return env.Detect('mkoctfile')
