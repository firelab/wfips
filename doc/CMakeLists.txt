#/*****************************************************************************
# *
# * Project:  Wildland Fire Investment Planning System

# * Author:   Kyle Shannon <kyle at pobox dot com>
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

# Check for markdown
find_program(markdown markdown /usr/bin /usr/local/bin)
if(NOT markdown)
    message("-- Could not find markdown")
    return()
else(NOT markdown)
    message("-- Found markdown: " ${markdown})
endif(NOT markdown)

file(GLOB md_files "*.md")

foreach(md ${md_files})
    get_filename_component(base ${md} NAME_WE)
    set(html_out ${base}.html)
    execute_process(COMMAND ${markdown} ${md}
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/doc
                    RESULT_VARIABLE mdrc
                    OUTPUT_VARIABLE mdout)
    file(WRITE ${CMAKE_BINARY_DIR}/doc/${html_out} ${mdout})
endforeach(md ${md_files})

# Copy some files over for ease of use
set(copy_files "wfips.css;wfips.qhp;wfips.qhcp")
foreach(f ${copy_files})
    configure_file(${f} ${CMAKE_BINARY_DIR}/doc/${f} COPY_ONLY)
endforeach(f ${copy_files})

execute_process(COMMAND qhelpgenerator wfips.qhp -o wfips.qch
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/doc
                RESULT_VARIABLE qhelprc
                OUTPUT_VARIABLE qhelpout)

execute_process(COMMAND qcollectiongenerator wfips.qhcp -o wfips.qhc
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/doc
                RESULT_VARIABLE qhelprc
                OUTPUT_VARIABLE qhelpout)

