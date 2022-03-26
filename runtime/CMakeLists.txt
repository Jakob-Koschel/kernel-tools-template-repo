cmake_minimum_required(VERSION 3.4.3)
project(kernel-tools-template-runtime)

set(CMAKE_CXX_STANDARD 11)

if(NOT DEFINED ENV{KERNEL})
  message(FATAL_ERROR "$KERNEL is not defined")
endif()

SET (CMAKE_C_FLAGS " -nostdinc -I$ENV{KERNEL}/arch/x86/include -I$ENV{KERNEL}/arch/x86/include/generated -I$ENV{KERNEL}/include -I$ENV{KERNEL}/arch/x86/include/uapi -I$ENV{KERNEL}/arch/x86/include/generated/uapi -I$ENV{KERNEL}/include/uapi -I$ENV{KERNEL}/include/generated/uapi -include $ENV{KERNEL}/include/linux/kconfig.h -include $ENV{KERNEL}/include/linux/compiler_types.h -D__KERNEL__")

include_directories($ENV{KERNEL}/arch/x86/include)
include_directories($ENV{KERNEL}/arch/x86/include/generated)
include_directories($ENV{KERNEL}/include)
include_directories($ENV{KERNEL}/arch/x86/include/uapi)
include_directories($ENV{KERNEL}/arch/x86/include/uapi/generated)
include_directories($ENV{KERNEL}/include/uapi)
include_directories($ENV{KERNEL}/include/generated/uapi)

set(SOURCES
  kernel_tools_template.c
)

include_directories(./include)

# enable_language(ASM)

add_library(kernel_tools_runtime STATIC
  ${SOURCES}
)
