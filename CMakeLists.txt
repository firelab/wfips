#/*****************************************************************************
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

PROJECT(wfips)


SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

SET(CMAKE_INCLUDE_CURRENT_DIR ON)

# *****************************************************************************
# Set the version for the project
# *****************************************************************************
FILE(STRINGS ${CMAKE_SOURCE_DIR}/VERSION WFIPS_VERSION_STRING)
STRING(REPLACE "." ";" V2 ${WFIPS_VERSION_STRING})
LIST(LENGTH V2 V_SIZE)
IF(V_SIZE GREATER 2)
    LIST(GET V2 0 WFIPS_VERSION_MAJOR)
    LIST(GET V2 1 WFIPS_VERSION_MINOR)
    LIST(GET V2 2 WFIPS_VERSION_PATCH)
ENDIF(V_SIZE GREATER 2)


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
ENDIF(BUILD_TESTING)

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

# *****************************************************************************
# Allow the simulation to skip the contain algorithm, if we think it will be
# contained.
# *****************************************************************************

OPTION(ALLOW_CONTAIN_SKIP "Allow ommission of some contain simulations" OFF)
IF(ALLOW_CONTAIN_SKIP)
    ADD_DEFINITIONS(-DIRS_ALLOW_SKIP_CONTAIN)
ENDIF(ALLOW_CONTAIN_SKIP)

# *****************************************************************************
# Put common libs here
# *****************************************************************************
INCLUDE(FindGDAL)
FIND_PACKAGE(GDAL REQUIRED)

# Define SPATIALITE_EXT ??
INCLUDE(FindSPATIALITE)
FIND_PACKAGE(SPATIALITE REQUIRED)

# *****************************************************************************
# Recurse through subdirectories
# *****************************************************************************

ADD_SUBDIRECTORY(common)

ADD_SUBDIRECTORY(doc)

ADD_SUBDIRECTORY(sqlite)

IF(BUILD_FIG)
    ADD_DEFINITIONS(-DFLMP_ENABLE_OMFFR)
    ADD_SUBDIRECTORY(nfdrs)
    ADD_SUBDIRECTORY(flammap)
    ADD_SUBDIRECTORY(fig)
ENDIF(BUILD_FIG)

ADD_SUBDIRECTORY(irs)

IF(BUILD_TESTING)
    ADD_SUBDIRECTORY(autotest)
    INCLUDE(CTest)
    ADD_CUSTOM_TARGET(check COMMAND ${CMAKE_CTEST_COMMAND})
ENDIF(BUILD_TESTING)

# *****************************************************************************
# CPack
# *****************************************************************************
IF(WIN32)
    OPTION(USE_OSGEO4W "Use dependencies from OSGeo4W" OFF)
    IF(USE_OSGEO4W)
        FIND_PATH(OSGEO4W_HOME OSGeo4W CACHE PATH)
    ENDIF(USE_OSGEO4W)
ENDIF(WIN32)

INCLUDE(InstallRequiredSystemLibraries)

# *****************************************************************************
# CPack variables
# *****************************************************************************

SET(CPACK_NSIS_PACKAGE_NAME WFIPS-${WFIPS_VERSION_STRING})
SET(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE)
# version
SET(CPACK_VERSION_MAJOR ${WFIPS_VERSION_MAJOR})
SET(CPACK_VERSION_MINOR ${WFIPS_VERSION_MINOR})
SET(CPACK_VERSION_PATCH ${WFIPS_VERSION_PATCH})
SET(CPACK_PACKAGE_VERSION ${WFIPS_VERSION_STRING})

SET(CPACK_NSIS_INSTALL_ROOT "C:")
CMAKE_POLICY(PUSH)
CMAKE_POLICY(SET CMP0010 OLD)
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "WFIPS\\\\${WFIPS_VERSION_STRING}")
SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "WFIPS\\\\${WFIPS_VERSION_STRING}")
CMAKE_POLICY(POP)
# *****************************************************************************
# Add/Remove Programs support
# *****************************************************************************
SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\wfips.exe")
# *****************************************************************************
# NSIS Specific stuff
# *****************************************************************************
SET(CPACK_NSIS_DISPLAY_NAME "WFIPS\\\\${WFIPS_VERSION_STRING}")
#TODO
#SET(CPACK_NSIS_INSTALLED_ICON_NAME "${CMAKE_SOURCE_DIR}/images/icons/wn-desktop.ico")

SET(CPACK_CREATE_DESKTOP_LINKS WFIPS)
SET(CPACK_NSIS_MODIFY_PATH WFIPS)

SET(CPACK_PACKAGE_EXECUTABLES "wfips" ${WFIPS_VERSION_STRING})

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

IF(WIN32 AND MSVC AND OSGEO4W_HOME)
    IF(BUILD_64)
        SET(CPACK_PACKAGE_FILE_NAME "${WFIPS_VERSION_STRING}-win64-install")
    ELSE(BUILD_64)
        SET(CPACK_PACKAGE_FILE_NAME "${WFIPS_VERSION_STRING}-win32-install")
    ENDIF(BUILD_64)
    # All osgeo4w dlls
    FILE(GLOB DLLS "${OSGEO4W_HOME}/bin/*.dll")
    FOREACH(DLL ${DLLS})
        INSTALL(PROGRAMS ${DLL} DESTINATION bin COMPONENT apps)
    ENDFOREACH(DLL ${DLLS})

ENDIF(WIN32 AND MSVC AND OSGEO4W_HOME)
SET(CPACK_COMPONENT_APPS_REQUIRED TRUE)
SET(CPACK_COMPONENT_LIBS_HIDDEN TRUE)
SET(CPACK_COMPONENT_LIBS_DISABLED TRUE)
SET(CPACK_COMPONENT_INCLUDES_HIDDEN TRUE)
SET(CPACK_COMPONENT_INCLUDES_DISABLED TRUE)

# *****************************************************************************
# CPack call
# *****************************************************************************
INCLUDE(CPack)

