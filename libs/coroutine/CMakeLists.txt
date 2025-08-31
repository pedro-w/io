# Base Io build system
# Written by Jeremy Tregunna <jeremy.tregunna@me.com>
#
# Build the coroutine library.

# Output our dynamic library to the top-level _build hierarchy
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/_build/dll)

# Definitions, love them.
add_definitions("-DBUILDING_CORO_DLL")

# Check if we're on FreeBSD, if so, use ucontext.
if(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    add_definitions("-DUSE_UCONTEXT")
endif(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")

# Set up Coroutine include dirs, -I args to compiler.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../basekit/source)

# Sources... in all their wonderous glory!
file(GLOB SRCS "source/*.c")
file(GLOB HEADERS "source/*.h")

# This has changed, policy CMP0119
list(APPEND SRCS source/asm.S)


# Now build the shared library
add_library(coroutine SHARED ${SRCS})
set_target_properties(coroutine PROPERTIES PUBLIC_HEADER "${HEADERS}")

# The following add the install target, so we put libcoroutine.* in
# our install prefix.
if(WIN32)
    install(TARGETS coroutine
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}
        PUBLIC_HEADER DESTINATION include/io
        )
else()
    install(TARGETS coroutine
        LIBRARY DESTINATION lib
        PUBLIC_HEADER DESTINATION include/io
        )
endif(WIN32)
