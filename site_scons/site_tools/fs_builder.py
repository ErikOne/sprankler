# scons builder to construct a root file system
#
import json
import os
import sys
import SCons.Script

def trim_heading_slash(instr):
    out = instr
    if instr[0] == '/':
        out = instr[1:]
    return out


class CmdScript():
  def __init__(self,env):
    self.cmdlist = []
    self.env = env
    self.debug = env.GetOption("debug_build")

    self.cmdlist.append("rm -rf $ROOTFS_ROOT")
    self.cmdlist.append("mkdir -p $ROOTFS_ROOT")
    
    if self.debug == True:
      print("WARNING : This is a debug build, which will result in a larger filesystem")
    

# Write all the commands into $ROOTFS_ROOT/fs_install.sh and execute that script
# with fakeroot later 
# Please note that all the commands are executed between one big pair of parentheses.
 
    self.addCommand("rm -fr fs")
    self.addCommand("mkdir fs")
    self.addCommand("(")
    self.addCommand("cd fs")

  def addCommand(self, cmdstr):
    self.cmdlist.append('echo "%s" >>$ROOTFS_ROOT/fs_install.sh' % cmdstr)

  def getCommands(self):
    return self.cmdlist
    
  def closeCommandList(self,target_triple):
    self.addCommand(")")
    # Strip everything 
    
    if self.debug == False:
      self.addCommand('echo saving file permissions')
      self.addCommand("find fs -type f  -exec stat --format '%a' {} \; -print  | xargs -n 2 | sed 's/^/chmod  /' > restore_permissions.sh" )
      self.addCommand("chmod 755 restore_permissions.sh")

      self.addCommand('echo stripping binaries ...')
      self.addCommand('find fs -type f | xargs -n 1 %s-strip 2>/dev/null' %target_triple)

      self.addCommand('echo restoring file permissions')
      self.addCommand('./restore_permissions.sh')

  
    
    self.addCommand('echo creating tar bal ...')
    self.addCommand('tar  pcjf rootfs.bz2 -C fs .')
    self.addCommand('echo Filesystem created at $ROOTFS_ROOT/rootfs.bz2, unpack it as root')
    
    fakeroot = self.env.File('OneTools/buildtools/fakeroot/fakeroot',self.env.Dir('#'))
    
    self.cmdlist.append('cd $ROOTFS_ROOT; chmod +x fs_install.sh')
    self.cmdlist.append('cd $ROOTFS_ROOT; %s ./fs_install.sh' %fakeroot.abspath)

def fsEmitter(target, source, env):
  description = json.loads(open(source[0].srcnode().abspath).read())

  if not env['TARGET_TAG'] in description:
    return [],[]
  
  target_info = description[env['TARGET_TAG'] ]
  
  # All the local files must be treated as source files
  
  for f in target_info.get("files",[]):
    s = env.File(f[0],source[0].Dir('.').srcnode())
    source.append(s)

  for folder in target_info.get("folders",[]):
    src_dir = env.Dir(folder[0],source[0].Dir('.').srcnode()).abspath
    for root, dirs, files in os.walk(src_dir):
        length = len(src_dir)
        if src_dir[-1] != '/':
          length += 1
        
        newroot = root[length:]
    
        for file in sorted(files):
          src_file = os.path.join(src_dir,newroot,file)
          source.append(src_file)

   
  if "packages" in target_info:
    for p in target_info["packages"]:
      found = False
      for ep in env['PACKAGE_LIST']:
        if p == ep["name"]:
          found = True
          if "copy" in ep["action_list"]: 
            for generated_file in ep["action_list"]["copy"]:
              source.append(generated_file[0])
      if found == False:
        print("ERROR: missing package: " + p)
        raise SCons.Errors.BuildError("Missing package: " + p)



  target.append(env.File('$ROOTFS_ROOT/rootfs.bz2') )
  return target,source
  
def fsGenerator(target, source, env, for_signature):
  script = CmdScript(env)
  description = json.loads(open(source[0].srcnode().abspath).read())
  currentDir = srcdir = source[0].Dir('.').srcnode().abspath 
  tag = env['TARGET_TAG']
  
  for cmd in description[tag].get("commands",[]):
    script.addCommand(cmd)

  runtime_lib_list = description[tag].get("toolchain_runtime_libs",[])
  for lib in runtime_lib_list:
    script.addCommand("(cd %s; tar -c %s) | tar -x" % (env['Toolchain_Runtime'].abspath, lib))

  folder_list = description[tag].get("folders",[])
  for folder in folder_list:
    src = os.path.join(currentDir,folder[0])
    dst = trim_heading_slash(folder[1])
    owner = folder[2]
    group = folder[3]
    mode  = folder[4]
    
    for root, dirs, files in os.walk(src):
        length = len(src)
        if src[-1] != '/':
          length += 1
        
        newroot = root[length:]
    
        for file in sorted(files):
          src_file = os.path.join(src,newroot,file)
          target_file = os.path.join(dst,newroot,file)
          
          if os.path.islink(src_file):
            src_dir = os.path.join(src,newroot)
            link_file = os.path.relpath(os.path.realpath(src_file),src_dir)
            script.addCommand("ln -s %s %s" % (link_file, target_file))
          elif os.path.isfile(src_file):
            target_file = os.path.join(dst,newroot,file)
            script.addCommand("install -D -o %s -g %s -m %s %s %s" % (owner, group, mode, src_file, target_file))



  file_list = description[tag].get("files",[])
  for file in file_list:
    src = os.path.join(currentDir,file[0])
    dst = trim_heading_slash(file[1])
    owner = file[2]
    group = file[3]
    mode  = file[4]
    script.addCommand("install -D -o %s -g %s -m %s %s %s" % (owner, group, mode, src, dst))

  symlink_list = description[tag].get("symlinks",[])
  for item in symlink_list:
    src = item[0]
    dst = trim_heading_slash(item[1])
    script.addCommand("ln -s %s %s" % (src, dst))

  
  pkglist = env.get("PACKAGE_LIST",[])
  for pkg in pkglist:
    if pkg['name'] in description[tag].get('packages',[]):
      action_list = pkg.get("action_list",[])
    
      for item in  action_list.get('commands',[]):
        script.addCommand(item)
      
      for item in action_list.get('copy',[]):
        src = env.File(item[0]).abspath
        dst = trim_heading_slash(item[1])
        owner = item[2]
        group = item[3]
        mode  = item[4]
        script.addCommand("install -D -o %s -g %s -m %s %s %s" % (owner, group, mode, src, dst))

      for item in action_list.get('symlink',[]):
        src = item[0]
        dst = trim_heading_slash(item[1])
        script.addCommand("ln -s %s %s" % (src, dst))
    
  script.closeCommandList(env['TARGET_TRIPLE'])
    
  return script.getCommands() 


def generate(env, **kw):
    fsBuilder = SCons.Builder.Builder(generator=fsGenerator, 
                                      emitter=fsEmitter)
                                      
    env['BUILDERS']['FsBuilder'] = fsBuilder

def exists(env):
  return 1
