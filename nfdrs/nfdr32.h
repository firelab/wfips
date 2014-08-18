/******************************************************************************
 *
 * $Id: nfdr32.h 79 2012-07-26 15:36:43Z kyle.shannon $
 *
 * Project:  NFDRS library
 * Purpose:
 * Author:   Stu Britan <stu at fire dot org>
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

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
/**************************** NFDRDLL.H ***********************************

First Letter of Variable or Function Name:

        i = integer (2-byte)
   f = doubleing Point

Variable Names Used in Function Calls:

        iAdjust                 <desired +/- addition to 1-hour fuel moisture>
        iAspect         1=N, 2=E, 3=S, 4=W
   iBI                          Burning Index
   fDEPTH                       Fuel Model's depth
   iDoHerbXfer          1=Transfer (appropriate amt of) Herb Load to 1Hr, 0=No
   fERC                         Energy Release Component
   iExposure            1=Exposed, 0=Not
   iFIL                         Fire Intensity Level
        fFL                             Flame Length
   iFM                          ASCII Value of Fuel Model ID:  65=A, 66=B, ...
   iHD                          Fuel Model's Dead (& Live) Heat of Combustion
   iHour                                1, 2, ... 24
   iIC                          Ignition Component
   iJulian                      Annualized Julian:  1=Jan1,... , 365=Dec31
   iLat                         Nearest Degree of Latitude
   iMaxRH                       24-Hour Max RH (-999 means missing)
   iMaxTemp                     24-Hour Max Temp (F) (-999 means missing)
   fMCn                         Fuel Moistures for fuel class "n"
   iMinRH                       24-Hour Min RH (-999 means missing)
   iMinTemp                     24-Hour Min Temp (F) (-999 means missing)
        iMonth                  1=Jan, ... 12=Dec
   iMXD                         Fuel Model's Moisture of Extinction (%)
   iNFMAS                       1=Use NFMAS Variations, 0=No (Use standard)
   iRH                          Observed RH
   fROS                         Rate of Spread
   iSAn                         Fuel Model's Surface/Area for fuel class "n"
   iSC                          Spread Component
   iSCM                         Fuel Model's Spread At Which All Ignitiona -> Reportable
   iSite          1=Low, 2=Mid, 3=Upper
   iSlope30       1=30% or less, 2=greater than 30%
   iSlopeCls            NFDRS Slope Class (1-5)
   iSOW                         State of Weather (0=Clear, ...) (-1 allowed in iCalcIC)
   iTemp                                Observed Temp (F) (Can be fuel temp in iCalcIC)
   fWn                          Fuel Model's Loading for fuel class "n"
   fWNDFC                       Fuel Model's (or desired) Wind Reduction Factor
   iWS                          20-Foot Windspeed


Functions:

        Keywords in function names:

        Calc            Performs a calculation, returns value(s)
        Do                      Performs an action
        Get             Returns values from the DLL
      Set               Sets values in the DLL

        Some functions return calculated values. Most, especially integer
   typed functions, return "success status": 1=Success, 0=Failure.

   Most requested values are returned as function arguments.

Visual BASIC Notes:

        integer used throughout, to be compatable with VB Integer Type.
   Strings Avoided to prevent calling issues between VB/C.

   In VB, use "ByVal" unless pointer (*) in DLL.

   Example DECLARE statements and calling conventions:

        DECLARE FUNCTION iSetFuelModel LIB "NFDRCALC.DLL"
                (ByVal intFuelMod) as integer

      DECLARE FUNCTION iGetFuelModel LIB "NFDRCALC.DLL"
                (intFuelModel) as integer

      Dim strFM as string
      dim intFM as integer
      dim intReturn as integer

      strFM = "A"
      intFM = ASC(strFM)
      intReturn = iSetFuelModel (intFM)
      if intReturn = 0 then
        msgbox "Invalid Fuel Model"
      end if

      ...

      intReturn = iGetFuelModel (intFM)
      if intReturn = 0 then
        msgbox "No Fuel Model Loaded Into DLL."
      else
        msgbox "Current Fuel Model Is " & CHR$(intFM)
      end if

***************************************************************************/
#ifndef max
#define max(a, b) (a > b ? a : b)
#endif
#ifndef min
#define min(a, b) (a < b ? a : b)
#endif

class NFDRCalcState
{
public:
        //NFDRCCalcState();
        void Copy(NFDRCalcState *state);
//private:
        int StartKBDI;
        double Hist1000[7];
        double HistBndryT[7];
        int FuelModel;
        int SG1, SG10, SG100, SG1000, SGWOOD, SGHERB;
        double W1, W10, W100, W1000, WWOOD, WHERB, WDROUGHT, W1P, WHERBP;
        double L1, L10, L100, L1000, LWOOD, LHERB, LDROUGHT;
        double DEPTH, WNDFC, WNDFCMax, WNDFCMin, WNDFCSet;
        int MXD, HD, SCM;

        double MC1, MC10, MC100, MC1000, MCWOOD, MCHERB;

        double CTA;

        double X1000, YX1000, Y1000, Y100, Gren, Start100, Start1000;
        int PrevStage, PrevYear, PrevJulian, HerbAnnual, GreenDays, ClimateClass;
        int ColdDays, JulianGreenUp, JulianStartedGreen;
        int JulianLookFreeze, Lat, Stage, OneIsTen;
        int WoodyDeciduous, NFDRSVersion;
        int Season, KBDI, YKBDI, GreenHerb, GreenWoody, RainEvent;
        double PrevPrecip, CummPrecip, AvgPrecip;
        int SOW;
        bool valid;
};



class NFDRCalc
{
public:
        void SetVersion(int version);
        NFDRCalc();
        ~NFDRCalc();
/* iCalcAdjustMC1 ********************************************************
Returns adjustment to 1-hr fuel moisture. No adjustment to the 1-hr
fuel moisture is made. (Use iDoAdjustMC1).
**************************************************************************/
        int iCalcAdjustMC1 (int iMonth, int iExposure, int iAspect, int iSite,
            int iSlope30, int iHour);
/* iCalcIC **************************************************************
Calculates Ignition Component, based on air temp, SOW, SC, and
1-hour fuel moisture. Supply SOW = -1 to indicate that temperature is
already adjusted to the fuel, and need not be adjusted based on SOW.
Fuel Bed parameter SCM must have been previously set. Returns success status.
***********************************************************************/
        //int iCalcIC(int iTemp, int iSOW, double fMC1, int iSC, int* iIC);//   CHANGED SB
        int iCalcIC(int iTemp, int iSOW, double fMC1, double fROS, int* iIC);

/* iCalcIndexes ********************************************************
Calculates and provides NFDRS indexes, based on previously-loaded fuel bed
and fuel moistures, plus user-supplied Wind/Slope/Transfer. Returns
success status.
*************************************************************************/
        int iCalcIndexes (int iWS, int iSlopeCls,
                                        double* fROS, int* iSC,
                                        double* fERC, double* fFL, int* iFIL,
               int* iBI);


/* fCalculate DaylightHours ***********************************************
Returns number of hours of daylight.
***************************************************************************/
        double fCalcDaylightHours (int iLat, int iJulian);



/* iDoAdjustMC1 **********************************************************
Adds iAdjust to the internal variable storing 1-hr moisture content.
Note:  User can easily perform this externally prior to
passing fuel moistures to the DLL. Returns success status.
*************************************************************************/
        int iDoAdjustMC1 (int iAdjust);



/* iGetFuelBed **********************************************************
Retrieves fuel bed characteristics. Returns success status.
************************************************************************/
        int iGetFuelBed (int* iSA1, int* iSA10,
                                int* iSA100, int* iSA1000, int* iSAWOOD,
            int* iSAHERB, double* fW1, double* fW10,
            double* fW100, double* fW1000, double* fWWOOD, double* fWHERB,
            double* fWDROUGHT,
            double* fDEPTH, int* iMXD, int* iHD,
            int* iSCM, double* fWNDFC,
            double* fWNDFCMin, double* fWNDFCMax);


/* iGetFuelModel ********************************************************
Retrieve fuel model most recently supplied by iSetFuelModel. Returns
0 if either no fuel model has been set OR if user has supplied custom
fuel model by calling iSetFuelBed.
************************************************************************/
        int iGetFuelModel (int* iFM);


/* iSetFuelBed ***********************************************************
Sets fuel bed characteristics. Returns success status.
*************************************************************************/
int iSetFuelBed (int iSA1, int iSA10,
                                int iSA100, int iSA1000, int iSAWOOD,
            int iSAHERB, double fW1, double fW10,
            double fW100, double fW1000, double fWWOOD, double fWHERB,
            double fWDROUGHT,
            double fDEPTH, int iMXD, int iHD,
            int iSCM, double fWNDFC,
            double fWNDFCMin, double fWNDFCMax);


/* iSetFuelModel ********************************************************
Loads fuel bed characteristics for user-specified NFDRS fuel model.
*************************************************************************/
        int iSetFuelModel (int iFM, int iNFMAS);


/* iSetInitMoist *********************************************************
Sets values needed in order to make fuel moisture calculations. Returns
success status.
*************************************************************************/
        //int iSetInitMoist (int iClimateCls,
        //                              int iLat, int iOneIsTen);


/* iSetMoistures **********************************************************
Sets internal fuel moistures to be those provided by user. Returns
success status.
**************************************************************************/
        int iSetMoistures (double fMC1, double fMC10,
                                        double fMC100, double fMC1000, double fMCWood, double fMCHerb,
               int iRainEvent, int iKBDI, int iGreenHerb,
               int iGreenWoody, int iSeason, int iSOW);

        int GetMoistures (double& fMC1, double& fMC10, double& fMC100, double& fMC1000, 
                double& fMCWood, double& fMCHerb, int& iRainEvent, int& iKBDI, double& fX1000, int& iGreenHerb,
               int& iGreenWoody, int& iSeason, int& iSOW);

        int iInitialize (int iNFDRSVersion,
                        int iClimateClass,
                        int iJulianLookFreeze, int iJulianGreenUp,
         int iLat, int iHerbAnnual,
         int iWoodyDeciduous, int iOneIsTen,
         double fStart100, double fStart1000,
         int iStartKBDI, double fAvgPrecip);

        int iSetAnnualInit(int iJulianGreenUp, double fStart1000, int iStartKBDI);

        int iCalcMoist (int iInit,
                                        int iTemp, int iRH,
                                        int iMaxTemp, int iMaxRH,
               int iMinTemp, int iMinRH,
               int iPrecipDur, double fPrecipAmt,
               int iSOW, double f10stick,
                    int iJulian, int iYear,
                                        int iDeclareGreenUp, int iDeclareFreeze,
               int iGreenHerb, int iGreenWoody,
               int iSeason,
                double* f1, double* f10,
               double* f100, double* f1000,
               double* fHerb, double* fWood,
               int* iStage, double* fGren, double* fX1000,
               int* iRainEvent, int* iKBDI);
        //overloaded version to implement wetflag
        int iCalcMoistWetFlag (int iInit,
                                        int iTemp, int iRH,
                                        int iMaxTemp, int iMaxRH,
               int iMinTemp, int iMinRH,
               int iPrecipDur, double fPrecipAmt,
               int iSOW, double f10stick,
                    int iJulian, int iYear,
                                        int iDeclareGreenUp, int iDeclareFreeze,
               int iGreenHerb, int iGreenWoody,
               int iSeason, int iWetFlag,
                double* f1, double* f10,
               double* f100, double* f1000,
               double* fHerb, double* fWood,
               int* iStage, double* fGren, double* fX1000,
               int* iRainEvent, int* iKBDI);
        int iCalcKBDI (double fPrecipAmt, int iMaxTemp,
                                double fCummPrecip, int iYKBDI, double fAvgPrecip);
        int SetHists(double h1000s[], double hBndrys[]);
        int GetHists(double h1000s[], double hBndrys[]);
/* SaveState **********************************************************
Saves all internal variables to a data structure
**************************************************************************/
        int SaveState(NFDRCalcState *state);
/* LoadState **********************************************************
Loads a previously saved data structure
**************************************************************************/
        int LoadState(NFDRCalcState *state);

/* GetL1 ***************************************************************
Retrieves 1 Hour Loading
**************************************************************************/
        double GetL1();
/* GetL10 ***************************************************************
Retrieves 10 Hour Loading
**************************************************************************/
        double GetL10();
/* GetL100 ***************************************************************
Retrieves 100 Hour Loading
**************************************************************************/
        double GetL100();
/* GetL1000 ***************************************************************
Retrieves 1000 Hour Loading
**************************************************************************/
        double GetL1000();
/* GetLHerb ***************************************************************
Retrieves Herbaceous Loading
**************************************************************************/
        double GetLHerb();
/* GetLWood ***************************************************************
Retrieves Woody Hour Loading
**************************************************************************/
        double GetLWood();
/* GetLDrought ***************************************************************
Retrieves Drought Loading
**************************************************************************/
        double GetLDrought();
/* GetSG1 ***************************************************************
Retrieves 1 Hour Surface Area to Volume Ratio
**************************************************************************/
        int GetSG1();
/* GetSG10 ***************************************************************
Retrieves 10 Hour Surface Area to Volume Ratio
**************************************************************************/
        int GetSG10();
/* GetSG100 ***************************************************************
Retrieves 100 Hour Surface Area to Volume Ratio
**************************************************************************/
        int GetSG100();
/* GetSG1000 ***************************************************************
Retrieves 1000 Hour Surface Area to Volume Ratio
**************************************************************************/
        int GetSG1000();
/* GetSGHerb ***************************************************************
Retrieves Herb Surface Area to Volume Ratio
**************************************************************************/
        int GetSGHerb();
/* GetSGWood ***************************************************************
Retrieves Woody Surface Area to Volume Ratio
**************************************************************************/
        int GetSGWood();
/* GetHD ***************************************************************
Retrieves Heating Number
**************************************************************************/
        int GetHD();
/* GetMXD ***************************************************************
Retrieves Moisture Extinction
**************************************************************************/
        int GetMXD();
/* GetSCM ***************************************************************
Retrieves Max Spread Component
**************************************************************************/
        int GetSCM();
/* GetWindFC ***************************************************************
Retrieves Wind Factor
**************************************************************************/
        double GetWindFC();
/* GetDepth ***************************************************************
Retrieves Depth
**************************************************************************/
        double GetDepth();

/*iSetJulianShrubDormant(int iJulianDormant) */
//sets date for setting herbacious shrubs into dormant state
int iSetJulianShrubDormant(int iJulianDormant);


private:

        double eqmc (double fTemp, double fRH);
        int irnd(double fIn);

        int oneten (int iTemp, int iRH, int iSOW,
                                double f10stick, int iWetFlag);

        int hundredthous (int iTemp, int iRH, int iMaxTemp,
                                        int iMaxRH, int iMinTemp, int iMinRH,
               int iJulian,
               int iPrecipDur, double fPrecipAmt, int iSOW, int iWetFlag);


        double satvap (int iTemp);

        double XferHerb (double fMCHerb);
        double fCalcX1000 (int iMaxTemp, int iMinTemp);

        int Cure (int iJulian, int iYear,
                                 int iTemp, int iMaxTemp, int iMinTemp,
             int iDeclareGreenUp, int iDeclareFreeze);

        int InitFM(void);

//variables
        int StartKBDI;
        double Hist1000[7];
        double HistBndryT[7];
        //double HistX1000[7];
        int FuelModel;
        int SG1, SG10, SG100, SG1000, SGWOOD, SGHERB;
        double W1, W10, W100, W1000, WWOOD, WHERB, WDROUGHT, W1P, WHERBP;
        double L1, L10, L100, L1000, LWOOD, LHERB, LDROUGHT;
        double DEPTH, WNDFC, WNDFCMax, WNDFCMin, WNDFCSet;
        int MXD, HD, SCM;

        double MC1, MC10, MC100, MC1000, MCWOOD, MCHERB;

        double CTA;

        double X1000, YX1000, Y1000, Y100, Gren, Start100, Start1000;
        int PrevStage, PrevYear, PrevJulian, HerbAnnual, GreenDays, ClimateClass, PrevSeason;
        int ColdDays, JulianGreenUp, JulianStartedGreen;
        int JulianLookFreeze, Lat, Stage, OneIsTen;
        int WoodyDeciduous, NFDRSVersion;
        int Season, KBDI, YKBDI, GreenHerb, GreenWoody, RainEvent;
        double PrevPrecip, CummPrecip, AvgPrecip;
        int SOW;
        int JulianShrubDormant;
        int hasGreenedupThisYear;

};





