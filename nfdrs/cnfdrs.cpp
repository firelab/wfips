/******************************************************************************
 *
 * $Id: cnfdrs.cpp 79 2012-07-26 15:36:43Z kyle.shannon $
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
// CNFDRS.cpp : Defines the entry point for the DLL application.
//
#include <stdlib.h>
#include <stdio.h>

#include "cnfdrs.h"
#include "nfdr32.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

#ifdef NOT_DEFINED
bool APIENTRY DllMain( HMODULE hModule,
        DWORD  ul_reason_for_call,
        LPVOID lpReserved
        )
{
    return TRUE;
}
#endif

#ifdef _MANAGED
#pragma managed(pop)
#endif

CNFDRS::CNFDRS()
{
    m_nfdrs = new NFDRCalc();
}

CNFDRS::~CNFDRS()
{
    if(m_nfdrs)
        delete m_nfdrs;
}

double CNFDRS::fCalcDaylightHours(short iLat, short iJulian)
{
    if(m_nfdrs)
        return m_nfdrs->fCalcDaylightHours(iLat, iJulian);
    return -1;
}
short CNFDRS::iCalcIndexes(short iWS, short iSlopeCls, double * fROS, int * iSC, double * fERC, 
        double * fFL, int * iFIL, int * iBI)
{
    if(m_nfdrs)
        return m_nfdrs->iCalcIndexes(iWS, iSlopeCls, fROS, iSC, fERC, fFL, iFIL, iBI);
    return -1;
}
short CNFDRS::iCalcAdjustMC1(short iMonth, short iExposure, short iAspect, short iSite, short iSlope30, short iHour)
{
    if(m_nfdrs)
        return m_nfdrs->iCalcAdjustMC1(iMonth, iExposure, iAspect, iSite, iSlope30, iHour);
    return -1;
}
short CNFDRS::iCalcIC(short iTemp, short iSOW, double fMC1, double fROS, int * iIC)
{
    if(m_nfdrs)
        return m_nfdrs->iCalcIC(iTemp, iSOW, fMC1, fROS, iIC);
    return -1;
}
short CNFDRS::iDoAdjustMC1(short iAdjust)
{
    if(m_nfdrs)
        return m_nfdrs->iDoAdjustMC1(iAdjust);
    return -1;
}
short CNFDRS::iGetFuelBed(int * iSA1, int * iSA10, int * iSA100, int * iSA1000, 
        int * iSAWOOD, int * iSAHERB, double * fW1, double * fW10, double * fW100, 
        double * fW1000, double * fWWOOD, double * fWHERB, double * fWDROUGHT, double * fDEPTH, 
        int * iMXD, int * iHD, int * iSCM, double * fWNDFC, double * fWNDFCMin, double * fWNDFCMax)
{
    if(m_nfdrs)
        return m_nfdrs->iGetFuelBed(iSA1, iSA10, iSA100, iSA1000, 
                iSAWOOD, iSAHERB, fW1, fW10, fW100, 
                fW1000, fWWOOD, fWHERB, fWDROUGHT, fDEPTH, 
                iMXD, iHD, iSCM, fWNDFC, fWNDFCMin, fWNDFCMax);
    return -1;
}
short CNFDRS::iGetFuelModel(int * iFM)
{
    if(m_nfdrs)
        return m_nfdrs->iGetFuelModel(iFM);
    return -1;
}
short CNFDRS::iSetFuelBed(short iSA1, short iSA10, short iSA100, short iSA1000, short iSAWOOD, short iSAHERB, 
        double fW1, double fW10, double fW100, double fW1000, double fWWOOD, double fWHERB, double fWDROUGHT, 
        double fWDEPTH, short iMXD, short iHD, short iSCM, double fWNDFC, double fWNDFCMin, double fWNDFCMax)
{
    if(m_nfdrs)
        return m_nfdrs->iSetFuelBed(iSA1, iSA10, iSA100, iSA1000, iSAWOOD, iSAHERB, 
                fW1, fW10, fW100, fW1000, fWWOOD, fWHERB, fWDROUGHT, 
                fWDEPTH, iMXD, iHD, iSCM, fWNDFC, fWNDFCMin, fWNDFCMax);
    return -1;
}
short CNFDRS::iSetFuelModel(short iFM, short iNFMAS)
{
    if(m_nfdrs)
        return m_nfdrs->iSetFuelModel(iFM, iNFMAS);
    return -1;
}
short CNFDRS::iSetMoistures(double fMC1, double fMC10, double fMC100, double fMC1000, double fMCWood, double fMCHerb, 
        short iRainEvent, short iKBDI, short iGreenHerb, short iGreenWoody, short iSeason, short iSOW)
{
    if(m_nfdrs)
        return m_nfdrs->iSetMoistures(fMC1, fMC10, fMC100, fMC1000, fMCWood, fMCHerb, 
                iRainEvent, iKBDI, iGreenHerb, iGreenWoody, iSeason, iSOW);
    return -1;
}
short CNFDRS::iInitialize(short iNFDRSVersion, short iClimateClass, short iJulianLookFreeze, short iJulianGreenUp, 
        short iLat, short iHerbAnnual, short iWoodyDeciduous, short iOneIsTen, double fStart100, double fStart1000, 
        short iStartKBDI, double fAvgPrecip)
{
    if(m_nfdrs)
        return m_nfdrs->iInitialize(iNFDRSVersion, iClimateClass, iJulianLookFreeze, iJulianGreenUp, 
                iLat, iHerbAnnual, iWoodyDeciduous, iOneIsTen, fStart100, fStart1000, 
                iStartKBDI, fAvgPrecip);
    return -1;
}
short CNFDRS::iCalcMoist(short iInit, short iTemp, short iRH, short iMaxTemp, short iMaxRH, short iMinTemp, 
        short iMinRH, short iPrecipDur, double fPrecipAmt, short iSOW, double f10stick, short iJulian, short iYear, 
        short iDeclareGreenUp, short iDeclareFreeze, short iGreenHerb, short iGreenWoody, short iSeason, double * f1, 
        double * f10, double * f100, double * f1000, double * fHerb, double * fWood, int * iStage, 
        double * fGren, double * fX1000, int * iRainEvent, int * iKBDI)
{
    if(m_nfdrs)
        return m_nfdrs->iCalcMoist(iInit, iTemp, iRH, iMaxTemp, iMaxRH, iMinTemp, 
                iMinRH, iPrecipDur, fPrecipAmt, iSOW, f10stick, iJulian, iYear, 
                iDeclareGreenUp, iDeclareFreeze, iGreenHerb, iGreenWoody, iSeason, f1, 
                f10, f100, f1000, fHerb, fWood, iStage, 
                fGren, fX1000, iRainEvent, iKBDI);
    return -1;
}
short CNFDRS::iCalcKBDI(double fPrecipAmt, short iMaxTemp, double fCummPrecip, short iYKBDI, double fAvgPrecip)
{
    if(m_nfdrs)
        return m_nfdrs->iCalcKBDI(fPrecipAmt, iMaxTemp, fCummPrecip, iYKBDI, fAvgPrecip);
    return -1;
}
short CNFDRS::LoadState(LPCSTR fileName)
{
    int ret = 0;
    if(m_nfdrs)
    {

        NFDRCalcState state;
        FILE *in = fopen(fileName, "rt");
        if(!in)
            return -1;
        char buf[640];
        if(fgets(buf, 639, in))
            state.PrevJulian = atoi(buf);
        else
            ret = 1;
        if(!ret && fgets(buf, 639, in))
            state.PrevYear = atoi(buf);
        else
            if(!ret)
                ret = 2;
        if(!ret && fgets(buf, 639, in))
            state.AvgPrecip = atof(buf);
        else
            if(!ret)
                ret = 3;
        if(!ret && fgets(buf, 639, in))
            state.ClimateClass = atoi(buf);
        else
            if(!ret)
                ret = 4;
        if(!ret && fgets(buf, 639, in))
            state.ColdDays = atoi(buf);
        else
            if(!ret)
                ret = 5;
        if(!ret && fgets(buf, 639, in))
            state.CTA = atof(buf);
        else
            if(!ret)
                ret = 6;
        if(!ret && fgets(buf, 639, in))
            state.CummPrecip = atof(buf);
        else
            if(!ret)
                ret = 7;
        if(!ret && fgets(buf, 639, in))
            state.DEPTH = atof(buf);
        else
            if(!ret)
                ret = 8;
        if(!ret && fgets(buf, 639, in))
            state.FuelModel = atoi(buf);
        else
            if(!ret)
                ret = 9;
        if(!ret && fgets(buf, 639, in))
            state.GreenDays = atoi(buf);
        else
            if(!ret)
                ret = 10;
        if(!ret && fgets(buf, 639, in))
            state.GreenHerb = atoi(buf);
        else
            if(!ret)
                ret = 11;
        if(!ret && fgets(buf, 639, in))
            state.GreenWoody = atoi(buf);
        else
            if(!ret)
                ret = 12;
        if(!ret && fgets(buf, 639, in))
            state.Gren = atof(buf);
        else
            if(!ret)
                ret = 13;
        if(!ret && fgets(buf, 639, in))
            state.HD = atoi(buf);
        else
            if(!ret)
                ret = 14;
        if(!ret && fgets(buf, 639, in))
            state.HerbAnnual = atoi(buf);
        else
            if(!ret)
                ret = 15;
        for(int i = 0; i < 7; i++)
        {
            if(!ret && fgets(buf, 639, in))
                state.Hist1000[i] = atof(buf);
            else
                if(!ret)
                    ret = 16 + i;
        }
        for(int i = 0; i < 7; i++)
        {
            if(!ret && fgets(buf, 639, in))
                state.HistBndryT[i] = atof(buf);
            else
                if(!ret)
                    ret = 23 + i;
        }
        if(!ret && fgets(buf, 639, in))
            state.JulianGreenUp = atoi(buf);
        else
            if(!ret)
                ret = 30;
        if(!ret && fgets(buf, 639, in))
            state.JulianLookFreeze = atoi(buf);
        else
            if(!ret)
                ret = 31;
        if(!ret && fgets(buf, 639, in))
            state.JulianStartedGreen = atoi(buf);
        else
            if(!ret)
                ret = 32;
        if(!ret && fgets(buf, 639, in))
            state.KBDI = atoi(buf);
        else
            if(!ret)
                ret = 33;
        if(!ret && fgets(buf, 639, in))
            state.L1 = atof(buf);
        else
            if(!ret)
                ret = 34;
        if(!ret && fgets(buf, 639, in))
            state.L10 = atof(buf);
        else
            if(!ret)
                ret = 35;
        if(!ret && fgets(buf, 639, in))
            state.L100 = atof(buf);
        else
            if(!ret)
                ret = 36;
        if(!ret && fgets(buf, 639, in))
            state.L1000 = atof(buf);
        else
            if(!ret)
                ret = 37;
        if(!ret && fgets(buf, 639, in))
            state.Lat = atoi(buf);
        else
            if(!ret)
                ret = 38;
        if(!ret && fgets(buf, 639, in))
            state.LDROUGHT = atof(buf);
        else
            if(!ret)
                ret = 39;
        if(!ret && fgets(buf, 639, in))
            state.LHERB = atof(buf);
        else
            if(!ret)
                ret = 40;
        if(!ret && fgets(buf, 639, in))
            state.LWOOD = atof(buf);
        else
            if(!ret)
                ret = 41;
        if(!ret && fgets(buf, 639, in))
            state.MC1 = atof(buf);
        else
            if(!ret)
                ret = 42;
        if(!ret && fgets(buf, 639, in))
            state.MC10 = atof(buf);
        else
            if(!ret)
                ret = 43;
        if(!ret && fgets(buf, 639, in))
            state.MC100 = atof(buf);
        else
            if(!ret)
                ret = 44;
        if(!ret && fgets(buf, 639, in))
            state.MC1000 = atof(buf);
        else
            if(!ret)
                ret = 45;
        if(!ret && fgets(buf, 639, in))
            state.MCHERB = atof(buf);
        else
            if(!ret)
                ret = 46;
        if(!ret && fgets(buf, 639, in))
            state.MCWOOD = atof(buf);
        else
            if(!ret)
                ret = 47;
        if(!ret && fgets(buf, 639, in))
            state.MXD = atoi(buf);
        else
            if(!ret)
                ret = 48;
        if(!ret && fgets(buf, 639, in))
            state.NFDRSVersion = atoi(buf);
        else
            if(!ret)
                ret = 49;
        if(!ret && fgets(buf, 639, in))
            state.OneIsTen = atoi(buf);
        else
            if(!ret)
                ret = 50;
        if(!ret && fgets(buf, 639, in))
            state.PrevPrecip = atof(buf);
        else
            if(!ret)
                ret = 51;
        if(!ret && fgets(buf, 639, in))
            state.PrevStage = atoi(buf);
        else
            if(!ret)
                ret = 52;
        if(!ret && fgets(buf, 639, in))
            state.RainEvent = atoi(buf);
        else
            if(!ret)
                ret = 53;
        if(!ret && fgets(buf, 639, in))
            state.SCM = atoi(buf);
        else
            if(!ret)
                ret = 54;
        if(!ret && fgets(buf, 639, in))
            state.Season = atoi(buf);
        else
            if(!ret)
                ret = 55;
        if(!ret && fgets(buf, 639, in))
            state.SG1 = atoi(buf);
        else
            if(!ret)
                ret = 56;
        if(!ret && fgets(buf, 639, in))
            state.SG10 = atoi(buf);
        else
            if(!ret)
                ret = 57;
        if(!ret && fgets(buf, 639, in))
            state.SG100 = atoi(buf);
        else
            if(!ret)
                ret = 58;
        if(!ret && fgets(buf, 639, in))
            state.SG1000 = atoi(buf);
        else
            if(!ret)
                ret = 59;
        if(!ret && fgets(buf, 639, in))
            state.SGHERB = atoi(buf);
        else
            if(!ret)
                ret = 60;
        if(!ret && fgets(buf, 639, in))
            state.SGWOOD = atoi(buf);
        else
            if(!ret)
                ret = 61;
        if(!ret && fgets(buf, 639, in))
            state.SOW = atoi(buf);
        else
            if(!ret)
                ret = 62;
        if(!ret && fgets(buf, 639, in))
            state.Stage = atoi(buf);
        else
            if(!ret)
                ret = 63;
        if(!ret && fgets(buf, 639, in))
            state.Start100 = atof(buf);
        else
            if(!ret)
                ret = 64;
        if(!ret && fgets(buf, 639, in))
            state.Start1000 = atof(buf);
        else
            if(!ret)
                ret = 65;
        if(!ret && fgets(buf, 639, in))
            state.StartKBDI = atoi(buf);
        else
            if(!ret)
                ret = 66;
        if(!ret && fgets(buf, 639, in))
            state.valid = (atoi(buf) == 0) ? false : true;
        else
            if(!ret)
                ret = 67;
        if(!ret && fgets(buf, 639, in))
            state.W1 = atof(buf);
        else
            if(!ret)
                ret = 68;
        if(!ret && fgets(buf, 639, in))
            state.W10 = atof(buf);
        else
            if(!ret)
                ret = 69;
        if(!ret && fgets(buf, 639, in))
            state.W100 = atof(buf);
        else
            if(!ret)
                ret = 70;
        if(!ret && fgets(buf, 639, in))
            state.W1000 = atof(buf);
        else
            if(!ret)
                ret = 71;
        if(!ret && fgets(buf, 639, in))
            state.W1P = atof(buf);
        else
            if(!ret)
                ret = 72;
        if(!ret && fgets(buf, 639, in))
            state.WDROUGHT = atof(buf);
        else
            if(!ret)
                ret = 73;
        if(!ret && fgets(buf, 639, in))
            state.WHERB = atof(buf);
        else
            if(!ret)
                ret = 74;
        if(!ret && fgets(buf, 639, in))
            state.WHERBP = atof(buf);
        else
            if(!ret)
                ret = 75;
        if(!ret && fgets(buf, 639, in))
            state.WNDFC = atof(buf);
        else
            if(!ret)
                ret = 76;
        if(!ret && fgets(buf, 639, in))
            state.WNDFCMax = atof(buf);
        else
            if(!ret)
                ret = 77;
        if(!ret && fgets(buf, 639, in))
            state.WNDFCMin = atof(buf);
        else
            if(!ret)
                ret = 78;
        if(!ret && fgets(buf, 639, in))
            state.WNDFCSet = atof(buf);
        else
            if(!ret)
                ret = 79;
        if(!ret && fgets(buf, 639, in))
            state.WoodyDeciduous = atoi(buf);
        else
            if(!ret)
                ret = 80;
        if(!ret && fgets(buf, 639, in))
            state.WWOOD = atof(buf);
        else
            if(!ret)
                ret = 81;
        if(!ret && fgets(buf, 639, in))
            state.X1000 = atof(buf);
        else
            if(!ret)
                ret = 82;
        if(!ret && fgets(buf, 639, in))
            state.Y100 = atof(buf);
        else
            if(!ret)
                ret = 83;
        if(!ret && fgets(buf, 639, in))
            state.Y1000 = atof(buf);
        else
            if(!ret)
                ret = 84;
        if(!ret && fgets(buf, 639, in))
            state.YKBDI = atoi(buf);
        else
            if(!ret)
                ret = 85;
        if(!ret && fgets(buf, 639, in))
            state.YX1000 = atof(buf);
        else
            if(!ret)
                ret = 86;       
        fclose(in);
        if(!ret)
        {
            return m_nfdrs->LoadState(&state);
        }
        return ret;
    }
    return -1;
}
short CNFDRS::SaveState(LPCSTR fileName)
{
    if(m_nfdrs)
    {
        FILE *out = fopen(fileName, "wt");
        if(!out)
            return -1;
        NFDRCalcState state;
        m_nfdrs->SaveState(&state);
        fprintf(out, "%d\n", state.PrevJulian);
        fprintf(out, "%d\n", state.PrevYear);
        fprintf(out, "%f\n", state.AvgPrecip);
        fprintf(out, "%d\n", state.ClimateClass);
        fprintf(out, "%d\n", state.ColdDays);
        fprintf(out, "%f\n", state.CTA);
        fprintf(out, "%f\n", state.CummPrecip);
        fprintf(out, "%f\n", state.DEPTH);
        fprintf(out, "%d\n", state.FuelModel);
        fprintf(out, "%d\n", state.GreenDays);
        fprintf(out, "%d\n", state.GreenHerb);
        fprintf(out, "%d\n", state.GreenWoody);
        fprintf(out, "%f\n", state.Gren);
        fprintf(out, "%d\n", state.HD);
        fprintf(out, "%d\n", state.HerbAnnual);
        for(int i = 0; i < 7; i++)
            fprintf(out, "%f\n", state.Hist1000[i]);
        for(int i = 0; i < 7; i++)
            fprintf(out, "%f\n", state.HistBndryT[i]);
        fprintf(out, "%d\n", state.JulianGreenUp);
        fprintf(out, "%d\n", state.JulianLookFreeze);
        fprintf(out, "%d\n", state.JulianStartedGreen);
        fprintf(out, "%d\n", state.KBDI);
        fprintf(out, "%f\n", state.L1);
        fprintf(out, "%f\n", state.L10);
        fprintf(out, "%f\n", state.L100);
        fprintf(out, "%f\n", state.L1000);
        fprintf(out, "%d\n", state.Lat);
        fprintf(out, "%f\n", state.LDROUGHT);
        fprintf(out, "%f\n", state.LHERB);
        fprintf(out, "%f\n", state.LWOOD);
        fprintf(out, "%f\n", state.MC1);
        fprintf(out, "%f\n", state.MC10);
        fprintf(out, "%f\n", state.MC100);
        fprintf(out, "%f\n", state.MC1000);
        fprintf(out, "%f\n", state.MCHERB);
        fprintf(out, "%f\n", state.MCWOOD);
        fprintf(out, "%d\n", state.MXD);
        fprintf(out, "%d\n", state.NFDRSVersion);
        fprintf(out, "%d\n", state.OneIsTen);
        fprintf(out, "%f\n", state.PrevPrecip);
        fprintf(out, "%d\n", state.PrevStage);
        fprintf(out, "%d\n", state.RainEvent);
        fprintf(out, "%d\n", state.SCM);
        fprintf(out, "%d\n", state.Season);
        fprintf(out, "%d\n", state.SG1);
        fprintf(out, "%d\n", state.SG10);
        fprintf(out, "%d\n", state.SG100);
        fprintf(out, "%d\n", state.SG1000);
        fprintf(out, "%d\n", state.SGHERB);
        fprintf(out, "%d\n", state.SGWOOD);
        fprintf(out, "%d\n", state.SOW);
        fprintf(out, "%d\n", state.Stage);
        fprintf(out, "%f\n", state.Start100);
        fprintf(out, "%f\n", state.Start1000);
        fprintf(out, "%d\n", state.StartKBDI);
        fprintf(out, "%d\n", state.valid ? 1 : 0);
        fprintf(out, "%f\n", state.W1);
        fprintf(out, "%f\n", state.W10);
        fprintf(out, "%f\n", state.W100);
        fprintf(out, "%f\n", state.W1000);
        fprintf(out, "%f\n", state.W1P);
        fprintf(out, "%f\n", state.WDROUGHT);
        fprintf(out, "%f\n", state.WHERB);
        fprintf(out, "%f\n", state.WHERBP);
        fprintf(out, "%f\n", state.WNDFC);
        fprintf(out, "%f\n", state.WNDFCMax);
        fprintf(out, "%f\n", state.WNDFCMin);
        fprintf(out, "%f\n", state.WNDFCSet);
        fprintf(out, "%d\n", state.WoodyDeciduous);
        fprintf(out, "%f\n", state.WWOOD);
        fprintf(out, "%f\n", state.X1000);
        fprintf(out, "%f\n", state.Y100);
        fprintf(out, "%f\n", state.Y1000);
        fprintf(out, "%d\n", state.YKBDI);
        fprintf(out, "%f\n", state.YX1000);
        fclose(out);
        return 0;
    }
    return -1;
}
double CNFDRS::GetL1()
{
    if(m_nfdrs)
        return m_nfdrs->GetL1();
    return -1;
}
double CNFDRS::GetL10()
{
    if(m_nfdrs)
        return m_nfdrs->GetL10();
    return -1;
}
double CNFDRS::GetL100()
{
    if(m_nfdrs)
        return m_nfdrs->GetL100();
    return -1;
}
double CNFDRS::GetL1000()
{
    if(m_nfdrs)
        return m_nfdrs->GetL1000();
    return -1;
}
double CNFDRS::GetLHerb()
{
    if(m_nfdrs)
        return m_nfdrs->GetLHerb();
    return -1;
}
double CNFDRS::GetLWood()
{
    if(m_nfdrs)
        return m_nfdrs->GetLWood();
    return -1;
}
double CNFDRS::GetLDrought()
{
    if(m_nfdrs)
        return m_nfdrs->GetLDrought();
    return -1;
}
short CNFDRS::GetSG1()
{
    if(m_nfdrs)
        return m_nfdrs->GetSG1();
    return -1;
}
short CNFDRS::GetSG10()
{
    if(m_nfdrs)
        return m_nfdrs->GetSG10();
    return -1;
}
short CNFDRS::GetSG100()
{
    if(m_nfdrs)
        return m_nfdrs->GetSG100();
    return -1;
}
short CNFDRS::GetSG1000()
{
    if(m_nfdrs)
        return m_nfdrs->GetSG1000();
    return -1;
}
short CNFDRS::GetSGHerb()
{
    if(m_nfdrs)
        return m_nfdrs->GetSGHerb();
    return -1;
}
short CNFDRS::GetSGWood()
{
    if(m_nfdrs)
        return m_nfdrs->GetSGWood();
    return -1;
}
short CNFDRS::GetHD()
{
    if(m_nfdrs)
        return m_nfdrs->GetHD();
    return -1;
}
short CNFDRS::GetMXD()
{
    if(m_nfdrs)
        return m_nfdrs->GetMXD();
    return -1;
}
short CNFDRS::GetSCM()
{
    if(m_nfdrs)
        return m_nfdrs->GetSCM();
    return -1;
}
double CNFDRS::GetWindFC()
{
    if(m_nfdrs)
        return m_nfdrs->GetWindFC();
    return -1;
}
double CNFDRS::GetDepth()
{
    if(m_nfdrs)
        return m_nfdrs->GetDepth();
    return -1;
}
short CNFDRS::SetNFDRSVersion(short version)
{
    if(m_nfdrs)
    {
        m_nfdrs->SetVersion(version);
        return 1;
    }
    return -1;
}
short CNFDRS::iCalcMoistWetFlag(short iInit, short iTemp, short iRH, short iMaxTemp, short iMaxRH, 
        short iMinTemp, short iMinRH, short iPrecipDur, double fPrecipAmt, short iSOW, double f10stick, 
        short iJulian, short iYear, short iDeclareGreenUp, short iDeclareFreeze, short iGreenHerb, 
        short iGreenWoody, short iSeason, int iWetFlag, double * f1, double * f10, double * f100, 
        double * f1000, double * fHerb, double * fWood, int * iStage, double * fGren, 
        double * fX1000, int * iRainEvent, int * iKBDI)
{
    if(m_nfdrs)
        return m_nfdrs->iCalcMoistWetFlag(iInit, iTemp, iRH, iMaxTemp, iMaxRH, 
                iMinTemp, iMinRH, iPrecipDur, fPrecipAmt, iSOW, f10stick, 
                iJulian, iYear, iDeclareGreenUp, iDeclareFreeze, iGreenHerb, 
                iGreenWoody, iSeason, iWetFlag, f1, f10, f100, 
                f1000, fHerb, fWood, iStage, fGren, 
                fX1000, iRainEvent, iKBDI);
    return -1;
}
short CNFDRS::iSetAnnualInit(short iJulianGreenUp, double fStart1000, short iStartKBDI)
{
    if(m_nfdrs)
        return m_nfdrs->iSetAnnualInit(iJulianGreenUp, fStart1000, iStartKBDI);
    return -1;
}
short CNFDRS::iSetJulianShrubDormant(short iJulianDormant)
{
    if(m_nfdrs)
        return m_nfdrs->iSetJulianShrubDormant(iJulianDormant);
    return -1;
}
