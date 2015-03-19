#/*****************************************************************************
# *
# * Project:  WFIPS
# * Purpose:  CMake script
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

# *****************************************************************************
# Optionally disable extremely slow tests
# *****************************************************************************

OPTION(RUN_SLOW_TESTS "Run slow tests" OFF)

Include(FindBoost)
SET(Boost_DEBUG ON)
IF(WIN32)
    SET(Boost_USE_STATIC_LIBS ON)
ELSE(WIN32)
    SET(Boost_USE_STATIC_LIBS OFF)
ENDIF(WIN32)
SET(Boost_USE_MULTITHREAD OFF)
FIND_PACKAGE(Boost 1.41.1 COMPONENTS unit_test_framework REQUIRED)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/common/src
                    ${PROJECT_SOURCE_DIR}/fig/src
                    ${PROJECT_SOURCE_DIR}/flammap/src
                    ${Boost_INCLUDE_DIRS}
                    ${PROJECT_SOURCE_DIR}/irs/src
                    ${GDAL_INCLUDE_DIR}
                    ${SQLITE3_INCLUDE_DIR})

SET(TEST_SRCS test_main.cpp
              test_omffr_tree.cpp
              $<TARGET_OBJECTS:omffr_conv>
              $<TARGET_OBJECTS:omffr_rand>)
IF(BUILD_FIG)
    SET(TEST_SRCS ${TEST_SRCS} test_frisk.cpp
                               ${PROJECT_SOURCE_DIR}/fig/src/frisk.cpp)
ENDIF(BUILD_FIG)

IF(BUILD_IRS)
    SET(TEST_SRCS ${TEST_SRCS} test_irs.cpp)
    SET(TEST_SRCS ${TEST_SRCS} test_wfipsdata.cpp)
    SET(TEST_SRCS ${TEST_SRCS} test_irs_result.cpp)
ENDIF(BUILD_IRS)

SET(TEST_SRCS ${TEST_SRCS} test_omffr_conv.cpp)

ADD_EXECUTABLE(test_main ${TEST_SRCS})

SET(LINK_LIBS ${GDAL_LIBRARY} ${Boost_LIBRARIES} ${SQLITE3_LIBRARIES})
IF(BUILD_IRS)
    SET(LINK_LIBS ${LINK_LIBS} ${CMAKE_BINARY_DIR}/irs/src/${CMAKE_CFG_INTDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}irs${CMAKE_STATIC_LIBRARY_SUFFIX})
ENDIF(BUILD_IRS)

TARGET_LINK_LIBRARIES(test_main ${LINK_LIBS})
IF(NOT WIN32)
    FIND_PACKAGE (Threads)
    TARGET_LINK_LIBRARIES(test_main ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
ENDIF(NOT WIN32)

ADD_DEPENDENCIES(test_main common)
IF(BUILD_FIG)
    ADD_DEPENDENCIES(test_main fig)
ENDIF(BUILD_FIG)
IF(BUILD_IRS)
    ADD_DEPENDENCIES(test_main irs)
ENDIF(BUILD_IRS)

# *****************************************************************************
# OmffrTree tests
# *****************************************************************************
ADD_TEST(omffr_tree_test_creation_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
         --run_test=omffr_tree/creation_1)
ADD_TEST(omffr_tree_test_creation_2 ${EXECUTABLE_OUTPUT_PATH}/test_main
         --run_test=omffr_tree/creation_2)
ADD_TEST(omffr_tree_test_creation_3 ${EXECUTABLE_OUTPUT_PATH}/test_main
         --run_test=omffr_tree/creation_3)
ADD_TEST(omffr_tree_test_external_nodes_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
         --run_test=omffr_tree/external_nodes_1)
ADD_TEST(omffr_tree_test_postorder_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
         --run_test=omffr_tree/postorder_1)
ADD_TEST(omffr_tree_test_postorder_2 ${EXECUTABLE_OUTPUT_PATH}/test_main
         --run_test=omffr_tree/postorder_2)
ADD_TEST(omffr_tree_test_preorder_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
         --run_test=omffr_tree/preorder_1)
ADD_TEST(omffr_tree_test_creation_faulty ${EXECUTABLE_OUTPUT_PATH}/test_main
         --run_test=omffr_tree/creation_faulty)

# *****************************************************************************
# FRisk tests
# *****************************************************************************
IF(BUILD_FIG)
    ADD_TEST(frisk_test_wind_prob_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
             --run_test=frisk/wind_prob_1)
    ADD_TEST(frisk_test_ascii_read_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
             --run_test=frisk/ascii_read_1)
    ADD_TEST(frisk_test_ascii_read_2 ${EXECUTABLE_OUTPUT_PATH}/test_main
             --run_test=frisk/ascii_read_2)
    ADD_TEST(frisk_test_equal_op ${EXECUTABLE_OUTPUT_PATH}/test_main
             --run_test=frisk/equal_op)
    ADD_TEST(frisk_test_ascii_write ${EXECUTABLE_OUTPUT_PATH}/test_main
             --run_test=frisk/ascii_write)
    ADD_TEST(frisk_test_binary_write ${EXECUTABLE_OUTPUT_PATH}/test_main
             --run_test=frisk/binary_write)
    ADD_TEST(frisk_test_julian_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
             --run_test=frisk/julian_1)
    ADD_TEST(frisk_test_memcheck_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
             --run_test=frisk/memcheck_1)
ENDIF(BUILD_FIG)

# *****************************************************************************
# IRS tests
# *****************************************************************************
IF(BUILD_IRS)
    ADD_TEST(irs_test_db_fpu_query ${EXECUTABLE_OUTPUT_PATH}/test_main
             --run_test=irs/db_fpu_query)
    ADD_TEST(irs_test_smooth_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
             --run_test=irs/smooth_1)
    ADD_TEST(irs_test_smooth_2 ${EXECUTABLE_OUTPUT_PATH}/test_main
             --run_test=irs/smooth_2)
    ADD_TEST(irs_test_smooth_3 ${EXECUTABLE_OUTPUT_PATH}/test_main
             --run_test=irs/smooth_3)
    ADD_TEST(irs_test_resource_dist_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
             --run_test=irs/resource_dist_1)
    ADD_TEST(irs_test_point_query_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
             --run_test=irs/point_query_1)
    ADD_TEST(irs_test_load_resc_type ${EXECUTABLE_OUTPUT_PATH}/test_main
             --run_test=irs/load_resc_type)
    ADD_TEST(irs_test_general_resc_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
             --run_test=irs/general_resc_1)
    ADD_TEST(irs_test_load_disploc_fpu ${EXECUTABLE_OUTPUT_PATH}/test_main
             --run_test=irs/load_disploc_fpu)
    ADD_TEST(irs_test_get_fpu_name ${EXECUTABLE_OUTPUT_PATH}/test_main
             --run_test=irs/get_fpu_name)
    ADD_TEST(irs_test_fpu_filter ${EXECUTABLE_OUTPUT_PATH}/test_main
             --run_test=irs/fpu_filter)
    ADD_TEST(irs_test_bad_filter ${EXECUTABLE_OUTPUT_PATH}/test_main
             --run_test=irs/bad_filter)
    ADD_TEST(irs_test_resc_copy_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
             --run_test=irs/resc_copy_1)
    ADD_TEST(irs_test_resc_remove_set_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
             --run_test=irs/resc_remove_set_1)
    ADD_TEST(irs_test_resc_remove_set_2 ${EXECUTABLE_OUTPUT_PATH}/test_main
             --run_test=irs/resc_remove_set_2)
    ADD_TEST(irs_test_resc_remove_set_3 ${EXECUTABLE_OUTPUT_PATH}/test_main
             --run_test=irs/resc_remove_set_3)
    ADD_TEST(irs_test_resc_remove_set_4 ${EXECUTABLE_OUTPUT_PATH}/test_main
             --run_test=irs/resc_remove_set_4)
    ADD_TEST(irs_test_resc_remove_set_5 ${EXECUTABLE_OUTPUT_PATH}/test_main
             --run_test=irs/resc_remove_set_6)
    ADD_TEST(irs_test_resc_remove_set_6 ${EXECUTABLE_OUTPUT_PATH}/test_main
             --run_test=irs/resc_remove_set_6)
    ADD_TEST(irs_test_resc_remove_set_6 ${EXECUTABLE_OUTPUT_PATH}/test_main
             --run_test=irs/resc_remove_set_6)
    ADD_TEST(irs_test_resc_remove_set_7 ${EXECUTABLE_OUTPUT_PATH}/test_main
             --run_test=irs/resc_remove_set_7)
    ADD_TEST(irs_test_resc_remove_set_8 ${EXECUTABLE_OUTPUT_PATH}/test_main
             --run_test=irs/resc_remove_set_8)
    ADD_TEST(irs_test_resc_remove_set_9 ${EXECUTABLE_OUTPUT_PATH}/test_main
             --run_test=irs/resc_remove_set_9)
    ADD_TEST(irs_test_resc_remove_set_10 ${EXECUTABLE_OUTPUT_PATH}/test_main
             --run_test=irs/resc_remove_set_10)
    ADD_TEST(irs_test_resc_remove_set_11 ${EXECUTABLE_OUTPUT_PATH}/test_main
             --run_test=irs/resc_remove_set_11)
    ADD_TEST(irs_test_resc_remove_set_12 ${EXECUTABLE_OUTPUT_PATH}/test_main
             --run_test=irs/resc_remove_set_12)
    ADD_TEST(irs_test_resc_remove_set_13 ${EXECUTABLE_OUTPUT_PATH}/test_main
             --run_test=irs/resc_remove_set_13)
    # IRS Results
    ADD_TEST(irs_test_single_resc_usage ${EXECUTABLE_OUTPUT_PATH}/test_main
             --run_test=irs_result/single_resc_usage)
    ADD_TEST(irs_test_remove_resc_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
             --run_test=irs_result/remove_resc_1)
    #ADD_TEST(irs_test_result_attributes_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
    #         --run_test=result_attributes_1)
ENDIF(BUILD_IRS)

ADD_TEST(omffr_conv_path_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
         --run_test=conv/path_1)
ADD_TEST(omffr_conv_join_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
         --run_test=conv/join_1)
ADD_TEST(omffr_conv_join_2 ${EXECUTABLE_OUTPUT_PATH}/test_main
         --run_test=conv/join_2)
ADD_TEST(omffr_conv_join_3 ${EXECUTABLE_OUTPUT_PATH}/test_main
         --run_test=conv/join_3)
ADD_TEST(omffr_conv_join_4 ${EXECUTABLE_OUTPUT_PATH}/test_main
         --run_test=conv/join_4)
ADD_TEST(omffr_conv_join_5 ${EXECUTABLE_OUTPUT_PATH}/test_main
         --run_test=conv/join_5)
ADD_TEST(omffr_conv_count_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
         --run_test=conv/count_1)
ADD_TEST(omffr_conv_count_2 ${EXECUTABLE_OUTPUT_PATH}/test_main
         --run_test=conv/count_2)
ADD_TEST(omffr_conv_count_3 ${EXECUTABLE_OUTPUT_PATH}/test_main
         --run_test=conv/count_3)
ADD_TEST(omffr_conv_count_4 ${EXECUTABLE_OUTPUT_PATH}/test_main
         --run_test=conv/count_4)
ADD_TEST(omffr_conv_add_string_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
         --run_test=conv/add_string_1)
ADD_TEST(omffr_conv_add_string_2 ${EXECUTABLE_OUTPUT_PATH}/test_main
         --run_test=conv/add_string_2)
ADD_TEST( wfips_create_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/create_1 )
ADD_TEST( wfips_create_2 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/create_2 )
ADD_TEST( wfips_sql_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/sql_1 )
ADD_TEST( wfips_resc_copy_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/resc_copy_1 )
ADD_TEST( wfips_resc_copy_2 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/resc_copy_2 )
ADD_TEST( wfips_resc_copy_3 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/resc_copy_3 )
ADD_TEST( wfips_assoc_disploc_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/assoc_disploc_1 )
ADD_TEST( wfips_build_set_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/build_set_1 )
ADD_TEST( wfips_build_set_2 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/build_set_2 )
ADD_TEST( wfips_build_set_3 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/build_set_3 )
ADD_TEST( wfips_build_set_4 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/build_set_4 )
ADD_TEST( wfips_build_set_5 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/build_set_5 )
ADD_TEST( wfips_build_fid_set_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/build_fid_set_1 )
ADD_TEST( wfips_assoc_resource_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/assoc_resource_1 )
ADD_TEST( wfips_assoc_resource_2 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/assoc_resource_2 )
ADD_TEST( wfips_assoc_resource_3 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/assoc_resource_3 )
ADD_TEST( wfips_load_scen_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/load_scen_1 )
ADD_TEST( wfips_load_scen_2 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/load_scen_2 )
ADD_TEST( wfips_load_scen_3 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/load_scen_3 )
ADD_TEST( wfips_load_scen_4 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/load_scen_4 )
ADD_TEST( wfips_load_scen_5 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/load_scen_5 )
ADD_TEST( wfips_load_scen_6 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/load_scen_6 )
ADD_TEST( wfips_load_scen_7 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/load_scen_7 )
ADD_TEST( wfips_load_scen_8 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/load_scen_8 )
ADD_TEST( wfips_load_scen_9 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/load_scen_9 )
ADD_TEST( wfips_load_scen_10 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/load_scen_10 )
ADD_TEST( wfips_load_scen_11 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/load_scen_11 )
ADD_TEST( wfips_load_scen_12 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/load_scen_12 )
ADD_TEST( wfips_set_resource_db_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/set_resource_db_1 )
ADD_TEST( wfips_scen_count_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/scen_count_1 )
ADD_TEST( wfips_load_resc_types_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/load_resc_types_1 )
ADD_TEST( wfips_load_prod_rates_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/load_prod_rates_1 )
ADD_TEST( wfips_load_disp_log_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/load_disp_log_1 )
ADD_TEST( wfips_load_disp_log_2 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/load_disp_log_2 )
ADD_TEST( wfips_load_fwa_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/load_fwa_1 )
ADD_TEST( wfips_load_fwa_2 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/load_fwa_2 )
ADD_TEST( wfips_load_disp_loc_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/load_disp_loc_1 )
ADD_TEST( wfips_load_disp_loc_2 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/load_disp_loc_2 )
ADD_TEST( wfips_load_resource_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/load_resource_1 )
ADD_TEST( wfips_load_resource_2 ${EXECUTABLE_OUTPUT_PATH}/test_main
          --run_test=wfipsdata/load_resource_2 )
IF(RUN_SLOW_TESTS)
    IF(BUILD_IRS)
        ADD_TEST( wfips_load_all_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
                  --run_test=wfipsdata/load_all_1 )
        ADD_TEST( wfips_run_small_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
                  --run_test=wfipsdata/run_small_1 )
        ADD_TEST( wfips_small_output_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
                  --run_test=wfipsdata/run_small_output_1 )
              #ADD_TEST( wfips_small_output_2 ${EXECUTABLE_OUTPUT_PATH}/test_main
              #--run_test=wfipsdata/run_small_output_2 )
        ADD_TEST( wfips_small_output_3 ${EXECUTABLE_OUTPUT_PATH}/test_main
                  --run_test=wfipsdata/run_small_output_3 )
        ADD_TEST( wfips_run_gb_id_002 ${EXECUTABLE_OUTPUT_PATH}/test_main
                  --run_test=wfipsdata/run_gb_id_002 )
        ADD_TEST( wfips_run_ca_ca_015 ${EXECUTABLE_OUTPUT_PATH}/test_main
                  --run_test=wfipsdata/run_ca_ca_015 )
        ADD_TEST( wfips_run_full_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
                  --run_test=wfipsdata/run_full_1 )
        ADD_TEST( wfips_run_full_no_space_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
                  --run_test=wfipsdata/run_full_no_space_1 )
        #ADD_TEST(irs_test_avg_fires_1 ${EXECUTABLE_OUTPUT_PATH}/test_main
        #         --run_test=irs/avg_fires_1)
        ADD_TEST(irs_test_scenario_order ${EXECUTABLE_OUTPUT_PATH}/test_main
                 --run_test=irs/scenario_order)
        ADD_TEST(irs_test_load_airtankers ${EXECUTABLE_OUTPUT_PATH}/test_main
                 --run_test=irs/load_airtankers)
        ADD_TEST(irs_test_gacc_filter ${EXECUTABLE_OUTPUT_PATH}/test_main
                 --run_test=irs/gacc_filter)
        ADD_TEST(irs_test_state_filter ${EXECUTABLE_OUTPUT_PATH}/test_main
                 --run_test=irs/state_filter)
    ENDIF(BUILD_IRS)
ENDIF(RUN_SLOW_TESTS)

