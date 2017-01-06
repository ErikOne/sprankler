# Custom builder to build a kernel from a tar file, patch files and config

import json
import shlex
import shutil
import os
import sys
import multiprocessing
import SCons.Script

def kernelEmitter(target, source, env):
# I wil add the following files as sources so whenever the get changes scons wil rebuild
#   the kernel.desc ( which is source[0] already)
#   the tarfile with the kernel code
#   the config file

    jsondesc = json.loads(open(source[0].srcnode().abspath).read())
    source_node = env.File(jsondesc['source'], source[0].dir)
    source.append(source_node)
    source_node = env.File(jsondesc['config'], source[0].dir)
    source.append(source_node)

    build_device_tree = jsondesc.get('device-tree-config', None)
    
    if build_device_tree:
      source_node = env.File(jsondesc['device-tree-config'], source[0].dir)
      source.append(source_node)

#Add patches as sources 
    tag = env['CrossCompiler']
    patch_list = jsondesc.get('patches', None)
    if not patch_list:
      patch_list = jsondesc.get('patches', None)

    if patch_list:
     for patch in patch_list:
      patch_node = env.File(patch, source[0].dir)
      source.append(patch_node)
       
    kernelImage = env.File('uImage',env.Dir(env['INSTALL_DIR']))
    target.append(kernelImage)
    
    if build_device_tree:
      dtbFile = env.File('devicetree.dtb',env.Dir(env['INSTALL_DIR']))
      target.append(dtbFile)
    
    env.SideEffect(env.Dir('.').path, target)

    return target, source

def kernelGenerator(target, source, env, for_signature):
  jsondesc = json.loads(open(source[0].srcnode().abspath).read())
  source_node = env.File(jsondesc['source'], source[0].dir)
  architecture = jsondesc.get('arch', "NO_ARCH_SPECIFIED")

  tag = env['CrossCompiler']
  buildroot = env.Dir(env['BUILDROOT']).abspath

  unpack_dir = env.Dir(source[0].dir)
  subdir_name = jsondesc['kernel-dir']
  unpack_subdir = env.Dir(subdir_name, unpack_dir)

# The patches start at position 4 in the sources array after runnnig the emiter
  patch_source_number = 4
  
  srcs = source[patch_source_number:]
  build_device_tree = jsondesc.get('device-tree-config', None)
  
  patch_list = jsondesc.get('patches', None)
  if not patch_list:
      patch_list = jsondesc.get('patches', None)
  if not patch_list:
      patch_list = []
      
  patches = srcs[:len(patch_list)]
  prune_level = jsondesc.get('patch-prune-level', 1)
  
  env['UNPACK_DIR'] = unpack_dir
  env['UNPACK_SUB_DIR'] = unpack_subdir
  env['PREFIX'] = env.Dir(env['THIRDPARTY_ROOT'])
        
# Start generating the actual commands
  commands = [
      'rm -rf $UNPACK_DIR',
      'mkdir -p $UNPACK_DIR',
      '$TAR -C $UNPACK_DIR -x -f ${SOURCES[1]}',
      SCons.Script.Copy('${UNPACK_SUB_DIR}/.config','${SOURCES[2]}'),
      SCons.Script.Copy('${UNPACK_SUB_DIR}/arch/%s/boot/dts' %architecture ,'${SOURCES[3]}')
  ]

  for patch in patches:
    commands += [
          'patch -d $UNPACK_SUB_DIR -p%d < ${SOURCES[%d]}' % (prune_level, patch_source_number),
    ]
    patch_source_number += 1

  # build the command that puts us in the right directory with the right path
  
  cd_command = 'cd $UNPACK_SUB_DIR && '
  compiler_binaries = env.get('COMPILER_BINARIES', None)
  
  make_command = cd_command
  
  # I'm not taking GetOption('num_jobs') because that would result in a rebuild every time the -j option is changed
  jobs = 2*multiprocessing.cpu_count()
  
  make_command += "ARCH=%s CROSS_COMPILE=%s- make -j %d uImage" %( architecture,env['TARGET_TRIPLE'],jobs)
  commands.append(make_command)

  if build_device_tree:
    dtb_command = cd_command
    dtb_command += "scripts/dtc/dtc -o ${TARGETS[1].abspath} -O dtb -I dts arch/arm/boot/dts/${SOURCES[3].name}"
    commands.append(dtb_command)
  
  commands.append( SCons.Script.Copy('${TARGETS[0]}',
                   '${UNPACK_SUB_DIR}/arch/%s/boot/uImage' %architecture) )
  
  return commands

def generate(env, **kw):
    kernelBuilder = SCons.Builder.Builder(generator=kernelGenerator, emitter=kernelEmitter, suffix = '.kdesc')
    env['BUILDERS']['Kernel'] = kernelBuilder

def exists(env):
    return 1
