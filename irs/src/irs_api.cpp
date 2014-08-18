/******************************************************************************
 *
 * $Id$
 *
 * Project:  Optimization Modeling of Fire Fighter Resources
 * Purpose:  C API for IRS
 * Author:   Kyle Shannon <kyle@pobox.com>
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

#include "irs.h"

IRS_C_START

    IRS_API IRSSuiteH IRSCreate( char *pszDataPath, char **papszOptions )
    {
        return (IRSSuiteH)(new IRSSuite( pszDataPath, papszOptions ));
    }

    IRS_API int IRSSetSpatialFilter( IRSSuiteH hSuite, const char *pszWktGeometry )
    {
        return ((IRSSuite*)(hSuite))->SetSpatialFilter( pszWktGeometry );
    }

    IRS_API int SetFpuFilter( IRSSuiteH hSuite, const char *pszFpuCode )
    {
        return ((IRSSuite)(hSuite))->SetFpuFilter( pszFpuCode );
    }
    IRS_API int SetExternalResourceDb( IRSSuiteH hSuite, const char *pszPath )
    {
        return 0;
    }
    IRS_API int IRSRunScenario( IRSSuiteH hSuite, int nYearIndex )
    {
        return ((IRSSuite)(hSuite))->RunScenario( nYearIndex );
    }

    IRS_API int IRSRunScenarios( IRSSuiteH hSuite, int nYearCount )
    {
        return ((IRSSuite*)(hSuite))->RunScenarios( nYearCount );
    }

    IRS_API int IRSRunAllScenarios( IRSSuiteH hSuite )
    {
        return ((IRSSuite*)(hSuite))->RunAllScenarios();
    }

    IRS_API int IRSDestroy( IRSSuiteH hSuite )
    {
        delete (IRSSuite*)hSuite;
        return 0;
    }

#endif /* IRS_SUITE_H_ */

IRS_C_END

