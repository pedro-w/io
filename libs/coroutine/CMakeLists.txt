# Base Io build system
# Written by Jeremy Tregunna <jeremy.tregunna@me.com>
#
# Build the coroutine library.

# Definitions, love them.
add_definitions("-DBUILDING_CORO_DLL")

# Check if we're on FreeBSD, if so, use ucontext.
if(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    add_definitions("-DUSE_UCONTEXT")
endif(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")

# Set up Coroutine include dirs, -I args to compiler.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../basekit/source)

# Sources... in all their wondrous glory!
set(SRCS Coro.c Coro-ucontext.c)
list(TRANSFORM SRCS PREPEND "source/")
file(GLOB HEADERS "source/*.h")

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
