cmake .. ^
-DOsmium_DEBUG=TRUE ^
-DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
-DBOOST_ROOT=%LODEPSDIR%\boost ^
-DBoost_PROGRAM_OPTIONS_LIBRARY=%LODEPSDIR%\boost\lib\libboost_program_options-vc140-mt-1_57.lib ^
-DOSMPBF_LIBRARY=%LODEPSDIR%\osmpbf\lib\osmpbf.lib ^
-DOSMPBF_INCLUDE_DIR=%LODEPSDIR%\osmpbf\include ^
-DPROTOBUF_LIBRARY=%LODEPSDIR%\protobuf\lib\libprotobuf.lib ^
-DPROTOBUF_LITE_LIBRARY=%LODEPSDIR%\protobuf\lib\libprotobuf-lite.lib ^
-DPROTOBUF_INCLUDE_DIR=%LODEPSDIR%\protobuf\include ^
-DZLIB_LIBRARY=%LODEPSDIR%\zlib\lib\zlibwapi.lib ^
-DZLIB_INCLUDE_DIR=%LODEPSDIR%\zlib\include ^
-DEXPAT_LIBRARY=%LODEPSDIR%\expat\lib\libexpat.lib ^
-DEXPAT_INCLUDE_DIR=%LODEPSDIR%\expat\include ^
-DBZIP2_LIBRARIES=%LIBBZIP2% ^
-DBZIP2_INCLUDE_DIR=%LODEPSDIR%\bzip2\include ^
-DGDAL_LIBRARY=%LODEPSDIR%\gdal\lib\gdal_i.lib ^
-DGDAL_INCLUDE_DIR=%LODEPSDIR%\gdal\include ^
-DGEOS_LIBRARY=%LODEPSDIR%\geos\lib\geos_i.lib ^
-DGEOS_INCLUDE_DIR=%LODEPSDIR%\geos\include ^
-DPROJ_LIBRARY=%LODEPSDIR%\proj\lib\proj.lib ^
-DPROJ_INCLUDE_DIR=%LODEPSDIR%\proj\include ^
-DSPARSEHASH_INCLUDE_DIR=%LODEPSDIR%\sparsehash\include ^
-DGETOPT_LIBRARY=%LODEPSDIR%\wingetopt\lib\wingetopt.lib ^
-DGETOPT_INCLUDE_DIR=%LODEPSDIR%\wingetopt\include

nmake
