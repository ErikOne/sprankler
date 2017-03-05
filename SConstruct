import os
import copy

from custombuilder import ProgramBuilder
from custombuilder import LibBuilder

OPTION_BUILDTYPE_CHOICES = (
	'production',
	'unittests'
)

OPTION_TARGET_CHOICES = (
	'native',
	'rpi2',
)

OPTION_BUILDTYPE_CHOICES = (
	'production',
	'unittests'
)

AddOption('--target', dest='target', nargs=1, choices=OPTION_TARGET_CHOICES, action="store", metavar='TARGET',default='rpi2',
    help='Sets the build target type: valid choices are [native, rpi2 ].  The default is atsama5.')


AddOption('--buildtype', dest='build_type', nargs=1, choices=OPTION_BUILDTYPE_CHOICES, action="store",default='production',
    help='Sets the build type: valid choices are [production, unittests].  The default is production.')
    
AddOption('--debugbuild', dest='debug_build',action="store_true",default=False,
    help='Is the debug enabled on this build')

AddOption('--yocto', dest='yocto_build',action="store_true",default=False,
    help='Is this build run from within a Yocto SDK')


basic_variables = {
    'LIBS'          : [],
    'SHARED_LIBS'   : ['-lrt', '-lpthread' ],
    'CCFLAGS'       : [ '-g3', '-Wall','-Werror', '-Wstrict-prototypes'],
    'CFLAGS'        : [],
    'CXXFLAGS'      : [],
    'CPPPATH'       : [],
    'CPPDEFINES'    : {},
    'LINKFLAGS'     : [],
    'PACKAGE_LIST'  : [],
    'targetEnv'     : False,
    'isHostToolsEnv': False
}

if GetOption("debug_build") == True:
  basic_variables['CCFLAGS'].append("-g3")


target_variables = copy.deepcopy(basic_variables)
target_variables['BUILDROOT'] = "#/do/"+GetOption('target')

if GetOption('yocto_build') == True:
	CC, CC_OPTIONS = os.environ['CC'].split(" ", 1)
	target_variables['CC'] = CC
	target_variables['CFLAGS'].extend(CC_OPTIONS.split())
	target_variables['LINKFLAGS'].extend(CC_OPTIONS.split())



if GetOption('build_type') == 'unittests':
    target_variables['CPPDEFINES'].update ({ 'UNITTESTS' : None })

env = Environment(**target_variables)

env['PURE_CC'] = env['CC']


env['INSTALL_DIR'] = env.Dir(env['BUILDROOT'])
env['THIRDPARTY_ROOT'] = env.Dir('third-party',env['INSTALL_DIR'])
env['THIRDPARTY_INC_DIR'] = env.Dir('include',env['THIRDPARTY_ROOT'])
env['THIRDPARTY_LIB_DIR'] = env.Dir('lib',env['THIRDPARTY_ROOT'])

env['CPPPATH'].append('#/globalincludes')
env['CPPPATH'].append(env['THIRDPARTY_INC_DIR'])

env['BIN_DIR'] = env.Dir('bin',env['INSTALL_DIR'])
env['LIB_DIR'] = env.Dir('lib',env['INSTALL_DIR'])
env['LIBPATH'] = [env['LIB_DIR'],env['THIRDPARTY_LIB_DIR']]

env.VariantDir('%s' %env['BUILDROOT'], '.', duplicate=0)
env.SConsignFile('%s/.scons_signatures' %env.Dir(env['BUILDROOT']) )
 
env.App = ProgramBuilder(env)
env.Lib = LibBuilder(env)

env.PrependENVPath('PATH',os.environ['PATH'])

env.SConscript('%s/Sconscript' %env['BUILDROOT'], exports={'env':env})
