#/*****************************************************************************
# *
# * $Id: CMakeLists.txt 589 2014-05-11 21:54:02Z kyle.shannon $
# *
# * Project:  Optimization Modeling of Fire Fighter Resources
# * Purpose:  CMake script
# * Author:   Kyle Shannon <kyle@pobox.com>
# *
# *****************************************************************************
# *
# * THIS SOFTWARE WAS DEVELOPED AT THE ROCKY MOUNTAIN RESEARCH STATION (RMRS)
# * MISSOULA FIRE SCIENCES LABORATORY BY EMPLOYEES OF THE FEDERAL GOVERNMENT
# * IN THE COURSE OF THEIR OFFICIAL DUTIES. PURSUANT TO TITLE 17 SECTION 105
# * OF THE UNITED STATES CODE, THIS SOFTWARE IS NOT SUBJECT TO COPYRIGHT
# * PROTECTION AND IS IN THE PUBLIC DOMAIN. RMRS MISSOULA FIRE SCIENCES
# * LABORATORY ASSUMES NO RESPONSIBILITY WHATSOEVER FOR ITS USE BY OTHER
# * PARTIES,  AND MAKES NO GUARANTEES, EXPRESSED OR IMPLIED, ABOUT ITS QUALITY,
# * RELIABILITY, OR ANY OTHER CHARACTERISTIC.
# *
# * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# * DEALINGS IN THE SOFTWARE.
# *
# ****************************************************************************/

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.8)

PROJECT(omffr)

ADD_DEFINITIONS(-DOMFFR)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

SET(CMAKE_INCLUDE_CURRENT_DIR ON)

# *****************************************************************************
# Set the version for the project
# *****************************************************************************
FILE(STRINGS ${CMAKE_SOURCE_DIR}/VERSION OMFFR_VERSION_STRING)
STRING(REPLACE "." ";" V2 ${OMFFR_VERSION_STRING})
LIST(LENGTH V2 V_SIZE)
IF(V_SIZE GREATER 2)
    LIST(GET V2 0 OMFFR_VERSION_MAJOR)
    LIST(GET V2 1 OMFFR_VERSION_MINOR)
    LIST(GET V2 2 OMFFR_VERSION_PATCH)
ENDIF(V_SIZE GREATER 2)

# *****************************************************************************
# # OSGeo4W support
# *****************************************************************************
IF(WIN32)
    OPTION(USE_OSGEO4W "Use dependencies (as many as possible) from OSGeo4W" OFF)
    IF(USE_OSGEO4W)
        FIND_PATH(OSGEO4W_HOME OSGeo4W CACHE PATH)
    ENDIF(USE_OSGEO4W)
ENDIF(WIN32)

# *****************************************************************************
# Enable or disable autotesting
# *****************************************************************************
OPTION(BUILD_TESTING "Enable test suite" ON)
IF(BUILD_TESTING)
    ENABLE_TESTING()
    ADD_DEFINITIONS(-DOMMFR_ENABLE_TESTING)
ENDIF(BUILD_TESTING)

# *****************************************************************************
# Artificial Neural Network library modules
# We use FANN for ANN simulation.  It is comprised of 3 separate libs for
# different precisions, fixed, float and double.
# *****************************************************************************

OPTION(BUILD_ANN "Build ANN module" OFF)

# *****************************************************************************
# Fire Ignition Generator (FIG).  This module builds the fig from raw simulated
# ignitions.  It currently has some window dependencies and can't be built on
# non-msvc builds.
# *****************************************************************************

OPTION(BUILD_FIG "Build FIG module" OFF)

# *****************************************************************************
# Build the Initial Response Simulator module.  This does the actual initial
# attack simulations using supplied resources.
# *****************************************************************************

OPTION(BUILD_IRS "Build IRS module" OFF)

# *****************************************************************************
# Custom executable for Diane, not for production
# *****************************************************************************

IF(BUILD_IRS)
    OPTION(DIANE_BUILD "Build Diane's executables" OFF)
    IF(DIANE_BUILD)
        ADD_DEFINITIONS(-DIRS_DIANE_BUILD)
    ENDIF(DIANE_BUILD)
    MARK_AS_ADVANCED(DIANE_BUILD)
ENDIF(BUILD_IRS)

# *****************************************************************************
# Allow the simulation to skip the contain algorithm, if we think it will be
# contained.
# *****************************************************************************

OPTION(ALLOW_CONTAIN_SKIP "Allow ommission of some contain simulations" ON)
IF(ALLOW_CONTAIN_SKIP)
    ADD_DEFINITIONS(-DIRS_ALLOW_SKIP_CONTAIN)
ENDIF(ALLOW_CONTAIN_SKIP)

# *****************************************************************************
# Thread support, to be added.  May be OpenMP, may be simple threading.
# *****************************************************************************

OPTION(IRS_ENABLE_THREADING "Enable thread support." OFF)
IF(IRS_ENABLE_THREADING)
    ADD_DEFINITIONS(-DIRS_ENABLE_THREADING)
    OPTION(IRS_LOAD_DATA_CONCURRENT "Use multithreaded data access" OFF)
    IF(IRS_LOAD_DATA_CONCURRENT)
        ADD_DEFINITIONS(-DIRS_LOAD_DATA_CONCURRENT)
    ELSE(IRS_LOAD_DATA_CONCURRENT)
        ADD_DEFINITIONS(-DIRS_SERIAL_DATA_ACCESS)
    ENDIF(IRS_LOAD_DATA_CONCURRENT)
    OPTION(IRS_RUN_SCEN_CONCURRENT "Use multithreaded data access" ON)
    IF(IRS_RUN_SCEN_CONCURRENT)
        ADD_DEFINITIONS(-DIRS_RUN_SCEN_CONCURRENT)
    ENDIF(IRS_RUN_SCEN_CONCURRENT)
ENDIF(IRS_ENABLE_THREADING)

# Deprecate when db is fixed.
OPTION(IRS_USE_DBID_AS_RESC_NAME "Use database pk for resource name(unique)" OFF)
IF(IRS_USE_DBID_AS_RESC_NAME)
    ADD_DEFINITIONS(-DIRS_USE_DBID_AS_RESC_NAME)
ENDIF(IRS_USE_DBID_AS_RESC_NAME)

# Unlink a sqlite3 db on overwrite
OPTION(IRS_RESULT_UNLINK_RESET "Delete a result db for overwrite" ON)
IF(IRS_RESULT_UNLINK_RESET)
    ADD_DEFINITIONS(-DIRS_RESULT_UNLINK_RESET)
ENDIF(IRS_RESULT_UNLINK_RESET)

# Use spatial query for fpu data instead of crawling through fwa bad data.
OPTION(IRS_RESULT_USE_SPATIAL "Use spatial queries for fire results" OFF)
IF(IRS_RESULT_USE_SPATIAL)
    ADD_DEFINITIONS(-DIRS_RESULT_USE_SPATIAL)
ENDIF(IRS_RESULT_USE_SPATIAL)

# Be careful about destroying objects and re-creating
OPTION(IRS_BE_OVERLY_SAFE "Destroy and recreate objects if their state is questionable" OFF)
IF(IRS_BE_OVERLY_SAFE)
    ADD_DEFINITIONS(-DIRS_BE_OVERLY_SAFE)
ENDIF(IRS_BE_OVERLY_SAFE)

# Use Diane's old text files (deprecated).
OPTION(IRS_USE_DIANE_FILES "Use text files for some inputs")
IF(IRS_USE_DIANE_FILES)
    ADD_DEFINITIONS(-DIRS_USE_DIANE_FILES)
ENDIF(IRS_USE_DIANE_FILES)

OPTION(IRS_RESC_REDUCE_SQL "Use sql to exclude resources instead of arrays" OFF)
IF(IRS_RESC_REDUCE_SQL)
    ADD_DEFINITIONS(-DIRS_RESC_REDUCE_SQL)
ENDIF(IRS_RESC_REDUCE_SQL)

# *****************************************************************************
# Put common libs here
# *****************************************************************************
INCLUDE(FindGDAL)
IF(WIN32)
    IF(NOT OSGEO4W_HOME)
        FIND_PACKAGE(GDAL REQUIRED)
    ELSE(NOT OSGEO4W_HOME)
        FIND_PACKAGE(GDAL)
    ENDIF(NOT OSGEO4W_HOME)
ELSE(WIN32)
    FIND_PACKAGE(GDAL REQUIRED)
ENDIF(WIN32)

IF(WIN32)
    IF(NOT GDAL_INCLUDE_DIR-NOTFOUND AND OSGEO4W_HOME)
        SET(GDAL_INCLUDE_DIR ${OSGEO4W_HOME}/include CACHE TYPE PATH FORCE)
        SET(GDAL_LIBRARY ${OSGEO4W_HOME}/lib/gdal_i.lib CACHE TYPE NAME FORCE)
        SET(GDAL_INCLUDE_DIR-FOUND TRUE)
        SET(GDAL_LIBRARY-FOUND TRUE)
    ENDIF(NOT GDAL_INCLUDE_DIR-NOTFOUND AND OSGEO4W_HOME)
ENDIF(WIN32)

#
# Handle sqlite.
#
OPTION(USE_INTERNAL_SQLITE "Use the internal SQLite3 implementation" OFF)
IF(USE_INTERNAL_SQLITE)
    ADD_DEFINITIONS(-DSQLITE_ENABLE_LOAD_EXTENSION=1)
    ADD_DEFINITIONS(-DSQLITE_ENABLE_RTREE=1)
    ADD_SUBDIRECTORY(sqlite)
ELSE(USE_INTERNAL_SQLITE)
    INCLUDE(FindSqlite3)
    IF(WIN32)
        IF(NOT SQLITE3_INCLUDE_DIR-NOTFOUND AND OSGEO4W_HOME)
            SET(SQLITE3_INCLUDE_DIR ${OSGEO4W_HOME}/include CACHE TYPE PATH FORCE)
            SET(SQLITE3_LIBRARY ${OSGEO4W_HOME}/lib/sqlite3_i.lib CACHE TYPE NAME FORCE)
            SET(SQLITE3_INCLUDE_DIR-FOUND TRUE)
            SET(SQLITE3_LIBRARY-FOUND TRUE)
        ELSE(NOT SQLITE3_INCLUDE_DIR-NOTFOUND AND OSGEO4W_HOME)
            FIND_PACKAGE(Sqlite3 REQUIRED)
        ENDIF(NOT SQLITE3_INCLUDE_DIR-NOTFOUND AND OSGEO4W_HOME)
    ELSE(WIN32)
        FIND_PACKAGE(Sqlite3 REQUIRED)
    ENDIF(WIN32)
ENDIF(USE_INTERNAL_SQLITE)
SET(SQLITE3_LIBRARY_DEBUG ${SQLITE3_LIBRARY} CACHE TYPE NAME FORCE)
SET(SQLITE3_LIBRARY_RELEASE ${SQLITE3_LIBRARY} CACHE TYPE NAME FORCE)

INCLUDE(FindSPATIALITE)
IF(WIN32)
    IF(NOT SPATIALITE_INCLUDE_DIR-NOTFOUND AND OSGEO4W_HOME)
        SET(SPATIALITE_INCLUDE_DIR ${OSGEO4W_HOME}/include CACHE TYPE PATH FORCE)
        SET(SPATIALITE_LIBRARY ${OSGEO4W_HOME}/lib/spatialite.lib CACHE TYPE NAME FORCE)
        SET(SPATIALITE_INCLUDE_DIR-FOUND TRUE)
        SET(SPATIALITE_LIBRARY-FOUND TRUE)
    ELSE(NOT SPATIALITE_INCLUDE_DIR-NOTFOUND AND OSGEO4W_HOME)
        FIND_PACKAGE(SPATIALITE REQUIRED)
    ENDIF(NOT SPATIALITE_INCLUDE_DIR-NOTFOUND AND OSGEO4W_HOME)
ELSE(WIN32)
    FIND_PACKAGE(SPATIALITE REQUIRED)
ENDIF(WIN32)

FIND_PATH(OMFFR_DATA_PATH CACHE PATH)
IF(OMFFR_DATA_PATH)
    ADD_DEFINITIONS(-DOMFFR_DATA="${OMFFR_DATA_PATH}")
ENDIF(OMFFR_DATA_PATH)

# *****************************************************************************
# Recurse through subdirectories
# *****************************************************************************

ADD_SUBDIRECTORY(common)

IF(BUILD_ANN)
    ADD_SUBDIRECTORY(ann)
ENDIF(BUILD_ANN)

IF(BUILD_FIG)
    ADD_DEFINITIONS(-DFLMP_ENABLE_OMFFR)
    ADD_SUBDIRECTORY(nfdrs)
    ADD_SUBDIRECTORY(flammap)
    ADD_SUBDIRECTORY(fig)
ENDIF(BUILD_FIG)

IF(BUILD_IRS)
    ADD_SUBDIRECTORY(irs)
ENDIF(BUILD_IRS)

IF(BUILD_TESTING)
    ADD_SUBDIRECTORY(autotest)
    INCLUDE(CTest)
    ADD_CUSTOM_TARGET(check COMMAND ${CMAKE_CTEST_COMMAND})
ENDIF(BUILD_TESTING)

# *****************************************************************************
# CPack
# *****************************************************************************
#INCLUDE(InstallRequiredSystemLibraries)
# *****************************************************************************
# CPack Debian
# *****************************************************************************
#TODO

# *****************************************************************************
# CPack variables
# *****************************************************************************

SET(CPACK_NSIS_PACKAGE_NAME OMFFR-${OMFFR_VERSION_STRING})
SET(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE)
# version
SET(CPACK_VERSION_MAJOR ${OMFFR_VERSION_MAJOR})
SET(CPACK_VERSION_MINOR ${OMFFR_VERSION_MINOR})
SET(CPACK_VERSION_PATCH ${OMFFR_VERSION_PATCH})
SET(CPACK_PACKAGE_VERSION ${OMFFR_VERSION_STRING})

SET(CPACK_NSIS_INSTALL_ROOT "C:")
CMAKE_POLICY(PUSH)
CMAKE_POLICY(SET CMP0010 OLD)
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "OMFFR\\\\${OMFFR_VERSION_STRING}")
SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "OMFFR\\\\${OMFFR_VERSION_STRING}")
CMAKE_POLICY(POP)
# *****************************************************************************
# Add/Remove Programs support
# *****************************************************************************
#SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\WindNinja.exe")
#SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\windninja.firemodels.org")
# *****************************************************************************
# NSIS Specific stuff
# *****************************************************************************
SET(CPACK_NSIS_DISPLAY_NAME "OMFFR\\\\${OMFFR_VERSION_STRING}")
#TODO
#SET(CPACK_NSIS_INSTALLED_ICON_NAME "${CMAKE_SOURCE_DIR}/images/icons/wn-desktop.ico")

SET(CPACK_CREATE_DESKTOP_LINKS OMFFR)
SET(CPACK_NSIS_MODIFY_PATH OMFFR)

SET(CPACK_PACKAGE_EXECUTABLES "omffr_gui" ${OMFFR_VERSION_STRING})
#SET(CPACK_NSIS_CREATE_ICONS_EXTRA "
#CreateDirectory \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Tutorials\\\"
#CreateDirectory \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Documents\\\"
#CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Tutorials\\\\Tutorial 1.lnk\\\"  \\\"$INSTDIR\\\\share\\\\windninja\\\\doc\\\\tutorials\\\\WindNinja_Tutorial1.pdf\\\"
#CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Tutorials\\\\Tutorial 2.lnk\\\"  \\\"$INSTDIR\\\\share\\\\windninja\\\\doc\\\\tutorials\\\\WindNinja_Tutorial2.pdf\\\"
#CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Tutorials\\\\Tutorial 3.lnk\\\"  \\\"$INSTDIR\\\\share\\\\windninja\\\\doc\\\\tutorials\\\\WindNinja_Tutorial3.pdf\\\"
#CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Tutorials\\\\Tutorial 4.lnk\\\"  \\\"$INSTDIR\\\\share\\\\windninja\\\\doc\\\\tutorials\\\\WindNinja_Tutorial4.pdf\\\" 
#CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Documents\\\\CLI Instructions.lnk\\\"  \\\"$INSTDIR\\\\share\\\\windninja\\\\doc\\\\CLI_instructions.pdf\\\"
#CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Documents\\\\ArcMap Instructions.lnk\\\"  \\\"$INSTDIR\\\\share\\\\windninja\\\\doc\\\\Displaying_wind_vectors_in_ArcMap.pdf\\\"
#CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Documents\\\\ArcView Instructions.lnk\\\"  \\\"$INSTDIR\\\\share\\\\windninja\\\\doc\\\\Displaying_wind_vectors_in_ArcView.pdf\\\"
#CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Documents\\\\DEM Download Instructions.lnk\\\" \\\"$INSTDIR\\\\share\\\\windninja\\\\doc\\\\download_elevation_file.pdf\\\"
#CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Example Files.lnk\\\" \\\"$INSTDIR\\\\etc\\\\windninja\\\\example-files\\\" ")

#SET(CPACK_NSIS_DELETE_ICONS_EXTRA "
#Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Tutorials\\\\Tutorial 1.lnk\\\"
#Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Tutorials\\\\Tutorial 2.lnk\\\"
#Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Tutorials\\\\Tutorial 3.lnk\\\"
#Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Tutorials\\\\Tutorial 4.lnk\\\"
#Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Documents\\\\CLI Instructions.lnk\\\"
#Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Documents\\\\ArcMap Instructions.lnk\\\"
#Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Documents\\\\ArcView Instructions.lnk\\\"
#Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Documents\\\\DEM Download Instructions.lnk\\\"
#Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Example Files.lnk\\\"
#RMDir \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Tutorials\\\"
#RMDir \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Documents\\\" ")

# *****************************************************************************
# Check the windows architecture for 64 or 32
# *****************************************************************************
IF(CMAKE_SIZEOF_VOID_P MATCHES "8")
    SET(BUILD_64 TRUE)
    IF(PACKAGE_DEBUG)
        MESSAGE("Building 64-bit installer")
    ENDIF(PACKAGE_DEBUG)
ELSEIF(CMAKE_SIZEOF_VOID_P MATCHES "4")
    SET(BUILD_64 FALSE)
    IF(PACKAGE_DEBUG)
        MESSAGE("Building 32-bit installer")
    ENDIF(PACKAGE_DEBUG)
ELSE(CMAKE_SIZEOF_VOID_P MATCHES "8")
    SET(BUILD_64 FALSE)
    IF(PACKAGE_DEBUG)
        MESSAGE("Cannot determine architecture, using 32-bit by default")
    ENDIF(PACKAGE_DEBUG)
ENDIF(CMAKE_SIZEOF_VOID_P MATCHES "8")

# *****************************************************************************
# Build the nsis installer on the firelab machine(s).  Not for external use.
# *****************************************************************************
#IF(FIRELAB_PACKAGE AND WIN32 AND MSVC)
#    IF(BUILD_64)
#        SET(CPACK_PACKAGE_FILE_NAME "${OMFFR_VERSION_STRING}-win64-install")
#    ELSE(BUILD_64)
#        SET(CPACK_PACKAGE_FILE_NAME "${OMFFR_VERSION_STRING}-win32-install")
#    ENDIF(BUILD_64)
#
#    INSTALL(PROGRAMS "c:/src/gdal/gdal-1.10.1/bin/gdal110.dll" DESTINATION bin COMPONENT apps)
#    INSTALL(PROGRAMS "c:/src/netcdf/netcdf-4.1.1/win32/NET/Release/netcdf.dll" DESTINATION bin COMPONENT apps)
#    INSTALL(PROGRAMS "c:/src/hdf5/hdf5-1.8.6/hdf5lib/release/dll/hdf5dll.dll" DESTINATION bin COMPONENT apps)
#    INSTALL(PROGRAMS "c:/src/hdf5/hdf5-1.8.6/hdf5lib/release/dll/hdf5_hldll.dll" DESTINATION bin COMPONENT apps)
#    INSTALL(PROGRAMS "c:/src/geos/geos-3.3.5/bin/geos_c.dll" DESTINATION bin COMPONENT apps)
#    IF(BUILD_TYPE MATCHES "release")
#        MESSAGE("IN RELEASE PACKAGE MODE")
#        INSTALL(PROGRAMS "c:/src/libcurl/curl-7.21.4/lib/dll-release/libcurl.dll" DESTINATION bin COMPONENT apps)
#        INSTALL(PROGRAMS "c:/src/qt/qt-everywhere-opensource-src-4.8.4/bin/qtgui4.dll" DESTINATION bin COMPONENT apps)
#        INSTALL(PROGRAMS "c:/src/qt/qt-everywhere-opensource-src-4.8.4/bin/qtcore4.dll" DESTINATION bin COMPONENT apps)
#        INSTALL(PROGRAMS "c:/src/qt/qt-everywhere-opensource-src-4.8.4/bin/qtnetwork4.dll" DESTINATION bin COMPONENT apps)
#        INSTALL(PROGRAMS "c:/src/qt/qt-everywhere-opensource-src-4.8.4/bin/qtwebkit4.dll" DESTINATION bin COMPONENT apps)
#        INSTALL(PROGRAMS "c:/src/qt/qt-everywhere-opensource-src-4.8.4/bin/phonon4.dll" DESTINATION bin COMPONENT apps)
#        INSTALL(PROGRAMS "c:/src/qt/qt-everywhere-opensource-src-4.8.4/bin/qtnetwork4.dll" DESTINATION bin COMPONENT apps)
#        INSTALL(PROGRAMS "c:/src/qt/qt-everywhere-opensource-src-4.8.4/plugins/imageformats/qjpeg4.dll" DESTINATION bin/imageformats)
#    ELSE(BUILD_TYPE MATCHES "release")
#        INSTALL(PROGRAMS "c:/src/libcurl/curl-7.21.4/lib/dll-debug/libcurld.dll" DESTINATION bin COMPONENT apps)
#        INSTALL(PROGRAMS "c:/src/qt/qt-everywhere-opensource-src-4.8.4/bin/qtguid4.dll" DESTINATION bin COMPONENT apps)
#        INSTALL(PROGRAMS "c:/src/qt/qt-everywhere-opensource-src-4.8.4/bin/qtcored4.dll" DESTINATION bin COMPONENT apps)
#        INSTALL(PROGRAMS "c:/src/qt/qt-everywhere-opensource-src-4.8.4/bin/qtwebkitd4.dll" DESTINATION bin COMPONENT apps)
#        INSTALL(PROGRAMS "c:/src/qt/qt-everywhere-opensource-src-4.8.4/bin/phonond4.dll" DESTINATION bin COMPONENT apps)
#        INSTALL(PROGRAMS "c:/src/qt/qt-everywhere-opensource-src-4.8.4/bin/qtnetworkd4.dll" DESTINATION bin COMPONENT apps)
#        INSTALL(PROGRAMS "c:/src/qt/qt-everywhere-opensource-src-4.8.4/plugins/imageformats/qjpegd4.dll" DESTINATION bin/imageformats)
#    ENDIF(BUILD_TYPE MATCHES "release")
# HACK FOR OPENMP
#    IF(CMAKE_SIZEOF_VOID_P MATCHES "4")
#        INSTALL(PROGRAMS ${MSVC10_REDIST_DIR}/x86/Microsoft.VC100.OPENMP/vcomp100.dll DESTINATION bin COMPONENT apps)
#    ELSEIF(CMAKE_SIZEOF_VOID_P MATCHES "8")
#        INSTALL(PROGRAMS ${MSVC10_REDIST_DIR}/x64/Microsoft.VC100.OPENMP/vcomp100.dll DESTINATION bin COMPONENT apps)
#    ENDIF(CMAKE_SIZEOF_VOID_P MATCHES "4")
#ENDIF(FIRELAB_PACKAGE AND WIN32 AND MSVC)
#SET(CPACK_COMPONENT_APPS_REQUIRED TRUE)
#SET(CPACK_COMPONENT_LIBS_HIDDEN TRUE)
#SET(CPACK_COMPONENT_LIBS_DISABLED TRUE)
#SET(CPACK_COMPONENT_INCLUDES_HIDDEN TRUE)
#SET(CPACK_COMPONENT_INCLUDES_DISABLED TRUE)

# *****************************************************************************
# CPack call
# *****************************************************************************
#INCLUDE(CPack)

# *****************************************************************************
# CPack groupings
# *****************************************************************************
#CPACK_ADD_COMPONENT(apps 
#                    DISPLAY_NAME "WindNinja"
#                    DESCRIPTION "WindNinja binaries and documentation"
#                    GROUP Application
#                    )
#CPACK_ADD_COMPONENT(libs
#                    DISPLAY_NAME "WindNinja Libraries"
#                    DESCRIPTION "Development libraries"
#                    #GROUP Development
#                    )
#CPACK_ADD_COMPONENT(includes
#                    DISPLAY_NAME "WindNinja Headers"
#                    DESCRIPTION "Development include files"
#                    #GROUP Development
#                    )

