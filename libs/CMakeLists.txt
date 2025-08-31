# Base Io build system
# Written by Jeremy Tregunna <jeremy.tregunna@me.com>
#
#
# CMake file for libs subdirectory.
#

# Add a definition -- explicitly required on Windows, but shouldn't
# hurt other platforms. If it does, let me know.
add_definitions("-DBUILDING_IOVMALL_DLL")

# Output our static library to the top-level _build hierarchy
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/_build/lib)

# This has changed, policy CMP0119
# Hackery for CMake's horrible ASM support

# Define the subdirectories we can reach from here that we want
# to go into and build stuff.
add_subdirectory(coroutine)
add_subdirectory(basekit)
add_subdirectory(garbagecollector)
add_subdirectory(iovm)
