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

basic_variables = {
    'LIBS'          : [],
    'SHARED_LIBS'   : ['-lrt', '-lpthread' ],
    'CCFLAGS'       : [ '-Wall','-Werror' ],
    'CFLAGS'        : [],
    'CXXFLAGS'      : [],
    'CPPPATH'       : [],
    'CPPDEFINES'    : {},
    'LINKFLAGS'     : [],
    'PACKAGE_LIST'  : [],
    'targetEnv'     : False,
    'isHostToolsEnv': False
}

target_variables = copy.deepcopy(basic_variables)
target_variables['BUILDROOT'] = "#/do/"+GetOption('target')

if GetOption('build_type') == 'unittests':
    target_variables['CPPDEFINES'].update ({ 'UNITTESTS' : None })

env = Environment(**target_variables)

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

env.SConscript('%s/Sconscript' %env['BUILDROOT'], exports={'env':env})