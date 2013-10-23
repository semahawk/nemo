cmake_minimum_required(VERSION 2.8)

project(nemo)

include(CheckIncludeFiles)
include(CheckLibraryExists)

option(LIBDIR "Where to install the libraries" "/usr/local/lib")
if (LIBDIR STREQUAL OFF)
  set(LIBDIR "/usr/local/lib")
else(LIBDIR STREQUAL OFF)
  set(LIBDIR "${LIBDIR}")
endif(LIBDIR STREQUAL OFF)

option(DEBUG "Enable debugging" OFF)
set(DEBUG "${DEBUG}")

CHECK_INCLUDE_FILES(stdbool.h HAVE_STDBOOL_H)

CHECK_LIBRARY_EXISTS(readline readline "" HAVE_LIB_READLINE)

set(src
    array.c
    ast.c
    debug.c
    error.c
    file.c
    float.c
    gc.c
    int.c
    lexer.c
    mem.c
    namespace.c
    nemo.c
    null.c
    object.c
    parser.c
    predef.c
    string.c
    vars.c
  )

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -std=c99")
configure_file("${PROJECT_SOURCE_DIR}/config.h.in" "${PROJECT_SOURCE_DIR}/config.h")

add_library(libnemo SHARED ${src})
add_executable(nemo ${src})
set_target_properties(libnemo PROPERTIES OUTPUT_NAME nemo)
target_link_libraries(nemo libnemo)

add_subdirectory(lib)

target_link_libraries(nemo readline)

# Linux needs the libdl to be linked into the program
# FreeBSD has those dl* functions implemented in libc
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  target_link_libraries(nemo dl)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
