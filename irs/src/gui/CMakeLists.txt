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
SET (QT_USE_QT3SUPPORT FALSE)
SET (QT_USE_QTGUI TRUE)
SET (QT_USE_QTSQL TRUE)
SET (QT_USE_QTSVG TRUE)
SET (QT_USE_QTXML TRUE)
SET (QT_USE_QTNETWORK TRUE)
FIND_PACKAGE(Qt4 REQUIRED)

# *****************************************************************************
# QGIS specific, straight from QGIS examples
# *****************************************************************************
FIND_PACKAGE(QGIS REQUIRED)
IF(NOT QGIS_FOUND)
  MESSAGE(SEND_ERROR "QGIS dependency was not found!")
ENDIF(NOT QGIS_FOUND)
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

# *****************************************************************************
# Geos required for Qt/QGIS build, but probably elsewhere.
# *****************************************************************************
FIND_PACKAGE(GEOS REQUIRED)
IF(NOT GEOS_FOUND)
  MESSAGE(SEND_ERROR "GEOS dependency was not found!")
ENDIF(NOT GEOS_FOUND)

# *****************************************************************************
# Qwt so we can draw graphs.
# *****************************************************************************
#FIND_PACKAGE(QWT REQUIRED)
FIND_PATH(QWT_INCLUDE_DIR qwt.h /usr/include
                                /usr/include/qwt
                                /usr/local/include
                                /usr/local/include/qwt)
FIND_LIBRARY(QWT_LIBRARY qwt /usr/lib /usr/local/lib)
IF(${QWT_INCLUDE_DIR})
    SET(QWT_FOUND "YES")
ENDIF(${QWT_INCLUDE_DIR})

IF(NOT QWT_FOUND)
    #MESSAGE(ERROR "Could not locate Qwt")
ENDIF(NOT QWT_FOUND)

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

INCLUDE(${QT_USE_FILE})
########################################################
# Build

QT4_WRAP_UI (OMFFR_GUI_UIS_H  ${OMFFR_GUI_UIS})

QT4_WRAP_CPP (OMFFR_GUI_MOC_SRCS  ${OMFFR_GUI_MOC_HDRS})

QT4_ADD_RESOURCES(OMFFR_GUI_RCC_SRCS ${OMFFR_GUI_RCCS})

IF(WIN32)
    SET(GUI_TYPE WIN32)
ELSE(NOT ENABLE_CONSOLE)
    SET(GUI_TYPE "")
ENDIF(WIN32)
SET(GUI_TYPE "")

ADD_EXECUTABLE (omffr_gui ${GUI_TYPE}
                          ${OMFFR_GUI_SRCS}
                          ${OMFFR_GUI_MOC_SRCS}
                          ${OMFFR_GUI_RCC_SRCS}
                          ${OMFFR_GUI_UIS_H})

INCLUDE_DIRECTORIES(${GDAL_INCLUDE_DIR}
                    ${GEOS_INCLUDE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${QT_INCLUDE_DIR}
                    ${QGIS_INCLUDE_DIR}
                    ${QWT_INCLUDE_DIR}
                    ${SQLITE3_INCLUDE_DIR}
                    ${PROJECT_SOURCE_DIR}/irs/src)

ADD_DEFINITIONS(-DQGIS_PLUGIN_DIR="${QGIS_PLUGIN_DIR}")
#This is probably no longer needed, but I will leave it in for Win machines for the moment
IF(QT_QTSQL_FOUND)
   FIND_LIBRARY(QT_QTSQL_LIBRARY NAMES QtSql QtSql4 PATHS ${QT_LIBRARY_DIR} NO_DEFAULT_PATH)
   SET(QT_LIBRARIES ${QT_LIBRARIES} ${QT_QTSQL_LIBRARY})
ENDIF(QT_QTSQL_FOUND)

TARGET_LINK_LIBRARIES(omffr_gui ${CMAKE_BINARY_DIR}/irs/src/${CMAKE_CFG_INTDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}irs${CMAKE_STATIC_LIBRARY_SUFFIX})

TARGET_LINK_LIBRARIES(omffr_gui ${QT_LIBRARIES}
                                ${QGIS_CORE_LIBRARY}
                                ${QGIS_GUI_LIBRARY}
                                #${GEOS_LIBRARY}
                                ${QWT_LIBRARY}
                                ${GDAL_LIBRARY})
IF(NOT USE_INTERNAL_SQLITE)
    TARGET_LINK_LIBRARIES(omffr_gui ${SQLITE3_LIBRARY})
ENDIF(NOT USE_INTERNAL_SQLITE)

IF (MSVC)
    TARGET_LINK_LIBRARIES(omffr_gui ${QT_QTMAIN_LIBRARY})
ENDIF (MSVC)

ADD_DEPENDENCIES(omffr_gui irs)

