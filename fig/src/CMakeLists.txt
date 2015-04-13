#/*****************************************************************************
# *
# * $Id: CMakeLists.txt 452 2013-10-31 18:27:45Z kyle.shannon $
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

INCLUDE_DIRECTORIES(${SQLITE3_INCLUDE_DIR}
                    ${PROJECT_SOURCE_DIR}/common/src
                    ${PROJECT_SOURCE_DIR}/nfdrs
                    ${PROJECT_SOURCE_DIR}/irs/src
                    ${GDAL_INCLUDE_DIR})
# FIG and win32
IF(MSVC)
    ADD_DEFINITIONS(-DFLMP_ENABLE_OMFFR)
    INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/flammap/win32)
    SET(FIG_API_SOURCES $<TARGET_OBJECTS:omffr_conv>
                        $<TARGET_OBJECTS:omffr_rand>
                        $<TARGET_OBJECTS:nfdrs>
                        raster_access.cpp
                        frisk.cpp
                        fig.cpp
                        fig_api.cpp
                        $<TARGET_OBJECTS:sqlite3>
                        $<TARGET_OBJECTS:flammap32>)
    ADD_LIBRARY(fig SHARED ${FIG_API_SOURCES})
    SET(LINK_LIBS ${LINK_LIBS} ${GDAL_LIBRARY})
    TARGET_LINK_LIBRARIES(fig ${LINK_LIBS})
    ADD_DEPENDENCIES(fig omffr_conv omffr_rand flammap32)
    ADD_EXECUTABLE(fig_load fig_load.cpp)
    TARGET_LINK_LIBRARIES(fig_load fig)
    ADD_DEPENDENCIES(fig_load fig)
ENDIF(MSVC)

SET(LINK_LIBS ${GDAL_LIBRARY})
IF(NOT WIN32)
    FIND_PACKAGE (Threads)
    SET(LINK_LIBS ${LINK_LIBS} ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
ENDIF(NOT WIN32)

ADD_EXECUTABLE(fig2scenario $<TARGET_OBJECTS:omffr_conv>
                            $<TARGET_OBJECTS:omffr_rand>
                            ${PROJECT_SOURCE_DIR}/sqlite/sqlite3.c fig2scenario.cpp)
TARGET_LINK_LIBRARIES(fig2scenario ${LINK_LIBS} ${CMAKE_BINARY_DIR}/irs/src/${CMAKE_CFG_INTDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}irs${CMAKE_STATIC_LIBRARY_SUFFIX})
ADD_DEPENDENCIES(fig2scenario irs)

