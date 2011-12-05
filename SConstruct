import os.path

# TODO: remove this TARGET_ARCH='x86' hardcoded setting. It's a workaround for
# VS2010 Express.
env = Environment(TARGET_ARCH='x86')
Export('env')

# Do we care about debug/release?

# TODO: Modify build directory name to include architecture and variant
# (debug/release).
dll = env.SConscript('source/SConscript', variant_dir = 'build', duplicate=0)

install_actions = [env.Install('install', dll[0]),
                   env.Install('install', env.File('manifest.json')),
                   env.Install('install', env.Dir('css')),
                   env.Install('install', env.Dir('js')),
                   env.Install('install', env.Dir('img')),
                   env.Install('install', env.Glob('*.html'))]
env.Default(install_actions)

# TODO: Add a target to use chrome to create packaged extension.

#if str(Platform()) == 'win32':
#  # Windows build of libcurl is stored in 'external' subdirectory.
#  env.Append(LIBPATH = [os.path.join('external', 'lib')])
#  env.Append(CPPPATH = [os.path.join('external', 'include')])
#
#  # /EHsc is necessary for exceptions to work. /MD is necessary since libcurl
#  # was built this way too and if this option isn't specified, the default
#  # seems to be /MT which is incompatible.
#  env.Append(CPPFLAGS = ['/EHsc', '/MD'])
#
#  # On windows, the windows socket library is also necessary to link against.
#  env.Append(LIBS = ['libcurl', 'ws2_32'])
#
#  # When linking against libcurl statically, this #define is necessary.
#  env.Append(CPPDEFINES = ['CURL_STATICLIB'])
#
#elif str(Platform()) == 'posix':
#  # As far as I know, libcurl is present by default at least on 
#  # Ubuntu systems.
#  env.Append(LIBS = ['curl'])
#
#  # These options were provided by curl-config, a tool that exists on linux.
#  env.Append(LIBPATH = ['/usr/lib/i386-linux-gnu'])
#  env.Append(LINKFLAGS=['-Bsymbolic-functions'])
#
#  # NOTE: Unlike windows, we link against the shared-library version of libcurl
#  # since this seems to be present by default on linux systems (or at least
#  # Ubuntu).
#else:
#  # Unknown platform
#  assert False
#
#env.SharedLibrary('setwallpaper_plugin', sources)
#env.Program('simple', 'simple.cpp')
