import os

env = Environment(
    CXX = 'clang++',
    CPPFLAGS = [
        '-Wall',
        '-Wextra',
        '--std=c++11',
    ],

    # Allow clang++ to use color
    ENV = {'TERM': os.environ['TERM']},
)

env.ParseConfig('llvm-config --cxxflags --ldflags --libs core')
env.ParseConfig('pkg-config --libs --cflags libedit')

env.Program(
    target = 'fiddle',
    source = [
        'main.cpp',
        'editline.cpp',
    ],
)

# Print C++ flags for the YouCompleteMe vim plugin.
if ARGUMENTS.get('ycm', 0):
    print(env.subst('$CXXFLAGS $CCFLAGS $_CCCOMCOM'))
    exit()
