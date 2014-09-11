set TESTDATA_DIR=%1

del multipolygon.db multipolygon-tests.json
%2\testdata-multipolygon %TESTDATA_DIR%\grid\data\all.osm 2>&1 >multipolygon.log
if ERRORLEVEL 1 (exit /b 1)
%TESTDATA_DIR%\bin\compare-areas.rb %TESTDATA_DIR%\grid\data\tests.json multipolygon-tests.json
