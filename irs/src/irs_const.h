/******************************************************************************
 *
 * $Id$
 *
 * Project:  Optimization Modeling of Fire Fighter Resources
 * Purpose:  Constants for IRS
 * Author:   Kyle Shannon <kyle at pobox dot com>
 *
 ******************************************************************************
 *
 * THIS SOFTWARE WAS DEVELOPED AT THE ROCKY MOUNTAIN RESEARCH STATION (RMRS)
 * MISSOULA FIRE SCIENCES LABORATORY BY EMPLOYEES OF THE FEDERAL GOVERNMENT 
 * IN THE COURSE OF THEIR OFFICIAL DUTIES. PURSUANT TO TITLE 17 SECTION 105 
 * OF THE UNITED STATES CODE, THIS SOFTWARE IS NOT SUBJECT TO COPYRIGHT 
 * PROTECTION AND IS IN THE PUBLIC DOMAIN. RMRS MISSOULA FIRE SCIENCES 
 * LABORATORY ASSUMES NO RESPONSIBILITY WHATSOEVER FOR ITS USE BY OTHER 
 * PARTIES,  AND MAKES NO GUARANTEES, EXPRESSED OR IMPLIED, ABOUT ITS QUALITY, 
 * RELIABILITY, OR ANY OTHER CHARACTERISTIC.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/

#ifndef IRS_CONST_H_
#define IRS_CONST_H_

#ifndef OMFFR_DB_DBG
#ifdef WIN32
#define OMFFR_DB_DBG "C:/src/omffr/data/omffr.sqlite"
#else
#define OMFFR_DB_DBG "/home/kyle/src/omffr/trunk/data/omffr.sqlite"
#endif
#endif

#ifdef WIN32
#define SPATIALITE_EXTENSION "spatialite.dll"
#else
//#define SPATIALITE_EXTENSION "libspatialite.so"
#define SPATIALITE_EXTENSION "libspatialite.so"
#endif

#if SQLITE_VERSION_NUMBER > 3007017
#define SPATIALITE_INIT NULL //"sqlite3_modspatialite_init"
#else
#define SPATIALITE_INIT      NULL
#endif

#ifdef __cplusplus
#   define IRS_C_START       extern "C" {
#   define IRS_C_END         }
#else
#   define IRS_C_START
#   define IRS_C_END
#endif

#ifndef IRS_API
#if defined(WIN32) && defined(omffr_EXPORTS)
#  define IRS_API declspec(dllexport)
#else
#  define IRS_API
#endif
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifdef MAX_PATH
#define IRS_MAX_PATH MAXPATH
#else
#define IRS_MAX_PATH 8192
#endif


/*
** Region bit masks
*/
#define NORTHERN_ROCKIES      0x0001
#define ROCKY_MOUNTAINS       0x0002
#define SOUTHWEST             0x0004
#define GREAT_BASIN           0x0008
#define CALIFORNIA            0x0010
#define NORTHWEST             0x0020
#define SOUTHERN              0x0040
#define EASTERN               0x0080
#define ALASKA                0x0100
#define NATIONAL              0x0200
#define REGION_WEST           (NORTHERN_ROCKIES | ROCKY_MOUNTAINS | \
                               SOUTHWEST        | GREAT_BASIN     | \
                               CALIFORNIA       | NORTHWEST | NATIONAL)
#define REGION_ALL            (NORTHERN_ROCKIES | ROCKY_MOUNTAINS | \
                               SOUTHWEST        | GREAT_BASIN     | \
                               CALIFORNIA       | NORTHWEST       | \
                               SOUTHERN         | EASTERN         | \
                               ALASKA | NATIONAL)

/* Region names in db */
#define R1                    "NR"
#define R2                    "RM"
#define R3                    "NW"
#define R4                    "GB"
#define R5                    "CA"
#define R6                    "SW"
#define R7                    "NR"
#define R8                    "SA"
#define R9                    "EA"
#define R10                   "AK"
#define NA                    "NA"

/*
** Agencies
*/
#define USFS                  0x0001
#define DOI_BIA               0x0002
#define DOI_BLM               0x0004
#define DOI_FWS               0x0008
#define DOI_NPS               0x0010
#define STATE_LOCAL           0x0020
#define REGIONAL              0x0040
#define DOI_ALL               (DOI_BIA | DOI_BLM | DOI_FWS | DOI_NPS)
#define FED_ALL               (USFS | DOI_ALL)
#define AGENCY_ALL            (FED_ALL | STATE_LOCAL)


/* Identifiers for resources in sql */
#define USFS_NAME             "'FS'"
#define DOI_BIA_NAME          "'BIA'"
#define DOI_BLM_NAME          "'BLM'"
#define DOI_FWS_NAME          "'FWS'"
#define DOI_NPS_NAME          "'NPS'"
#define STATE_LOCAL_NAME      "'STATE/LOCAL'"

#define DOI_SET             "'BIA','BLM','FWS','NPS'"
#define FED_SET             "'BIA','BLM','FWS','NPS','FS'"

/* Methods to extract resources */
#define RESC_REDUCE_RAND      0
#define RESC_REDUCE_PERFORM   1

/* Fire outcomes */
#define NO_RESC_SENT          0x0001
#define TIME_LIMIT_EXCEED     0x0002
#define SIZE_LIMIT_EXCEED     0x0004
#define EXHAUSTED             0x0008
#define CONTAINED             0x0010
#define ALL_ESCAPES           (NO_RESC_SENT | TIME_LIMIT_EXCEED | \
                               SIZE_LIMIT_EXCEED | EXHAUSTED)

#define NO_RESC_SENT_STR      "No Resources Sent"
#define TIME_LIMIT_EXCEED_STR "TimeLimitExceeded"
#define SIZE_LIMIT_EXCEED_STR "SizeLimitExceeded"
#define EXHAUSTED_STR         "Exhausted"
#define CONTAINED_STR         "Contained"

static const char *apszStatusStrings[] = { NO_RESC_SENT_STR,
                                           TIME_LIMIT_EXCEED_STR,
                                           SIZE_LIMIT_EXCEED_STR,
                                           EXHAUSTED_STR,
                                           NULL };

/*
** General resource types
*/
#define ATT                   0x0001
#define CRW                   0x0002
#define DZR                   0x0004
#define ENG                   0x0008
#define HEL                   0x0010
#define HELI                  0x0020
#define SEAT                  0x0040
#define SMJR                  0x0080
#define SJAC                  0x0100
#define FRBT                  0x0200
#define WT                    0x0400
#define ALL_SMJR              (SMJR | SJAC)
#define AIR_ATTACK            (ATT | SEAT)
#define RESC_ALL              (ATT | CRW | DZR | ENG | HEL | HELI | SEAT | \
                               SMJR | SJAC | FRBT | WT)

/*
** Geometry
*/
#define FWA_GEOMETRY  0x0001
#define FPU_GEOMETRY  0x0002
#define GACC_GEOMETRY 0x0004

/*
** Various Outputs
*/
#define FIRE_OUTCOME  0x0001
#define RESC_USAGE    0x0002
#define LARGE_FIRE    0x0004

/*
** Output column defintions
*/
#define CONTAIN_COLUMN     "contain_count"
#define TIME_LIMIT_COLUMN  "time_limit_count"
#define SIZE_LIMIT_COLUMN  "size_limit_count"
#define EXHAUST_COLUMN     "exhaust_count"
#define NO_RESC_COLUMN     "no_resc_count"
#define PERC_CONT_COLUMN   "perc_contain"
#define LF_COST_COLUMN     "lf_cost"
#define LF_ACRE_COLUMN     "lf_acres"
#define LF_POP_COLUMN      "lf_pop"
#define RESC_USAGE_COLUMN  "resc_usage"

/*
** Output errors/issues
*/
#define IRS_MISSING_TABLES      0x0001
#define IRS_MISSING_SPLITE      0x0002
#define IRS_NULL_DB             0x0004

/*
** Metadata keys.  These should be used in lieu of hard coded strings.
*/
#define IRS_MAX_METADATA_SIZE 1024
#define IRS_PREV_RUN_KEY      "previous_result"
#define IRS_PREV_RUN_DIR      1
#define IRS_NEXT_RUN_KEY      "next_result"
#define IRS_NEXT_RUN_DIR      2
#define IRS_PERC_TREAT_KEY    "perc_treat"
#define IRS_RESC_RED_FRC_KEY  "resc_red_frac"
#define IRS_RESC_RED_MTD_KEY  "resc_red_method"
#define IRS_RESC_RED_CNT_KEY  "resc_red_count"


#endif /* IRS_CONST_H_ */
