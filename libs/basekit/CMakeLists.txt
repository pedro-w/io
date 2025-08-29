# Base Io build system
# Written by Jeremy Tregunna <jeremy.tregunna@me.com>
#
# Build the garbage collector library.

# Output our dynamic library to the top-level _build hierarchy
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/_build/dll)

# Marvelous flags, likely compiler dependent.
add_definitions("-DBUILDING_BASEKIT_DLL")

# Include dirs, -I flags and whatnot
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/source/simd_cph/include)

set(BASEKIT_LIBS ${CMAKE_DL_LIBS})

# Add math library for those who need it
find_library(MATH_LIBRARY m)
if(MATH_LIBRARY)
	list(APPEND BASEKIT_LIBS ${MATH_LIBRARY})
endif()

# Our library sources.
file(GLOB SRCS "source/*.c")
file(GLOB HEADERS "source/*.h")

# Now build the shared library
add_library(basekit SHARED ${SRCS})
set_target_properties(basekit PROPERTIES PUBLIC_HEADER "${HEADERS}")
target_link_libraries(basekit ${BASEKIT_LIBS})

# ...And the static library
#add_library(basekit_static STATIC ${SRCS})

# The following add the install target, so we put libbasekit.* in our
# install prefix.
if(WIN32)
    install(TARGETS basekit 
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}
        PUBLIC_HEADER DESTINATION include/io
        )
else()
    install(TARGETS basekit 
        LIBRARY DESTINATION lib
        PUBLIC_HEADER DESTINATION include/io
        )
endif(WIN32)
