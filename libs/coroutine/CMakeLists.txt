# Base Io build system
# Written by Jeremy Tregunna <jeremy.tregunna@me.com>
#
# Build the coroutine library.

# Definitions, love them.
add_definitions("-DBUILDING_CORO_DLL")

# Set up Coroutine include dirs, -I args to compiler.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../basekit/source)

# Sources... in all their wondrous glory!
set(SRCS Coro.c)
if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  list(APPEND SRCS "Coro-fibers.c")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
  list(APPEND SRCS "Coro-arm64.c")
else()
  list(APPEND SRCS "Coro-ucontext.c")
endif()

list(TRANSFORM SRCS PREPEND "source/")
set(HEADERS "source/Coro.h")

# Now build the shared library
add_library(coroutine SHARED ${SRCS})
set_target_properties(coroutine PROPERTIES PUBLIC_HEADER "${HEADERS}")

# Now build the static library
add_library(coroutine_static STATIC ${SRCS})
set_target_properties(coroutine PROPERTIES PUBLIC_HEADER "${HEADERS}")

# The following add the install target, so we put libcoroutine.* in
# our install prefix.
install (
  TARGETS coroutine
  FILE_SET HEADERS
)
