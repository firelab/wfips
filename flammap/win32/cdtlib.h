/*----------------------------------------------------------------------------*/
/*! \file cdtlib.h
 *  \version BehavePlus2
 *  \author Copyright (C) 2002 by Collin D. Bevins.  All rights reserved.
 *
 *  \brief Calendar-Date-Time (CDT) library C source header.
 *
 *  Fundamental calendar, date, and time routines for the Western
 * (Julian-Gregorian) calendar.
 */

#ifndef _CDTLIB_H_
/*! \def _CDTLIB_H_
    \internal
    \brief Prevents redundant inclusion of the cdtlib.h header file.
 */
#define _CDTLIB_H_ 1

#ifdef __cplusplus
/*! \def EXTERN
    \brief Prevents name-mangling of C source code by C++ compilers.
    \internal
 */
#define EXTERN extern "C"
#else
#define EXTERN extern
#endif

/*! \enum CDT_Event
    \brief Indicates the type of event to be performed upon, or was last
    performed upon, a DateTime object.
*/

enum CDT_Event
{
    CDT_User             =  0, /*!< Contains a date and time last set by a user function call. */
    CDT_System           =  1, /*!< Contains a date and time set from the system clock. */
    CDT_SunRise          =  2, /*!< Contains a sun rise event for the date. */
    CDT_SunSet           =  3, /*!< Contains a sun set event for the date. */
    CDT_MoonRise         =  4, /*!< Contains a moon rise event for the date. */
    CDT_MoonSet          =  5, /*!< Contains a moon set event for the date. */
    CDT_CivilDawn        =  6, /*!< Contains a civil dawn event for the date. */
    CDT_CivilDusk        =  7, /*!< Contains a civil dusk event for the date. */
    CDT_NauticalDawn     =  8, /*!< Contains a nautical dawn event for the date. */
    CDT_NauticalDusk     =  9, /*!< Contains a nautical dusk event for the date. */
    CDT_AstronomicalDawn = 10, /*!< Contains an astronomical dawn time for date. */
    CDT_AstronomicalDusk = 11, /*!< Contains an astronomical dusk time for date. */
    CDT_Spring           = 12, /*!< Contains a spring equinox event for the year. */
    CDT_Summer           = 13, /*!< Contains a summer solstice event for the year. */
    CDT_Fall             = 14, /*!< Contains a fall equinox event for the year. */
    CDT_Winter           = 15, /*!< Contains a winter solstice event for the year. */
    CDT_Easter           = 16, /*!< Contains a Gregorian Easter event for the year. */
    CDT_NewMoon          = 17, /*!< Contains a new moon event for a year and lunation period. */
    CDT_FullMoon         = 18  /*!< Contains a full moon event following a new moon year and lunation period. */
};

/*! \enum CDT_Flag
    \brief Indicates the result of the last event or operation performed on
    a DateTime object.
*/

enum CDT_Flag
{
    CDT_None               =  0, /*!< Indicates no event has occurred. */
    CDT_HasValidDateTime   =  1, /*!< Indicates the date and time are valid. */
    CDT_HasValidDate       =  2, /*!< Indicates all the date elements are valid. */
    CDT_HasValidTime       =  3, /*!< Indicates the time elements are valid. */
    CDT_HasInvalidYear     =  4, /*!< Indicates the year is less than -4712 (earlier than 4713 B.C.). */
    CDT_HasInvalidMonth    =  5, /*!< Indicates the month of the year is outside the range 1-12. */
    CDT_HasInvalidDay      =  6, /*!< Indicates the day of the month is invalid for the year. */
    CDT_HasInvalidHour     =  7, /*!< Indicates the hour of the day is outside the range 0-59). */
    CDT_HasInvalidMinute   =  8, /*!< Indicates the minute of the hour is outside the range 0-59. */
    CDT_HasInvalidSecond   =  9, /*!< Indicates the second of the minute is outside the range 0-59. */
    CDT_HasInvalidMillisecond=10,/*!< Indicates the millisecond of the second is outside the range 0-999. */
    CDT_Rises              = 11, /*!< Indicates a moon rise, sun rise, or dawn does occur on the date. */
    CDT_NeverRises         = 12, /*!< Indicates a moon rise doesn't occur on the date. */
    CDT_Sets               = 13, /*!< Indicates a moon rise, sun rise, or dusk does occur on the date. */
    CDT_NeverSets          = 14, /*!< Indicates a moon set doesn't occur on the date. */
    CDT_Visible            = 15, /*!< Indicates the moon or sun is always visible on the date. */
    CDT_Invisible          = 16, /*!< Indicates the moon or sun is always invisible on the date. */
    CDT_Light              = 17, /*!< Indicates the day has continuous twilight. */
    CDT_Dark               = 18  /*!< Indicates the day has continuous darkness. */
};

/*----------------------------------------------------------------------------*/
/*  Static function prototypes                                                */
/*----------------------------------------------------------------------------*/

EXTERN void     CDT_CalendarDate( double jd, int *year, int *month, int *day,
                    int *hour, int *minute, int *second, int *millisecond ) ;

EXTERN int      CDT_DayOfWeek( double jdate ) ;

EXTERN const char *CDT_DayOfWeekAbbreviation( int dowIndex ) ;

EXTERN const char *CDT_DayOfWeekName( int dowIndex ) ;

EXTERN int      CDT_DayOfYear( int year, int month, int day ) ;

EXTERN int      CDT_DaysInMonth( int year, int month ) ;

EXTERN int      CDT_DaysInYear( int year ) ;

EXTERN double   CDT_DecimalDay( int hour, int minute, int second,
                    int millisecond ) ;

EXTERN double   CDT_DecimalHour( int hour, int minute, int second,
                    int millisecond ) ;

EXTERN void     CDT_EasterDay( int year, int *month, int *day ) ;

EXTERN const char *CDT_EventName( int event ) ;

EXTERN const char *CDT_FlagName( int event ) ;

EXTERN double   CDT_FractionalPart( double value ) ;

EXTERN double   CDT_JulianDate( int year, int month, int day, int hour,
                    int minute, int second, int millisecond ) ;

EXTERN int      CDT_LeapYear( int year ) ;

EXTERN double   CDT_LocalMeanSiderealTime( double mjd, double lambda ) ;

EXTERN int      CDT_MillisecondOfDay( int hour, int minute, int second,
                    int millisecond ) ;

EXTERN double   CDT_ModifiedJulianDate( double jdate ) ;

EXTERN const char *CDT_MonthAbbreviation( int month ) ;

EXTERN const char *CDT_MonthName( int month ) ;

EXTERN double   CDT_NewMoonGMT( int year, int period ) ;

EXTERN int      CDT_RiseSet( int event, double jdate, double lon, double lat,
                    double gmtDiff, double *hours ) ;

EXTERN int      CDT_QuadraticRoots( double y_minus, double y_0, double y_plus,
                    double *xe, double *ye, double *zero1, double *zero2 ) ;

EXTERN double   CDT_SineAltitude( int event, double mjd0, double hour,
                    double lambda, double cphi, double sphi ) ;

EXTERN double   CDT_SolarAngle( double slope, double aspect, double altitude,
                    double azimuth ) ;

EXTERN void     CDT_SunPosition( double jdate, double lon, double lat,
                    double gmtDiff, double *altitude, double *azimuth ) ;

EXTERN double   CDT_SolarRadiation ( double jdate, double lon, double lat,
                    double gmtDiff, double slope, double aspect, double elev,
                    double atmTransparency, double cloudTransmittance,
                    double canopyTransmittance ) ;

EXTERN double   CDT_SolsticeGMT( int event, int year ) ;

EXTERN int      CDT_ValidDate( int year, int month, int day ) ;

EXTERN int      CDT_ValidDateTime( int year, int month, int day,
                    int hour, int minute, int second, int millisecond ) ;

EXTERN int      CDT_ValidTime( int hour, int minute, int second,
                    int millisecond ) ;

#endif

/*----------------------------------------------------------------------------*/
/*  End of cdtlib.h                                                           */
/*----------------------------------------------------------------------------*/

