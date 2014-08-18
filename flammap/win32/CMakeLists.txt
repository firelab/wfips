#/*****************************************************************************
# *
# * $Id: CMakeLists.txt 79 2012-07-26 15:36:43Z kyle.shannon $
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
# We use the OBJECT library of cmake.  We have to have the earliest version
# that supports that, 2.8.8
# *****************************************************************************
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.8)
SET(FLAMMAP_SOURCES_WIN32 FSXW.CPP
                          Fsxwatm.cpp
                          Fsxwspot.cpp
                          Fsxwutil.cpp
                          Newclip.cpp
                          VEC.CPP
                          burnupw.cpp
                          cdtlib.cpp
                          crossthread.cpp
                          flammap.cpp
                          flmburn4.cpp
                          flmcrwn.cpp
                          flmenvt2.cpp
                          flmfms2.cpp
                          flmglbvar.cpp
                          flmmech.cpp
                          fsxpfront.cpp
                          fsxsync.cpp
                          newfms.cpp
                          rand2.cpp
                          splinex.cpp)
#ADD_EXECUTABLE(flammap32 ${FLAMMAP_SOURCES_WIN32})
ADD_LIBRARY(flammap32 OBJECT ${FLAMMAP_SOURCES_WIN32})

