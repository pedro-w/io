# Base Io build system
# Written by Jeremy Tregunna <jeremy.tregunna@me.com>
#
#
# CMake file for libs subdirectory.
#

# Define the subdirectories we can reach from here that we want
# to go into and build stuff.
add_subdirectory(coroutine)
add_subdirectory(basekit)
add_subdirectory(garbagecollector)
add_subdirectory(iovm)
