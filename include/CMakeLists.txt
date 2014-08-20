IF (NOT (PROJ_FOUND AND GEOS_FOUND AND SPARSEHASH_FOUND))
  MESSAGE(FATAL_ERROR "Proj+Geos+Sparsehash are needed to check all includes")
ENDIF()


FILE(GLOB_RECURSE ALL_HPPS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.hpp)

SET (GENFILE ${CMAKE_BINARY_DIR}/all_including.cpp)

FILE(WRITE ${GENFILE} "// genereated file with all includes")
FOREACH(hpp ${ALL_HPPS})
  # GDAL IS SKIPPED FOR NOW
  IF (NOT "${hpp}" MATCHES ".*ogr.*") 
    STRING(REGEX REPLACE "\\.hpp" "" hname ${hpp})
    STRING(REGEX REPLACE "/" "_" longpath ${hname})
    FILE(APPEND ${GENFILE} "#include <${hpp}>\n")
  ENDIF()
ENDFOREACH()

FILE(APPEND ${GENFILE} "int main() { return 0; }\n")
set_source_files_properties(${GENFILE} PROPERTIES GENERATED TRUE)
add_executable(allincude.exe ${GENFILE})
target_link_libraries (allincude.exe 
  ${EXPAT_LIBRARY} ${OSMPBF_LIBRARIES} 
  ${BZIP2_LIBRARIES} ${WIN_LIBS} ${CMAKE_THREAD_LIBS_INIT})

