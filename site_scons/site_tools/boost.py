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
  x = result.substitute( THIRDPARTY_INC_DIR= env.Dir(env['THIRDPARTY_INC_DIR']).abspath,
                         THIRDPARTY_LIB_DIR= env.Dir(env['THIRDPARTY_LIB_DIR']).abspath,
                         THIRDPARTY_DIR= env.Dir(env['THIRDPARTY_ROOT']).abspath
                       )
  return x


def boostEmitter(target, source, env):
  boostdesc = json.loads(open(source[0].srcnode().abspath).read())
  emitter_node = env.File(boostdesc['emitterlist'], source[0].dir)
  source.append(emitter_node)
    
  source_node = env.File(boostdesc['source'], source[0].dir)
  source.append(source_node)

#Add patches as sources 
  tag = env['CrossCompiler']
  patch_list = boostdesc['tags'][tag].get('patches', None)

  if patch_list:
    for patch in patch_list:
      print patch
      patch_node = env.File(patch,source[0].dir)
      source.append(patch_node)

  emitter_list = json.loads(open(emitter_node.srcnode().abspath, 'r').read())
  
  headers = emitter_list.get('headers',None)
  
  if headers:
    for h in headers:
      target.append(env.File(h, env['BUILDROOT']))

  libs = emitter_list.get('libraries',None)
  
  if libs:
    for l in libs:
      target.append(env.File(l, env['BUILDROOT']))
      
  # Scan for explicit dependecies
  deps = emitter_list.get('dependencies',None)

  if deps:
    for dep in deps:
      for targ in target:
        env.Depends(targ, env.File(dep, env['BUILDROOT']))
      
  return target,source
  

def boostGenerator(target, source, env, for_signature):
  boostdesc = json.loads(open(source[0].srcnode().abspath).read())
  emitterlist = json.loads(open(source[1].srcnode().abspath, 'r').read())
  source_node = env.File(boostdesc['source'], source[0].dir)

  tag = env['CrossCompiler']
  buildroot = env.Dir(env['BUILDROOT']).abspath
  
  unpack_dir = env.Dir(source[0].dir)
  unpack_subdir_name = emitterlist['unpack.dir']
  unpack_subdir = env.Dir(unpack_subdir_name, unpack_dir)

  env['UNPACK_DIR'] = unpack_dir
  env['SOURCE_DIR'] = unpack_subdir
  env['PREFIX'] = env.Dir(env['THIRDPARTY_ROOT']).abspath

  commands = [
      SCons.Script.Delete('$UNPACK_DIR'),
      SCons.Script.Mkdir('$UNPACK_DIR'),
      '$TAR -C $UNPACK_DIR -x -f ${SOURCES[2]}'
  ]

  bootstrap_command = "cd $SOURCE_DIR && "
  bootstrap_command += "./bootstrap.sh --prefix=$PREFIX"
                    
  if boostdesc.get('exclude-libraries',None):
    without = ""
    for l in boostdesc['exclude-libraries']:
      without += "%s," %l
    
    without = without[0:len(without)-1]
    bootstrap_command += " --without-libraries=%s" %without
  
  
  commands.append(bootstrap_command)
  
  # After the bootstrap ran,patch the toolchain, patches start at index 3 after the emitter ran
  
  patch_source_number = 3
  srcs = source[patch_source_number:]
  patches = boostdesc['tags'][tag].get('patches', None)
  if patches:
    for patch in patches:
      commands += [
        'patch -d $SOURCE_DIR -p1 < ${SOURCES[%d]}' % (patch_source_number),
      ]
      patch_source_number += 1
  
  # now start building boost 
  
  boost_build_command = 'cd $SOURCE_DIR &&  '
  compiler_binaries = env.get('COMPILER_BINARIES', None)
    
  if compiler_binaries:
    boost_build_command += "PATH=%s:$$PATH " %compiler_binaries
  
  env_flags = ""
  flag_list = boostdesc.get('environment-flags',None)
  if flag_list:
    for flag in flag_list:
      env_flags += "%s " %flag
    
    env_flags = resolve_dynamic_flag(env,env_flags)

  feat_flags = ""
  flag_list = boostdesc.get('feature-flags',None)
  if flag_list:
    for flag in flag_list:
      feat_flags += "%s " %flag
    
    feat_flags = resolve_dynamic_flag(env,feat_flags)

  toolset_str=""
  toolset = boostdesc['tags'][tag].get('toolset', None)
  if toolset:
    toolset_str = "toolset=%s " %toolset
  
  jobs = 2*multiprocessing.cpu_count()
    
  boost_build_command += "%s ./b2 %s -q -j %d %s install" %(env_flags, toolset_str, jobs, feat_flags)
  
  commands.append(boost_build_command)
  return commands
  

def generate(env, **kw):
    boostBuilder = SCons.Builder.Builder(generator=boostGenerator, emitter=boostEmitter)
    env['BUILDERS']['BoostBuilder'] = boostBuilder

def exists(env):
    return 1
  
