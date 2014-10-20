#/*****************************************************************************
# *
# * $Id$
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

# *****************************************************************************
# General Qt stuff
# *****************************************************************************
SET(QT_USE_QT3SUPPORT FALSE)
SET(QT_USE_QTGUI TRUE)
SET(QT_USE_QTSQL TRUE)
SET(QT_USE_QTSVG TRUE)
SET(QT_USE_QTXML TRUE)
SET(QT_USE_QTNETWORK TRUE)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/irs/src/gui
                    ${CMAKE_BINARY_DIR}
                    ${CMAKE_BINARY_DIR}/irs/src/gui)
# *****************************************************************************
# qwt.  The OSGeo4W build has the release libs, but not debug.  Check build
# type, then force user to supply if it is debug, or we don't have OSGeo4W.
# *****************************************************************************
STRING(TOLOWER "${CMAKE_BUILD_TYPE}" BUILD_TYPE)

IF(NOT WIN32 OR NOT OSGEO4W_HOME)# OR NOT BUILD_TYPE MATCHES "release")
    FIND_PATH(QWT_INCLUDE_DIR qwt.h /usr/include
                                    /usr/include/qwt
                                    /usr/local/include
                                    /usr/local/include/qwt
                                    NO_DEFAULT_PATH)
    FIND_LIBRARY(QWT_LIBRARY qwt /usr/lib
                                 /usr/local/lib
                                 NO_DEFAULT_PATH)
    IF(${QWT_INCLUDE_DIR})
        SET(QWT_FOUND TRUE)
    ENDIF(${QWT_INCLUDE_DIR})
ENDIF(NOT WIN32 OR NOT OSGEO4W_HOME)# OR NOT BUILD_TYPE MATCHES "release")

IF(WIN32)
    IF(NOT OSGEO4W_HOME)
        FIND_PACKAGE(QGIS REQUIRED)
        FIND_PACKAGE(GEOS REQUIRED)
        FIND_PACKAGE(QWT REQUIRED)
        FIND_PACKAGE(Qt4 REQUIRED)
    ELSE(NOT OSGEO4W_HOME)
        FIND_PACKAGE(QGIS)
        FIND_PACKAGE(GEOS)
        FIND_PACKAGE(Qt4)
    ENDIF(NOT OSGEO4W_HOME)
ELSE(WIN32)
    FIND_PACKAGE(QGIS REQUIRED)
    FIND_PACKAGE(GEOS REQUIRED)
    FIND_PACKAGE(Qt4 REQUIRED)
ENDIF(WIN32)

IF(WIN32)
    IF(NOT QGIS_INCLUDE_DIR-NOTFOUND AND OSGEO4W_HOME)
        OPTION(QGIS_USE_UNTSTABLE "Use the develepment version of QGIS in OSGEO4W (-dev)" OFF)
        IF(QGIS_USE_UNTSTABLE)
            SET(QGIS_STABLE "qgis-dev")
        ELSE(QGIS_USE_UNTSTABLE)
            SET(QGIS_STABLE "qgis")
        ENDIF(QGIS_USE_UNTSTABLE)
        # QGIS
        SET(QGIS_INCLUDE_DIR ${OSGEO4W_HOME}/apps/${QGIS_STABLE}/include CACHE TYPE PATH FORCE)
        SET(QGIS_CORE_LIBRARY ${OSGEO4W_HOME}/apps/${QGIS_STABLE}/lib/qgis_core.lib CACHE TYPE NAME FORCE)
        SET(QGIS_GUI_LIBRARY ${OSGEO4W_HOME}/apps/${QGIS_STABLE}/lib/qgis_gui.lib CACHE TYPE NAME FORCE)
        SET(QGIS_PLUGIN_DIR ${OSGEO4W_HOME}/apps/${QGIS_STABLE}/plugins CACHE TYPE NAME FORCE)
        SET(QGIS_INCLUDE_DIR-FOUND TRUE)
        SET(QGIS_LIBRARY-FOUND TRUE)
        # GEOS
        SET(GEOS_INCLUDE_DIR ${OSGEO4W_HOME}/include CACHE TYPE PATH FORCE)
        SET(GEOS_LIBRARY ${OSGEO4W_HOME}/lib/geos_c.lib CACHE TYPE NAME FORCE)
        SET(GEOS_INCLUDE_DIR-FOUND TRUE)
        SET(GEOS_LIBRARY-FOUND TRUE)
        # QWT
        #IF(BUILD_TYPE MATCHES "release")
            SET(QWT_INCLUDE_DIR ${OSGEO4W_HOME}/include/qwt CACHE TYPE PATH FORCE)
            SET(QWT_LIBRARY ${OSGEO4W_HOME}/lib/qwt5.lib CACHE TYPE NAME FORCE)
            SET(QWT_INCLUDE_DIR-FOUND TRUE)
            SET(QWT_LIBRARY-FOUND TRUE)
        #ENDIF(BUILD_TYPE MATCHES "release")
        #QT
        IF(NOT QT_QMAKE_EXECUTABLE_NOTFOUND AND OSGEO4W_HOME)
            SET(QT_QMAKE_EXECUTABLE ${OSGEO4W_HOME}/bin/qmake.exe CACHE TYPE NAME FORCE)
        ENDIF(NOT QT_QMAKE_EXECUTABLE_NOTFOUND AND OSGEO4W_HOME)
    ENDIF(NOT QGIS_INCLUDE_DIR-NOTFOUND AND OSGEO4W_HOME)
ENDIF(WIN32)

IF (WIN32)
  # expect that classes are being imported by default
  # Note: MSVC doesn't like when the macros are quotes
  # and MSYS doesn't like them unqouted (bacause of braces)
  # import qgis classes
  IF (MSVC)
    ADD_DEFINITIONS("-DGUI_EXPORT=__declspec(dllimport)")
    ADD_DEFINITIONS("-DCORE_EXPORT=__declspec(dllimport)")
  ELSE (MSVC)
    ADD_DEFINITIONS("\"-DGUI_EXPORT=__declspec(dllimport)\"")
    ADD_DEFINITIONS("\"-DCORE_EXPORT=__declspec(dllimport)\"")
  ENDIF (MSVC)
ELSE (WIN32)
  ADD_DEFINITIONS(-DGUI_EXPORT=)
  ADD_DEFINITIONS(-DCORE_EXPORT=)
ENDIF (WIN32)

SET(OMFFR_GUI_SRCS main.cpp
                   omffr_gui.cpp
                   simpleidentifymaptool.cpp
                   simpleareamaptool.cpp
                   boxplotitem.cpp
                   textscaledraw.cpp
                   piewidget.cpp)
IF(USE_INTERNAL_SQLITE)
    SET(IRS_SOURCES ${IRS_SOURCES} $<TARGET_OBJECTS:sqlite3>)
ENDIF(USE_INTERNAL_SQLITE)

SET(OMFFR_GUI_UIS omffr_gui.ui)

SET(OMFFR_GUI_MOC_HDRS ${PROJECT_SOURCE_DIR}/irs/src/data_load.h
                       omffr_gui.h
                       simpleidentifymaptool.h
                       simpleareamaptool.h
                       piewidget.h)

SET(OMFFR_GUI_RCCS resources.qrc)

OPTION(BUILD_GUI_V2 "Build the new interface dummy project" OFF)
IF(BUILD_GUI_V2)
    QT4_WRAP_UI(WFIPS_GUI_UI_H wfipsmainwindow.ui)
    QT4_WRAP_CPP(WFIPS_GUI_MOC wfipsmainwindow.h)
ENDIF(BUILD_GUI_V2)

INCLUDE(${QT_USE_FILE})
########################################################
# Build

QT4_WRAP_UI(OMFFR_GUI_UIS_H  ${OMFFR_GUI_UIS})

QT4_WRAP_CPP(OMFFR_GUI_MOC_SRCS  ${OMFFR_GUI_MOC_HDRS})

QT4_ADD_RESOURCES(OMFFR_GUI_RCC_SRCS ${OMFFR_GUI_RCCS})

IF(WIN32)
    SET(GUI_TYPE WIN32)
ELSE(NOT ENABLE_CONSOLE)
    SET(GUI_TYPE "")
ENDIF(WIN32)
SET(GUI_TYPE "")

INCLUDE_DIRECTORIES(${GDAL_INCLUDE_DIR}
                    ${GEOS_INCLUDE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${QT_INCLUDE_DIR}
                    ${QGIS_INCLUDE_DIR}
                    ${QWT_INCLUDE_DIR}
                    ${SQLITE3_INCLUDE_DIR}
                    ${PROJECT_SOURCE_DIR}/irs/src)

ADD_DEFINITIONS(-DQGIS_PLUGIN_DIR="${QGIS_PLUGIN_DIR}")

# Link to qt release libs always
SET(OMFFR_QT_LIBS ${QT_QTCORE_LIBRARY_RELEASE}
                  ${QT_QTGUI_LIBRARY_RELEASE}
                  ${QT_QTSQL_LIBRARY_RELEASE}
                  ${QT_QTSVG_LIBRARY_RELEASE}
                  ${QT_QTXML_LIBRARY_RELEASE}
                  ${QT_QTNETWORK_LIBRARY_RELEASE})

OPTION(QGIS_OVERRIDE_RELEASE "Use the release libs for qt, even with debug builds" ON)
IF(QGIS_OVERRIDE_RELEASE)
    SET(QT_LIBS ${OMFFR_QT_LIBS})
    IF(MSVC)
        SET(QT_LIBS ${QT_LIBS} ${QT_QTMAIN_LIBRARY_RELEASE})
    ENDIF(MSVC)
ELSE(QGIS_OVERRIDE_RELEASE)
    SET(QT_LIBS ${QT_LIBRARIES})
    IF(MSVC)
        SET(QT_LIBS ${QT_LIBS} ${QT_QTMAIN_LIBRARY})
    ENDIF(MSVC)
ENDIF(QGIS_OVERRIDE_RELEASE)

SET(LINK_LIBS ${QT_LIBS}
              ${QGIS_CORE_LIBRARY}
              ${QGIS_GUI_LIBRARY}
              ${QWT_LIBRARY}
              ${GDAL_LIBRARY})

OPTION(BUILD_GUI_V2 "Build the new interface dummy project" OFF)
IF(BUILD_GUI_V2)
    ADD_DEFINITIONS(-DWFIPS_GUI_V2)
    ADD_EXECUTABLE(wfips wfipsmainwindow.cpp
                         ${WFIPS_GUI_UI_H}
                         ${WFIPS_GUI_MOC}
                         ${OMFFR_GUI_RCC_SRCS}
                         main.cpp)
    TARGET_LINK_LIBRARIES(wfips ${LINK_LIBS})
    IF(NOT USE_INTERNAL_SQLITE)
        TARGET_LINK_LIBRARIES(wfips ${SQLITE3_LIBRARY})
    ENDIF(NOT USE_INTERNAL_SQLITE)
ELSE(BUILD_GUI_V2)
    ADD_EXECUTABLE (omffr_gui ${GUI_TYPE}
                    ${OMFFR_GUI_SRCS}
                    ${OMFFR_GUI_MOC_SRCS}
                    ${OMFFR_GUI_RCC_SRCS}
                    ${OMFFR_GUI_UIS_H})
    TARGET_LINK_LIBRARIES(omffr_gui ${CMAKE_BINARY_DIR}/irs/src/${CMAKE_CFG_INTDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}irs${CMAKE_STATIC_LIBRARY_SUFFIX})
    IF (MSVC)
        TARGET_LINK_LIBRARIES(omffr_gui ${QT_QTMAIN_LIBRARY_RELEASE})
    ENDIF (MSVC)
    IF(NOT USE_INTERNAL_SQLITE)
        TARGET_LINK_LIBRARIES(omffr_gui ${SQLITE3_LIBRARY})
    ENDIF(NOT USE_INTERNAL_SQLITE)
    TARGET_LINK_LIBRARIES(omffr_gui ${LINK_LIBS})
    ADD_DEPENDENCIES(omffr_gui irs)
ENDIF(BUILD_GUI_V2)

