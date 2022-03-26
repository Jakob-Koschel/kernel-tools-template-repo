cmake_minimum_required(VERSION 3.4.3)

if(NOT DEFINED ENV{LLVMPREFIX})
  message(FATAL_ERROR "$LLVMPREFIX is not defined")
else ()
  set(ENV{LLVM_DIR} $ENV{LLVMPREFIX}/lib/cmake/llvm)
endif()

SET (CMAKE_C_COMPILER             $ENV{LLVMPREFIX}/bin/clang)
SET (CMAKE_CXX_COMPILER           $ENV{LLVMPREFIX}/bin/clang++)

project(kernel-tools-template-repo)

set(CMAKE_CXX_STANDARD 11)

add_subdirectory(passes)
add_subdirectory(runtime)
