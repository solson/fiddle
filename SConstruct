import os

env = Environment(
    CXX = 'clang++',
    CPPFLAGS = [
        '-Wall',
        '-Wextra',
        '--std=c++11',
    ],

    # Allow clang++ to use color.
    ENV = {'TERM': os.environ['TERM']},
)

env.ParseConfig('llvm-config --cxxflags --ldflags --libs core')
env.ParseConfig('pkg-config --libs --cflags libedit icu-uc')

env.Program(
    target = 'fiddle',
    source = [
        'editline.cpp',
        'main.cpp',
        'parser.cpp',
    ],
)

debug = ARGUMENTS.get('debug', 0)
if int(debug):
   env.Append(CPPFLAGS = ['-g'])

# Print C++ flags for the YouCompleteMe vim plugin.
if ARGUMENTS.get('ycm', 0):
    print(env.subst('$CXXFLAGS $CCFLAGS $_CCCOMCOM'))
    exit()
