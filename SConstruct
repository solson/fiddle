import os
import subprocess

def shell(command):
  return subprocess.check_output(command, shell=True).rstrip()

env = Environment(
  CXX = 'clang++',
  CPPFLAGS = [
    '-Wall',
    '-Wextra',
    '-std=c++11',

    # From llvm-config --cxxflags. Not using env.ParseConfig because we don't
    # want all of the flags it prints.
    '-D_GNU_SOURCE',
    '-D__STDC_CONSTANT_MACROS',
    '-D__STDC_FORMAT_MACROS',
    '-D__STDC_LIMIT_MACROS',
    '-fomit-frame-pointer',
    '-fvisibility-inlines-hidden',
    '-fno-exceptions',
    '-fPIC',
    '-Woverloaded-virtual',
    '-Wcast-qual',

    # Temporarily silence this warning because Clang 3.4 erroneously detects
    # it in some LLVM header files.
    '-Wno-unused-parameter',
  ],

  CPPPATH = [shell('llvm-config --includedir')],

  # Allow clang++ to use color.
  ENV = {'TERM': os.environ['TERM']},
)

env.ParseConfig('llvm-config --ldflags --libs core')
env.ParseConfig('pkg-config --libs --cflags libedit icu-uc')

env.Program(
  target = 'fiddle',
  source = [
    'ast.cpp',
    'codegen.cpp',
    'editline.cpp',
    'lexer.cpp',
    'main.cpp',
    'parser.cpp',
    'token.cpp',
    'types.cpp',
  ],
)

if int(ARGUMENTS.get('debug', 0)):
  env.Append(CPPFLAGS = ['-g'])
else:
  env.Append(CPPFLAGS = ['-DNDEBUG', '-O2'])

# Print C++ flags for the YouCompleteMe vim plugin.
if ARGUMENTS.get('ycm', 0):
  print(env.subst('$CXXFLAGS $CCFLAGS $_CCCOMCOM'))
  exit()
