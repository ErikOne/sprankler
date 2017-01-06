#Custom builder to untar a package file that uses cmake to get build

import json
import shlex
import shutil
import os
import sys
import multiprocessing
import SCons.Script
from string import Template

def resolve_dynamic_flag(env,flag):

  result= Template(flag)
  x = result.substitute( THIRDPARTY_INC_DIR= env['THIRDPARTY_INC_DIR'].abspath,
                         THIRDPARTY_LIB_DIR= env['THIRDPARTY_LIB_DIR'].abspath
                       )
  return x

def cmakeEmitter(target, source, env):

    
    cmakedesc = json.loads(open(source[0].srcnode().abspath).read())
    descriptor_node = env.File(cmakedesc['descriptor'], source[0].dir)
    descriptor = json.loads(open(descriptor_node.srcnode().abspath, 'r').read())
    source.append(descriptor_node)
    source_node = env.File(cmakedesc['source'], source[0].dir)
    source.append(source_node)

#Add patches as sources 
    tag = env['CrossCompiler']
    patch_list = cmakedesc['tags'][tag].get('patches', None)
    if not patch_list:
      patch_list = cmakedesc.get('patches', None)

    if patch_list:
     for patch in patch_list:
      patch_node = env.File(patch, source[0].dir)
      source.append(patch_node)

    tag = env['CrossCompiler']
    
    # KAZA : Why do I add this SideEffect ? Some parts of the stuff in import is build via automake, other
    # parts are using SCons directly. The Automake creates a directory in do/$platform/import. This directory is
    # not seen by SCons, so the parts in import that use SCons directly will try to create this directory again. 
    # To avoid this I explicitly announce this directory as a SideEffect. That way Scons knows that the directory
    # could be there because of the Automake buikds and he does not try to recreate it. (which would result in an 
    # error) 
    
    defer_side_effects = [env.Dir('.').path]
    
    for fntype, nodetype, subpath in descriptor[tag]['install']:
      if fntype == 'F':
        if 'T' == nodetype:
          target.append(env.File(subpath, env['BUILDROOT']))  
        else:
          node = env.File(subpath, env['BUILDROOT'])
          defer_side_effects.append(node)
      else:
        if 'T' == nodetype:
          target.append(env.Dir(subpath, env['BUILDROOT']))
        else:
          node = env.Dir(subpath, env['BUILDROOT'])
          defer_side_effects.append(node)
    
    # register all side effects as side effects of all targets, to be cleaned when
    # those targets are cleaned
    for se in defer_side_effects:
        for targ in target:
            env.SideEffect(se, targ)
            env.Clean(targ, se)
    
    # items in descriptor[tag]['depends'] are relative to the fsroot for this variant,
    # which is stored, as a Node, in env['BUILDROOT'].  Each target produced
    # by this build must depend on all dependency targets
    if 'depends' in descriptor[tag]:
      deps = descriptor[tag]['depends']
      if deps:
        for dep in deps:
          for targ in target:
            if dep[0] == 'F':
              env.Depends(targ, env.File(dep[2], env['BUILDROOT']))
   
    return target, source

def cmakeGenerator(target, source, env, for_signature):
  cmakedesc = json.loads(open(source[0].srcnode().abspath).read())
  descriptor = json.loads(open(source[1].srcnode().abspath, 'r').read())
  source_node = env.File(cmakedesc['source'], source[0].dir)
  # patches we work out again in a while...

  tag = env['CrossCompiler']
    
  buildroot = env.Dir(env['BUILDROOT']).abspath

  unpack_dir = env.Dir(source[0].dir)
  unpack_subdir_name = descriptor['unpack.subdir']
  unpack_subdir = env.Dir(unpack_subdir_name, unpack_dir)
  build_subdir = unpack_subdir

# The patches start at position 3 in the sources array after runnnig the emiter
  patch_source_number = 3
  
  srcs = source[patch_source_number:]
  patch_list = cmakedesc['tags'][tag].get('patches', None)
  if not patch_list:
      patch_list = cmakedesc.get('patches', None)
  if not patch_list:
      patch_list = []
      
  patches = srcs[:len(patch_list)]
  prune_level = cmakedesc.get('patch-prune-level', 1)
  
  env['UNPACK_DIR'] = unpack_dir
  env['SOURCE_DIR'] = unpack_subdir
  env['BUILD_DIR'] = build_subdir
  env['PREFIX'] = env.Dir(env['THIRDPARTY_ROOT'])
        
# Start generating the actual commands

  commands = [
      SCons.Script.Delete('$UNPACK_DIR'),
      SCons.Script.Mkdir('$UNPACK_DIR'),
      '$TAR -C $UNPACK_DIR -x -f ${SOURCES[2]}'
  ]

  for patch in patches:
    commands += [
          'patch -d $SOURCE_DIR -p%d < ${SOURCES[%d]}' % (prune_level, patch_source_number),
    ]
    patch_source_number += 1

  # build the command that puts us in the right directory with the right path
  
  cd_command = 'cd $BUILD_DIR && '
  compiler_binaries = env.get('COMPILER_BINARIES', None)
    
  if compiler_binaries:
    cd_command += "PATH=%s:$$PATH " %compiler_binaries

  cmake_command = cd_command

  flags = cmakedesc['tags'][tag].get('environment_flags', '')
  if not flags:
      flags = cmakedesc.get('environment_flags', '')
  if flags:
      if cmake_command[-1] != ' ': cmake_command += ' '
      for flag in flags:
        cmake_command += '%s ' %resolve_dynamic_flag(env,flag) 

  make_command  = cd_command
  make_install_command = cd_command

  cmake_command += ' cmake -DCMAKE_INSTALL_PREFIX=${PREFIX.abspath} '
  
  if env['CrossCompiling'] == True:
    target_triple = env['TARGET_TRIPLE']
    cmake_command += "-DCMAKE_C_COMPILER=%s-gcc " %target_triple
    cmake_command += "-DCMAKE_CXX_COMPILER=%s-g++ " %target_triple

  flags = cmakedesc['tags'][tag].get('basic_flags', '')
  if not flags:
      flags = cmakedesc.get('basic_flags', '')
  if flags:
      if cmake_command[-1] != ' ': cmake_command += ' '
      for flag in flags:
        cmake_command += '%s ' %flag

  flags = cmakedesc['tags'][tag].get('dynamic_flags', '')
  if not flags:
      flags = cmakedesc.get('dynamic_flags', '')
  if flags:
      if cmake_command[-1] != ' ': cmake_command += ' '
      for flag in flags:
        cmake_command += '%s ' %resolve_dynamic_flag(env,flag)      
    
  commands.append(cmake_command)

  # I'm not taking GetOption('num_jobs') because that would result in a rebuild every time the -j option is changed
  jobs = 2*multiprocessing.cpu_count()
  
  make_command += 'make -j %d ' %jobs
  make_install_command += ' make install'

  commands.append(make_command)
  commands.append(make_install_command)

  return commands

def generate(env, **kw):
    cmakeBuilder = SCons.Builder.Builder(generator=cmakeGenerator, emitter=cmakeEmitter, suffix = '.cmake')
    env['BUILDERS']['TgzCMake'] = cmakeBuilder

def exists(env):
    return 1
