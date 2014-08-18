/******************************************************************************
 *
 * $Id: nfdr32.cpp 79 2012-07-26 15:36:43Z kyle.shannon $
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
#include "nfdr32.h"
#include <stdlib.h>
#include <math.h>
#include <float.h>

/*              Fuel Moisture Adjustment if Exposed                                                                               */
char sAdjExposed[433] = "234111001001111234344122112112122344223111001001"
                        "112344122001001112234456233111001001111233233112"
                        "011011112233234112001001011233456234112001001122"
                        "345123112112123345345334234234334345345123111112"
                        "123345334111111123345456345122111111123345345122"
                        "011011122345345123111111123345456345123111111334"
                        "456345234234345456456456456456456456456344233233"
                        "345456456234223344456456456345233223344456456233"
                        "112112233456456345233233344456456456344223234456";

/*              Fuel Moisture Adjustment if Shaded                                                                */
char  sAdjShaded[217] = "455345334334345455445345334344345456445345334334"
                        "345455456345334334345445456455345345455456456345"
                        "345345456456456345345345345456456456345345345456"
                        "456456456456456456456456456456456456"
                        "456456456456456456456456456456456456";

NFDRCalc::NFDRCalc()
{
    CTA = 0.046;
    NFDRSVersion = 78;
    PrevSeason = -1;
    JulianShrubDormant = -1;
}

NFDRCalc::~NFDRCalc()
{
}


/* begin iCalcIndexes *****************************************************
 *
 *
 ***************************************************************************/
int NFDRCalc::iCalcIndexes (int iWS, int iSlopeCls,double* fROS, int* iSC,double* fERC, double* fFL, int* iFIL,int* iBI)
{

    double STD = .0555, STL = .0555;
    double RHOD = 32, RHOL = 32;
    double ETASD = 0.4173969, ETASL = 0.4173969;

    double WTOTD, WTOTL, WTOT, W1N, W10N, W100N, WHERBN, WWOODN, WTOTLN;
    double RHOBED, RHOBAR, BETBAR;
    double HN1, HN10, HN100, HNHERB, HNWOOD, WRAT, MCLFE, MXL;
    /* SA1000 is currently unused, commented to silence warnings */
    double SA1, SA10, SA100, /*SA1000,*/ SAWOOD, SAHERB, SADEAD, SALIVE;
    double F1, F10, F100, FHERB, FWOOD, FDEAD, FLIVE, WDEADN, WLIVEN;
    double SGBRD, SGBRL, SGBRT, BETOP, GMAMX, AD, GMAOP, ZETA;
    double WTMCD, WTMCL, DEDRT, LIVRT, ETAMD, ETAML, B, C, E, UFACT;
    double IR, PHIWND, PHISLP;
    double XF1, XF10, XF100, XFHERB, XFWOOD, HTSINK;

    double F1E, F10E, F100E, F1000E, FHERBE, FWOODE, FDEADE, FLIVEE, WDEDNE, WLIVNE;
    double SGBRDE, SGBRLE, SGBRTE, BETOPE, GMAMXE, ADE, GMAOPE;
    double WTMCDE, WTMCLE, DEDRTE, LIVRTE, ETAMDE, ETAMLE;
    double IRE, TAU;
    double fWNDFC, PackingRatio, DroughtUnit;
    double fDEPTH;

    double fTemp = 0;
    int itempWoody = 0;

    if ((iSlopeCls < 1) || (iWS < 0) || (iSlopeCls > 5) || (DEPTH <= 0))
        return(0);

    /* Reset */
    W1 = L1 * CTA;
    W10 = L10 * CTA;
    W100 = L100 * CTA;
    W1000 = L1000 * CTA;
    WWOOD = LWOOD * CTA;
    fDEPTH = DEPTH;


    if ((NFDRSVersion == 88) && (KBDI > 100))
    {
        WTOTD = W1 + W10 + W100;
        WTOTL = WHERB + WWOOD;
        WTOT = WTOTD + WTOTL;
        PackingRatio = WTOT / fDEPTH;
        if (PackingRatio == 0) PackingRatio = 1.0;
        WTOTD = WTOTD + W1000;
        DroughtUnit = WDROUGHT / 700;
        W1 = W1 + (W1 / WTOTD) * (KBDI - 100) * DroughtUnit;
        W10 = W10 + (W10 / WTOTD) * (KBDI - 100) * DroughtUnit;
        W100 = W100 + (W100 / WTOTD) * (KBDI - 100) * DroughtUnit;
        W1000 = W1000 + (W1000 / WTOTD) * (KBDI - 100) * DroughtUnit;
        WTOT = W1 + W10 + W100 + W1000 + WTOTL;
        fDEPTH = (WTOT - W1000) / PackingRatio;
    }

    // Herbaceous Load Transfers, both 78 and 88    
    fTemp = XferHerb(MCHERB);

    // Woody Load Transfer for 88 Deciduous Brush Models ... assumes winter GF are zero

    if ((NFDRSVersion == 88) && (WoodyDeciduous))
    {
        itempWoody = GreenWoody;
        if (Season == 1)
            itempWoody = 0;
        fTemp = (1 - itempWoody / 20.0) * WWOOD;
        W1P = W1P + fTemp;
        WWOOD = WWOOD - fTemp;
    }

    WTOTD = W1P + W10 + W100 + W1000;
    WTOTL = WHERBP + WWOOD;
    WTOT = WTOTD + WTOTL;

    W1N = W1P * (1.0 - STD);
    W10N = W10 * (1.0 - STD);
    W100N = W100 * (1.0 - STD);
    WHERBN = WHERBP * (1.0 - STL);
    WWOODN = WWOOD * (1.0 - STL);
    WTOTLN = WTOTL * (1.0 - STL);

    /* NOTE 1 */
    RHOBED = (WTOT - W1000) / fDEPTH;
    RHOBAR = ((WTOTL * RHOL) + (WTOTD * RHOD)) / WTOT;

    BETBAR = RHOBED / RHOBAR;


    if (WTOTLN > 0)
    {
        HN1 = W1N * exp(-138.0 / SG1);
        HN10 =W10N  * exp(-138.0 / SG10);
        HN100 = W100N * exp(-138.0 / SG100);

        /* NOTE 2 */
        if ((-500 / SGHERB) < -180.218)
        {HNHERB = 0;}
        else
        {HNHERB = WHERBN * exp(-500.0 / SGHERB);}

        if ((-500 / SGWOOD) < -180.218)
        {HNWOOD = 0;}
        else
        {HNWOOD = WWOODN * exp(-500.0 / SGWOOD);}

        /* NOTE 3 */
        if ((HNHERB + HNWOOD) == 0)
        {WRAT = 0;}
        else
        {WRAT = (HN1 + HN10 + HN100) / (HNHERB + HNWOOD);}
        MCLFE = ((MC1 * HN1) + (MC10 * HN10) + (MC100 * HN100)) / (HN1 + HN10 + HN100);
        MXL = (2.9 * WRAT * (1.0 - MCLFE / MXD) - 0.226) * 100;
    }
    else
    {MXL = 0;}

    if (MXL < MXD) MXL = MXD;

    SA1 = (W1P / RHOD) * SG1;
    SA10 = (W10 / RHOD) * SG10;
    SA100 = (W100 / RHOD) * SG100;
    SAHERB = (WHERBP / RHOL) * SGHERB;
    SAWOOD = (WWOOD / RHOL) * SGWOOD;
    SADEAD = SA1 + SA10 + SA100;
    SALIVE = SAHERB + SAWOOD;

    if (SADEAD <= 0)
        return(0);

    F1 = SA1 / SADEAD;
    F10 = SA10 / SADEAD;
    F100 = SA100 / SADEAD;
    /* NOTE 4 */
    if (WTOTL <=0)
    {
        FHERB = 0;
        FWOOD = 0;
    }
    else
    {
        FHERB = SAHERB / SALIVE;
        FWOOD = SAWOOD / SALIVE;
    }
    FDEAD = SADEAD / (SADEAD + SALIVE);
    FLIVE = SALIVE / (SADEAD + SALIVE);

    WDEADN = (F1 * W1N) + (F10 * W10N) + (F100 * W100N);
    /* NOTE 5*/
    if (SGWOOD > 1200 && SGHERB > 1200)
    {WLIVEN = WTOTLN;}
    else
    {WLIVEN = (FWOOD * WWOODN) + (FHERB * WHERBN);}

    SGBRD = (F1 * SG1) + (F10 * SG10) + (F100 * SG100);
    SGBRL = (FHERB * SGHERB) + (FWOOD * SGWOOD);
    SGBRT = (FDEAD * SGBRD) + (FLIVE * SGBRL);
    BETOP = 3.348 * pow(SGBRT, -0.8189);
    GMAMX = pow(SGBRT, 1.5) / (495.0 + 0.0594 * pow(SGBRT, 1.5));
    AD = 133 * pow(SGBRT, -0.7913);
    GMAOP = GMAMX * pow( (BETBAR/BETOP), AD) * exp(AD * (1.0 - (BETBAR / BETOP)));

    ZETA = exp((0.792 + 0.681 * pow(SGBRT, 0.5)) * (BETBAR + 0.1));
    ZETA = ZETA / (192.0 + 0.2595 * SGBRT);

    WTMCD = (F1 * MC1) + (F10 * MC10) + (F100 * MC100);
    WTMCL = (FHERB * MCHERB) + (FWOOD * MCWOOD);
    DEDRT = WTMCD / MXD;
    LIVRT = WTMCL / MXL;
    ETAMD = 1.0 - 2.59 * DEDRT + 5.11 * pow(DEDRT,2.0) - 3.52 * pow(DEDRT, 3.0);
    ETAML = 1.0 - 2.59 * LIVRT + 5.11 * pow(LIVRT,2.0) - 3.52 * pow(LIVRT, 3.0);
    if (ETAMD < 0) ETAMD = 0;
    if (ETAMD > 1) ETAMD = 1;
    if (ETAML < 0) ETAML = 0;
    if (ETAML > 1) ETAML = 1;

    B = 0.02526 * pow(SGBRT, 0.54);
    C = 7.47 * exp(-0.133 * pow(SGBRT,0.55));
    E = 0.715 * exp(-3.59 * pow(10.0, -4.0) * SGBRT);
    UFACT = C * pow(BETBAR / BETOP, -1 * E);

    /* HL = HD */
    IR = GMAOP * ((WDEADN * HD * ETASD * ETAMD) + (WLIVEN * HD * ETASL * ETAML));

    if ((NFDRSVersion == 88) && (Season > 0))
    {
        if (WoodyDeciduous)
        {
            switch (Season)
            {
                case 1:
                    fWNDFC = WNDFCMax;
                    break;
                case 2:
                    fWNDFC = WNDFCMax - (WNDFCMax - WNDFCMin) * (GreenWoody / 20.0);
                    break;
                case 3:
                    fWNDFC = WNDFCMin;
                    break;
                case 4:
                    fWNDFC = WNDFCMax - (WNDFCMax - WNDFCMin) * (GreenWoody / 20.0);
                    break;
            }
        }
        else
        {
            if (Season == 3)
            {
                fWNDFC = WNDFCMin;
            }
            else
            {
                fWNDFC = WNDFCSet;
            }
        }
        if (RainEvent)
            fWNDFC = .3 * fWNDFC;
    }
    else
    {
        fWNDFC = WNDFC;
    }

    if (88.0 * iWS * fWNDFC > 0.9 * IR)
    {
        PHIWND = UFACT * pow(0.9 * IR, B);
    }
    else
    {
        PHIWND = UFACT * pow(iWS * 88.0 * fWNDFC, B);
    }

    switch (iSlopeCls)
    {
        case 1:
            PHISLP = 0.267 * pow(BETBAR, -0.3);
            break;
        case 2:
            PHISLP = 0.533 * pow(BETBAR, -0.3);
            break;
        case 3:
            PHISLP = 1.068 * pow(BETBAR, -0.3);
            break;
        case 4:
            PHISLP = 2.134 * pow(BETBAR, -0.3);
            break;
        default:
            PHISLP = 4.273 * pow(BETBAR, -0.3);
            break;
    }

    XF1 = F1 * exp(-138.0 / SG1) * (250.0 + 11.16 * MC1);
    XF10 = F10 * exp(-138.0 / SG10) * (250.0 + 11.16 * MC10);
    XF100 = F100 * exp(-138.0 / SG100) * (250.0 + 11.16 * MC100);
    XFHERB = FHERB * exp(-138.0 / SGHERB) * (250.0 + 11.16 * MCHERB);
    XFWOOD = FWOOD * exp(-138.0 / SGWOOD) * (250.0 + 11.16 * MCWOOD);
    HTSINK = RHOBED * (FDEAD * (XF1 + XF10 + XF100) + FLIVE * (XFHERB + XFWOOD));

    *fROS = IR * ZETA * (1.0 + PHISLP + PHIWND) / HTSINK;
    *iSC = irnd(*fROS);

    F1E = W1P / WTOTD;
    F10E = W10 / WTOTD;
    F100E = W100 / WTOTD;
    F1000E = W1000 / WTOTD;
    /* NOTE 4 */
    if (WTOTL <=0)
    {FHERBE = 0;
        FWOODE = 0;
    }
    else
    {FHERBE = WHERBP / WTOTL;
        FWOODE = WWOOD / WTOTL;
    }
    FDEADE = WTOTD / WTOT;
    FLIVEE = WTOTL / WTOT;

    WDEDNE = WTOTD * (1.0 - STD);
    WLIVNE = WTOTL * (1.0 - STL);

    SGBRDE = (F1E * SG1) + (F10E * SG10) + (F100E * SG100) + (F1000E * SG1000);
    SGBRLE = (FHERBE * SGHERB) + (FWOODE * SGWOOD);
    SGBRTE = (FDEADE * SGBRDE) + (FLIVEE * SGBRLE);
    BETOPE = 3.348 * pow(SGBRTE, -0.8189);
    GMAMXE = pow(SGBRTE, 1.5) / (495.0 + 0.0594 * pow(SGBRTE, 1.5));
    ADE = 133 * pow(SGBRTE, -0.7913);
    GMAOPE = GMAMXE * pow( (BETBAR/BETOPE), ADE) * exp(ADE * (1.0 - (BETBAR / BETOPE)));

    WTMCDE = (F1E * MC1) + (F10E * MC10) + (F100E * MC100) + (F1000E * MC1000);
    WTMCLE = (FHERBE * MCHERB) + (FWOODE * MCWOOD);
    DEDRTE = WTMCDE / MXD;
    LIVRTE = WTMCLE / MXL;
    ETAMDE = 1.0 - 2.0 * DEDRTE + 1.5 * pow(DEDRTE,2.0) - 0.5 * pow(DEDRTE, 3.0);
    ETAMLE = 1.0 - 2.0 * LIVRTE + 1.5 * pow(LIVRTE,2.0) - 0.5 * pow(LIVRTE, 3.0);
    if (ETAMDE < 0) ETAMDE = 0;
    if (ETAMDE > 1) ETAMDE = 1;
    if (ETAMLE < 0) ETAMLE = 0;
    if (ETAMLE > 1) ETAMLE = 1;

    IRE = (FDEADE * WDEDNE * HD * ETASD * ETAMDE);
    /* HL = HD */
    IRE = GMAOPE * (IRE + (FLIVEE * WLIVNE * HD * ETASL * ETAMLE));
    TAU = 384.0 / SGBRT;
    *fERC = 0.04 * IRE * TAU;
    *fFL = .301 * pow((*iSC * *fERC), 0.46);//CHANGED SB
    //*fFL = .301 * pow((*fROS * *fERC), 0.46);
    *iBI = irnd(10.0 * *fFL);

    if (*fFL <= 2)
        *iFIL = 1;
    else
    {
        if (*fFL <= 4)
            *iFIL = 2;
        else
        {
            if (*fFL <= 6)
                *iFIL = 3;
            else
            {
                if (*fFL <= 8)
                    *iFIL = 4;
                else
                {
                    if (*fFL <= 12)
                        *iFIL = 5;
                    else
                        *iFIL = 6;
                }
            }
        }
    }
    // CHANGED SB *fROS = *fROS / 1.1; /* convert to ch/hr */

    if ((SOW >= 5) && (SOW <= 7))
    {
        *fROS = 0.00;
        *iSC = 0;
        *iBI = 0;
        *fFL = 0;
        *iFIL = 1;
        /* Leave ERC */
    }



    //////////////////////////////////////////////////////////////////////////DEBUG!!!!!!!!!!!!!!!!!!!!!!!!!!
    //*iFIL = Stage;

    /* Reset Just To Be Safe */
    W1 = L1 * CTA;
    W10 = L10 * CTA;
    W100 = L100 * CTA;
    W1000 = L1000 * CTA;
    WWOOD = LWOOD * CTA;

    return (1);

}


/* begin iCalcIC  *********************************************************
 *
 *
 ***************************************************************************/
//int NFDRCalc::iCalcIC (int iTemp, int iSOW,CHANGED SB
//        double fMC1, int iSC, int* iIC)
int NFDRCalc::iCalcIC (int iTemp, int iSOW, double fMC1, double fROS, int* iIC)
{
    double tfact = 0.0, TMPPRM = 0.0;
    double PNORM1 = 0.00232, PNORM2 = 0.99767;//, PNORM3 = 0.0000185;
    double QIGN = 0.0, CHI = 0.0, PI = 0.0, SCN = 0.0, PFI = 0.0;

    if (SCM <= 0)
    {
        *iIC = 0;
        return(0);
    }
    switch (iSOW)
    {
        case -1:
            tfact = 0.0;
            break;
        case 0:
            tfact = 25.0;
            break;
        case 1:
            tfact = 19.0;
            break;
        case 2:
            tfact = 12.0;
            break;
        default:
            tfact = 5.0;
    }
    TMPPRM = (iTemp + tfact - 32) / 1.8;
    QIGN = 144.5 - (0.266 * TMPPRM) - (0.00058 * TMPPRM * TMPPRM)
        - (0.01 * TMPPRM * fMC1)
        + 18.54 * (1.0 - exp(-0.151 * fMC1))
        + 6.4 * fMC1;
    if (QIGN >= 344.0)
    {
        *iIC = 0;
        return(1);
    }

    CHI = (344.0 - QIGN) / 10.0;
    if ((pow(CHI, 3.66) * 0.000923 / 50) <= PNORM1)
    {
        *iIC = 0;
        return(1);
    }

    PI = ((pow(CHI, 3.66) * 0.000923 / 50) - PNORM1) * 100.0 / PNORM2;
    if (PI < 0) PI = 0;
    if (PI > 100) PI = 100;
    SCN = 100.0 * fROS / SCM;
    if (SCN > 100.0) SCN = 100.0;
    PFI = pow(SCN, 0.5);
    *iIC = irnd(0.10 * PI * PFI);

    if ((SOW >= 5) && (SOW <= 7))
        *iIC = 0;

    return (1);
}

/* begin iInitialize *****************************************************
 *
 *
 ***************************************************************************/
int NFDRCalc::iInitialize (int iNFDRSVersion, int iClimateClass,int iJulianLookFreeze, int iJulianGreenUp,
        int iLat, int iHerbAnnual, int iWoodyDeciduous, int iOneIsTen, double fStart100, double fStart1000,
        int iStartKBDI, double fAvgPrecip)

{
    NFDRSVersion = iNFDRSVersion;
    ClimateClass = iClimateClass;
    JulianLookFreeze = iJulianLookFreeze;
    JulianGreenUp = iJulianGreenUp;
    Lat = iLat;
    HerbAnnual = iHerbAnnual;
    WoodyDeciduous = iWoodyDeciduous;
    OneIsTen = iOneIsTen;
    if (fStart100 > 0.0)
        Start100 = fStart100;
    else
        Start100 = 5.0 + 5.0 * ClimateClass;
    if (fStart1000 > 0)
        Start1000 = fStart1000;
    else
        Start1000 = 10.0 + 5.0 * ClimateClass;
    if (iStartKBDI >= 0)//changed SB
        StartKBDI = iStartKBDI;
    else
        StartKBDI = 100;
    KBDI = iStartKBDI;
    YKBDI = iStartKBDI;
    AvgPrecip = fAvgPrecip;
    CummPrecip = 0.0;
    PrevPrecip = 0.0;
    PrevJulian = 0;
    PrevYear = 0;
    JulianShrubDormant = -1;
    hasGreenedupThisYear = 0;
    return(1);
}

int NFDRCalc::iSetAnnualInit(int iJulianGreenUp, double fStart1000, int iStartKBDI)
{
    JulianGreenUp = iJulianGreenUp;
    if (iStartKBDI >= 0)
        StartKBDI = iStartKBDI;
    else
        StartKBDI = 100;
    if (fStart1000 > 0)
        Start1000 = fStart1000;
    else
        Start1000 = 10.0 + 5.0 * ClimateClass;
    return(1);
}


/* begin InitFM *********************************************************
 *
 *
 ***************************************************************************/
int NFDRCalc::InitFM (void)
{
    int i;

    for(i = 0; i < 7 ; i++)
    {
        HistBndryT[i] = Start1000;
        Hist1000[i] = Start1000;
    }
    MC100 = Start100;
    Y100 = Start100;
    Y1000 = Start1000;
    return(1);
}

//sets date for setting shrubs into dormant state
int NFDRCalc::iSetJulianShrubDormant(int iJulianDormant)
{
    JulianShrubDormant = iJulianDormant;
    return 1;
}


/* begin iCalcMoist     *******************************************************
 *
 *
 ***************************************************************************/
int NFDRCalc::iCalcMoist (int iInit, int iTemp, int iRH, int iMaxTemp, int iMaxRH,int iMinTemp, int iMinRH,
        int iPrecipDur, double fPrecipAmt, int iSOW, double f10stick, int iJulian, int iYear, 
        int iDeclareGreenUp, int iDeclareFreeze, int iGreenHerb, int iGreenWoody, int iSeason,
        double* f1, double* f10, double* f100, double* f1000, double* fHerb, double* fWood,
        int* iStage, double* fGren, double* fX1000, int* iRainEvent, int* iKBDI)
{
    return iCalcMoistWetFlag (iInit, iTemp, iRH, iMaxTemp, iMaxRH, iMinTemp, iMinRH,
            iPrecipDur, fPrecipAmt, iSOW, f10stick, iJulian, iYear, 
            iDeclareGreenUp, iDeclareFreeze, iGreenHerb, iGreenWoody, iSeason, 0, //int iWetFlag,
            f1, f10, f100, f1000, fHerb, fWood,
            iStage, fGren, fX1000, iRainEvent, iKBDI);
}

/* begin iCalcMoistWetFlag      *******************************************************
 * Overloaded version to implement wetflag
 *
 ***************************************************************************/
int NFDRCalc::iCalcMoistWetFlag (int iInit, int iTemp, int iRH, int iMaxTemp, int iMaxRH,int iMinTemp, int iMinRH,
        int iPrecipDur, double fPrecipAmt, int iSOW, double f10stick, int iJulian, int iYear, 
        int iDeclareGreenUp, int iDeclareFreeze, int iGreenHerb, int iGreenWoody, int iSeason, int iWetFlag,
        double* f1, double* f10, double* f100, double* f1000, double* fHerb, double* fWood,
        int* iStage, double* fGren, double* fX1000, int* iRainEvent, int* iKBDI)
{
    int iReturn = 0;
    int iBegin = 0;
    int iReInit = 0;


    RainEvent = 0;
    GreenHerb = iGreenHerb;
    GreenWoody = iGreenWoody;
    if(PrevSeason == -1)
        PrevSeason = iSeason;
    else
        PrevSeason = Season;
    Season = iSeason;
    if (NFDRSVersion == 88)
    {
        if (fPrecipAmt > 0.1)
        {
            RainEvent = 1;
        }
        else
        {
            if ((PrevPrecip > 0.1) &&
                    (((iYear == PrevYear) && (iJulian == PrevJulian + 1)) ||
                     ((iYear == PrevYear + 1) && (iJulian == 1) && (PrevJulian >= 365))))
            {
                RainEvent = 1;
            }
        }
    }
    //YKBDI = KBDI;CHANGED SB
    //KBDI = iCalcKBDI(fPrecipAmt, iMaxTemp, CummPrecip, YKBDI, AvgPrecip);

    if ((iYear < PrevYear) || (iYear > (PrevYear + 1)))
    {
        iReInit = 1;
    }
    else
    {
        if (365 * (iYear - PrevYear) + iJulian - PrevJulian > 30)
            iReInit = 1;
    }

    if (iInit != 0 || iReInit == 1)
    {
        iReturn = InitFM();
        YKBDI = StartKBDI;
        KBDI = StartKBDI;
        MCHERB = 250.0;
        MCWOOD = 200.0;
        iBegin = 1;
        if (iJulian >= JulianGreenUp)
        {
            PrevYear = iYear;
            PrevJulian = iJulian -1;
            if (iJulian > JulianGreenUp + 7 * ClimateClass)
            {
                PrevStage = 3;
            }
            else
            {
                PrevStage = 2;
                JulianStartedGreen = iJulian;
            }
        }
        else
        {
            PrevStage = 1;
            PrevYear = iYear - 2;
            PrevJulian = 1;
        }
        GreenDays = 0;
        ColdDays = 0;
        CummPrecip = 0.0;
    }
    if(iJulian >= JulianGreenUp && !hasGreenedupThisYear)
    {
        PrevStage = 1;
        iDeclareGreenUp = 1;
    }

    YKBDI = KBDI;
    KBDI = iCalcKBDI(fPrecipAmt, max(iTemp, iMaxTemp), CummPrecip, YKBDI, AvgPrecip);


    iReturn = hundredthous(iTemp, iRH, iMaxTemp, iMaxRH, iMinTemp, iMinRH,
            iJulian, iPrecipDur, fPrecipAmt, iSOW, iWetFlag);

    iReturn = oneten (iTemp, iRH, iSOW, f10stick, iWetFlag);

    if (iBegin == 1)
    {
        Y1000 = MC1000;
        X1000 = MC1000;
        YX1000 = MC1000;
        Y100 = MC100;
    }

    iReturn = Cure (iJulian, iYear, iTemp, iMaxTemp, iMinTemp,
            iDeclareGreenUp, iDeclareFreeze);

    //dump data to stdout
    //      TRACE2("Year: %d  DOY: %d ", iYear, iJulian);
    //      TRACE3("f1000 = %f  x1000 = %f  Herb = %f  ", MC1000, X1000, MCHERB);
    //      TRACE2("PrevStage = %d  Stage = %d\n",  PrevStage, Stage);
    //printf("f1000 = %f  x1000 = %f  Herb = %f  PrevStage = %d  Stage = %f\n", 
    //      MC1000, X1000, MCHERB, PrevStage, Stage);

    *f1 = MC1;
    *f10 = MC10;
    *f100 = MC100;
    *f1000 = MC1000;
    *fHerb = MCHERB;
    *fWood = MCWOOD;
    *iStage = Stage;
    *fGren = Gren;
    *fX1000 = X1000;
    *iRainEvent = RainEvent;
    *iKBDI = KBDI;

    PrevJulian = iJulian;
    PrevYear = iYear;
    PrevPrecip = fPrecipAmt;
    Y1000 = MC1000;
    YX1000 = X1000;
    Y100 = MC100;
    PrevStage = Stage;
    SOW = iSOW;

    return(1);
}



/* begin hundredthous   ****************************************************
 *
 *
 ***************************************************************************/
/*int NFDRCalc::hundredthous (int iTemp, int iRH, int iMaxTemp, int iMaxRH, int iMinTemp, int iMinRH,
  int iJulian, int iPrecipDur, double fPrecipAmt)
  {
  double emcMin = 0,
  emcMax = 0,
  emcBar = 0,
  bndryH = 0,
  bndryT = 0,
  bndryBar = 0,
  Daylight = 0,
  ambvp = 0;

  int i = 0;

// Estimate Min/Max RH if Min/Max Temp available: 
if ( ((iMinRH <= 0) || (iMaxRH <= 0)) && (iMinTemp > -999) && (iMaxTemp > -999))
{
//ambvp = ((double) iRH) * satvap((double) iTemp) / 100.0;
ambvp = ((double) iRH) * satvap(iTemp) / 100.0;
iMaxRH = (int) (100.0 * ambvp / satvap(iMinTemp));
iMinRH = (int) (100.0 * ambvp / satvap(iMaxTemp));
//changed sb rh comparisons, next two lines(< = > && < = >)
if (iMaxRH < iRH) iMaxRH = iRH;
if (iMinRH > iRH) iMinRH = iRH;
if (iMaxRH > 100) iMaxRH = 100;
}
Daylight = fCalcDaylightHours(Lat, iJulian);
if ( (iMinTemp <= -999) || (iMaxTemp <= -999))
{
emcMin = eqmc ((double) iTemp, (double) iRH);
if (ClimateClass < 3)
emcMax = 15;
else
emcMax = 23;
}
else
{
emcMin = eqmc ((double) iMaxTemp, (double) iMinRH);
emcMax = eqmc ((double) iMinTemp, (double) iMaxRH);
}
if (fPrecipAmt > 0 && iPrecipDur <= 0)
{
{if (ClimateClass < 3)
iPrecipDur = irnd(fPrecipAmt / .25 + .499999);
else
iPrecipDur = irnd(fPrecipAmt / .05 + .499999);
}
if (iPrecipDur > 8)
iPrecipDur = 8;
}


emcBar = (Daylight * emcMin + (24 - Daylight) * emcMax) / 24;
bndryH = ((24.0 - (double) iPrecipDur) * emcBar 
+ (double) iPrecipDur * (.5 * (double) iPrecipDur + 41)) / 24;
MC100 = Y100 + .3156 * (bndryH - Y100);
bndryT = ((24.0 - (double) iPrecipDur) * emcBar 
+ (double) iPrecipDur * (2.7 * (double) iPrecipDur + 76)) / 24;

//      for(i=1; i<7 ; i++)
for(i=0; i<6 ; i++)
{
HistBndryT[i] = HistBndryT[i+1];
bndryBar = bndryBar + HistBndryT[i];
}
HistBndryT[6] = bndryT;
bndryBar = (bndryBar + bndryT) / 7;
MC1000 = Hist1000[0] + (bndryBar - Hist1000[0]) * .3068;

for(i=0; i<6 ; i++)
{
Hist1000[i] = Hist1000[i+1];
}
Hist1000[i] = MC1000;
return(1);
}
*/
/* begin hundredthous   ****************************************************
 * Overloaded version to implement WetFlag 
 *
 ***************************************************************************/
int NFDRCalc::hundredthous (int iTemp, int iRH, int iMaxTemp,
        int iMaxRH, int iMinTemp, int iMinRH,
        int iJulian,
        int iPrecipDur, double fPrecipAmt, int iSOW, int iWetFlag)
{
    double emcMin = 0,
           emcMax = 0,
           emcBar = 0,
           bndryH = 0,
           bndryT = 0,
           bndryBar = 0,
           Daylight = 0,
           ambvp = 0;
    int iWFPrecipDur = iPrecipDur;
    int i = 0;

    /* Estimate Min/Max RH if Min/Max Temp available: */
    if ( ((iMinRH <= 0) || (iMaxRH <= 0)) && (iMinTemp > -999) && (iMaxTemp > -999))
    {
        //ambvp = ((double) iRH) * satvap((double) iTemp) / 100.0;
        ambvp = ((double) iRH) * satvap(iTemp) / 100.0;
        iMaxRH = (int) (100.0 * ambvp / satvap(iMinTemp));
        iMinRH = (int) (100.0 * ambvp / satvap(iMaxTemp));
        //changed sb rh comparisons, next two lines(< = > && < = >)
        if (iMaxRH < iRH) iMaxRH = iRH;
        if (iMinRH > iRH) iMinRH = iRH;
        if (iMaxRH > 100) iMaxRH = 100;
    }
    /***********************************************************************
     ** Add logic for winter observations when fuels are covered by snow  **
     ** by modifing rh ranges and precip duration for fuel/snow interface **
     ** snow is assumed if a wetflg is declared, but not precipitating    **
     ** at observation time.  The assumption is further modified by the   **
     ** observed temperature to model thawing as precip duration on the   **
     ** heavy fuels.  These local modifications are not updated in the    **
     ** weather observation for the day                                   **
     ** Code by Larry Bradshaw, Fire Sciences Lab. Missoula.  Apr. 1999   **
     **********************************************************************/

    if( iWetFlag > 0 && (iSOW < 5 || iSOW > 7))
    {
        iMaxRH = 100;
        iMinRH = 100;
        // if(iTemp <= 35) phours = 0;
        if(iTemp <= 35) iWFPrecipDur = 0;
        if(iTemp > 35 && iTemp <= 40) iWFPrecipDur = 2;
        if(iTemp > 40 && iTemp <= 50) iWFPrecipDur = 4;
        if(iTemp > 50 && iTemp <= 60) iWFPrecipDur = 6;
        if(iTemp > 60) iWFPrecipDur = 8;
    }

    // End of wetflag code



    Daylight = fCalcDaylightHours(Lat, iJulian);
    if ( (iMinTemp <= -999) || (iMaxTemp <= -999))
    {
        emcMin = eqmc ((double) iTemp, (double) iRH);
        if (ClimateClass < 3)
            emcMax = 15;
        else
            emcMax = 23;
    }
    else
    {
        emcMin = eqmc ((double) iMaxTemp, (double) iMinRH);
        emcMax = eqmc ((double) iMinTemp, (double) iMaxRH);
    }
    if (fPrecipAmt > 0 && iWFPrecipDur <= 0)
    {
        {if (ClimateClass < 3)
            iWFPrecipDur = irnd(fPrecipAmt / .25 + .499999);
            else
                iWFPrecipDur = irnd(fPrecipAmt / .05 + .499999);
        }
        if (iWFPrecipDur > 8)
            iWFPrecipDur = 8;
    }


    emcBar = (Daylight * emcMin + (24 - Daylight) * emcMax) / 24;
    bndryH = ((24.0 - (double) iWFPrecipDur) * emcBar 
            + (double) iWFPrecipDur * (.5 * (double) iWFPrecipDur + 41)) / 24;
    MC100 = Y100 + .3156 * (bndryH - Y100);
    bndryT = ((24.0 - (double) iWFPrecipDur) * emcBar 
            + (double) iWFPrecipDur * (2.7 * (double) iWFPrecipDur + 76)) / 24;

    //      for(i=1; i<7 ; i++)
    for(i=0; i<6 ; i++)
    {
        HistBndryT[i] = HistBndryT[i+1];
        bndryBar = bndryBar + HistBndryT[i];
    }
    HistBndryT[6] = bndryT;
    bndryBar = (bndryBar + bndryT) / 7;
    MC1000 = Hist1000[0] + (bndryBar - Hist1000[0]) * .3068;

    for(i=0; i<6 ; i++)
    {
        Hist1000[i] = Hist1000[i+1];
    }
    Hist1000[i] = MC1000;
    return(1);
}



/* begin oneten *********************************************************
 *
 *
 ***************************************************************************/
/*int NFDRCalc::oneten (int iTemp, int iRH, int iSOW,
  double f10stick)

  {
  double tfact = 0,
  hfact = 0,
  emc = 0;

  switch (iSOW)
  {
  case 0:
  tfact = 25.0;
  hfact = 0.75;
  break;
  case 1:
  tfact = 19.0;
  hfact = 0.83;
  break;
  case 2:
  tfact = 12.0;
  hfact = 0.92;
  break;
  default:
  tfact = 5.0;
  hfact = 1.0;
  }

  emc = eqmc(tfact + (double)iTemp, hfact * (double)iRH);

  if (f10stick <= 0)                   // No Fuelstick 
  {
  MC1 = 1.03 * emc;
//changed sb 7/29/99
if (iSOW > 4 && iSOW < 8)
MC10 = 35;
else
MC10 = 1.03 * emc * .8 + .2 * MC100;
}
else                       // Have Fuelstick /
{
if (f10stick > 2)
{
MC10 = f10stick;
}
else
{
MC10 = 2.0;
}
MC1 = (4.0 * emc + MC10) / 5.0;
}

// Not in WIMS:
//if (iSOW > 4)
// *f1 = 35;
//

if ((NFDRSVersion == 88) && (OneIsTen || RainEvent))
MC1 = MC10;

return(1);

}*/

int NFDRCalc::oneten (int iTemp, int iRH, int iSOW, double f10stick, int iWetFlag)

{
    double tfact = 0,
           hfact = 0,
           emc = 0;

    switch (iSOW)
    {
        case 0:
            tfact = 25.0;
            hfact = 0.75;
            break;
        case 1:
            tfact = 19.0;
            hfact = 0.83;
            break;
        case 2:
            tfact = 12.0;
            hfact = 0.92;
            break;
        default:
            tfact = 5.0;
            hfact = 1.0;
    }

    emc = eqmc(tfact + (double)iTemp, hfact * (double)iRH);

    if (f10stick <= 0)                   /* No Fuelstick */
    {
        MC1 = 1.03 * emc;
        //changed sb 7/29/99
        if ((iSOW > 4 && iSOW < 8) || iWetFlag > 0)
            MC1 = MC10 = 35;
        else
            MC10 = 1.03 * emc * .8 + .2 * MC100;  // bug test uncommented 8/7/2002
        //MC10 = 1.28 * emc;
    }
    else                       /* Have Fuelstick */
    {
        if (f10stick > 2)
        {
            MC10 = f10stick;
        }
        else
        {
            MC10 = 2.0;
        }
        MC1 = (4.0 * emc + MC10) / 5.0;
    }

    /* Not in WIMS:
       if (iSOW > 4)
     *f1 = 35;
     */

    /*  stuart:  this should read:
        if (88 system) then
        if (oneisten or rainevent) then
        mc1=mc10
        else
        mc1 = 1.03 * emc *.8 + .2 * mc100
        endif
        endif

        Did i get it right?  */

    if (NFDRSVersion == 88)
    {
        if (OneIsTen || RainEvent) 
            MC1= MC10;
        else
            MC1 = 1.03 * emc;// * .8 + .2 * MC100;  
    }

    return(1);

}


/* begin fCalcX1000     ******************************************************
 *
 *
 ***************************************************************************/
double NFDRCalc::fCalcX1000 (int iMaxTemp, int iMinTemp)
{
    double fDayDiff = 0.0;
    double fDampNo = 0.0;

    fDayDiff = MC1000 - Y1000;
    if (MC1000 > 25.0 || fDayDiff <=0)
        fDampNo = 1.0;
    else
    {
        //modified 9/23/98 sb&lb
        //if (MC1000 >= 10.0)
        //        fDampNo = .1675 + .0333 * MC1000;
        fDampNo = .167 + .033 * MC1000;
        // else
        //        fDampNo = 0.5;
    }
    if (iMaxTemp > -999 && iMinTemp > -999 && ((double)iMaxTemp + (double)iMinTemp) / 2.0 <= 50.0)
        fDampNo = 0.6 * fDampNo;

    return max(2.0, (YX1000 + fDampNo * fDayDiff));
}




/* begin Cure   *********************************************************
 *
 *
 ***************************************************************************/
int NFDRCalc::Cure (int iJulian, int iYear,int iTemp, int iMaxTemp, int iMinTemp,int iDeclareGreenUp, int iDeclareFreeze)
{
    //
    // Routine Updated Apr 2004 to Finally Fix the 88 Greenness Model (LSB)
    //


    double fHerbP = 0.0;
    double fWoodP = 0.0,
           fWoodI = 0.0;
    int transition = 0;     // Added LSB, Apr 2004 

    double yMCHERB = MCHERB;//CHANGED SB
    if (NFDRSVersion == 78  || (NFDRSVersion == 88 && Season == 0))
    {
        if (PrevStage < 1 || PrevStage > 6)// || iYear > PrevYear)
        {
            PrevStage = 1;
            //yMCHERB = 250.0;
        }
        //block added 5/14/3 SB & LB to force to pregreen state for year round data
        if ((PrevStage == 5 || PrevStage == 6) && iYear > PrevYear)
        {
            PrevStage = 1;
        }
        if(iYear != PrevYear)
            hasGreenedupThisYear = 0;
        Stage = PrevStage;

        //X1000 = fCalcX1000 (iMaxTemp, iMinTemp);

        if (iDeclareFreeze != 0)
            Stage = 6;

        if (Stage == 1)//PREGREEN
        {
            if (iDeclareGreenUp != 0 || iJulian >= JulianGreenUp)
            {
                GreenDays = 0;
                Stage = 2;
                if (iDeclareGreenUp)
                {
                    JulianStartedGreen = iJulian;
                }
                else
                {
                    JulianStartedGreen = JulianGreenUp;
                }
            }
            else
            {
                MCHERB = MC1;
                Gren = 0;
                //LINE ADDED 9/23/98 SB & LB
                X1000 = MC1000;
            }
        }

        if (Stage == 2)//GREENUP
        {
            /* Following Block Added March-2004 to ensure x1000 and colddays get reset each green up where
               there may have not been a freeze the previous year or GU = 1 Jan AND Freeze Date = 12/31    */

            if (PrevStage != 2) 
            {
                ColdDays = 0;
                YX1000 = MC1000;
            }
            hasGreenedupThisYear = 1;
            X1000 = fCalcX1000 (iMaxTemp, iMinTemp);
            GreenDays++;
            Gren = 100.0 * (double) GreenDays / (7.0 * (double) ClimateClass);
            switch (ClimateClass)
            {
                case 1:
                    fHerbP = 12.8 * X1000 - 70;
                    break;
                case 2:
                    fHerbP = 14 * X1000 - 100;
                    break;
                case 3:
                    fHerbP = 15.5 * X1000 - 137.5;
                    break;
                case 4:
                    fHerbP = 17.4 * X1000 - 185;
            }
            if (fHerbP > 250.0) fHerbP = 250.0;
            MCHERB = MC1 + Gren * (fHerbP - MC1) / 100;

            if (Gren >= 100.0)
            {
                if (MCHERB >= 120.0) Stage = 3;
                else
                    Stage = 4;
            }
        }

        if (Stage == 3)//GREEN
        {
            X1000 = fCalcX1000 (iMaxTemp, iMinTemp);
            Gren = 100.0;
            switch (ClimateClass)
            {
                case 1:
                    MCHERB = 12.8 * X1000 - 70;
                    break;
                case 2:
                    MCHERB = 14 * X1000 - 100;
                    break;
                case 3:
                    MCHERB = 15.5 * X1000 - 137.5;
                    break;
                case 4:
                    MCHERB = 17.4 * X1000 - 185;
            }
            if(HerbAnnual != 0)  MCHERB = min(MCHERB, yMCHERB);  //CHANGED SB
            if (MCHERB > 250.0)  MCHERB = 250.0;

            if (MCHERB < 120) Stage = 4;   //changed from <= 5/14/3 SB & LB


            if ((iJulian >= JulianLookFreeze || iJulian < JulianGreenUp) && iMinTemp > -999)
            {
                if (iMinTemp <= 25) 
                {
                    Stage = 6;
                }
                else
                {
                    if (iMinTemp <= 32) 
                        ColdDays++;
                    if (ColdDays >4) 
                        Stage = 6;
                }
            }
        }

        if (Stage == 4)//TRANSITION
        {
            X1000 = fCalcX1000 (iMaxTemp, iMinTemp);        
            Gren = 100.0;
            if (HerbAnnual == 0)
            {
                switch (ClimateClass)
                {
                    case 1:
                        MCHERB = 7.4 * X1000 + 11.2;
                        break;
                    case 2:
                        MCHERB = 8.3 * X1000 - 10.3;
                        break;
                    case 3:
                        MCHERB = 9.8 * X1000 - 42.7;
                        break;
                    case 4:
                        MCHERB = 12.2 * X1000 - 93.5;
                }
            }
            else
            {
                switch (ClimateClass)
                {
                    case 1:
                        MCHERB = 18.4 * X1000 - 150.5;
                        break;
                    case 2:
                        MCHERB = 19.6 * X1000 - 187.7;
                        break;
                    case 3:
                        MCHERB = 22.0 * X1000 - 245.2;
                        break;
                    case 4:
                        MCHERB = 24.3 * X1000 - 305.2;
                }
            }

            if(HerbAnnual != 0) MCHERB = min(MCHERB, yMCHERB);  //CHANGED SB
            if (MCHERB < 30.0)  MCHERB = 30.0;
            if (MCHERB > 150.0) MCHERB = 150.0;
            if (MCHERB == 30.0) Stage = 5;

            //if (iJulian >= JulianLookFreeze && iMinTemp > -999)
            if ((iJulian >= JulianLookFreeze || iJulian < JulianGreenUp) && iMinTemp > -999)
            {
                if (iMinTemp <= 25)
                {
                    Stage = 6;
                }
                else
                {
                    if (iMinTemp <= 32) ColdDays++;
                    if (ColdDays >4) Stage = 6;
                }
            }
        }

        if (Stage == 5)//CURE
        {
            //X1000 = MC1000;
            X1000 = fCalcX1000 (iMaxTemp, iMinTemp);        
            Gren = 0.0;
            if (HerbAnnual == 0)
            {
                switch (ClimateClass)
                {
                    case 1:
                        MCHERB = 7.4 * X1000 + 11.2;
                        break;
                    case 2:
                        MCHERB = 8.3 * X1000 - 10.3;
                        break;
                    case 3:
                        MCHERB = 9.8 * X1000 - 42.7;
                        break;
                    case 4:
                        MCHERB = 12.2 * X1000 - 93.5;
                }
                if (MCHERB < 30.0 )  MCHERB = 30.0;
                if (MCHERB > 150.0) MCHERB = 150.0;
            }
            else
            {
                MCHERB = MC1;
            }

            //if (iJulian >= JulianLookFreeze && iMinTemp > -999)
            if ((iJulian >= JulianLookFreeze || iJulian < JulianGreenUp) && iMinTemp > -999)
            {
                if (iMinTemp <= 25)
                {
                    Stage = 6;
                }
                else
                {
                    if (iMinTemp <= 32) ColdDays++;
                    if (ColdDays > 4) Stage = 6;
                }
            }
        }

        if (Stage == 6)//FROZEN
        {
            ColdDays = 0;
            X1000 = MC1000;
            Gren = 0;
            MCHERB = MC1;
            if(iJulian <= JulianGreenUp)
                Stage = 1;
        }

        MCHERB = max(MCHERB, MC1);//CHANGED SB 2/17/2003

        /* Compute Woody Fuel Moistures */

        switch (ClimateClass)
        {
            case 1:
                fWoodP = 7.5 * MC1000 + 12.5;
                fWoodI = 50.0;
                break;
            case 2:
                fWoodP = 8.2 * MC1000 - 5;
                fWoodI = 60.0;
                break;
            case 3:
                fWoodP = 8.9 * MC1000 - 22.5;
                fWoodI = 70.0;
                break;
            case 4:
                fWoodP = 9.8 * MC1000 - 45;
                fWoodI = 80.0;
        }
        ///check for Dormant date on shrubs
        //if(Stage == 5 && JulianShrubDormant > 0 && iJulian >= JulianShrubDormant)// && fWoodP <= fWoodI)
        //2012/05/30 - change to remove stage 5 requirement
        if(JulianShrubDormant > 0 && iJulian >= JulianShrubDormant)
            Stage = 6;
        if (Stage == 1 || Stage == 6)
        {
            MCWOOD = fWoodI;
        }
        else
        {
            MCWOOD = min(200.0, fWoodP);
        }

        /* Following allows different method for counting greeenup for woody */

        if (Stage == 2 && iJulian <= JulianGreenUp + 7 * ClimateClass) 
            MCWOOD = fWoodI + Gren * (fWoodP - fWoodI) / 100.0;

        MCWOOD = min(200,max(fWoodI,MCWOOD));
    }
    // Block for 88 Curing Models, updated April 2004 to follow WIMS Code.

    if ((NFDRSVersion == 88) && (Season > 0))
    {

        // Initial Blocks are for Herbaceous Moistures
        // Note:  All Load Transfers are done in iCalcIndexes

        // Set transition value based on yesterdays herbaceous moisture. 

        transition = 0;
        if ( (Season == 3 || Season == 4) && yMCHERB < 120)
            transition = 1;

        if (Season == 1)      // Winter
        {
            MCHERB = MC1;
            X1000 = MC1000;
        }

        if (Season == 2 || (Season == 3 && transition == 0) )     // Spring or Summer Green Up Phase
        {//if prevseason != 1 set x1000 = mc1000
            if(PrevSeason == 1 || PrevSeason == 4 || (PrevSeason == 3 && Season == 2))
                X1000 = YX1000 = MC1000;
            X1000 = fCalcX1000 (iMaxTemp, iMinTemp);
            switch (ClimateClass)
            {
                case 1:
                    fHerbP = 12.8 * X1000 - 70;
                    break;
                case 2:
                    fHerbP = 14 * X1000 - 100;
                    break;
                case 3:
                    fHerbP = 15.5 * X1000 - 137.5;
                    break;
                case 4:
                    fHerbP = 17.4 * X1000 - 185;
            }

            if (fHerbP > 250.0) fHerbP = 250.0;
            if(fHerbP < MC1)
                fHerbP = MC1;
            MCHERB = MC1 + (GreenHerb / 20.0) * (fHerbP - MC1);
            transition = 0;
            if (Season == 3 && MCHERB < 120)
                transition = 1;
        }

        if (Season == 3 && transition == 1)  //Summer and drying 
        {       
            X1000 = fCalcX1000 (iMaxTemp, iMinTemp);

            if (HerbAnnual == 0)   // Perennials
            {
                switch (ClimateClass)
                {
                    case 1:
                        MCHERB = 7.4 * X1000 + 11.2;
                        break;
                    case 2:
                        MCHERB = 8.3 * X1000 - 10.3;
                        break;
                    case 3:
                        MCHERB = 9.8 * X1000 - 42.7;
                        break;
                    case 4:
                        MCHERB = 12.2 * X1000 - 93.5;
                }
                MCHERB = MCHERB * (GreenHerb / 20.0);
                if (MCHERB < 30.0)  MCHERB = 30.0;
                if (MCHERB > 150.0) MCHERB = 150.0;
            }
            else                    // Annuals
            {
                switch (ClimateClass)
                {
                    case 1:
                        MCHERB = 18.4 * X1000 - 150.5;
                        break;
                    case 2:
                        MCHERB = 19.6 * X1000 - 187.7;
                        break;
                    case 3:
                        MCHERB = 22.0 * X1000 - 245.2;
                        break;
                    case 4:
                        MCHERB = 24.3 * X1000 - 305.2;
                }
                MCHERB = MCHERB * (GreenHerb / 20.0);
                MCHERB = min(MCHERB, yMCHERB);  //CHANGED SB
                MCHERB = max(MC1,MCHERB) ;
            }
        }

        if (Season == 4)  //Fall 
        {       
            X1000 = fCalcX1000 (iMaxTemp, iMinTemp);

            if (HerbAnnual == 0)   // Perennials
            {
                switch (ClimateClass)
                {
                    case 1:
                        MCHERB = 7.4 * X1000 + 11.2;
                        break;
                    case 2:
                        MCHERB = 8.3 * X1000 - 10.3;
                        break;
                    case 3:
                        MCHERB = 9.8 * X1000 - 42.7;
                        break;
                    case 4:
                        MCHERB = 12.2 * X1000 - 93.5;
                }
                MCHERB = MCHERB * (GreenHerb / 20.0);
                if (MCHERB < 30.0)  MCHERB = MC1;
                if (MCHERB > 150.0) MCHERB = 150.0;

            }
            else                    // Annuals
            {
                switch (ClimateClass)
                {
                    case 1:
                        MCHERB = 18.4 * X1000 - 150.5;
                        break;
                    case 2:
                        MCHERB = 19.6 * X1000 - 187.7;
                        break;
                    case 3:
                        MCHERB = 22.0 * X1000 - 245.2;
                        break;
                    case 4:
                        MCHERB = 24.3 * X1000 - 305.2;
                }
                MCHERB = MCHERB * (GreenHerb / 20.0);
                MCHERB = min(MCHERB, yMCHERB);  //CHANGED SB
                MCHERB = min(30.,MC1) ;
            }
        }

        // Computations for Woody Fuel Moisture and 88 Live Fuel Moisture Model

        // fWoodP and fWoodI are set the same in woody block of 78 Model above

        // Woody Load Transfers for Deciduous 88 Brush Model is done in iCalcIndexes

        switch (ClimateClass)
        {
            case 1:
                fWoodP = 7.5 * MC1000 + 12.5;
                fWoodI = 50.0;
                break;
            case 2:
                fWoodP = 8.2 * MC1000 - 5.0;
                fWoodI = 60.0;
                break;
            case 3:
                fWoodP = 8.9 * MC1000 - 22.5;
                fWoodI = 70.0;
                break;
            case 4:
                fWoodP = 9.8 * MC1000 - 45;
                fWoodI = 80.0;
        }

        if (Season ==1 )   // Winter
        {
            MCWOOD = fWoodI;
        }
        else               // Spring/Summer/Fall
        {
            MCWOOD = fWoodI + (GreenWoody / 20.0) * (fWoodP - fWoodI);
            MCWOOD = min(200,max(fWoodI,MCWOOD));
        }
    }       
    return(1);
}



/* begin satvap *********************************************************
 *
 *
 ***************************************************************************/
double NFDRCalc::satvap (int iTemp)
{
    double kTemp = 0;

    kTemp = (iTemp - 32) / 1.8 + 273.16;
    return exp((double)1.81 + (kTemp * 17.27 - 4717.31) / (kTemp - 35.86));
}



/* begin eqmc   ************************************************************
 *
 *
 ***************************************************************************/
double NFDRCalc::eqmc (double fTemp, double fRH)
{
    double eqmc_ans = 0.0;

    if(fRH > 50)
        return ( 21.0606 + 0.005565 * pow((double)fRH,2) - 0.00035 * fRH * fTemp - 0.483199 * fRH);
    if ((fRH > 10) && (fRH < 51))
        return (2.22749 + 0.160107 * fRH - 0.014784 * fTemp);

    return (0.03229 + 0.281073 * fRH - 0.000578 * fRH * fTemp);
}

/* begin iCalcKBDI **********************************************************
 *
 *
 ************************************************************************/
int NFDRCalc::iCalcKBDI (double fPrecipAmt, int iMaxTemp,
        double fCummPrecip, int iYKBDI, double fAvgPrecip)
{
    int     net = 0,
            idq = 0;
    double pptnet = 0.00,
           xkbdi = 0.00,
           xtemp = 0.00;

    CummPrecip = fCummPrecip;
    KBDI = iYKBDI;

    if(fPrecipAmt == 0.0)
    {CummPrecip = 0;}
    else
    {if(CummPrecip > 0.20)
        {pptnet = fPrecipAmt;
            CummPrecip = CummPrecip + fPrecipAmt;
        }
        else
        {CummPrecip = CummPrecip + fPrecipAmt;
            if(CummPrecip > 0.20) pptnet = CummPrecip - 0.20;
        }
        net = (int)(100.0 * pptnet) + 0.0005;
        net = KBDI - net;
        KBDI = (net > 0) ? net : 0;
    }

    if(iMaxTemp > 50)
    {xkbdi = (int)KBDI;
        xtemp = (double)iMaxTemp;
        idq = (int)(800.0 - xkbdi) * (0.9679 * exp((double) 0.0486 * xtemp) -
                8.299) * 0.001 / (1.0 + 10.88 *
                    exp((double)-0.04409 * fAvgPrecip)) + 0.5;
        KBDI = KBDI + idq;
    }
    return (KBDI);
}


/* begin fCalcDaylightHours     *********************************************
 *
 *
 ***************************************************************************/
double NFDRCalc::fCalcDaylightHours (int iLat, int iJulian)
{
    double phi = 0.0,
           xfact = 0.0,
           decl = 0.0,
           tla = 0.0;

    /*   if (iLat < 0) iJulian = iJulian + 182;

         if (iJulian > 365) iJulian = iJulian - 365; */

    phi = tan((double)iLat * 0.01745) * -1.0;
    xfact = (iJulian - 80) * 0.01745;
    decl = 23.5 * sin(xfact);
    decl = decl * 0.01745;
    tla = phi * sin(decl);

    /* WIMS uses abs() */
    if (fabs(tla) < .01)
        tla = 0.01;
    else
    {
        if (tla >= 1.0)
            tla = 0.99999999;
        else
        {
            if (tla <= -1.0)
                tla = -.9999999;
        }
    }
    tla = atan(sqrt((1.0 - tla * tla))/tla);
    if (tla < 0.0)
        tla = tla + 3.141593;
    return tla * 7.64;
}



/* begin iCalcAdjustMC1 ****************************************************
 *
 *
 ***************************************************************************/
int NFDRCalc::iCalcAdjustMC1 (int iMonth, int iExposure,
        int iAspect, int iSite,
        int iSlope, int iHour)
{
    int iSeason = 1;
    int iPtr = 0;
    char sAdjust[2] = "0";

    if (iHour == 0)
        iHour = 14;
    if ((iHour < 8) || (iHour > 18))
        return 6;
    if ((iMonth > 4) && (iMonth < 8))
        iSeason = 0;
    else
        if ((iMonth == 1) || (iMonth > 10))
            iSeason = 2;
    iHour = ((iHour - 6) / 2) -1;

    if (iExposure == 1)
    {
        iPtr = 144 * iSeason + 36 * (iAspect - 1) + 18 * (iSlope -1) + 3 * iHour + iSite - 1;
        sAdjust[0] = sAdjExposed[iPtr];
        sAdjust[1] = 0;
        return atoi(sAdjust);
    }
    else
    {
        iPtr = 72 * iSeason + 18 * (iAspect - 1) + 3 * iHour + iSite -1;
        sAdjust[0] = sAdjShaded[iPtr];
        sAdjust[1] = 0;
        return atoi(sAdjust);
    }
}



/* begin iDoAdjustMC1 *****************************************************
 *
 *
 ***************************************************************************/
int NFDRCalc::iDoAdjustMC1 (int iAdjust)
{
    MC1 = MC1 + iAdjust;
    return(1);
}



/* begin iGetFuelBed    ******************************************************
 *
 *
 ***************************************************************************/
int NFDRCalc::iGetFuelBed (int* iSG1, int* iSG10,int* iSG100, int* iSG1000, int* iSGWOOD, int* iSGHERB, 
        double* fW1, double* fW10, double* fW100, double* fW1000, double* fWWOOD, double* fWHERB, double* fWDROUGHT,
        double* fDEPTH, int* iMXD, int* iHD, int* iSCM, double* fWNDFC, double* fWNDFCMin, double* fWNDFCMax)
{
    *iSG1 = SG1;
    *iSG10 = SG10;
    *iSG100 = SG100;
    *iSG1000 = SG1000;
    *iSGWOOD = SGWOOD;
    *iSGHERB = SGHERB;
    *fW1 = L1;
    *fW10 = L10;
    *fW100 = L100;
    *fW1000 = L1000;
    *fWWOOD = LWOOD;
    *fWHERB = LHERB;
    *fWDROUGHT = LDROUGHT;
    *fDEPTH = DEPTH;
    *iMXD = MXD;
    *iHD = HD;
    *iSCM = SCM;
    *fWNDFC = WNDFC;
    *fWNDFCMin = WNDFCMin;
    *fWNDFCMax = WNDFCMax;
    return (1);
}



/* begin iSetFuelBed    ******************************************************
 *
 *
 ***************************************************************************/
int NFDRCalc::iSetFuelBed (int iSG1, int iSG10,
        int iSG100, int iSG1000, int iSGWOOD,
        int iSGHERB, double fW1, double fW10,
        double fW100, double fW1000, double fWWOOD, double fWHERB,
        double fWDROUGHT,
        double fDEPTH, int iMXD, int iHD,
        int iSCM, double fWNDFC,
        double fWNDFCMin, double fWNDFCMax)
{
    if ((iMXD <= 0) || (fDEPTH <= 0) || (fWNDFC <= 0))
    { return (0);}
    else
    {
        SG1 = iSG1;
        SG10 = iSG10;
        SG100 = iSG100;
        SG1000 = iSG1000;
        SGWOOD = iSGWOOD;
        SGHERB = iSGHERB;
        L1 = fW1;
        L10 = fW10;
        L100 = fW100;
        L1000 = fW1000;
        LWOOD = fWWOOD;
        LHERB = fWHERB;
        LDROUGHT = fWDROUGHT;
        DEPTH = fDEPTH;
        MXD = iMXD;
        HD = iHD;
        SCM = iSCM;
        WNDFC = fWNDFC;
        WNDFCMin = fWNDFCMin;
        WNDFCMax = fWNDFCMax;

        if (SG1 <= 0) SG1 = 2000;
        if (SG10 <= 0) SG10 = 109;
        if (SG100 <=0 ) SG100 = 30;
        if (SG1000 <= 0) SG1000 = 8;
        if (SGWOOD <= 0) SGWOOD = 1;
        if (SGHERB <= 0) SGHERB = 1;

        W1 = L1 * CTA;
        W10 = L10 * CTA;
        W100 = L100 * CTA;
        W1000 = L1000 * CTA;
        WWOOD = LWOOD * CTA;
        WHERB = LHERB * CTA;

        FuelModel = 0;
        return (1);
    }
}



/* begin XferHerb       *********************************************************
 *
 *
 ***************************************************************************/
double NFDRCalc::XferHerb (double fMCHerb)
{
    double fctCur;

    fctCur = 1.33 - .0111 * fMCHerb;
    if (fctCur < 0) fctCur = 0.0;
    if (fctCur > 1) fctCur = 1.0;
    W1P = W1 + WHERB * fctCur;
    WHERBP = WHERB * (1 - fctCur);
    return (fctCur);
}



/* begin iSetMoistures  ****************************************************
 *
 *
 ***************************************************************************/
int NFDRCalc::iSetMoistures (double fMC1, double fMC10,
        double fMC100, double fMC1000, double fMCWood, double fMCHerb,
        int iRainEvent, int iKBDI, int iGreenHerb,
        int iGreenWoody, int iSeason, int iSOW)
{
    MC1 = fMC1;
    MC10 = fMC10;
    MC100 = fMC100;
    MC1000 = fMC1000;
    MCWOOD = fMCWood;
    MCHERB = fMCHerb;
    RainEvent = iRainEvent;
    KBDI = iKBDI;
    GreenHerb = iGreenHerb;
    GreenWoody = iGreenWoody;
    Season = iSeason;
    SOW = iSOW;
    return(1);
}



/* begin iGetFuelModel  ****************************************************
 *
 *
 ***************************************************************************/
int NFDRCalc::iGetFuelModel (int* iFM)
{
    *iFM = FuelModel;
    return(FuelModel);
}



/* begin irnd   ************************************************************
 *
 *
 ***************************************************************************/
int NFDRCalc::irnd(double fIn)
{
    int x = 0;
    double y = 0.0;
    x = (int) fIn;
    y = fIn - (double) x;
    if (y >= 0.5) x++;
    return (x);
}



/* begin iSetFuelModel  ****************************************************
 *
 *
 ***************************************************************************/
int NFDRCalc::iSetFuelModel (int iFM, int iNFMAS)
{
    /* Assure MXD > 0 */
    int iExit = 1;

    LDROUGHT = 0.0;
    WNDFCMin = 0.0;
    WNDFCMax = 0.0;
    ///added SB 5/14/02
    WNDFCSet = 0.0;
    switch (iFM)
    {
        case 65:      /* A */
            SG1 = 3000;
            SG10 = 0;
            SG100 = 0;
            SG1000 = 0;
            SGWOOD = 0;
            SGHERB = 3000;
            L1 = .2;
            L10 = 0;
            L100 = 0;
            L1000 = 0;
            LWOOD = 0;
            LHERB = .3;
            DEPTH = .8;
            MXD = 15;
            HD = 8000;
            SCM = 301;
            if (iNFMAS == 0)
            {
                WNDFC = 0.6;
            }
            else
            {
                WNDFC = .36;
            }
            break;

        case 66:      /* B */
            SG1 = 700;
            SG10 = 109;
            SG100 = 30;
            SG1000 = 8;
            SGWOOD = 1250;
            SGHERB = 0;
            L1 = 3.5;
            L10 = 4;
            L100 = .5;
            L1000 = 0;
            LWOOD = 11.5;
            LHERB = 0;
            DEPTH = 4.5;
            MXD = 15;
            HD = 9500;
            SCM = 58;
            if (iNFMAS == 0)
            {
                WNDFC = 0.5;
            }
            else
            {
                WNDFC = .55;
            }
            break;

        case 67:      /* C */
            SG1 = 2000;
            SG10 = 109;
            SG100 = 30;
            SG1000 = 0;
            SGWOOD = 1500;
            SGHERB = 2500;
            L1 = .4;
            L10 = 1;
            L100 = 0;
            L1000 = 0;
            LWOOD = .5;
            LHERB = .8;
            DEPTH = .75;
            MXD = 20;
            HD = 8000;
            SCM = 32;
            if (iNFMAS == 0)
            {
                WNDFC = 0.4;
            }
            else
            {
                WNDFC = .25;
            }
            break;

        case 68:      /* D */
            SG1 = 1250;
            SG10 = 109;
            SG100 = 0;
            SG1000 = 0;
            SGWOOD = 1500;
            SGHERB = 1500;
            L1 = 2;
            L10 = 1;
            L100 = 0;
            L1000 = 0;
            LWOOD = 3;
            LHERB = .75;
            DEPTH = 2;
            MXD = 30;
            HD = 9000;
            SCM = 68;
            if (iNFMAS == 0)
            {
                WNDFC = 0.4;
            }
            else
            {
                WNDFC = .12;
            }
            break;

        case 69:      /* E */
            SG1 = 2000;
            SG10 = 109;
            SG100 = 30;
            SG1000 = 0;
            SGWOOD = 1500;
            SGHERB = 2000;
            L1 = 1.5;
            L10 = .5;
            L100 = .25;
            L1000 = 0;
            LWOOD = .5;
            LHERB = .5;
            DEPTH = .4;
            MXD = 25;
            HD = 8000;
            SCM = 25;
            if (iNFMAS == 0)
            {
                //WNDFC = 0.5; CHANGED SB
                WNDFC = 0.4;
            }
            else
            {
                WNDFC = .25;
            }
            break;

        case 70:      /* F */
            SG1 = 700;
            SG10 = 109;
            SG100 = 30;
            SG1000 = 0;
            SGWOOD = 1250;
            SGHERB = 0;
            L1 = 2.5;
            L10 = 2;
            L100 = 1.5;
            L1000 = 0;
            LWOOD = 9;
            LHERB = 0;
            DEPTH = 4.5;
            MXD = 15;
            HD = 9500;
            SCM = 24;
            if (iNFMAS == 0)
            {
                WNDFC = 0.5;
            }
            else
            {
                WNDFC = .42;
            }
            break;

        case 71:      /* G */
            SG1 = 2000;
            SG10 = 109;
            SG100 = 30;
            SG1000 = 8;
            SGWOOD = 1500;
            SGHERB = 2000;
            L1 = 2.5;
            L10 = 2;
            L100 = 5;
            L1000 = 12;
            LWOOD = .5;
            LHERB = .5;
            DEPTH = 1;
            MXD = 25;
            HD = 8000;
            SCM = 30;
            if (iNFMAS == 0)
            {
                WNDFC = 0.4;
            }
            else
            {
                WNDFC = .12;
            }
            break;

        case 72:      /* H */
            SG1 = 2000;
            SG10 = 109;
            SG100 = 30;
            SG1000 = 8;
            SGWOOD = 1500;
            SGHERB = 2000;
            L1 = 1.5;
            L10 = 1;
            L100 = 2;
            L1000 = 2;
            LWOOD = .5;
            LHERB = .5;
            DEPTH = .3;
            MXD = 20;
            HD = 8000;
            SCM = 8;
            if (iNFMAS == 0)
            {
                WNDFC = 0.4;
            }
            else
            {
                WNDFC = .17;
            }
            break;

        case 73:      /* I */
            SG1 = 1500;
            SG10 = 109;
            SG100 = 30;
            SG1000 = 8;
            SGWOOD = 0;
            SGHERB = 0;
            L1 = 12;
            L10 = 12;
            L100 = 10;
            L1000 = 12;
            LWOOD = 0;
            LHERB = 0;
            DEPTH = 2;
            MXD = 25;
            HD = 8000;
            SCM = 65;
            if (iNFMAS == 0)
            {
                WNDFC = 0.5;
            }
            else
            {
                WNDFC = .46;
            }
            break;

        case 74:      /* J */
            SG1 = 1500;
            SG10 = 109;
            SG100 = 30;
            SG1000 = 8;
            SGWOOD = 0;
            SGHERB = 0;
            L1 = 7;
            L10 = 7;
            L100 = 6;
            L1000 = 5.5;
            LWOOD = 0;
            LHERB = 0;
            DEPTH = 1.3;
            MXD = 25;
            HD = 8000;
            SCM = 44;
            if (iNFMAS == 0)
            {
                WNDFC = 0.5;
            }
            else
            {
                WNDFC = .43;
            }
            break;

        case 75:      /* K */
            SG1 = 1500;
            SG10 = 109;
            SG100 = 30;
            SG1000 = 8;
            SGWOOD = 0;
            SGHERB = 0;
            L1 = 2.5;
            L10 = 2.5;
            L100 = 2;
            L1000 = 2.5;
            LWOOD = 0;
            LHERB = 0;
            DEPTH = .6;
            MXD = 25;
            HD = 8000;
            SCM = 23;
            if (iNFMAS == 0)
            {
                WNDFC = 0.5;
            }
            else
            {
                WNDFC = .36;
            }
            break;

        case 76:      /* L */
            SG1 = 2000;
            SG10 = 0;
            SG100 = 0;
            SG1000 = 0;
            SGWOOD = 0;
            SGHERB = 2000;
            L1 = .25;
            L10 = 0;
            L100 = 0;
            L1000 = 0;
            LWOOD = 0;
            LHERB = .5;
            DEPTH = 1;
            MXD = 15;
            HD = 8000;
            SCM = 178;
            if (iNFMAS == 0)
            {
                WNDFC = 0.6;
            }
            else
            {
                WNDFC = .44;
            }
            break;

        case 78:      /* N */
            SG1 = 1600;
            SG10 = 109;
            SG100 = 0;
            SG1000 = 0;
            SGWOOD = 1500;
            SGHERB = 0;
            L1 = 1.5;
            L10 = 1.5;
            L100 = 0;
            L1000 = 0;
            LWOOD = 2;
            LHERB = 0;
            DEPTH = 3;
            MXD = 25;
            HD = 8700;
            SCM = 167;
            if (iNFMAS == 0)
            {
                WNDFC = 0.6;
            }
            else
            {
                WNDFC = .55;
            }
            break;

        case 79:      /* O */
            SG1 = 1500;
            SG10 = 109;
            SG100 = 30;
            SG1000 = 8;
            SGWOOD = 1500;
            SGHERB = 1500;
            L1 = 2;
            L10 = 3;
            L100 = 3;
            L1000 = 2;
            LWOOD = 7;
            LHERB = 0;
            DEPTH = 4;
            MXD = 30;
            HD = 9000;
            SCM = 99;
            if (iNFMAS == 0)
            {
                WNDFC = 0.5;
            }
            else
            {
                WNDFC = .55;
            }
            break;

        case 80:      /* P */
            SG1 = 1750;
            SG10 = 109;
            SG100 = 30;
            SG1000 = 0;
            SGWOOD = 1500;
            SGHERB = 2000;
            L1 = 1;
            L10 = 1;
            L100 = .5;
            L1000 = 0;
            LWOOD = .5;
            LHERB = .5;
            DEPTH = .4;
            MXD = 30;
            HD = 8000;
            SCM = 14;
            if (iNFMAS == 0)
            {
                WNDFC = 0.4;
            }
            else
            {
                WNDFC = .12;
            }
            break;

        case 81:      /* Q */
            SG1 = 1500;
            SG10 = 109;
            SG100 = 30;
            SG1000 = 8;
            SGWOOD = 1200;
            SGHERB = 1500;
            L1 = 2;
            L10 = 2.5;
            L100 = 2;
            L1000 = 1;
            LWOOD = 4;
            LHERB = .5;
            DEPTH = 3;
            MXD = 25;
            HD = 8000;
            SCM = 59;
            if (iNFMAS == 0)
            {
                WNDFC = 0.4;
            }
            else
            {
                WNDFC = .63;
            }
            break;

        case 82:      /* R */
            SG1 = 1500;
            SG10 = 109;
            SG100 = 30;
            SG1000 = 8;
            SGWOOD = 1500;
            SGHERB = 2000;
            L1 = .5;
            L10 = .5;
            L100 = .5;
            L1000 = 0;
            LWOOD = .5;
            LHERB = .5;
            DEPTH = .25;
            MXD = 25;
            HD = 8000;
            SCM = 6;
            if (iNFMAS == 0)
            {
                WNDFC = 0.4;
            }
            else
            {
                WNDFC = .08;
            }
            break;

        case 83:      /* S */
            SG1 = 1500;
            SG10 = 106;
            SG100 = 30;
            SG1000 = 8;
            SGWOOD = 1200;
            SGHERB = 1500;
            L1 = .5;
            L10 = .5;
            L100 = .5;
            L1000 = .5;
            LWOOD = .5;
            LHERB = .5;
            DEPTH = .4;
            MXD = 25;
            HD = 8000;
            SCM = 17;
            if (iNFMAS == 0)
            {
                WNDFC = 0.6;
            }
            else
            {
                WNDFC = .45;
            }
            break;

        case 84:      /* T */
            SG1 = 2500;
            SG10 = 109;
            SG100 = 0;
            SG1000 = 0;
            SGWOOD = 1500;
            SGHERB = 2000;
            L1 = 1;
            L10 = .5;
            L100 = 0;
            L1000 = 0;
            LWOOD = 2.5;
            LHERB = .5;
            DEPTH = 1.25;
            MXD = 15;
            HD = 8000;
            SCM = 96;
            if (iNFMAS == 0)
            {
                WNDFC = 0.6;
            }
            else
            {
                WNDFC = .42;
            }
            break;

        case 85:      /* U */
            SG1 = 1750;
            SG10 = 109;
            SG100 = 30;
            SG1000 = 0;
            SGWOOD = 1500;
            SGHERB = 2000;
            L1 = 1.5;
            L10 = 1.5;
            L100 = 1;
            L1000 = 0;
            LWOOD = .5;
            LHERB = .5;
            DEPTH = .5;
            MXD = 20;
            HD = 8000;
            SCM = 16;
            if (iNFMAS == 0)
            {
                WNDFC = 0.4;
            }
            else
            {
                WNDFC = .12;
            }
            break;

        default:
            iExit = 0;

    }

    if (iExit == 1)
    {
        if (SG1 <= 0) SG1 = 2000;
        if (SG10 <= 0) SG10 = 109;
        if (SG100 <=0 ) SG100 = 30;
        if (SG1000 <= 0) SG1000 = 8;
        if (SGWOOD <= 0) SGWOOD = 1;
        if (SGHERB <= 0) SGHERB = 1;

        if (NFDRSVersion == 88)
        {
            /*             WNDFCSet = WNDFC;  This should be done at end now to pick up changed WAF values in 5 88 Models */
            switch (iFM)
            {
                case 65: /* A */
                    LDROUGHT = .2;
                    WNDFCMin = .6;
                    WNDFCMax = .6;
                    break;

                case 66: /* B */
                    LDROUGHT = 3.5;
                    WNDFCMin = .5;
                    WNDFCMax = .5;
                    break;

                case 67: /* C */
                    LDROUGHT = 1.8;
                    WNDFCMin = .3;
                    WNDFCMax = .5;
                    LWOOD = 0.8;
                    /*             DEPTH = 0.25;  Removed by LSB 15-Mar-2004 to correct R Burgans typo in 1988 publication */
                    break;

                case 68: /* D */
                    LDROUGHT = 1.5;
                    WNDFCMin = .4;
                    WNDFCMax = .4;
                    LHERB = 1.0;
                    MXD = 40;
                    break;

                case 69: /* E */
                    LDROUGHT = 1.5;
                    WNDFCMin = .3;
                    WNDFCMax = .5;
                    L1 = 1.0;
                    LWOOD = 1.0;
                    break;

                case 70: /* F */
                    LDROUGHT = 2.5;
                    WNDFCMin = .5;
                    WNDFCMax = .5;
                    LWOOD = 7.0;
                    LHERB = 1.0;
                    SGHERB = 1500;
                    break;

                case 71: /* G */
                    LDROUGHT = 5.0;
                    WNDFCMin = 0.3;
                    WNDFCMax = 0.3;
                    break;

                case 72: /* H */
                    LDROUGHT = 2.0;
                    WNDFCMin = .3;
                    WNDFCMax = .3;
                    break;

                case 73: /* I */
                    LDROUGHT = 12.0;
                    WNDFCMin = .5;
                    WNDFCMax = .5;
                    break;

                case 74: /* J */
                    LDROUGHT = 7.0;
                    WNDFCMin = .5;
                    WNDFCMax = .5;
                    break;

                case 75: /* K */
                    LDROUGHT = 2.5;
                    WNDFCMin = .5;
                    WNDFCMax = .5;
                    break;

                case 76: /* L */
                    LDROUGHT = 0.25;
                    WNDFC = 0.5;   /* 88 changed from 0.6 to 0.5 */
                    WNDFCMin = .5;
                    WNDFCMax = .5;
                    break;

                case 78: /* N */
                    LDROUGHT = 2.0;
                    WNDFC = 0.5;   /* 88 changed from 0.6 to 0.5 */
                    WNDFCMin = .5;
                    WNDFCMax = .5;
                    MXD = 40;
                    break;

                case 79: /* O */
                    LDROUGHT = 3.5;
                    WNDFCMin = .5;
                    WNDFCMax = .5;
                    break;

                case 80: /* P */
                    LDROUGHT = 1.0;
                    WNDFC = 0.3;   /* 88 changed from 0.4 to 0.3 */
                    WNDFCMin = .3;
                    WNDFCMax = .3;
                    break;

                case 81: /* Q */
                    LDROUGHT = 3.5;
                    WNDFC = 0.2;   /* 88 changed from 0.4 to 0.2 */
                    WNDFCMin = .2;
                    WNDFCMax = .3;
                    L1 = 2.5;
                    L10 = 5.4;
                    L100 = 2.9;
                    LWOOD = 3.0;
                    LHERB = 1.0;
                    SG1 = 3500;
                    SGWOOD = 1500;
                    MXD = 18;
                    DEPTH = 2.0;
                    break;

                case 82: /* R */
                    LDROUGHT = 0.5;
                    WNDFCMin = .3;
                    WNDFCMax = .5;
                    break;
                case 83: /* S */
                    LDROUGHT = 1.5;
                    WNDFCMin = .6;
                    WNDFCMax = .6;
                    break;
                case 84: /* T */
                    LDROUGHT = 1.0;
                    WNDFCMin = .6;
                    WNDFCMax = .6;
                    break;
                case 85: /* U */
                    LDROUGHT = 2.0;
                    WNDFC = 0.3;   /* 88 changed from 0.4 to 0.3 */
                    WNDFCMin = .3;
                    WNDFCMax = .3;
                    break;

            }
            WNDFCSet = WNDFC;  /* Picks up changed WNDFC values in 5 88 Models - L, N, P, Q, U */
        }

        W1 = L1 * CTA;
        W10 = L10 * CTA;
        W100 = L100 * CTA;
        W1000 = L1000 * CTA;
        WWOOD = LWOOD * CTA;
        WHERB = LHERB * CTA;
        WDROUGHT = LDROUGHT * CTA;

        FuelModel = iFM;
    }
    return(iExit);
}

int NFDRCalc::GetMoistures (double& fMC1, double& fMC10, double& fMC100, double& fMC1000, 
        double& fMCWood, double& fMCHerb, int& iRainEvent, int& iKBDI, double& fX1000, int& iGreenHerb,
        int& iGreenWoody, int& iSeason, int& iSOW)
{
    fMC1 = MC1;
    fMC10 = MC10;
    fMC100 = MC100;
    fMC1000 = MC1000;
    fMCHerb = MCHERB;
    fMCWood = MCWOOD;
    iRainEvent = RainEvent;
    fX1000 = X1000;
    iKBDI = KBDI;
    iGreenHerb = GreenHerb;
    iGreenWoody = GreenWoody;
    iSeason = Season;
    iSOW = SOW;
    return 1;
}

int NFDRCalc::SetHists(double h1000s[], double hBndrys[])
{
    for(int i = 0; i < 7; i++)
    {
        Hist1000[i] = h1000s[i];
        HistBndryT[i] = hBndrys[i];
    }
    return 1;
}

int NFDRCalc::GetHists(double h1000s[], double hBndrys[])
{
    for(int i = 0; i < 7; i++)
    {
        h1000s[i] = Hist1000[i];
        hBndrys[i] = HistBndryT[i];
    }
    return 1;
}

/* SaveState **********************************************************
   Saves all internal variables to a data structure
 **************************************************************************/
int NFDRCalc::SaveState(NFDRCalcState *state)
{
    for(int i = 0; i < 7; i++)
    {
        state->Hist1000[i] = Hist1000[i];
        state->HistBndryT[i] = HistBndryT[i];
    }
    state->StartKBDI = StartKBDI;
    state->FuelModel = FuelModel;
    state->SG1 = SG1;
    state->SG10 = SG10;
    state->SG100 = SG100;
    state->SG1000 = SG1000;
    state->SGWOOD = SGWOOD; 
    state->SGHERB = SGHERB;
    state->W1 = W1; 
    state->W10 = W10; 
    state->W100 = W100; 
    state->W1000 = W1000; 
    state->WWOOD = WWOOD; 
    state->WHERB = WHERB; 
    state->WDROUGHT = WDROUGHT; 
    state->W1P = W1P; 
    state->WHERBP = WHERBP;
    state->L1 = L1; 
    state->L10 = L10; 
    state->L100 = L100; 
    state->L1000 = L1000; 
    state->LWOOD = LWOOD; 
    state->LHERB = LHERB; 
    state->LDROUGHT = LDROUGHT;
    state->DEPTH = DEPTH; 
    state->WNDFC = WNDFC; 
    state->WNDFCMax = WNDFCMax; 
    state->WNDFCMin = WNDFCMin; 
    state->WNDFCSet = WNDFCSet;
    state->MXD = MXD; 
    state->HD = HD; 
    state->SCM = SCM;       
    state->MC1 = MC1; 
    state->MC10 = MC10; 
    state->MC100 = MC100; 
    state->MC1000 = MC1000; 
    state->MCWOOD = MCWOOD; 
    state->MCHERB = MCHERB; 
    state->CTA = CTA;       
    state->X1000 = X1000; 
    state->YX1000 = YX1000; 
    state->Y1000 = Y1000; 
    state->Y100 = Y100; 
    state->Gren = Gren; 
    state->Start100 = Start100; 
    state->Start1000 = Start1000;
    state->PrevStage = PrevStage; 
    state->PrevYear = PrevYear; 
    state->PrevJulian = PrevJulian; 
    state->HerbAnnual = HerbAnnual; 
    state->GreenDays = GreenDays; 
    state->ClimateClass = ClimateClass;
    state->ColdDays = ColdDays; 
    state->JulianGreenUp = JulianGreenUp; 
    state->JulianStartedGreen = JulianStartedGreen;
    state->JulianLookFreeze = JulianLookFreeze; 
    state->Lat = Lat; 
    state->Stage = Stage; 
    state->OneIsTen = OneIsTen;
    state->WoodyDeciduous = WoodyDeciduous; 
    state->NFDRSVersion = NFDRSVersion;
    state->Season = Season; 
    state->KBDI = KBDI; 
    state->YKBDI = YKBDI; 
    state->GreenHerb = GreenHerb; 
    state->GreenWoody = GreenWoody; 
    state->RainEvent = RainEvent;
    state->PrevPrecip = PrevPrecip; 
    state->CummPrecip = CummPrecip; 
    state->AvgPrecip = AvgPrecip;
    state->SOW = SOW;
    state->valid = true;
    return 1;
}
/* LoadState **********************************************************
   Loads a previously saved data structure
 **************************************************************************/
int NFDRCalc::LoadState(NFDRCalcState *state)
{
    //if(!state.valid)
    //      return 0;
    for(int i = 0; i < 7; i++)
    {
        Hist1000[i] = state->Hist1000[i];
        HistBndryT[i] = state->HistBndryT[i];
    }
    StartKBDI = state->StartKBDI;
    FuelModel = state->FuelModel;
    SG1 = state->SG1;
    SG10 = state->SG10;
    SG100 = state->SG100;
    SG1000 = state->SG1000;
    SGWOOD = state->SGWOOD; 
    SGHERB = state->SGHERB;
    W1 = state->W1; 
    W10 = state->W10; 
    W100 = state->W100; 
    W1000 = state->W1000; 
    WWOOD = state->WWOOD; 
    WHERB = state->WHERB; 
    WDROUGHT = state->WDROUGHT; 
    W1P = state->W1P; 
    WHERBP = state->WHERBP;
    L1 = state->L1; 
    L10 = state->L10; 
    L100 = state->L100; 
    L1000 = state->L1000; 
    LWOOD = state->LWOOD; 
    LHERB = state->LHERB; 
    LDROUGHT = state->LDROUGHT;
    DEPTH = state->DEPTH; 
    WNDFC = state->WNDFC; 
    WNDFCMax = state->WNDFCMax; 
    WNDFCMin = state->WNDFCMin; 
    WNDFCSet = state->WNDFCSet;
    MXD = state->MXD; 
    HD = state->HD; 
    SCM = state->SCM;       
    MC1 = state->MC1; 
    MC10 = state->MC10; 
    MC100 = state->MC100; 
    MC1000 = state->MC1000; 
    MCWOOD = state->MCWOOD; 
    MCHERB = state->MCHERB; 
    CTA = state->CTA;       
    X1000 = state->X1000; 
    YX1000 = state->YX1000; 
    Y1000 = state->Y1000; 
    Y100 = state->Y100; 
    Gren = state->Gren; 
    Start100 = state->Start100; 
    Start1000 = state->Start1000;
    PrevStage = state->PrevStage; 
    PrevYear = state->PrevYear; 
    PrevJulian = state->PrevJulian; 
    HerbAnnual = state->HerbAnnual; 
    GreenDays = state->GreenDays; 
    ClimateClass = state->ClimateClass;
    ColdDays = state->ColdDays; 
    JulianGreenUp = state->JulianGreenUp; 
    JulianStartedGreen = state->JulianStartedGreen;
    JulianLookFreeze = state->JulianLookFreeze; 
    Lat = state->Lat; 
    Stage = state->Stage; 
    OneIsTen = state->OneIsTen;
    WoodyDeciduous = state->WoodyDeciduous; 
    NFDRSVersion = state->NFDRSVersion;
    Season = state->Season; 
    KBDI = state->KBDI; 
    YKBDI = state->YKBDI; 
    GreenHerb = state->GreenHerb; 
    GreenWoody = state->GreenWoody; 
    RainEvent = state->RainEvent;
    PrevPrecip = state->PrevPrecip; 
    CummPrecip = state->CummPrecip; 
    AvgPrecip = state->AvgPrecip;
    SOW = state->SOW;
    return 1;
}

void NFDRCalcState::Copy(NFDRCalcState *state)
{
    for(int i = 0; i < 7; i++)
    {
        state->Hist1000[i] = Hist1000[i];
        state->HistBndryT[i] = HistBndryT[i];
    }
    state->StartKBDI = StartKBDI;
    state->FuelModel = FuelModel;
    state->SG1 = SG1;
    state->SG10 = SG10;
    state->SG100 = SG100;
    state->SG1000 = SG1000;
    state->SGWOOD = SGWOOD; 
    state->SGHERB = SGHERB;
    state->W1 = W1; 
    state->W10 = W10; 
    state->W100 = W100; 
    state->W1000 = W1000; 
    state->WWOOD = WWOOD; 
    state->WHERB = WHERB; 
    state->WDROUGHT = WDROUGHT; 
    state->W1P = W1P; 
    state->WHERBP = WHERBP;
    state->L1 = L1; 
    state->L10 = L10; 
    state->L100 = L100; 
    state->L1000 = L1000; 
    state->LWOOD = LWOOD; 
    state->LHERB = LHERB; 
    state->LDROUGHT = LDROUGHT;
    state->DEPTH = DEPTH; 
    state->WNDFC = WNDFC; 
    state->WNDFCMax = WNDFCMax; 
    state->WNDFCMin = WNDFCMin; 
    state->WNDFCSet = WNDFCSet;
    state->MXD = MXD; 
    state->HD = HD; 
    state->SCM = SCM;       
    state->MC1 = MC1; 
    state->MC10 = MC10; 
    state->MC100 = MC100; 
    state->MC1000 = MC1000; 
    state->MCWOOD = MCWOOD; 
    state->MCHERB = MCHERB; 
    state->CTA = CTA;       
    state->X1000 = X1000; 
    state->YX1000 = YX1000; 
    state->Y1000 = Y1000; 
    state->Y100 = Y100; 
    state->Gren = Gren; 
    state->Start100 = Start100; 
    state->Start1000 = Start1000;
    state->PrevStage = PrevStage; 
    state->PrevYear = PrevYear; 
    state->PrevJulian = PrevJulian; 
    state->HerbAnnual = HerbAnnual; 
    state->GreenDays = GreenDays; 
    state->ClimateClass = ClimateClass;
    state->ColdDays = ColdDays; 
    state->JulianGreenUp = JulianGreenUp; 
    state->JulianStartedGreen = JulianStartedGreen;
    state->JulianLookFreeze = JulianLookFreeze; 
    state->Lat = Lat; 
    state->Stage = Stage; 
    state->OneIsTen = OneIsTen;
    state->WoodyDeciduous = WoodyDeciduous; 
    state->NFDRSVersion = NFDRSVersion;
    state->Season = Season; 
    state->KBDI = KBDI; 
    state->YKBDI = YKBDI; 
    state->GreenHerb = GreenHerb; 
    state->GreenWoody = GreenWoody; 
    state->RainEvent = RainEvent;
    state->PrevPrecip = PrevPrecip; 
    state->CummPrecip = CummPrecip; 
    state->AvgPrecip = AvgPrecip;
    state->SOW = SOW;
    state->valid = valid;

}

/* GetL1 ***************************************************************
   Retrieves 1 Hour Loading
 **************************************************************************/
double NFDRCalc::GetL1()
{
    return L1;
}

/* GetL10 ***************************************************************
   Retrieves 10 Hour Loading
 **************************************************************************/
double NFDRCalc::GetL10()
{
    return L10;
}

/* GetL100 ***************************************************************
   Retrieves 100 Hour Loading
 **************************************************************************/
double NFDRCalc::GetL100()
{
    return L100;
}

/* GetL1000 ***************************************************************
   Retrieves 1000 Hour Loading
 **************************************************************************/
double NFDRCalc::GetL1000()
{
    return L1000;
}

/* GetLHerb ***************************************************************
   Retrieves Herbaceous Loading
 **************************************************************************/
double NFDRCalc::GetLHerb()
{
    return LHERB;
}

/* GetLWood ***************************************************************
   Retrieves Woody Hour Loading
 **************************************************************************/
double NFDRCalc::GetLWood()
{
    return LWOOD;
}

/* GetLDrought ***************************************************************
   Retrieves Drought Loading
 **************************************************************************/
double NFDRCalc::GetLDrought()
{
    return LDROUGHT;
}

/* GetSG1 ***************************************************************
   Retrieves 1 Hour Surface Area to Volume Ratio
 **************************************************************************/
int NFDRCalc::GetSG1()
{
    return SG1;
}

/* GetSG10 ***************************************************************
   Retrieves 10 Hour Surface Area to Volume Ratio
 **************************************************************************/
int NFDRCalc::GetSG10()
{
    return SG10;
}

/* GetSG100 ***************************************************************
   Retrieves 100 Hour Surface Area to Volume Ratio
 **************************************************************************/
int NFDRCalc::GetSG100()
{
    return SG100;
}

/* GetSG1000 ***************************************************************
   Retrieves 1000 Hour Surface Area to Volume Ratio
 **************************************************************************/
int NFDRCalc::GetSG1000()
{
    return SG1000;
}

/* GetSGHerb ***************************************************************
   Retrieves Herb Surface Area to Volume Ratio
 **************************************************************************/
int NFDRCalc::GetSGHerb()
{
    return SGHERB;
}

/* GetSGWood ***************************************************************
   Retrieves Woody Surface Area to Volume Ratio
 **************************************************************************/
int NFDRCalc::GetSGWood()
{
    return SGWOOD;
}

/* GetHD ***************************************************************
   Retrieves Heating Number
 **************************************************************************/
int NFDRCalc::GetHD()
{
    return HD;
}

/* GetMXD ***************************************************************
   Retrieves Moisture Extinction
 **************************************************************************/
int NFDRCalc::GetMXD()
{
    return MXD;
}

/* GetSCM ***************************************************************
   Retrieves Max Spread Component
 **************************************************************************/
int NFDRCalc::GetSCM()
{
    return SCM;
}

/* GetWindFC ***************************************************************
   Retrieves Wind Factor
 **************************************************************************/
double NFDRCalc::GetWindFC()
{
    return WNDFC;
}

/* GetDepth ***************************************************************
   Retrieves Depth
 **************************************************************************/
double NFDRCalc::GetDepth()
{
    return DEPTH;
}


void NFDRCalc::SetVersion(int version)
{
    NFDRSVersion = (version == 88) ? 88 : 78;
}

