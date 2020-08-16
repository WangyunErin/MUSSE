import os

env=Environment(ENV=os.environ, tools=['default', 'protoc', 'grpc'])

print(env['ENV']['HOME'])
print(env['ENV']['PATH'])

if 'CC' in os.environ:
    env['CC']=os.environ['CC']
    
if 'CXX' in os.environ:
    env['CXX']=os.environ['CXX']

root_dir = Dir('#').srcnode().abspath

config = {}


if 'config_file' in ARGUMENTS:
    SConscript(ARGUMENTS['config_file'], exports=['env','config'])



env.Append(CCFLAGS = ['-fPIC','-Wall', '-march=native'])
env.Append(CXXFLAGS = ['-std=c++14','-g','-O3'])
env.Append(CPPPATH = ['/usr/local/include','/usr/local/include/cryptopp'])
env.Append(LIBPATH = ['/usr/local/lib'])

env.Append(LIBS = ['crypto',  'grpc++_unsecure', 'grpc', 'protobuf', 'pthread', 'dl', 'snappy', 'z', 'bz2',  'lz4','cryptopp'])

env['STATIC_AND_SHARED_OBJECTS_ARE_THE_SAME']=1


static_relic = ARGUMENTS.get('static_relic', 0)

env.Append(CPPDEFINES = ['BENCHMARK'])

def run_test(target, source, env):
    app = str(source[0].abspath)
    if os.spawnl(os.P_WAIT, app, app)==0:
        return 0
    else:
        return 1

bld = Builder(action = run_test)
env.Append(BUILDERS = {'Test' :  bld})


objects = SConscript('src/build.scons', exports='env', variant_dir='build')

Clean(objects["musse"], 'build')

outter_env = env.Clone()
outter_env.Append(CPPPATH = ['build'])

#muses_interactive  = outter_env.Program('muses',    ['muses.cpp']     + objects["muses"])
NFNU_test         = outter_env.Program('NFNU_single',    ['test_NFNU.cpp']     + objects["musse"])
FU_test         = outter_env.Program('FU_single',    ['test_FU.cpp']     + objects["musse"])
FNU_test         = outter_env.Program('FNU_single',    ['test_FNU.cpp']     + objects["musse"])
NFNU_client         = outter_env.Program('NFNU_client',    ['test_NFNU_client.cpp']     + objects["musse"])
FU_client         = outter_env.Program('FU_client',    ['test_FU_client.cpp']     + objects["musse"])
FNU_client         = outter_env.Program('FNU_client',    ['test_FNU_client.cpp']     + objects["musse"])
MUSSE_server       = outter_env.Program('MUSSE_server',   ['test_MUSSE_server.cpp']   + objects["musse"])
mkse         = outter_env.Program('mkse',    ['test_mkse.cpp'] + objects["mkse"] )
mkse_server       = outter_env.Program('mkse_server',   ['test_mkse_server.cpp']   + objects["mkse"])
mkse_client        = outter_env.Program('mkse_client',    ['test_mkse_client.cpp'] + objects["mkse"] )

env.Alias('musse', [NFNU_test, FU_test, FNU_test, NFNU_client, FU_client, FNU_client, MUSSE_server, mkse,mkse_server,mkse_client])

env.Default(['musse'])
