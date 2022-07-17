# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "D:/STUDY/CG2020/Homeworks/2_ImageWarping/project/build/_deps/ucmake-src"
  "D:/STUDY/CG2020/Homeworks/2_ImageWarping/project/build/_deps/ucmake-build"
  "D:/STUDY/CG2020/Homeworks/2_ImageWarping/project/build/_deps/ucmake-subbuild/ucmake-populate-prefix"
  "D:/STUDY/CG2020/Homeworks/2_ImageWarping/project/build/_deps/ucmake-subbuild/ucmake-populate-prefix/tmp"
  "D:/STUDY/CG2020/Homeworks/2_ImageWarping/project/build/_deps/ucmake-subbuild/ucmake-populate-prefix/src/ucmake-populate-stamp"
  "D:/STUDY/CG2020/Homeworks/2_ImageWarping/project/build/_deps/ucmake-subbuild/ucmake-populate-prefix/src"
  "D:/STUDY/CG2020/Homeworks/2_ImageWarping/project/build/_deps/ucmake-subbuild/ucmake-populate-prefix/src/ucmake-populate-stamp"
)

set(configSubDirs Debug;Release;MinSizeRel;RelWithDebInfo)
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/STUDY/CG2020/Homeworks/2_ImageWarping/project/build/_deps/ucmake-subbuild/ucmake-populate-prefix/src/ucmake-populate-stamp/${subDir}")
endforeach()
