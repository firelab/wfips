/*! \file fms.h
    \brief Fuel Moisture Stick (FMS) C source header.

    The FMS function library models fuel stick temperature and moisture content 
    given an input weather stream of air temperature, air humidity, solar 
    radiation, and cumulative rainfall.

    See also:
    \arg README file for details on function usage, and
    \arg CHANGES file for details on updates and modifications.

    \author Copyright (c) 1997-2000 by Collin D. Bevins.  All rights reserved.
    \version 0.7.0
 */

#ifndef _FMSTICK_LIB
#define _FMSTICK_LIB 1

#define FMS_SIZECLASS_1HR 		1
#define FMS_SIZECLASS_10HR         2
#define FMS_SIZECLASS_100HR        3
#define FMS_SIZECLASS_1000HR       4
#define FMS_SIZECLASS_10000HR      5


#include <stdio.h>
#include "cdtlib.h"

#ifdef __cplusplus
#   define  EXTERN extern "C"
#else
#   define  EXTERN extern
#   define  true 1
#   define  false 0
#endif


/*
 *------------------------------------------------------------------------------
 *  FMS library return codes.
 *------------------------------------------------------------------------------
 */

#define FMS_OK                  (0)
#define FMS_ERROR               (-1)


/*----------------------------------------------------------------------------*/
//  Stick moisture states.
//----------------------------------------------------------------------------*/

#define FMS_STATE_None          (0)
#define FMS_STATE_Adsorption    (1)
#define FMS_STATE_Desorption    (2)
#define FMS_STATE_Condensation  (3)
#define FMS_STATE_Evaporation   (4)
#define FMS_STATE_Rainfall      (5)
#define FMS_STATE_Rainstorm     (6)
#define FMS_STATE_Stagnation    (7)
#define FMS_STATE_Error         (8)

/*! \struct Fms
    \brief Fuel moisture stick data structure.
 */

typedef struct
{
    /* Stick physical properties passed as input to Fms_Create(). */
    double a;       /*!< Stick radius                                   (cm). */
    double al;      /*!< Stick length                                   (cm). */
    double ddt;     /*!< Stick diffusivity computation interval          (h). */
    double dp;      /*!< Stick density                               (g/cm3). */
    double bp;      /*!< Stick barometric pressure                 (cal/cm3). */
    double hc;      /*!< Stick planar heat transfer            (cal/cm2-h-C). */
    double mdt;     /*!< Stick moisture content computation interval     (h). */
    double rai0;    /*!< Runoff factor during initial rainfall observation.   */
    double rai1;    /*!< Runoff factor after initial rainfall observation.    */
    double stca;    /*!< Surface mass transfer rate- adsorption ((cm3/cm2)/h) */
    double stcd;    /*!< Surface mass transfer rate- desorption ((cm3/cm2)/h) */
    double stv;     /*!< Storm transition value                       (cm/h). */
    double wfilmk;  /*!< Water film contribution to moisture content (gm/gm). */
    double wmx;     /*!< Stick maximum local moisture due to rain      (g/g). */

    /* Stick environmental variables. */
    double jdate;   /*!< Julian date of last update                   (days). */
    double ta0;     /*!< Previous air temperature                       (oC). */
    double ta1;     /*!< Current air temperature                        (oC). */
    double ha0;     /*!< Previous air humidity                          (dl). */
    double ha1;     /*!< Current air humidity                           (dl). */
    double sv0;     /*!< Previous solar radiation                       (mV). */
    double sv1;     /*!< Current solar radiation                        (mV). */
    double rc0;     /*!< Previous cumulative rainfall amount            (cm). */
    double rc1;     /*!< Current cumulative rainfall amount             (cm). */
    double ra0;     /*!< Previous observation period's rainfall amount  (cm). */
    double ra1;     /*!< Current observation period's rainfall amount   (cm). */

    /* Stick-dependent intermediates derived in Fms_CreateParameters() */
    double dx;      /*!< Internodal radial distance                     (cm). */
    double wmax;    /*!< Maximum possible stick moisture content       (g/g). */

    /* Stick-dependent optimization factors derived in Fms_CreateStick(). */
    double amlf;    /*!< \a aml optimization factor.                          */
    double capf;    /*!< \a cap optimization factor.                          */
    double hwf;     /*!< \a hw and \a aml computation factor.                 */
    double mdt_2;   /*!< 2 times the moisture time step                  (h). */
    double dx_2;    /*!< 2 times the internodal distance                (cm). */
    double sf;      /*!< optimization factor used in Fms_Update().            */
    double vf;      /*!< optimization factor used in Fms_Update().            */

    /* Stick derived output parameters. */
    double hf;      /*!< Stick surface humidity                        (g/g). */
    double wsa;     /*!< Stick fiber saturation point                  (g/g). */
    double sem;     /*!< Stick equilibrium moisture content            (g/g). */
    double wfilm;   /*!< Amount of water film (0 or \a wfilmk)         (g/g). */
    double *x;      /*!< Node's radial distance from center             (cm). */
    double *t;      /*!< Stick temperature                              (oC). */
    double *s;      /*!< Node's saturation                             (g/g). */
    double *d;      /*!< Node's bound water diffusivity              (cm2/h). */
    double *w;      /*!< Node's moisture content                       (g/g). */
    double *v;      /*!< Node's volume weighting fraction               (dl). */

    char  *name;    /*!< Stick name or other descriptive text.                */
    long   updates; /*!< Number of calls made to Fms_Update().                */
    int    state;   /*!< FMS_STATE_<state>.                                   */
    int    init;    /*!< Flag set by Fms_Initialize().                        */
    int    n;       /*!< Number of stick nodes in the radial direction.       */
} Fms;

/*----------------------------------------------------------------------------*/
//  Public Function prototypes.
//----------------------------------------------------------------------------*/

EXTERN Fms *Fms_Create1Hour( char *name ) ;
EXTERN Fms *Fms_Create10Hour( char *name ) ;
EXTERN Fms *Fms_Create100Hour( char *name ) ;
EXTERN Fms *Fms_Create1000Hour( char *name ) ;

EXTERN Fms *Fms_CreateStick(
    char  *name,        /* Name for the new stick instance.                   */
    int    nodes,       /* Number of calculation nodes.                       */
    double radius,      /* Stick radius                                  (cm) */
    double length,      /* Stick length                                  (cm) */
    double density,     /* Stick density                             (gm/cm3) */
    double mdt,         /* Moisture computation time step                 (h) */
    double ddt,         /* Diffusivity computation time step              (h) */
    double pressure,    /* Barometric pressure                      (cal/cm3) */
    double wmx,         /* Max local moisture due to rain               (g/g) */
    double hc,          /* Planar heat transfer                 (cal/cm2-h-C) */
    double stca,        /* surface mass transfer - adsorption   ((cm3/cm2)/h) */
    double stcd,        /* surface mass transfer - desorption   ((cm3/cm2)/h) */
    double raif0,       /* runoff factor during initial rainfall observation  */
    double raif1,       /* runoff factor after initial rainfall observation   */
    double stv,         /* storm transition value                      (cm/h) */
    double wfilmk       /* water film contribution to moisture content(gm/gm) */
) ;

EXTERN void Fms_Destroy( Fms *fms ) ;

EXTERN void Fms_Initialize(
    Fms    *fms,                /* Pointer to fuel moisture stick object      */
    double  ta,                 /* Initial air temperature                (C) */
    double  ha,                 /* Initial air humidity                  (g/g */
    double  sr,                 /* Initial solar radiation             (W/m2) */
    double  rc,                 /* Initial cumulative rainfall           (cm) */
    double  ti,                 /* Initial stick temperature              (C) */
    double  hi,                 /* Initial stick surface humidity       (g/g) */
    double  wi                  /* Initial stick moisture content       (g/g) */
) ;

void Fms_InitializeAt(
    Fms    *fms,                /* Pointer to fuel moisture stick object.     */
    int     year,               /* Year of the update              (4 digits) */
    int     month,              /* Month of the update        (1=Jan, 12=Dec) */
    int     day,                /* Day of the update                   (1-31) */
    int     hour,               /* Hour of the update                  (0-23) */
    int     minute,             /* Minute of the update                (0-59) */
    int     second,             /* Second of the update                (0-59) */
    int     millisecond,        /* Millisecond of the update          (0-999) */
    double  ta,                 /* Initial air temperature                (C) */
    double  ha,                 /* Initial air humidity                 (g/g) */
    double  sr,                 /* Initial solar radiation             (W/m2) */
    double  rc,                 /* Initial cumulative rainfall           (cm) */
    double  ti,                 /* Initial stick temperature              (C) */
    double  hi,                 /* Initial stick surface humidity       (g/g) */
    double  wi                  /* Initial stick moisture content       (g/g) */
) ;

EXTERN double Fms_MeanMoisture( Fms *fms ) ;

EXTERN double Fms_MeanWtdMoisture( Fms *fms ) ;

EXTERN double Fms_MeanWtdTemperature( Fms *fms ) ;

EXTERN void Fms_Update(
    Fms   *fms,                 /* Pointer to the fuel moisture stick object  */
    double eHr,                 /* Elapsed time since last observation   (h) */
    double aC,                  /* New air temperature                    (C) */
    double hFtn,                /* New air humidity                     (g/g) */
    double sW,                  /* New solar radiation                 (W/m2) */
    double rcumCm               /* New cumulative rainfall               (cm) */
) ;

EXTERN void Fms_UpdateAt(
    Fms   *fms,                 /* Pointer to the fuel moisture stick object. */
    int    year,                /* Year of the update              (4 digits) */
    int    month,               /* Month of the update        (1=Jan, 12=Dec) */
    int    day,                 /* Day of the update                   (1-31) */
    int    hour,                /* Hour of the update                  (0-23) */
    int    minute,              /* Minute of the update                (0-59) */
    int    second,              /* Second of the update                (0-59) */
    int    millisecond,         /* Millisecond of the update          (0-999) */
    double aC,                  /* New air temperature                    (C) */
    double hFtn,                /* New air humidity                     (g/g) */
    double sW,                  /* New solar radiation                 (W/m2) */
    double rcumCm               /* New cumulative rainfall               (cm) */
) ;

EXTERN void Fms_ReadFromAsciiStream( Fms *fms, FILE *fptr ) ;

EXTERN void Fms_WriteToAsciiStream( Fms *fms, FILE *fptr ) ;

/*
EXTERN void CDT_CalendarDate( double jd, int *year, int *month, int *day,
            int *hour, int *minute, int *second, int *millisecond ) ;

EXTERN int CDT_DayOfYear( int year, int month, int day ) ;

EXTERN double CDT_DecimalDay( int hour, int minute, int second,
            int millisecond ) ;

EXTERN double CDT_JulianDate( int year, int month, int day,
            int hour, int minute, int second, int millisecond ) ;

EXTERN int CDT_LeapYear( int year ) ;

EXTERN int CDT_MillisecondOfDay( int hour, int minute, int second,
            int millisecond ) ;
*/
#endif

//----------------------------------------------------------------------------*/
//  End of fms-0.7.0.h
//----------------------------------------------------------------------------*/
