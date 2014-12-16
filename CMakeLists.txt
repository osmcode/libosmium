#----------------------------------------------------------------------
#
#  Libosmium CMakeLists.txt
#
#----------------------------------------------------------------------

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")


#----------------------------------------------------------------------
#
#  Project version
#
#----------------------------------------------------------------------

project(libosmium)

set(LIBOSMIUM_VERSION_MAJOR 0)
set(LIBOSMIUM_VERSION_MINOR 0)
set(LIBOSMIUM_VERSION_PATCH 1)

set(LIBOSMIUM_VERSION ${OSMIUM_VERSION_MAJOR}.${OSMIUM_VERSION_MINOR}.${OSMIUM_VERSION_PATCH})


#----------------------------------------------------------------------
#
#  Build options
#
#  (Change with -DOPTION=VALUE on cmake command line.)
#
#----------------------------------------------------------------------

option(BUILD_EXAMPLES "compile example programs" ON)
option(BUILD_TESTING "compile tests, please run them with ctest" ON)
option(BUILD_TRY_HPPS "compile every hpp" ON)


#----------------------------------------------------------------------
#
#  Find external dependencies
#
#----------------------------------------------------------------------

# check that the essential libraries were found
if(BUILD_EXAMPLES OR BUILD_TESTING OR BUILD_TRY_HPPS)


    find_package(Boost 1.38)
    mark_as_advanced(CLEAR BOOST_ROOT)

    if(Boost_FOUND)
        include_directories(${Boost_INCLUDE_DIRS})
    else()
        set(BOOST_ROOT "NOT FOUND: please choose" CACHE PATH "")
        message(FATAL_ERROR "PLEASE, specify the directory where the Boost library is installed in BOOST_ROOT")
    endif()

    set(OSMIUM_INCLUDE_DIR include)
    find_package(Osmium COMPONENTS io gdal geos proj sparsehash)
    include_directories(${OSMIUM_INCLUDE_DIRS})

    if(MSVC)
        find_path(GETOPT_INCLUDE_DIR getopt.h)
        find_library(GETOPT_LIBRARY NAMES wingetopt)
        if(GETOPT_INCLUDE_DIR AND GETOPT_LIBRARY)
            include_directories(${GETOPT_INCLUDE_DIR})
            list(APPEND OSMIUM_LIBRARIES ${GETOPT_LIBRARY})
        else()
            set(GETOPT_MISSING 1)
        endif()
    endif()

    include_directories(include)
    include(OsmiumOptions)
endif()

#----------------------------------------------------------------------
#
#  Decide which C++ version to use (Minimum/default: C++11).
#
#----------------------------------------------------------------------

if(NOT USE_CPP_VERSION)
    set(USE_CPP_VERSION c++11)
endif()
message(STATUS "Use C++ version: ${USE_CPP_VERSION}")
add_compile_options(-std=${USE_CPP_VERSION})

#----------------------------------------------------------------------

if(WIN32)
    add_definitions(-DWIN32 -D_WIN32 -DMSWIN32 -DBGDWIN32 -DWINVER=0x0500 -D_WIN32_WINNT=0x0500 -D_WIN32_IE=0x0600)
    set(CPACK_GENERATOR ZIP)
else()
    set(CPACK_GENERATOR TGZ)
endif()

enable_testing()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BUILD_TESTING)
    add_subdirectory(test)
    add_subdirectory(test/osm-testdata)
endif()

if(BUILD_TRY_HPPS)
    add_subdirectory(include)
endif()

install(DIRECTORY include/osmium DESTINATION include)
install(FILES include/boost_unicode_iterator.hpp DESTINATION include)

include(CPack)

