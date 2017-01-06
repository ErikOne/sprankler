class OneBuilder():
    def createBuildEnv(self, kw):
        env = self.env.Clone()
        #Add the CPPPATH to the enviromment 
        env['CPPPATH'].append(env.Dir('header','..').srcnode())
        
        for key in kw:
          if 'TESTBUILD' == key:
            env['CPPPATH'].append(env.Dir('header','../..').srcnode())
          else:
            self.copyToEnv(env,key,kw[key])

        if 'USE_GLIB' in kw:
          env['CPPPATH'].append('%s/include/glib-2.0' %env['THIRDPARTY_ROOT'])
          env['CPPPATH'].append('%s/lib/glib-2.0/include' %env['THIRDPARTY_ROOT'])
          env['CPPPATH'].append('%s/include/gstreamer-1.0' %env['THIRDPARTY_ROOT'])
            
        return env
        
    def copyToEnv(self,newEnv,key,value):
      if type(value) in (tuple,list):
        newEnv[key] = value[:]
      elif type(value) == dict:
        newEnv[key] = value.copy()
      else:
        newEnv[key] = value
          

class ProgramBuilder(OneBuilder):
    def __init__(self,env):
        self.env = env

    def __call__(self, target, source, **kw):
        env = self.createBuildEnv(kw)

# In order to avoid circular dependencies on the linker, we put everything between
# the start-group and end-group flags.
# After that we assign the LIBS to the environment so we know that the libraries 
# are taken as a dependency.
# Than there is one last trick I add. Normally the LINKCOM ends with the variable
# $_LINKFLAGS, this flag is autogenerated by scons and lists all the -l option 
# for the linker. Because I already have the libraries between my group flags this
# is no longer necesary.

# Plugin libs are libraries which are only triggered by the observer pattern. In the 
# initialisation of the library the plugin has to subscribe to all the subjects it is want but 
# the initialisation function is never called explicitely from the rest of the code.
# To be sure that is is called we work with the special GNU attribute constructor. To be sure 
# the linker does not throw out these unused initialisation function we have to pass in a special 
# flag to the linker (--whole-archive) and end with (--no-whole-archive) 

        libs = ''
        system_libs = ''
        plugins = ''
        
        if 'SYSTEM_LIBS' in kw:
           for lib in kw['SYSTEM_LIBS']:
              system_libs += '-l%s ' %lib

        if 'PLUGINS' in kw:
           plugins = '-Wl,--whole-archive '
           for plugin in kw['PLUGINS']:
              plugins += '-l%s ' %plugin
           plugins += '-Wl,--no-whole-archive'

        
        for lib in kw['LIBS']:
           if 'SYSTEM_LIBS' in kw:
              if lib not in kw['SYSTEM_LIBS']:
                 libs += '-l%s ' %lib
           else:
              libs += '-l%s ' %lib
        
        env['LIBS'] = kw['LIBS'][:]
        if 'SYSTEM_LIBS' in kw:
           env['LIBS'].extend(kw['SYSTEM_LIBS'])
        if 'PLUGINS' in kw:
           env['LIBS'].extend(kw['PLUGINS'])
        
        
        env['LINKCOM'] = '$LINK -o $TARGET $LINKFLAGS $__RPATH $SOURCES $_LIBDIRFLAGS -Wl,--start-group ' + libs + ' -Wl,--end-group $SHARED_LIBS '+ system_libs + ' ' + plugins
          
        program = env.Install('$BIN_DIR',env.Program(target, source))
        return program

class LibBuilder(OneBuilder):
    def __init__(self,env):
        self.env = env

    def __call__(self, target, source, **kw):
        env = self.createBuildEnv(kw)
        lib = env.Library(target, source)
        env.Install('$LIB_DIR',lib)
        return lib

class SharedLibBuilder(OneBuilder):
    def __init__(self,env):
        self.env = env

    def __call__(self, target, source, **kw):
        env = self.createBuildEnv(kw)
        
        lib = env.SharedLibrary(target, source)
        env.Install('$LIB_DIR',lib)
        return lib

class ObjectBuilder(OneBuilder):
    def __init__(self,env):
        self.env = env

    def __call__(self, target, source, **kw):
        env = self.createBuildEnv(kw)
        
        o = env.Object(target, source)
        return o

def CopyEnv(env):
	clonedEnv = env.Clone()
	clonedEnv.App = ProgramBuilder(clonedEnv)
	clonedEnv.Lib = LibBuilder(clonedEnv)
	clonedEnv.SharedLib = SharedLibBuilder(clonedEnv)
	clonedEnv.Obj = ObjectBuilder(clonedEnv)
	
	return clonedEnv
