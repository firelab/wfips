#/*****************************************************************************
# *
# * $Id: CMakeLists.txt 599 2014-05-14 03:07:53Z kyle.shannon $
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
# Enable the Qt/QGIS based user interface, load all dependencies
# *****************************************************************************

OPTION(BUILD_OMFFR_GUI "Build the QGIS based graphical interface" OFF)
INCLUDE_DIRECTORIES(${SQLITE3_INCLUDE_DIR}
                    ${GDAL_INCLUDE_DIR}
                    ${PROJECT_SOURCE_DIR}/common/src)

SET(IRS_SOURCES irs.cpp
                irs_result.cpp
                wfips.cpp
                wfips_data.cpp
                wfips_data_load.cpp
                wfips_data_test.cpp
                #irs_api.cpp
                irs_error.cpp
                data_load.cpp
                Aerial.cpp
                AirDelivered.cpp
                Airtanker.cpp
                ConstProd.cpp
                Contain.cpp
                ContainForce.cpp
                ContainResource.cpp
                ContainSim.cpp
                ContainValues.cpp
                CRescType.cpp
                Crew.cpp
                DispLoc.cpp
                DispLogic.cpp
                Engine.cpp
                Fire.cpp
                FWA.cpp
                ground.cpp
                Helicopter.cpp
                Helitack.cpp
                ProdRates.cpp
                Rescbase.cpp
                RescWorkYear.cpp
                Results.cpp
                RunScenario.cpp
                SmallAT.cpp
                SMJAircraft.cpp
                Smokejumper.cpp
                WaterTender.cpp
                DispatchBase.cpp
                NatDispatcher.cpp
                GACCDispatcher.cpp
                LocalDispatcher.cpp
                DLDispatcher.cpp
                TypeTwoIACrew.cpp
                FPU.cpp
                Escape.cpp
                $<TARGET_OBJECTS:omffr_conv>
                $<TARGET_OBJECTS:omffr_rand>)
IF(USE_INTERNAL_SQLITE)
    SET(IRS_SOURCES ${IRS_SOURCES} $<TARGET_OBJECTS:sqlite3>)
ENDIF(USE_INTERNAL_SQLITE)

OPTION(BUILD_CLI_EXAMPLE "Build the example executables" OFF)

ADD_LIBRARY(irs STATIC ${IRS_SOURCES})

IF(BUILD_OMFFR_GUI)
    ADD_SUBDIRECTORY(gui)
ENDIF(BUILD_OMFFR_GUI)

TARGET_LINK_LIBRARIES(irs ${GDAL_LIBRARY})
IF(NOT USE_INTERNAL_SQLITE)
    TARGET_LINK_LIBRARIES(irs ${SQLITE3_LIBRARY})
ENDIF(NOT USE_INTERNAL_SQLITE)
IF(NOT WIN32)
    FIND_PACKAGE (Threads)
    TARGET_LINK_LIBRARIES(irs ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
ENDIF(NOT WIN32)
INSTALL(TARGETS irs DESTINATION lib COMPONENT libs)

IF(BUILD_CLI_EXAMPLE)
    ADD_EXECUTABLE(irs_examples irs_example.cpp)
    ADD_EXECUTABLE(irs_export_result irs_export_result.cpp)
    TARGET_LINK_LIBRARIES(irs_examples irs)
    TARGET_LINK_LIBRARIES(irs_export_result irs)
    ADD_DEPENDENCIES(irs_examples irs)
    ADD_DEPENDENCIES(irs_export_result irs)
ENDIF(BUILD_CLI_EXAMPLE)

