call "%VS120COMNTOOLS%\..\..\VC\vcvarsall.bat" x86_amd64
set CMAKE_PREFIX_PATH=C:\PROJ
set VERSION=Debug
set TESTS=ON
set ALLHPPS=ON
set PREFIX=d:\libs18d
set BOOST_ROOT=d:\boost

cmake .. -G "Visual Studio 12 Win64" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=%PREFIX% -DBOOST_ROOT=%BOOST_ROOT% -DBoost_USE_STATIC_LIBS=ON -DBUILD_TESTING=%TESTS% -DBUILD_TRY_HPPS=%ALLHPPS$ -T CTP_Nov2013
msbuild /clp:Verbosity=minimal /nologo OSMIUM.sln /flp1:logfile=build_errors.txt;errorsonly /flp2:logfile=build_warnings.txt;warningsonly
set PATH=%PATH%;%PREFIX%/bin
if "%TESTS%"=="ON" ctest -VV >build_tests.log
