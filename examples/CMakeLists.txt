

IF(CMAKE_COMPILER_IS_GNUCC)
    SET(WARNING_FLAGS "-Wall -Wextra -Wdisabled-optimization -pedantic -Wctor-dtor-privacy -Wnon-virtual-dtor -Woverloaded-virtual -Wsign-promo -Wno-long-long")
    SET(CXX_FLAGS "-O3 -g")
    SET(CMAKE_CXX_FLAGS ${CXX_FLAGS} ${WARNINGS_FLAGS})
ENDIF(CMAKE_COMPILER_IS_GNUCC)

add_definitions( -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 )


if(SPARSEHASH_FOUND)
   include_directories(${SPARSEHASH_INCLUDE_DIR})
endif()

SET(BASIC_LIBS ${EXPAT_LIBRARY} ${OSMPBF_LIBRARIES} ${BZIP2_LIBRARIES} ${WIN_LIBS} ${CMAKE_THREAD_LIBS_INIT})

SET(SIMPLE_EXAMPLES
  osmium_read
  osmium_debug
  osmium_convert
  osmium_area_test
  osmium_serdump
)

FOREACH(example ${SIMPLE_EXAMPLES})
MESSAGE(${example})
  add_executable(${example} "${example}.cpp")
  target_link_libraries (${example} ${BASIC_LIBS})
ENDFOREACH(example)

UNSET(Boost_LIBRARIES)
UNSET(Boost_FOUND)
find_package(Boost 1.38 COMPONENTS program_options)

if(Boost_PROGRAM_OPTIONS_FOUND)
  SET(example
    osmium_index
  )
  MESSAGE(${example})
  add_executable(${example} "${example}.cpp")
  target_link_libraries (${example} ${Boost_PROGRAM_OPTIONS_LIBRARY} ${BASIC_LIBS})
else()
  MESSAGE("Can not build osmium_index example without Boost program_options")
endif()

SET(OGR_EXAMPLES
  osmium_toogr.cpp
  osmium_toogr2.cpp
)


if (GDAL_FOUND AND GEOS_FOUND AND SPARSEHASH_FOUND)
  include_directories(${GDAL_INCLUDE_DIR})
  include_directories(${GEOS_INCLUDE_DIR})

  FOREACH(example ${OGR_EXAMPLES})
  MESSAGE(${example})
    add_executable(${example} "${example}.cpp")
    target_link_libraries (${example} ${GDAL_LIBRARIES} ${GEOS_LIBRARY} ${BASIC_LIBS})
  ENDFOREACH(example)
else()
  MESSAGE("GDAL+GEOS+SparseHash not found, skipping OGR examples")
endif()

