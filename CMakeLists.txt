cmake_minimum_required(VERSION 2.8)

project(nemo)

include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckLibraryExists)

########### Add uninstall target ###############
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)
add_custom_target(uninstall
  "${CMAKE_COMMAND}" -P
  "${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_uninstall.cmake")

# -DBINDIR
option(BINDIR "Where to install the binaries" "/usr/local/bin")
if (BINDIR STREQUAL OFF)
  set(BINDIR "/usr/local/bin")
else(BINDIR STREQUAL OFF)
  set(BINDIR "${BINDIR}")
endif(BINDIR STREQUAL OFF)
# -DINCDIR
option(INCDIR "Where to install the header files" "/usr/local/include")
if (INCDIR STREQUAL OFF)
  set(INCDIR "/usr/local/include")
else(INCDIR STREQUAL OFF)
  set(INCDIR "${INCDIR}")
endif(INCDIR STREQUAL OFF)
# -DLIBDIR
option(LIBDIR "Where to install the libraries" "/usr/local/lib")
if (LIBDIR STREQUAL OFF)
  set(LIBDIR "/usr/local/lib")
else(LIBDIR STREQUAL OFF)
  set(LIBDIR "${LIBDIR}")
endif(LIBDIR STREQUAL OFF)

message("\nBuild config:\n")
message("   BINDIR: ${BINDIR}")
message("   INCDIR: ${INCDIR}")
message("   LIBDIR: ${LIBDIR}")

option(DEBUG "Enable debugging" OFF)
if (DEBUG MATCHES "1" OR DEBUG MATCHES "ON" OR DEBUG MATCHES "YES")
  set(DEBUG "1")
  message("   DEBUG:  ON\n")
else(DEBUG MATCHES "1" OR DEBUG MATCHES "ON" OR DEBUG MATCHES "YES")
  unset(DEBUG)
  message("   DEBUG:  OFF\n")
endif(DEBUG MATCHES "1" OR DEBUG MATCHES "ON" OR DEBUG MATCHES "YES")

CHECK_FUNCTION_EXISTS(strdup HAVE_STRDUP)
CHECK_INCLUDE_FILES(stdbool.h HAVE_STDBOOL_H)

set(libsrc
  parser.c
  lexer.c
)

set(binsrc
  nemo.c
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -ansi -pedantic -g")
configure_file("${PROJECT_SOURCE_DIR}/config.h.in" "${PROJECT_SOURCE_DIR}/config.h")

add_library(libnemo SHARED ${libsrc})
add_executable(nemo ${binsrc})
set_target_properties(libnemo
  PROPERTIES OUTPUT_NAME nemo
  SOVERSION 0 VERSION 0.4.0)
target_link_libraries(nemo libnemo)

# Linux needs the libdl to be linked into the program
# FreeBSD has those dl* functions implemented in libc
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  target_link_libraries(nemo dl)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

install(PROGRAMS nemo DESTINATION ${BINDIR})
install(TARGETS libnemo DESTINATION ${LIBDIR})

