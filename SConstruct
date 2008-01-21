"""
Build system can be clean-up by sticking to a few core production factory, with automatic dependencies resolution.
4 basic project productions:
- library
- binary
- documentation
- tests

* Library:
  Input:
    - dependencies (other libraries)
    - headers: include path & files
    - sources
    - generated sources
    - resources
    - generated resources
  Production:
    - Static library
    - Dynamic library
    - Naming rule
  Life-cycle:
    - Library compilation
    - Compilation as a dependencies
    - Run-time
    - Packaging
  Identity:
    - Name
    - Version
* Binary:
  Input:
    - dependencies (other libraries)
    - headers: include path & files (usually empty)
    - sources
    - generated sources
    - resources
    - generated resources
    - supported variant (optimized/debug, dll/static...)
  Production:
    - Binary executable
    - Manifest [on some platforms]
    - Debug symbol [on some platforms]
  Life-cycle:
    - Compilation
    - Run-time
    - Packaging
  Identity:
    - Name
    - Version
* Documentation:
  Input:
    - dependencies (libraries, binaries)
    - additional sources
    - generated sources
    - resources
    - generated resources
    - supported variant (public/internal)
  Production:
    - HTML documentation
    - PDF documentation
    - CHM documentation
  Life-cycle:
    - Documentation
    - Packaging
    - Test
  Identity:
    - Name
    - Version
"""



import os
import os.path
import sys

JSONCPP_VERSION = '0.1'
DIST_DIR = '#dist'

options = Options()
options.Add( EnumOption('platform',
                        'Platform (compiler/stl) used to build the project',
                        'msvc71',
                        allowed_values='suncc vacpp mingw msvc6 msvc7 msvc71 msvc80 linux-gcc'.split(),
                        ignorecase=2) )

try:
    platform = ARGUMENTS['platform']
    if platform == 'linux-gcc':
        CXX = 'g++' # not quite right, but env is not yet available.
        import commands
        version = commands.getoutput('%s -dumpversion' %CXX)
        platform = 'linux-gcc-%s' %version
        print "Using platform '%s'" %platform
        LD_LIBRARY_PATH = os.environ.get('LD_LIBRARY_PATH', '')
        LD_LIBRARY_PATH = "%s:libs/%s" %(LD_LIBRARY_PATH, platform)
        os.environ['LD_LIBRARY_PATH'] = LD_LIBRARY_PATH
        print "LD_LIBRARY_PATH =", LD_LIBRARY_PATH
except KeyError:
    print 'You must specify a "platform"'
    sys.exit(2)

print "Building using PLATFORM =", platform

rootbuild_dir = Dir('#buildscons')
build_dir = os.path.join( '#buildscons', platform )
bin_dir = os.path.join( '#bin', platform )
lib_dir = os.path.join( '#libs', platform )
sconsign_dir_path = Dir(build_dir).abspath
sconsign_path = os.path.join( sconsign_dir_path, '.sconsign.dbm' )

# Ensure build directory exist (SConsignFile fail otherwise!)
if not os.path.exists( sconsign_dir_path ):
    os.makedirs( sconsign_dir_path )

# Store all dependencies signature in a database
SConsignFile( sconsign_path )

env = Environment( ENV = {'PATH' : os.environ['PATH']},
                   toolpath = ['scons-tools'],
                   tools=[] ) #, tools=['default'] )

if platform == 'suncc':
    env.Tool( 'sunc++' )
    env.Tool( 'sunlink' )
    env.Tool( 'sunar' )
    env.Append( LIBS = ['pthreads'] )
elif platform == 'vacpp':
    env.Tool( 'default' )
    env.Tool( 'aixcc' )
    env['CXX'] = 'xlC_r'   #scons does not pick-up the correct one !
    # using xlC_r ensure multi-threading is enabled:
    # http://publib.boulder.ibm.com/infocenter/pseries/index.jsp?topic=/com.ibm.vacpp7a.doc/compiler/ref/cuselect.htm
    env.Append( CCFLAGS = '-qrtti=all',
                LINKFLAGS='-bh:5' )  # -bh:5 remove duplicate symbol warning
elif platform == 'msvc6':
    env['MSVS_VERSION']='6.0'
    for tool in ['msvc', 'msvs', 'mslink', 'masm', 'mslib']:
        env.Tool( tool )
    env['CXXFLAGS']='-GR -GX /nologo /MT'
elif platform == 'msvc70':
    env['MSVS_VERSION']='7.0'
    for tool in ['msvc', 'msvs', 'mslink', 'masm', 'mslib']:
        env.Tool( tool )
    env['CXXFLAGS']='-GR -GX /nologo /MT'
elif platform == 'msvc71':
    env['MSVS_VERSION']='7.1'
    for tool in ['msvc', 'msvs', 'mslink', 'masm', 'mslib']:
        env.Tool( tool )
    env['CXXFLAGS']='-GR -GX /nologo /MT'
elif platform == 'msvc80':
    env['MSVS_VERSION']='8.0'
    for tool in ['msvc', 'msvs', 'mslink', 'masm', 'mslib']:
        env.Tool( tool )
    env['CXXFLAGS']='-GR -EHsc /nologo /MT'
elif platform == 'mingw':
    env.Tool( 'mingw' )
    env.Append( CPPDEFINES=[ "WIN32", "NDEBUG", "_MT" ] )
elif platform.startswith('linux-gcc'):
    env.Tool( 'default' )
    env.Append( LIBS = ['pthread'], CCFLAGS = "-Wall" )
else:
    print "UNSUPPORTED PLATFORM."
    env.Exit(1)

env.Tool('doxygen')
env.Tool('substinfile')
env.Tool('targz')
env.Tool('srcdist')
env.Tool('glob')

env.Append( CPPPATH = ['#include'],
            LIBPATH = lib_dir )
short_platform = platform
if short_platform.startswith('msvc'):
    short_platform = short_platform[2:]
env['LIB_PLATFORM'] = short_platform
env['LIB_LINK_TYPE'] = 'lib'    # static
env['LIB_CRUNTIME'] = 'mt'
env['LIB_NAME_SUFFIX'] = '${LIB_PLATFORM}_${LIB_LINK_TYPE}${LIB_CRUNTIME}'  # must match autolink naming convention
env['JSONCPP_VERSION'] = JSONCPP_VERSION
env['BUILD_DIR'] = env.Dir(build_dir)
env['ROOTBUILD_DIR'] = env.Dir(rootbuild_dir)
env['DIST_DIR'] = DIST_DIR
class SrcDistAdder:
    def __init__( self, env ):
        self.env = env
    def __call__( self, *args, **kw ):
        apply( self.env.SrcDist, (self.env['SRCDIST_TARGET'],) + args, kw )
env['SRCDIST_ADD'] = SrcDistAdder( env )
env['SRCDIST_TARGET'] = os.path.join( DIST_DIR, 'jsoncpp-src-%s.tar.gz' % env['JSONCPP_VERSION'] )
env['SRCDIST_BUILDER'] = env.TarGz
                      
env_testing = env.Copy( )
env_testing.Append( LIBS = ['json_${LIB_NAME_SUFFIX}'] )

def buildJSONExample( env, target_sources, target_name ):
    env = env.Copy()
    env.Append( CPPPATH = ['#'] )
    exe = env.Program( target=target_name,
                       source=target_sources )
    env['SRCDIST_ADD']( source=[target_sources] )
    global bin_dir
    return env.Install( bin_dir, exe )

def buildJSONTests( env, target_sources, target_name ):
    jsontests_node = buildJSONExample( env, target_sources, target_name )
    check_alias_target = env.Alias( 'check', jsontests_node, RunJSONTests( jsontests_node, jsontests_node ) )
    env.AlwaysBuild( check_alias_target )

def buildLibrary( env, target_sources, target_name ):
    static_lib = env.StaticLibrary( target=target_name + '_${LIB_NAME_SUFFIX}',
                                    source=target_sources )
    shared_lib = env.SharedLibrary( target=target_name + '_${LIB_NAME_SUFFIX}',
                                    source=target_sources )
    global lib_dir
    env.Install( lib_dir, static_lib )
    env.Install( lib_dir, shared_lib )
    env['SRCDIST_ADD']( source=[target_sources] )

Export( 'env env_testing buildJSONExample buildLibrary buildJSONTests' )

def buildProjectInDirectory( target_directory ):
    global build_dir
    target_build_dir = os.path.join( build_dir, target_directory )
    target = os.path.join( target_directory, 'sconscript' )
    SConscript( target, build_dir=target_build_dir, duplicate=0 )
    env['SRCDIST_ADD']( source=[target] )


def runJSONTests_action( target, source = None, env = None ):
    # Add test scripts to python path
    jsontest_path = Dir( '#test' ).abspath
    sys.path.insert( 0, jsontest_path )
    import runjsontests
    return runjsontests.runAllTests( os.path.abspath(source), jsontest_path )

def runJSONTests_string( target, source = None, env = None ):
    return 'RunJSONTests("%s")' % source

import SCons.Action
ActionFactory = SCons.Action.ActionFactory
RunJSONTests = ActionFactory(runJSONTests_action, runJSONTests_string )

env.Alias( 'check' )

srcdist_cmd = env['SRCDIST_ADD']( source = """
    AUTHORS README.txt SConstruct
    """.split() )
env.Alias( 'src-dist', srcdist_cmd )

buildProjectInDirectory( 'src/jsontestrunner' )
buildProjectInDirectory( 'src/lib_json' )
buildProjectInDirectory( 'doc' )
#print env.Dump()

