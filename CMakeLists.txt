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

# Stupid const string errrors in contain
if(NOT MSVC)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-writable-strings")
endif(NOT MSVC)

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
# Set up some stuff to build a windows installer
# *****************************************************************************
OPTION(WFIPS_TEST_INSTALLER "Build an installer based on known OSGeo4W locations" OFF )
IF(WFIPS_TEST_INSTALLER)
    IF(NOT OSGEO4W_HOME)
        MESSAGE("Must have OSGeo4W installed")
        SET(WFIPS_TEST_INSTALLER FALSE)
    ELSE(NOT OSGEO4W_HOME)
        SET(QGIS_PLUGIN_DIR "C:\\wfips\\bin\\plugins")
    ENDIF(NOT OSGEO4W_HOME)
ENDIF(WFIPS_TEST_INSTALLER)


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

ADD_SUBDIRECTORY(doc)

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
INCLUDE(InstallRequiredSystemLibraries)

SET(CPACK_NSIS_PACKAGE_NAME OMFFR-${OMFFR_VERSION_STRING})
SET(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE)
# version
SET(CPACK_VERSION_MAJOR ${OMFFR_VERSION_MAJOR})
SET(CPACK_VERSION_MINOR ${OMFFR_VERSION_MINOR})
SET(CPACK_VERSION_PATCH ${OMFFR_VERSION_PATCH})
SET(CPACK_PACKAGE_VERSION ${OMFFR_VERSION_STRING})

SET(CPACK_NSIS_INSTALL_ROOT "C:")
SET(CPACK_NSIS_DISPLAY_NAME "wfips")

SET(CPACK_CREATE_DESKTOP_LINKS OMFFR)

SET(CPACK_PACKAGE_EXECUTABLES "wfips" "wfips")

# *****************************************************************************
# Check the windows architecture for 64 or 32
# *****************************************************************************
IF(NOT CMAKE_SIZEOF_VOID_P MATCHES "8")
    MESSAGE("Must build in 64 bit for installer...")
ENDIF(NOT CMAKE_SIZEOF_VOID_P MATCHES "8")

# *****************************************************************************
# Build the nsis installer on the firelab machine(s).  Not for external use.
# *****************************************************************************
FILE(GLOB "${OSGEO4W_HOME}/bin/*.dll}" DLLS)
FOREACH(DLL ${DLLS})
    INSTALL(PROGRAMS ${DLL} DESTINATION bin)
ENDFOREACH(DLL ${DLLS})
FILE(GLOB "${OSGEO4W_HOME}/apps/qgis/bin/*.dll}" DLLS)
FOREACH(DLL ${DLLS})
    INSTALL(PROGRAMS ${DLL} DESTINATION bin)
ENDFOREACH(DLL ${DLLS})
INSTALL(DIRECTORY "${OSGEO4W_HOME}/share/gdal/" DESTINATION gdal/)

FILE(GLOB "c:/wfips/data/*.db" DBS)
FOREACH(DB ${DBS})
    INSTALL(PROGRAMS ${DB} DESTINATION data)
ENDFOREACH(DB ${DBS})

FILE(GLOB "c:/wfips/data/*.tif" TIFS)
FOREACH(TIF ${TIFS})
    INSTALL(PROGRAMS ${TIF} DESTINATION data)
ENDFOREACH(TIF ${TIFS})

#SET(CPACK_COMPONENT_APPS_REQUIRED TRUE)
#SET(CPACK_COMPONENT_LIBS_HIDDEN TRUE)
#SET(CPACK_COMPONENT_LIBS_DISABLED TRUE)
#SET(CPACK_COMPONENT_INCLUDES_HIDDEN TRUE)
#SET(CPACK_COMPONENT_INCLUDES_DISABLED TRUE)

# *****************************************************************************
# CPack call
# *****************************************************************************
INCLUDE(CPack)

