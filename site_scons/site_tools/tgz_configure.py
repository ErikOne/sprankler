# An SConscript file to untar a package, run configure and then make
# using a build directory.

import json
import shlex
import shutil
import os
import sys
import SCons.Script
import multiprocessing
from string import Template

def resolve_dynamic_flag(env,flag):

  result= Template(flag)
  x = result.substitute( THIRDPARTY_INC_DIR= env.Dir(env['THIRDPARTY_INC_DIR']).abspath,
                         THIRDPARTY_LIB_DIR= env.Dir(env['THIRDPARTY_LIB_DIR']).abspath,
                         THIRDPARTY_DIR    = env.Dir(env['THIRDPARTY_ROOT']).abspath
                       )
  return x



def tgzConfigureEmitter(target, source, env):
# Start by appending all the configuration files to the sources. This way if the
# configuration gets changed, a rebuild is forced.
# Currently the following files are tested
#     -The tgzdesc file : this is the main descriptor file having all the options
#     -The descriptor node : This is a generated json file that was created using
#                            the tool gen_descriptor.sh (in the root of the source tree)
#     - patch files listed in the tgsdesc
    
    btag = 'release'   
    tgzdesc = json.loads(open(source[0].srcnode().abspath).read())
    descriptor_node = env.File(tgzdesc['descriptor'], source[0].dir)
    descriptor = json.loads(open(descriptor_node.srcnode().abspath, 'r').read())
    source.append(descriptor_node)
    source_node = env.File(tgzdesc['source'], source[0].dir)
    source.append(source_node)

    tag = env['CrossCompiler']

    patch_list = getOverridableConfig(tag, tgzdesc, 'patches', [])

    for patch in patch_list:
        patch_node = env.File(patch, source[0].dir)
        source.append(patch_node)

    
    # KAZA : Why do I add this SideEffect ? Some parts of the stuff in import is build via automake, other
    # parts are using SCons directly. The Automake creates a directory in do/$platform/import. This directory is
    # not seen by SCons, so the parts in import that use SCons directly will try to create this directory again. 
    # To avoid this I explicitly announce this directory as a SideEffect. That way Scons knows that the directory
    # could be there because of the Automake buikds and he does not try to recreate it. (which would result in an 
    # error) 
    
    defer_side_effects = [env.Dir('.').path]

    install_paths = []
    if 'all' in descriptor[btag] and 'install' in descriptor[btag]['all']:
      install_paths = install_paths + descriptor[btag]['all']['install']
    if tag in descriptor[btag] and 'install' in descriptor[btag][tag]:
      install_paths = install_paths + descriptor[btag][tag]['install']

    for fntype, nodetype, subpath in install_paths:
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

    extraFiles = tgzdesc.get('ExplicitAnnounce', None)
    if extraFiles:
      for file in extraFiles:
        target.append(env.File(file, env['BUILDROOT']))
        
    # register all side effects as side effects of all targets, to be cleaned when
    # those targets are cleaned
    for se in defer_side_effects:
        for targ in target:
            env.SideEffect(se, targ)
            env.Clean(targ, se)
    
    # items in descriptor[btag][tag]['depends'] are relative to the fsroot for this variant,
    # which is stored, as a Node, in env['BUILDROOT'].  Each target produced
    # by this build must depend on all dependency targets

    depends_paths = []
    if 'all' in descriptor[btag] and 'depends' in descriptor[btag]['all']:
      depends_paths = depends_paths + descriptor[btag]['all']['depends']
    if tag in descriptor[btag] and 'depends' in descriptor[btag][tag]:
      depends_paths = depends_paths + descriptor[btag][tag]['depends']

    for dep in depends_paths:
      for targ in target:
        if dep[0] == 'F':
          env.Depends(targ, env.File(dep[2], env['BUILDROOT']))
   
    return target, source


def tgzConfigureGenerator(target, source, env, for_signature):
    tgzdesc = json.loads(open(source[0].srcnode().abspath).read())
    descriptor = json.loads(open(source[1].srcnode().abspath, 'r').read())
    source_node = env.File(tgzdesc['source'], source[0].dir)
    # patches we work out again in a while...

    tag = env['CrossCompiler']
    no_sysroot = tgzdesc.get('no-sys-root', False)
    build_zlib = tgzdesc.get('build-zlib', False)
    build_open_ssl = tgzdesc.get('build-open-ssl', False)
    build_in_tree = tgzdesc.get('build-in-tree', False)
    include_path = tgzdesc.get('special-include-path', None)
    lib_path =  tgzdesc.get('special-lib-path', None)
    build_glib = tgzdesc.get('build-glib', None)
    build_gstreamer = tgzdesc.get('build-gstreamer', None)
    build_gst_plugin = tgzdesc.get('build-gst-plugin', None)
    build_soup = tgzdesc.get('build-soup', None)
    no_prefix = tgzdesc.get('no_prefix', None)
    set_destdir = tgzdesc.get('set_destdir', None)
    
    buildroot = env.Dir(env['BUILDROOT']).abspath

    unpack_dir = env.Dir(source[0].dir)
    unpack_subdir_name = descriptor['unpack.subdir']
    unpack_subdir = env.Dir(unpack_subdir_name, unpack_dir)
    build_subdir = unpack_subdir

# The patches start at position 3 in the sources array after runnnig the emiter
    patch_source_number = 3
	
    srcs = source[patch_source_number:]
    patch_list = getOverridableConfig(tag, tgzdesc, 'patches', [])

    patches = srcs[:len(patch_list)]
    prune_level = tgzdesc.get('patch-prune-level', 1)

    env['UNPACK_DIR'] = unpack_dir
    env['SOURCE_DIR'] = unpack_subdir
    env['BUILD_DIR'] = build_subdir
    env['PREFIX'] = env.Dir(env['THIRDPARTY_ROOT'])
    env['INCLUDE_DIR'] = env.Dir(include_path,'%s/include' %env['PREFIX'])
    env['LIB_DIR'] = env.Dir(lib_path,'%s/lib' %env['PREFIX'])
    
    
# Start generating the actual commands
    commands = [
        SCons.Script.Mkdir('$UNPACK_DIR'),
        '$TAR -C $UNPACK_DIR -x -f ${SOURCES[2]}',
        SCons.Script.Mkdir('$BUILD_DIR'),
    ]

    for patch in patches:
        commands += [
            'patch -d $SOURCE_DIR -p%d < ${SOURCES[%d]}' % (prune_level, patch_source_number),
        ]
        patch_source_number += 1

    # build the configure command
    configure_command = 'cd $BUILD_DIR && '

    compiler_binaries = env.get('COMPILER_BINARIES', None)
    
    if compiler_binaries:
        configure_command += "PATH=%s:$$PATH" %compiler_binaries

    configure_command += ' '
    

    benv = getOverridableConfig(tag, tgzdesc, 'configure_environment', '')
    if benv:
        configure_command += benv
        configure_command += ' '

    make_depend_command = configure_command
    make_command = configure_command
    make_install_command = configure_command

    if build_zlib:
      configure_command += './configure '
    elif build_open_ssl:
      configure_command += 'perl ./Configure '
    elif tgzdesc.get('configure-path', '') != '':
      configure_command += '${SOURCE_DIR.abspath}/%s/configure ' %tgzdesc['configure-path']
    else:
      configure_command += '${SOURCE_DIR.abspath}/configure '
    
    if not build_in_tree:
        configure_command += '--srcdir=%s' %unpack_subdir.abspath + ' '
    
    if env['CrossCompiling']:
      if not build_zlib and not build_open_ssl:
        if tgzdesc.get('use-target-flag', '') == 'True':
          configure_command += '--target=${TARGET_TRIPLE} '
        else:
          configure_command += '--host=${TARGET_TRIPLE} '
      elif build_open_ssl:
        configure_command += '--cross-compile-prefix=${TARGET_TRIPLE}- '

    if not no_sysroot and not build_zlib and not build_open_ssl:
        sysroot = env.Dir(env['BUILDROOT']).abspath
        configure_command += '--with-sysroot=%s ' %sysroot

    cache_file = getOverridableConfig(tag, tgzdesc, 'cache-file', None)
    if (cache_file):
      file = cache_file.encode('utf-8')
      src = os.path.join(env.Dir(source[0].dir).srcnode().abspath,cache_file).encode('utf-8')
      dest = os.path.join(env.Dir(source[0].dir).abspath,cache_file).encode('utf-8')
      configure_command += ' --cache-file=%s ' %dest
      
      # Add a copy command to copy the cache file to the correct position 
      commands.append(SCons.Defaults.Copy(dest,src))
      

    thirdparty= '%s/third-party' %buildroot

    if build_glib == "True":
      extra_config_options =  'LIBFFI_CFLAGS=-I%s/include/ffi ' %thirdparty
      extra_config_options += 'LIBFFI_LIBS=-lffi '
      extra_config_options += 'ZLIB_CFLAGS=-I%s/include/zlib '%thirdparty
      extra_config_options += 'ZLIB_LIBS=-lz '
      extra_config_options += 'LDFLAGS="-L%s/lib" ' %thirdparty
      configure_command += extra_config_options

    elif build_gstreamer == "True":
      extra_config_options =  ' GLIB_CFLAGS="-I%s/include/glib-2.0 -I%s/lib/glib-2.0/include"' %(thirdparty,thirdparty)
      extra_config_options += ' GLIB_LIBS="-lglib-2.0 -lgobject-2.0 -lgmodule-2.0"'
      extra_config_options += ' GIO_CFLAGS=-I%s/include/glib-2.0/gio' %thirdparty
      extra_config_options += ' GIO_LIBS=-lgio-2.0'
      extra_config_options += ' LDFLAGS="-L%s/lib" ' %thirdparty
      configure_command += extra_config_options
      
    elif build_gst_plugin == "True":
      extra_config_options = ' --with-pkg-config-path="%s/lib/pkgconfig:' %thirdparty
      extra_config_options += '%s/gstreamer/gst-core/gstreamer-1.0.10/pkgconfig:' %thirdparty
      extra_config_options += '%s/gstreamer/base-plugins/gst-plugins-base-1.0.10/pkgconfig" ' %thirdparty
      configure_command += extra_config_options
    
    elif build_soup == "True":
      extra_config_options = ' PKG_CONFIG_PATH=%s/lib/pkgconfig ' %thirdparty
      extra_config_options += 'LD_LIBRARY_PATH=%s/lib ' %thirdparty
      configure_command += extra_config_options

    if no_prefix != "True":
      configure_command += '--prefix=${PREFIX.abspath} '

    if include_path:
      configure_command += '--includedir=${INCLUDE_DIR.abspath} '
    if lib_path:
      configure_command += '--libdir=${LIB_DIR.abspath} '

    flags = getOverridableConfig(tag, tgzdesc, 'basic_flags', '')
    if flags:
        if configure_command[-1] != ' ': configure_command += ' '
        for flag in flags:
          configure_command += '%s ' %resolve_dynamic_flag(env,flag)

    flags = getOverridableConfig(tag, tgzdesc, 'dynamic_flags', '')
    if flags:
        if configure_command[-1] != ' ': configure_command += ' '
        for flag in flags:
          configure_command += '%s ' %resolve_dynamic_flag(env,flag)



# This should come via get getOption
    debug = False

    if not build_zlib:
      flags = getOverridableConfig(tag, tgzdesc, debug and 'debug_flags' or 'release_flags', '')
      if flags:
        if configure_command[-1] != ' ': 
          configure_command += ' '
        configure_command += flags

      flags = getOverridableConfig(tag, tgzdesc, 'extra_flags', '')
      if flags:
        if configure_command[-1] != ' ':
          configure_command += ' '
        configure_command += flags

    commands.append(configure_command)

    make_depend_command += 'make depend'

    parallel_build = tgzdesc.get('parallel_build', True)
    if parallel_build:
        jobs = 2*multiprocessing.cpu_count()
    else:
        jobs = 1 

    if not build_open_ssl:
      make_command += 'make -j %d' %jobs
      commands.append(make_command)
      make_install_command += 'make install'

      if set_destdir == "True":
        make_install_command += ' DESTDIR=${PREFIX.abspath}'

      commands.append(make_install_command)
    else:
      # OpenSSL has a crapty makefile. Therefor build only with 1 jobs
      install_sw = make_command
      install_sw += 'make -j 1 install'
      commands.append(install_sw)
    
    return commands


def generate(env, **kw):
    tgzConfigureBuilder = SCons.Builder.Builder(generator=tgzConfigureGenerator, emitter=tgzConfigureEmitter, suffix = '.tgzdesc')
    env['BUILDERS']['TgzConfigure'] = tgzConfigureBuilder

def exists(env):
    return 1


def getOverridableConfig(tag, tgzdesc, key, default):
    result = default
    if 'tags' in tgzdesc and tag in tgzdesc['tags'] and key in tgzdesc['tags'][tag] and tgzdesc['tags'][tag][key]:
      result = tgzdesc['tags'][tag][key]
    elif key in tgzdesc and tgzdesc[key]:
      result = tgzdesc[key]

    return result
#YA
