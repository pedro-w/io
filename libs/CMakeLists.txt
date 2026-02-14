# Base Io build system
# Written by Jeremy Tregunna <jeremy.tregunna@me.com>
#
#
# CMake file for libs subdirectory.
#

# Add a definition -- explicitly required on Windows, but shouldn't
# hurt other platforms. If it does, let me know.
add_definitions("-DBUILDING_IOVMALL_DLL")

# Define the subdirectories we can reach from here that we want
# to go into and build stuff.
add_subdirectory(coroutine)
add_subdirectory(basekit)
add_subdirectory(garbagecollector)
add_subdirectory(iovm)
