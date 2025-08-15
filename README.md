# llvm-pass-skeleton

## LLVM MODULE ANALYSIS

Build:

    $ cd llvm-pass-skeleton
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make
    $ cd ..

Run:

    $ clang -fpass-plugin=`echo build/skeleton/SkeletonPass.*` something.c
