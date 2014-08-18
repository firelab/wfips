/*----------------------------------------------------------------------------*/
/*! \file cdtlib.c
 *  \version BehavePlus2
 *  \author Copyright (C) 2002 by Collin D. Bevins.  All rights reserved.
 *
 *  \brief Calendar-Date-Time (CDT) library C source code.
 *
 *  Fundamental calendar, date, and time routines for the Western
 * (Julian-Gregorian) calendar.
 *
 *  \par References:
 *
 *  The following references were used:
 *
 *  Duffett-Smith, Peter. 1981.  Practical astronomy with your calculator.
 *  Second Edition. Cambridge University Press. 188 pp.
 *
 *  Latham, Lance. 1998. Standard C date/time library.  Miller-Freeman.
 *  560 pp.
 *
 *  Meeus, Jean.  1988.  Astronomical formulae for calculators. Fourth
 *  Edition.  Willman-Bell, Inc. 218 pp.
 *
 *  Montenbruck, Oliver; Pfleger, Thomas.  Astronomy on the personal computer.
 *  Third Edition.  Springer.  312 pp.  (see page 13).
 */

/* Custom include files */
#include "cdtlib.h"

/* Standard include files */
#include <math.h>
#include <stdio.h>

/*! \var static const double Radians
 *  \brief Global constant defining the radians per degree.
 */
static const double Radians = 0.0174532925199433;
static const double Eps =1.0e-07;

/*! \var static const double D1
 *  \brief dD/dT of mean elongation of moon from sun in revolutions per century
 *  (1236.853086, see Montenbruck & Pfleger page 179).
 */
static const double D1 = 1236.853086;

/*! \var static const double D0
 *  \brief Mean elongation D of the moon from the sun for the epoch J2000
 *  in units of 1 rev = 360 degrees (0.827361, see Montenbruck & Pfleger,
 *  page 179).
 */
static const double D0 = 0.827361;

/*------------------------------------------------------------------------------
 *  Static function prototypes
 */

static double cs( double degrees ) ;
static void CDT_ImproveMoon( double *t0, double *b ) ;
static void CDT_MiniMoon( double t, double *ra, double *dec ) ;
static void CDT_MiniSun( double t, double *ra, double *dec ) ;
static double sn( double degrees ) ;

/*----------------------------------------------------------------------------*/
/*! \brief Determines the Western calendar date from the Julian date.
 *
 *  Calculates the Western (Julian-Gregorian) calendar date from the Julian
 *  date \a jdate using the method described by Duffett-Smith (and similarly
 *  by Meeus).
 *
 *  I used Montenbruck & Pfleger (p 13) because it gave the correct calendar
 *  date for Julian date 1.0, whereas the Meeus (p 26) and Duffett-Smith (p 11)
 *  algorithms said Julian date 1 is -4712 Jan 02.
 *
 *  \warning No date or time validation is performed.
 *  \param jdate Julian date as returned by CDT_JulianDate().
 *  \param *year Returned Julian-Gregorian year (-4712 (4713 B.C.) or greater).
 *  \param *month Returned month of the year (1-12).
 *  \param *day Returned day of the month (1-31).
 *  \param *hour Returned hours past midnight (0-23).
 *  \param *minute Returned minutes past the hour (0-59).
 *  \param *second Returned seconds past the minute (0-59).
 *  \param *milliseconds Returned milliseconds past the second (0-999).
 *
 *  \return All calculated values are returned in the function parameters.
 *  The function itself returns nothing.
 */

void CDT_CalendarDate( double jdate, int *year, int *month, int *day,
            int *hour, int *minute, int *second, int *millisecond )
{
    int b, d, f;
    double jd0, c, e, dh, dm, ds, ms;

    jd0 = (double) ( (long) ( jdate + 0.5 ) );

    if ( jd0 < 2299161.0 )
    {
        c = jd0 + 1524.0;
    }
    else
    {
        b = (int) ( ( jd0 - 1867216.25 ) / 36524.25 );
        c = jd0 + (b - (int) ( b / 4 ) ) + 1525.0;
    }
    d = (int) ( (c - 122.1) / 365.25 );
    e = 365.0 * d + (int) ( d / 4 );
    f = (int) ( (c - e) / 30.6001 );

    *day = (int) ( c - e + 0.5 ) - (int) ( 30.6001 * f );
    *month = f - 1 - 12 * (int) ( f / 14 );
    *year = d - 4715 - (int) ( ( 7 + *month ) / 10 );
    dh = 24.0 * ( jdate + 0.5 - jd0 );
    *hour = (int) dh;
    dm = 60. * (dh - (double) *hour);
    *minute = (int) dm;
    ds = 60. * (dm - (double) *minute);
    *second = (int) ds;
    ms = 1000. * (ds - (double) *second);
    *millisecond = (int) ms;
    return;
}

/*----------------------------------------------------------------------------*/
/*! \brief Cosine function that operates on \a x degrees.
 *
 *  \param degrees Angle in degrees.
 *
 *  \return Cosine in radians of the passed number of degrees.
 *
 *  \internal
 */

static double cs( double degrees )
{
    return cos( Radians * degrees );
}

/*----------------------------------------------------------------------------*/
/*! \brief Determines the day-of-the week index from the Julian date \a jdate.
 *
 *  \param jdate Julian date as determined by CDT_JulianDate().
 *
 *  \retval 0 = Sunday
 *  \retval 1 = Monday
 *  \retval 2 = Tuesday
 *  \retval 3 = Wednesday
 *  \retval 4 = Thursday
 *  \retval 5 = Friday
 *  \retval 6 = Saturday
 */

int CDT_DayOfWeek( double jdate )
{
    return( (int) (jdate + 1.5 ) % 7 );
}

/*----------------------------------------------------------------------------*/
/*! \brief Returns the 3-letter English abbreviation for the day-of-the-week
 *  index \a dowIndex.
 *
 *  \param dowIndex The day-of-the-week index with one of the following values:
 *  \arg 0 = Sun
 *  \arg 1 = Mon
 *  \arg 2 = Tue
 *  \arg 3 = Wed
 *  \arg 4 = Thu
 *  \arg 5 = Fri
 *  \arg 6 = Sat
 *
 *  \return Pointer to a static string containing the 3-letter English
 *  abbreviation for the day-of-the-week index \a dowIndex, or a pointer
 *  to the static string "Bad Day-of-Week Index".
 */

const char *CDT_DayOfWeekAbbreviation( int dowIndex )
{
    const char *DayOfWeekAbbreviation[] =
        { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

    if ( dowIndex >= 0 && dowIndex <= 6 )
    {
        return( DayOfWeekAbbreviation[ dowIndex ] );
    }
    return( "Bad Day-of-Week index" );
}

/*----------------------------------------------------------------------------*/
/*! \brief Returns the English name for the day-of-week index \a dowIndex.
 *
 *  \param dowIndex The day-of-the-week index with one of the following values:
 *  \arg 0 = Sunday
 *  \arg 1 = Monday
 *  \arg 2 = Tuesday
 *  \arg 3 = Wednesday
 *  \arg 4 = Thursday
 *  \arg 5 = Friday
 *  \arg 6 = Saturday
 *
 *  \return Pointer to a static string containing one of the above English
 *  names for the day-of-the-week index \a dowIndex, or a pointer to the
 *  static string "Bad Day-of-Week Index".
 */

const char *CDT_DayOfWeekName( int dowIndex )
{
    const char *DayOfWeekName[] = { "Sunday", "Monday", "Tuesday", "Wednesday",
        "Thursday", "Friday", "Saturday" };

    if ( dowIndex >= 0 && dowIndex <= 6 )
    {
        return( DayOfWeekName[ dowIndex ] );
    }
    return( "Bad Day-of-Week index" );
}

/*----------------------------------------------------------------------------*/
/*! \brief Determines the day-of-the-year number.
 *
 *  Day 1 is always January 1 of the \a year.  The day number is adjusted
 *  for the occurrence of leap years in the Julian and Gregorian calendars.
 *  An adjustment is also made for the Gregorian calendar reform of 1582
 *  when Pope Gregory dropped Oct 5-14 from the Julian calendar to begin the
 *  Gregorian calendar (thus, 1582 had only 355 days).
 *
 *  \param year Julian-Gregorian calendar year (-4712 or later).
 *  \param month Month of the year (1-12).
 *  \param day Day of the month (1-31).
 *
 *  \return Day of the year (1-366).
 */

int CDT_DayOfYear( int year, int month, int day )
{
    /*                       Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec */
    static int DaysToMonth[]={ 0, 31, 59, 90,120,151,181,212,243,273,304,334 };

    int doy = day + DaysToMonth[month-1];

    /* 1582 AD is missing the ten days of Oct 5-14 */
    if ( year == 1582 && doy > 277 )
    {
        doy -= 10;
    }
    else if ( doy > 59 && CDT_LeapYear( year ) )
    {
        doy++;
    }
    return( doy );
}

/*----------------------------------------------------------------------------*/
/*! \brief Determines the number of days in the month for the \a year.
 *
 *  The Julian calendar (prior to the Gregorian calendar reform) has 29 days
 *  in February every quadrennial.  October 1582 has only 21 days because of
 *  the reform.  The Gregorian calendar has exceptions to the quadrennial
 *  leap year rule.
 *
 *  \param year Julian-Gregorian calendar year (-4712 or later).
 *  \param month Month of the year (1=Jan, 12=Dec).
 *
 *  \return Number of days in the \a year's month.
 */

int CDT_DaysInMonth( int year, int month )
{
    /*                        Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec */
    static int DaysInMonth[]={ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    if ( year == 1582 && month == 10 )
    {
        return( 21 );
    }
    if ( month == 2 )
    {
        return ( DaysInMonth[month-1] + CDT_LeapYear( year ) );
    }
    return( DaysInMonth[month-1] );
}

/*----------------------------------------------------------------------------*/
/*! \brief Determines the number of days in the \a year.
 *
 *  The Julian calendar has quadrennial leap years.  The Gregorian calendar
 *  reform of 1582 has only 355 days.  The post-reform Gregorian calendar has
 *  quadrennial leap years with exceptions.
 *
 *  \param year Julian-Gregorian calendar year (-4712 or later).
 *
 *  \return Number of days in the \a year (355, 365, or 366).
 */

int CDT_DaysInYear( int year )
{
    if ( year == 1582 )
    {
        return( 355 );
    }
    return( 365 + CDT_LeapYear( year ) );
}

/*----------------------------------------------------------------------------*/
/*! \brief Determines the elapsed portion of the day since midnight.
 *
 *  \param hour            Hours past midnight (0-23).
 *  \param minute          Minutes past the hour (0-59).
 *  \param second          Seconds past the minute (0-59).
 *  \param milliseconds    Milliseconds past the second (0-999).
 *
 *  \return The elapsed portion of the day since midnight in days.
 *
 *  \sa CDT_MillisecondOfDay(), CDT_DecimalHour()
 */

double CDT_DecimalDay( int hour, int minute, int second, int millisecond )
{
    return( (double) CDT_MillisecondOfDay( hour, minute, second, millisecond )
        / 86400000. ) ;
}

/*----------------------------------------------------------------------------*/
/*! \brief Determines the elapsed hours since midnight.
 *
 *  \param hour            Hours past midnight (0-23).
 *  \param minute          Minutes past the hour (0-59).
 *  \param second          Seconds past the minute (0-59).
 *  \param milliseconds    Milliseconds past the second (0-999).
 *
 *  \return The elapsed portion of the day since midnight in hours.
 *
 *  \sa CDT_MillisecondOfDay(), CDT_DecimalDay()
 */

double CDT_DecimalHour( int hour, int minute, int second, int millisecond )
{
    return( CDT_MillisecondOfDay( hour, minute, second, millisecond )
        / 3600000. ) ;
}

/*----------------------------------------------------------------------------*/
/*! \brief Determines the date of Easter for the \a year.
 *
 *  Valid for the Gregorian calendar (1583 and later).  Uses the algorithm
 *  from Duffett-Smith (page 5), Meeus (page 31), and Latham (page 164).
 *
 *  \param year Julian-Gregorian calendar year for easter (1583 or later).
 *  \param *month Returned month of Easter (3=March, 4=April) for the \a year.
 *  \param *day Returned day of Easter for the \a year.
 *
 *  \return The month and day of Easter for the \a year are returned in the
 *  calling arguments.  The function itself returns nothing.
 */

void CDT_EasterDay( int year, int *month, int *day )
{
    int a, b, c, d, e, f, g, h, i, k, l, m, n, p;

    a = year % 19;
    b = year / 100;
    c = year % 100;
    d = b / 4;
    e = b % 4;
    f = (b + 8) / 25;
    g = (b - f + 1) / 3;
    h = (19*a + b - d - g + 15) % 30;
    i = c / 4;
    k = c % 4;
    l = (32 + 2*e + 2*i - h - k) % 7;
    m = (a + 11*h + 22*l) / 451;
    n = (h + l - 7*m + 114) / 31;
    p = (h + l - 7*m + 114) % 31;
    *day = p + 1;
    *month = n;
    return;
}

/*----------------------------------------------------------------------------*/
/*! \brief Returns the name of the passed #CDT_Event enum value.
 *
 *  #CDT_Event enumerates the possible events that can be determined for
 *  a date and/or time.
 *
 *  \param event #CDT_Event enumeration constant.
 *
 *  \return Pointer to a static string describing the event.
 */

const char *CDT_EventName( int event )
{
    static const char *EventName[] = {
       "User Time",
       "System Time",
       "Sun Rise",
       "Sun Set",
       "Moon Rise",
       "Moon Set",
       "Civil Dawn",
       "Civil Dusk",
       "Nautical Dawn",
       "Nautical Dusk",
       "Astronomical Dawn",
       "Astronomical Dusk",
       "Spring Equinox",
       "Summer Solstice",
       "Fall Equinox",
       "Winter Solstice",
       "Easter",
       "New Moon",
       "Full Moon"
    };
    return( EventName[event] );
}

/*----------------------------------------------------------------------------*/
/*! \brief Returns the name of the passed #CDT_Flag enum value.
 *
 *  #CDT_Flag enumerates the possible conditions that can result from a
 *  calendar date-time operation.
 *
 *  \param flag #CDT_Flag enumeration constant.
 *
 *  \return Pointer to a static string describing the flag.
 */

const char *CDT_FlagName( int event )
{
    static const char *FlagName[] = {
        "None",
        "Valid DateTime",
        "Valid Date",
        "Valid Time",
        "Invalid Year",
        "Invalid Month",
        "Invalid Day",
        "Invalid Hour",
        "Invalid Minute",
        "Invalid Second",
        "Invalid Millisecond",
        "Rises",
        "Never Rises",
        "Sets",
        "Never Sets",
        "Always Visible",
        "Never Visible",
        "Always Light",
        "Always Dark"
    };
    return( FlagName[event] );
}

/*----------------------------------------------------------------------------*/
/*! \brief Determines the fractional part of the passed \a value.
 *
 *  \return Returns the fractional part of the passed \a value.
 */

double CDT_FractionalPart( double value )
{
    value = value - (int) value;
    if ( value < 0. )
    {
        value += 1.;
    }
    return( value );
}

/*----------------------------------------------------------------------------*/
/*! \brief Improves an approximation for the time of the new moon.
 *
 *  This is an internal function from Montenbruck and Pfleger that improves
 *  an approximation \a t0 for the time of the new moon and finds the
 *  ecliptic longitude \a b of the moon for that date.
 *
 *  Called only by the lunation routine CDT_NewMoonGMT().
 *
 *  \param *t0 Time in Julian centuries since J2000 (t = (jd - 2451545) / 36526).
 *  \param *b  Ecliptic longitude of the moon for that date.
 *
 *  \return The adjusted time \a t0 and the ecliptic longitude \a b of the moon
 *  for the date are returned in the passed arguments.  The function itself
 *  returns nothing.
 */

static void CDT_ImproveMoon( double *t0, double *b )
{
    double p2, arc, t, l, ls, d, f, dlm, dls, dlambda;

    /* Two pies */
    p2 = 6.283185307;
    /* Arcseconds per radian */
    arc = 206264.8062;
    /* Store the input time */
    t = *t0;
    /* Mean anomoly of the moon */
    l  = p2 * CDT_FractionalPart(0.374897 + 1325.552410 * t);
    /* Mean anomoly of the sun */
    ls = p2 * CDT_FractionalPart(0.993133 +   99.997361 * t);
    /* Mean elongation of Moon-Sun */
    d  = p2 *(CDT_FractionalPart(0.5 + D0 + D1 * t) - 0.5);
    /* Mean argument of latitude */
    f  = p2 * CDT_FractionalPart(0.259086 + 1342.227825 * t);
    /* Periodic perturbations of the lunar and solar longitude (in ") */
    dlm = 22640. * sin(l)
        - 4586. * sin(l - 2.*d)
        + 2370. * sin(2.*d)
        + 769. * sin(2.*l)
        - 668. * sin(ls)
        - 412. * sin(2.*f)
        - 212. * sin(2.*l - 2.*d)
        - 206. * sin(l + ls - 2.*d)
        + 192 * sin(l + 2.*d)
        - 165. * sin(ls - 2.*d)
        - 125. * sin(d)
        - 110. * sin(l + ls)
        + 148. * sin(l - ls)
        - 55. * sin(2.*f - 2*d);
    dls = 6893. * sin(ls)
        + 72. * sin(2.*ls);
    /* Difference of the true longitudes of moon and sun in revolutions */
    dlambda = d / p2 + (dlm - dls) / 1296000.0;

    /* Correction for the time of the new moon */
    *t0 = t - dlambda / D1;

    /* Ecliptic latitude B of the moon in degrees */
    *b = ( 18520.0 * sin(f + dlm/arc) - 526. * sin(f - 2.*d) ) / 3600.0;
    return;
}

/*----------------------------------------------------------------------------*/
/*! \brief Calculates the Julian date from the passed date and time.
 *
 *  The Julian Date is the number of days that have elapsed since \e noon,
 *  12h Universal Time on January 1, 4713 B.C.
 *
 *  The Julian Date was developed in 1583 by French scholar Joseph Justus
 *  Scaliger.  The beginning day of January 1, 4713 B.C. is the previous
 *  coincidence of the 28-year solar cycle, the 19-year Golden Number (Metonic)
 *  lunar cycle, and the 15-year indiction cycle used for Roman taxation.
 *
 *  The Gregorian calendar reform is taken into account when calculating
 *  the Julian Date.  Thus the day following 1582 October 4 is 1582 October 15.
 *  The Julian calendar is used through 1582 Oct 4 (leap years every 4th year),
 *  and the Gregorian calendar is used for dates on and after 1582 Oct 15
 *  (leap year exceptions).  Together, the Julian-Gregorian calendar system
 *  may be referred to simply as the Western calendar.
 *
 *  The "B.C." years are counted astronomically; the year before A.D. 1
 *  is year 0, and the year preceeding this is -1.
 *
 *  \par References:
 *
 *  Duffett-Smith, Peter. 1981.  Practical astronomy with your calculator.
 *  Second Edition. Cambridge University Press. 188 pp. (see page 9).
 *
 *  Latham, Lance. 1998. Standard C date/time library.  Miller-Freeman.
 *  560 pp.  (see page 41).
 *
 *  Meeus, Jean.  1988.  Astronomical formulae for calculators. Fourth
 *  Edition.  Willman-Bell, Inc. 218 pp. (see page 24).
 *
 *  Montenbruck, Oliver; Pfleger, Thomas.  Astronomy on the personal computer.
 *  Third Edition.  Springer.  312 pp.  (see page 13).
 *
 *  \warning No date or time validation is performed.
 *
 *  \bug
 *  While all authors agree that the Julian Date starts at zero at 12:00
 *  \e noon of 4713 B.C. January  1, the Duffett-Smith and Meeus algorithms
 *  actually yields a JD of 1.0 for this date and time!  The function here
 *  correctly reproduces all the examples in their texts, but yield a JD of
 *  1.0 for -4712 Jan 1 12:00.
 *
 *  \htmlonly
 *  <table>
 *  <tr><td>Calendar Date</td> <td>Julian Date</td> <td>Source</td></tr>
 *  <tr><td>1985 Feb 17 06:00:00</td><td>2,446,113.75</td><td>(Duffett-Smith p 9)</td></tr>
 *  <tr><td>1980 Jan 40 00:00:00</td><td>2,444,238.50</td><td>(Duffett-Smith, p10)</td></tr>
 *  <tr><td>1957 Oct 04 19:26:24</td><td>2,436,116.31</td><td>(Meeus, p23)</td></tr>
 *  <tr><td>0333 Jan 27 12:00:00</td><td>1,842,713.00</td><td>(Meeus, p24)</td></tr>
 *  <tr><td>-4712 Jan 01 12:00:00</td><td>1.00</td><td>Should be 0.0! </td>
 *  </table>
 *  Note the last example.
 *  \endhtmlonly
 *
 *  While this certainly deviates from the formal definition of JD, I will
 *  continue to use this algorithm since I also use their other algorithms
 *  for Julian-to-calendar conversions and astronomical date derivations.
 *
 *  \param year            -4712 (4713 B.C.) or greater
 *  \param month           Month of the year (1-12)
 *  \param day             Day of the month (1-31)
 *  \param hour            Hours past midnight (0-23)
 *  \param minute          Minutes past the hour (0-59)
 *  \param second          Seconds past the minute (0-59)
 *  \param milliseconds    Milliseconds past the second (0-999)
 *
 *  \return The Julian date in decimal days since 1 Jan -4712.
 */

double CDT_JulianDate( int year, int month, int day,
            int hour, int minute, int second, int millisecond )
{
    double jdate;
    int a, b, c, d;

    jdate = 10000 * year + 100 * month + day;
    if ( month <= 2 )
    {
        year--;
        month+= 12;
    }
    a = 0;
    b = 0;
    if ( jdate >= 15821015.0 )
    {
        a = (int) (year / 100);
        b = 2 - a + (int) (a/4);
    }
    c = (int) (365.25 * year);
    d = (int) (30.6001 * ( month + 1 ));
    jdate = b + c + d + day
          + CDT_DecimalDay( hour, minute, second, millisecond )
          + 1720994.5;
    return( jdate );
}

/*----------------------------------------------------------------------------*/
/*! \brief Determines if the specified \a year is a Julian-Gregorian leap year.
 *
 *  All quadrennial years in the Julian calendar (prior to 1582) are leap
 *  years.
 *
 *  \param year Julian-Gregorian calendar year (-4712 or later).
 *
 *  \return Number of leap days in the year (0 or 1).
 */

int CDT_LeapYear( int year )
{
    /* If its not divisible by 4, its not a leap year. */
    if ( year % 4 != 0 )
    {
        return( 0 );
    }
    /* All years divisible by 4 prior to 1582 were leap years. */
    else if ( year < 1582 )
    {
        return( 1 );
    }
    /* If divisible by 4, but not by 100, its a leap year. */
    else if ( year % 100 != 0 )
    {
        return( 1 );
    }
    /* If divisible by 100, but not by 400, its not a leap year. */
    else if ( year % 400 != 0 )
    {
        return( 0 );
    }
    /* If divisible by 400, its a leap year. */
    else
    {
        return( 1 );
    }
}

/*----------------------------------------------------------------------------*/
/*! \brief Determines the local sidereal time for the modified Julian date
 *  \a mjd and longitude \a lambda.
 *
 *  From Montenbruch and Pfleger, page 41.
 *
 *  \param mjd Modified Julian date (JD - 2400000.5)
 *  \param lambda Longitude (positive \a west of Greenwich, negative \a east
 *  of Greenwich; Munich is lambda = -11.6 degrees).
 *
 *  \return The local sidereal time for \a mjd and \a lambda.
 */

double CDT_LocalMeanSiderealTime( double mjd, double lambda )
{
    double mjd0, ut, t, gmst;

    mjd0 = (int) mjd;
    ut   = 24. * (mjd - mjd0);
    t    = (mjd0 - 51544.5) / 36525.0;
    gmst = 6.697374558 + 1.0027379093 * ut
         + ( 8640184.812866 + ( 0.093104-6.2e-6 * t ) * t ) * t / 3600.0;
    return( 24.0 * CDT_FractionalPart( (gmst - lambda/15.0) / 24.0 ) );
}

/*----------------------------------------------------------------------------*/
/*! \brief Determines the milliseconds elapsed since midnight.
 *
 *  \param hour            Hours past midnight (0-23).
 *  \param minute          Minutes past the hour (0-59).
 *  \param second          Seconds past the minute (0-59).
 *  \param milliseconds    Milliseconds past the second (0-999).
 *
 *  \return The elapsed time since midnight in milliseconds.
 *
 *  \sa CDT_DecimalDay(), CDT_DecimalHour().
 */

int CDT_MillisecondOfDay( int hour, int minute, int second, int millisecond )
{
    return( millisecond + 1000 * second + 60000 * minute + 3600000 * hour );
}

/*----------------------------------------------------------------------------*/
/*! \brief Determines low precision lunar coordinates (approximately 1').
 *
 *  From Montenbruch and Pfleger, page 38.
 *
 *  \param t        Time in Julian centuries since J2000 (t = (jd - 2451545)
 *                  / 36526).
 *  \param *ra      Returned right ascension (hours, equinox of date).
 *  \param *dec     Returned declination (degrees, equinox of date).
 *
 *  \return The right ascension \a ra (hours) and declination \a dec
 *   (degrees) are returned in the passed arguments.  The function itself
 *   returns nothing.
 */

static void CDT_MiniMoon( double t, double *ra, double *dec )
{
    double p2, arc, coseps, sineps, l0, l, ls, d, f, dl, s, h, n;
    double l_moon, b_moon, cb, x, v,  w, y, z, rho;

    /* Two pies */
    p2 = 6.283185307;
    /* Arc-seconds per radian */
    arc = 206264.8062;
    /* cosine nad sine of the obliquity ecliptic */
    coseps = 0.91748;
    sineps = 0.39778;
    /* Mean longitude of the moon (in revolutions) */
    l0 = CDT_FractionalPart(0.606433 + 1336.855225 * t);
    /* Mean anomoly of the moon */
    l  = p2 * CDT_FractionalPart(0.374897 + 1325.552410 * t);
    /* Mean anomoly of the sun */
    ls = p2 * CDT_FractionalPart(0.993133 +   99.997361 * t);
    /* Difference in longitude between the moon and the sun */
    d  = p2 * CDT_FractionalPart(0.827361 + 1236.853086 * t);
    /* Mean argument of latitude */
    f  = p2 * CDT_FractionalPart(0.259086 + 1342.227825 * t);
    /* Periodic perturbations of the lunar and solar longitude (in ") */
    dl = 22640. * sin(l)
       - 4586. * sin(l - 2.*d)
       + 2370. * sin(2.*d)
       + 769. * sin(2.*l)
       - 668. * sin(ls)
       - 412. * sin(2.*f)
       - 212. * sin(2.*l - 2.*d)
       - 206. * sin(l + ls - 2.*d)
       + 192 * sin(l + 2.*d)
       - 165. * sin(ls - 2.*d)
       - 125. * sin(d)
       - 110. * sin(l + ls)
       + 148. * sin(l - ls)
       - 55. * sin(2.*f - 2*d);
    s = f + (dl + 412. * sin(2*f) + 541. * sin(ls)) / arc;
    h = f - 2.*d;
    n = -526. * sin(h)
      + 44. * sin(l + h)
      - 31. * sin(-l + h)
      - 23. * sin(ls + h )
      + 11. * sin(-ls + h)
      - 25. * sin(-2.*l + f)
      + 21. * sin(-l + f);
    l_moon = p2 * CDT_FractionalPart( l0 + dl / 1296e3 );   /* in radians */
    b_moon = (18520.0 * sin(s) + n) / arc;                  /* in radians */
    /* Equatorial coordinates */
    cb = cos(b_moon);
    x = cb * cos(l_moon);
    v = cb * sin(l_moon);
    w = sin(b_moon);
    y = coseps * v - sineps * w;
    z = sineps * v + coseps * w;
    rho = sqrt(1.0 - z*z);
    *dec = (360.0/p2) * atan(z/rho);
    *ra  = (48.0/p2) * atan(y/(x+rho));
    if ( *ra < 0.0 )
    {
        *ra += 24.0;
    }
    return;
}

/*----------------------------------------------------------------------------*/
/*! \brief Determines low precision solar coordinates (approximately 1').
 *
 *  From Montenbruch and Pfleger, page 39.
 *
 *  \param t        Time in Julian centuries since J2000 (t = (jd - 2451545)
 *                  / 36526).
 *  \param *ra      Returned right ascension (hours, equinox of date).
 *  \param *dec     Returned declination (degrees, equinox of date).
 *
 *  \return The right ascension \a ra (hours) and declination \a dec
 *   (degrees) are returned in the passed arguments.  The function itself
 *   returns nothing.
 */

static void CDT_MiniSun( double t, double *ra, double *dec )
{
    double p2, coseps, sineps, m, dl, l, sl, x, y, z, rho;

    /* Two pies */
    p2 = 6.283185307;
    coseps = 0.91748;
    sineps = 0.39778;

    /* Mean solar anomoly */
    m = p2 * CDT_FractionalPart( 0.993133 + 99.997361 * t );
    dl = 6893.0 * sin(m) + 72.0 * sin(m+m);
    l = p2 * CDT_FractionalPart( 0.7859453 + m/p2 + (6191.2 * t + dl)/1296e3 );
    sl = sin(l);
    x = cos(l);
    y = coseps * sl;
    z = sineps * sl;
    rho = sqrt(1.0 - z*z);
    *dec = (360.0/p2) * atan(z/rho);
    *ra = (48.0/p2) * atan(y/(x+rho));
    if ( *ra < 0.0 )
    {
        *ra += 24.0;
    }
    return;
}

/*----------------------------------------------------------------------------*/
/*! \brief Determines the \a modified Julian date from the \a jdate Julian
 *  date.
 *
 *  The \a modified Julian date is just the standard Julian date adjusted to
 *  the epoch starting midnight 1858 Nov 17:
 *
 *  mjd = jd - 2400000.5;
 *
 *  \param jdate Julian date
 *
 *  \return Modified Julian date.
 */

double CDT_ModifiedJulianDate( double jdate )
{
    return( jdate - 2400000.5 );
}

/*----------------------------------------------------------------------------*/
/*! \brief Returns the 3-letter English abbreviation for the \a month index
 *
 *  \param month Month of the year where 1=Jan and 12=Dec.
 *
 *  \return Pointer to a static string containing the 3-letter English
 *  abbreviation for the \a month or a pointer to the static string
 *  "Bad month index".
 */

const char *CDT_MonthAbbreviation( int month )
{
    const char *MonthAbbreviation[] =
        { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
          "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

    if ( month >= 1 && month <= 12 )
    {
        return( MonthAbbreviation[ month-1 ] );
    }
    return( "Bad month index" );
}

/*----------------------------------------------------------------------------*/
/*! \brief Returns the English name for the \a month index
 *
 *  \param month Month of the year where 1=Jan and 12=Dec.
 *
 *  \return Pointer to a static string containing the English name for the
 *  \a month or a pointer to the static string "Bad month index".
 */

const char *CDT_MonthName( int month )
{
    const char *MonthName[] =
        { "January", "February", "March", "April", "May", "June", "July",
          "August", "September", "October", "November", "December" };

    if ( month >= 1 && month <= 12 )
    {
        return( MonthName[ month-1 ] );
    }
    return( "Bad month index" );
}

/*----------------------------------------------------------------------------*/
/*! \brief Determines the GMT time of the \a period \eth new moon for the \a
 *  year.
 *
 *  Calling this with \a period == 0 will get the last new moon before the \a
 *  year.
 *
 *  \param year Julian-Gregorian calendar year (-4712 or later).
 *  \param period New moon of the \a year (1 == first new moon).
 *
 *  \return The Julian date (GMT) of the \a period's new moon for the \a year.
 */

double CDT_NewMoonGMT( int year, int period )
{
    double t_new_moon, b_moon;
    int lunation;

    /* Derive lunation number */
    lunation = (int) (D1 * ( year - 2000 ) / 100 ) + period;
    t_new_moon = ( lunation - D0 ) / D1;

    /* Improve the estimate */
    CDT_ImproveMoon( &t_new_moon, &b_moon );
    CDT_ImproveMoon( &t_new_moon, &b_moon );

    /* Greenwich time of new moon for this lunation paeriod */
    return ( 36525.0 * t_new_moon + 51544.5 + 2400000.5 );
}

/*----------------------------------------------------------------------------*/
/*! \brief Finds a parabola through three points (-1, \a y_minus), (0, \a y_0),
 *  and (1, \a y_plus) that do not lie on a straight line.
 *
 *  From Montenbruch and Pfleger, page 50.
 *
 *  \param y_minus First y value.
 *  \param y_0 Second y value.
 *  \param y_plus Third y value.
 *  \param *xe Returned x value of the extreme value of the parabola.
 *  \param *ye Returned y value of the extreme value of the parabola.
 *  \param *zero1 First root within [-1, +1] (if one or more roots).
 *  \param *zero2 Second root within [-1, +1] (if two roots)
 *
 *  \return Number of roots within the interval [-1, +1].  The \a xe and \a ye
 *  extreme coordinates are updated and returned in the passed arguments.  The
 *  \a zero1 and \a zero2 arguments may be updated depending upon the number
 *  of roots found.
 */

int CDT_QuadraticRoots(
    double y_minus,     /* in: first y value */
    double y_0,         /* in: second y value */
    double y_plus,      /* in: third y value */
    double *xe,         /* out: x of the extreme value of the parabola */
    double *ye,         /* out: y of the extreme value of the parabola */
    double *zero1,      /* out: first root within [-1, +1] (for nz=1,2) */
    double *zero2       /* out: second root within [-1, +1] (only for nz=2) */
)
{
    double a, b, c, d, dx, x, y, z1, z2;
    int nz = 0;
    a = 0.5 * (y_minus + y_plus) - y_0;
    b = 0.5 * (y_plus - y_minus);
    c = y_0;
    x = -b / (2.0 * a);
    y = (a * x + b) * x + c;
    /* Discriminant of y =  axx + bx + c */
    d = b * b - 4.0*a*c;
    /* Parabola intersects the x-axis */
    if ( d >= 0. )
    {
        dx = 0.5 * sqrt(d) / fabs(a);
        z1 = x - dx;
        z2 = x + dx;
        if ( fabs(z1) <= 1.0 )
            nz++;
        if ( fabs(z2) <= 1.0 )
            nz++;
        if ( z1 < -1.0 )
            z1 = z2;
        *zero1 = z1;
        *zero2 = z2;
    }
    *xe = x;
    *ye = y;
    return( nz );
}

/*----------------------------------------------------------------------------*/
/*! \brief Determines the rise or set times of the sun, moon, dawn, or dusk
 *  at an observer's position.
 *
 *  From Montenbruch and Pfleger, pages 51-54.
 *
 *  \param jdate    Julian date as determined by CDT_JulianDate().
 *  \param lon      Decimal degrees longitude (west GMT is positive).
 *  \param lat      Decimal degrees latitude (north equator is positive).
 *  \param *hours   Returned decimals hours of the event.
 *  \param event Event to determine, may be one of:
 *  \arg #CDT_SunRise
 *  \arg #CDT_SunSet
 *  \arg #CDT_MoonRise
 *  \arg #CDT_MoonSet
 *  \arg #CDT_AstronomicalDawn
 *  \arg #CDT_AstronomicalDusk
 *  \arg #CDT_CivilDawn
 *  \arg #CDT_CivilDusk
 *  \arg #CDT_NauticalDawn
 *  \arg #CDT_NauticalDusk
 *
 *  \retval #CDT_Rises (only if #CDT_Event is #CDT_SunRise or #CDT_MoonRise)
 *  \retval #CDT_NeverRises (only if #CDT_Event is #CDT_MoonRise)
 *  \retval #CDT_Sets (only if #CDT_Event is #CDT_SunSet or #CDT_MoonSet)
 *  \retval #CDT_NeverSets (only if #CDT_Event is #CDT_MoonSet)
 *  \retval #CDT_Visible (only if #CDT_Event is #CDT_SunRise, #CDT_SunSet,
 *  #CDT_MoonRise, or #CDT_MoonSet)
 *  \retval #CDT_Invisible  (only if #CDT_Event is #CDT_SunRise, #CDT_SunSet,
 *  #CDT_MoonRise, or #CDT_MoonSet)
 *  \retval #CDT_Light (only if #CDT_Event is #CDT_AstronomicalDawn,
 *  #CDT_AstronomicalDusk, #CDT_CivilDawn, #CDT_CivilDusk, #CDT_NauticalDawn,
 *  or #CDT_NauticalDusk)
 *  \retval #CDT_Dark  (only if #CDT_Event is #CDT_AstronomicalDawn,
 *  #CDT_AstronomicalDusk, #CDT_CivilDawn, #CDT_CivilDusk, #CDT_NauticalDawn,
 *  or #CDT_NauticalDusk)
 */

int CDT_RiseSet( int event, double jdate, double lon, double lat,
            double gmtDiff, double *hours )
{
    double amjd, sinh0;
    double y_minus, y_0, y_plus;
    double xe, ye, zero1, zero2, utset, utrise, sphi, cphi, hour;
    int doRise, doSet, above, rise, sett, nz, flag, jd;

    /* Strip time from the Julian date. */
    jd = (int) ( jdate - 2400000.5 );
    /* Convert to modified JD adjusted for time zone difference to GMT */
    amjd = (double) jd - gmtDiff / 24.;

    /* Determine the parameters for this type of event */
    doRise = 0;
    doSet = 0;
    flag = CDT_None;
    switch ( event )
    {
        /* Sunrise at h = -50' */
        case CDT_SunRise:
            sinh0 = sn( -50.0/60.0 );
            doRise = 1;
            break;
        case CDT_SunSet:
            sinh0 = sn( -50.0/60.0 );
            doSet = 1;
            break;
        /* Moonrise at h = +8' */
        case CDT_MoonRise:
            sinh0 = sn( 8.0/60.0 );
            doRise = 1;
            break;
        case CDT_MoonSet:
            sinh0 = sn( 8.0/60.0 );
            doSet = 1;
            break;
        /* Civil twilight occurs at -6 degrees */
        case CDT_CivilDawn:
            sinh0 = sn( -6.0 );
            doRise = 1;
            break;
        case CDT_CivilDusk:
            sinh0 = sn( -6.0 );
            doSet = 1;
            break;
        /* Nautical twilight occurs at -12 degrees */
        case CDT_NauticalDawn:
            sinh0 = sn( -12.0 );
            doRise = 1;
            break;
        case CDT_NauticalDusk:
            sinh0 = sn( -12.0 );
            doSet = 1;
            break;
        /* Astronomical twilight occurs at -18 degrees */
        case CDT_AstronomicalDawn:
            sinh0 = sn( -18.0 );
            doRise = 1;
            break;
        case CDT_AstronomicalDusk:
            sinh0 = sn( -18.0 );
            doSet = 1;
            break;
        default:
            return( flag );
    }

    /* Start */
    sphi = sn( lat );
    cphi = cs( lat );
    hour = 1.0;
    y_minus = CDT_SineAltitude( event, amjd, hour-1.0, lon, cphi, sphi) - sinh0;
    above = (y_minus > 0.);
    rise = 0;
    sett = 0;

    /* Loop over search intervals from [0h-2h] to [22h-24h] */
    do
    {
        y_0    = CDT_SineAltitude( event, amjd, hour,     lon, cphi, sphi )
               - sinh0;
        y_plus = CDT_SineAltitude( event, amjd, hour+1.0, lon, cphi, sphi )
               - sinh0;
        nz = CDT_QuadraticRoots( y_minus, y_0, y_plus, &xe, &ye, &zero1, &zero2 );
        if ( nz == 0 )
        {
        }
        else if ( nz == 1 )
        {
            if ( y_minus < 0.0 )
            {
                utrise = hour + zero1;
                rise = 1;
            }
            else
            {
                utset = hour + zero1;
                sett = 1;
            }
        }
        else if ( nz == 2 )
        {
            if ( ye < 0.0 )
            {
                utrise = hour + zero2;
                utset = hour + zero1;
            }
            else
            {
                utrise = hour + zero1;
                utset = hour + zero2;
            }
            rise = 1;
            sett = 1;
        }
        if ( rise && sett )
        {
            break;
        }
        /* Prepare for next interval */
        y_minus = y_plus;
        hour += 2.0;
    } while ( hour < 24.5 );

    /* Store results */
    if ( rise || sett )
    {
        if ( doRise )
        {
            if ( rise )
            {
                *hours = utrise;
                flag = CDT_Rises;
            }
            else
            {
                flag = CDT_NeverRises;
            }
        }
        else if ( doSet )
        {
            if ( sett )
            {
                *hours = utset;
                flag = CDT_Sets;
            }
            else
            {
                flag = CDT_NeverSets;
            }
        }
    }
    /* No rise or set occurred */
    else
    {
        /* If above horizon, then always visible or always light */
        if ( above )
        {
            if ( event == CDT_SunRise || event == CDT_SunSet
             || event == CDT_MoonRise || event == CDT_MoonSet )
            {
                flag = CDT_Visible;
            }
            else
            {
                flag = CDT_Light;
            }
        }
        /* If below horizon, then always invisible or always dark */
        else
        {
            if ( event == CDT_SunRise || event == CDT_SunSet
             || event == CDT_MoonRise || event == CDT_MoonSet )
            {
                flag = CDT_Invisible;
            }
            else
            {
                flag = CDT_Dark;
            }
        }
    }
    return( flag );
}

/*----------------------------------------------------------------------------*/
/*! \brief Determines the sine of the altitude of the moon or sun.
 *
 *  From Montenbruck and Pfleger, page 52.
 *
 *  \param event One of the #CDT_Event enumerations.
 *  \param mjd0 Modified Julian date.
 *  \param hour Hour of the day.
 *  \param lambda Longitude in degrees (west of Greenwich is positive).
 *  \param cphi Cosine of the latitude
 *  \param sphi Sine of the latitude
 *
 *  \return Sine of the altitude of the moon or sun.
 */

double CDT_SineAltitude( int event, double mjd0, double hour,
    double lambda, double cphi, double sphi )
{
    double ra, dec, mjd, t, tau;
    mjd = mjd0 + hour/24.0;
    t = (mjd - 51544.5) / 36525.0;
    /* Moon times */
    if ( event == CDT_MoonRise || event == CDT_MoonSet )
    {
        CDT_MiniMoon( t, &ra, &dec );
    }
    /* Sun times */
    else
    {
        CDT_MiniSun( t, &ra, &dec );
    }
    tau = 15.0 * ( CDT_LocalMeanSiderealTime( mjd, lambda ) - ra );
    return( sphi * sn(dec) + cphi * cs(dec) * cs(tau) );
}

/*----------------------------------------------------------------------------*/
/*! \brief Sine function that operates on \a x degrees.
 *
 *  \param degrees Angle in degrees.
 *
 *  \return Sine in radians of the passed number of degrees.
 *  \internal
 */

static double sn( double degrees )
{
    return sin( Radians * degrees );
}

/*----------------------------------------------------------------------------*/
/*! \brief  Determines the solar angle to the terrain slope.
 *
 *  \param slope Terrain slope in degrees.
 *  \param aspect Terrain aspect; downslope direction in degrees clockwise
 *  from north.
 *  \param altitude Sun altitude above horizon in degrees.
 *  \param azimuth Sun azimuth in degrees clockwise from north.
 *
 *  \return Solar angle to the slope in degrees.  A value of 90 indicates the
 *  sun is normal to the slope.  A negative number indicates that the slope is
 *  shaded.  The range is [-90..+90].
 *
 *  \sa CDT_SunPosition().
 */

double CDT_SolarAngle( double slope, double aspect, double altitude,
            double azimuth )
{
    double deg2Rad, rad2Deg, slpRad, aspRad, altRad, azmRad, sunRad;

    /* Slope-to-radians conversion factors */
    deg2Rad = Radians;
    rad2Deg = 1. / Radians;

    /* Convert slope, aspect, and sun positions from degrees to radians */
    slpRad = deg2Rad * ( 90. - slope );
    aspRad = deg2Rad * aspect;
    altRad = deg2Rad * altitude;
    azmRad = deg2Rad * azimuth;
    sunRad = sin( altRad ) * sin( slpRad )
           + cos( altRad ) * cos( slpRad ) * cos( azmRad - aspRad );
    return( rad2Deg * asin(sunRad) );
}

/*----------------------------------------------------------------------------*/
/*! \brief Determines the proportion [0..1] of the solar radiation constant
 *  arriving at the forest floor given:
 *
 *  Uses the algorithm from MTCLIM.
 *
 *  \param jdate Julian date-time.
 *  \param lon Site longitude in degrees (positive if west of Greenwich).
 *  \param lat Site latitude in degrees (positive if north of equator).
 *  \param elev Site elevation in meters.
 *  \param slope Terrain slope in degrees.
 *  \param aspect Terrain aspect (downslope direction in degrees clockwise
 *  from north)
 *  \param canopyTransmittance The canopy transmittance factor [0..1].
 *  \param canopyTransmittance The cloud transmittance factor [0..1].
 *  \param atmTransparency The atmospheric transparency coefficient ([0.6-0.8])
 *  \arg 0.80 Exceptionally clear atmosphere
 *  \arg 0.75 Average clear forest atmosphere
 *  \arg 0.70 Moderate forest (blue) haze
 *  \arg 0.60 Dense haze
 *
 *  \bug Does not account for reflected or diffuse radiation.  Therefore, a
 *  site will have zero radiation if any of the following are true:
 *  \arg the sun is below the horizon,
 *  \arg the slope is self-shaded,
 *  \arg the cloud transmittance is zero, or
 *  \arg the canopy transmittance is zero.
 *
 *  \return Proportion of the solar radiation constant arriving at the forest
 *  floor [0..1].
 */

double CDT_SolarRadiation( double jdate, double lon, double lat, double gmtDiff,
            double slope, double aspect, double elev,
            double atmTransparency, double cloudTransmittance,
            double canopyTransmittance )
{
    double alt, azim, angle, fraction, m;

    /* Get the sun position for this date. */
    //CDT_SunPosition( jdate, lat, lon, gmtDiff, &alt, &azim );
	CDT_SunPosition( jdate, lon, lat, gmtDiff, &alt, &azim );

    /* If the sun is below the horizon, return radiation fraction of zero. */
    if ( alt <= 0.0 )
    {
        return( 0.0 );
    }

    /* If the slope is self-shaded, return radiation fraction of zero. */
    angle = CDT_SolarAngle( slope, aspect, alt, azim );
    if ( angle < 0.0 )
    {
        return( 0.0 );
    }

    /* Optical air mass (from MTCLIM). */
    m = exp(-0.0001467 * (elev / 3.2808) ) / sin( Radians * alt );

    /* Proportion of sr arriving thu air mass, clouds, and canopy. */
    fraction = pow(atmTransparency, m)
             * cloudTransmittance
             * canopyTransmittance
             * sin( Radians * angle );
    return( fraction );
}

/*----------------------------------------------------------------------------*/
/*! \brief Determines the calendar date and time of the requested equinox or
 *  solstice.
 *
 *  Uses the method described by Meeus on page 90.
 *
 *  \param event One of #CDT_Spring, #CDT_SUmmer, #CDT_Fall, or #CDT_Winter.
 *  \param year Julian-Gregorian year of trhe event (-4712 or later).
 *
 *  \return Julian date of the requested solstice or equinox.
 */

double CDT_SolsticeGMT( int event, int year )
{
    static double a[] = { 1721139.2855, 1721233.2486, 1721325.6978, 1721414.3920 };
    static double b1[] = { 365.2421376, 365.2417284, 365.2425055, 365.2428898 };
    static double b2[] = {   0.0679190,  -0.0530180,  -0.1266890,  -0.0109650 };
    static double b3[] = {  -0.0027879,   0.0093320,   0.0019401,  -0.0084885 };
    double y;
    int i;

    /* Determine the event parameter set */
    if ( event == CDT_Spring )
    {
        i = 0;
    }
    else if ( event == CDT_Summer )
    {
        i = 1;
    }
    else if ( event == CDT_Fall )
    {
        i = 2;
    }
    else if ( event == CDT_Winter )
    {
        i = 3;
    }
    else
    {
        return( 0. );
    }

    /* Determine GMT Julian date of the event */
    y = year / 1000;
    return( a[i] + b1[i]*year + b2[i]*y*y + b3[i]*y*y*y );
}

/*----------------------------------------------------------------------------*/
/*! \brief Determines the position of the sun in the sky.
 *
 *  \param jdate        Julian date-time.
 *  \param lon          Observer's longitude (west of GMT is positive).
 *  \param lat          Observer's latitude in degrees.
 *  \param gmtDiff      Local time difference from GMT (local=GMT+gmtDiff).
 *  \param *altitude    Returned sun altitude in degrees from horizon.
 *  \param *azimuth     Returned sun azimuth in degrees clockwise from north.
 *
 *  \return Returns the sun \a altitude and \a azimuth in the passed
 *  arguments. The function returns nothing.
 *
 *  \sa CDT_SolarAngle().
 */

void CDT_SunPosition( double jdate, double lon, double lat,
        double gmtDiff, double *altitude, double *azimuth )
{
    double mjd, ra, dec, t, tau;
    double sinPhi, cosPhi, sinDec, cosDec, cosTau, sinAlt;

    /* Modified Julian date adjusted for GMT difference */
    mjd  = jdate - 2400000.5 - ( gmtDiff / 24. );

    /* Sun declination and right ascension */
    t = (mjd - 51544.5) / 36525.0;
    CDT_MiniSun( t, &ra, &dec );

    /* Sun azimuth */
    tau = 15.0 * ( CDT_LocalMeanSiderealTime( mjd, lon ) - ra );
    if ( ( *azimuth = tau - 180. ) < 0. )
    {
        *azimuth += 360.;
    }

    /* Sun altitude */
    sinPhi = sin( Radians * lat );
    cosPhi = cos( Radians * lat );
    sinDec = sin( Radians * dec );
    cosDec = cos( Radians * dec );
    cosTau = cos( Radians * tau );
    sinAlt = sinPhi * sinDec + cosPhi * cosDec * cosTau;
    *altitude = asin( sinAlt ) / Radians;
    return;
}

/*----------------------------------------------------------------------------*/
/*! \brief Determines if the passed arguments form a valid date and time in
 *  the Western (Julian-Gregorian) calendar.
 *
 *  \param year Julian-Gregorian year (-4712 (4713 B.C.) or greater).
 *  \param month Month of the year (1-12).
 *  \param day Day of the month (1-31).
 *  \param hour Hours past midnight (0-23).
 *  \param minute Minutes past the hour (0-59).
 *  \param second Seconds past the minute (0-59).
 *  \param milliseconds Milliseconds past the second (0-999).
 *
 *  \retval #CDT_HasValidDateTime if all the date and time fields are valid.
 *  \retval #CDT_HasInvalidYear
 *  \retval #CDT_HasInvalidMonth
 *  \retval #CDT_HasInvalidDay
 *  \retval #CDT_HasInvalidHour
 *  \retval #CDT_HasInvalidMinute
 *  \retval #CDT_HasInvalidSecond
 *  \retval #CDT_HasInvalidMillisecond
 */

int CDT_ValidDateTime( int year, int month, int day,
        int hour, int minute, int second, int millisecond )
{
    int flag;
    if ( ( flag = CDT_ValidDate( year, month, day ) ) != CDT_HasValidDate )
    {
        return( flag );
    }
    if ( ( flag = CDT_ValidTime( hour, minute, second, millisecond ) )
        != CDT_HasValidTime )
    {
        return( flag );
    }
    return( CDT_HasValidDateTime );
}

/*----------------------------------------------------------------------------*/
/*! \brief Determines if the passed arguments form a valid date in
 *  the Western (Julian-Gregorian) calendar.
 *
 *  \param year Julian-Gregorian year (-4712 (4713 B.C.) or greater).
 *  \param month Month of the year (1-12).
 *  \param day Day of the month (1-31).
 *
 *  \retval #CDT_HasValidDate if all the date fields are valid.
 *  \retval #CDT_HasInvalidYear
 *  \retval #CDT_HasInvalidMonth
 *  \retval #CDT_HasInvalidDay
 */

int CDT_ValidDate( int year, int month, int day )
{
    if ( year < -4712 )
    {
        return( CDT_HasInvalidYear );
    }
    if ( month < 1 || month > 12 )
    {
        return( CDT_HasInvalidMonth );
    }
    if ( day < 1 || day > CDT_DaysInMonth( year, month ) )
    {
        return( CDT_HasInvalidDay );
    }

    /* Gregorian calendar check. */
    if ( year == 1582 && month == 10 )
    {
        if ( day > 4 && day < 15 )
        {
            return( CDT_HasInvalidDay );
        }
    }
    return( CDT_HasValidDate );
}

/*----------------------------------------------------------------------------*/
/*! \brief Determines if the passed arguments form a valid time.
 *
 *  \param hour Hours past midnight (0-23).
 *  \param minute Minutes past the hour (0-59).
 *  \param second Seconds past the minute (0-59).
 *  \param milliseconds Milliseconds past the second (0-999).
 *
 *  \retval #CDT_HasValidTime if all the time fields are valid.
 *  \retval #CDT_HasInvalidHour
 *  \retval #CDT_HasInvalidMinute
 *  \retval #CDT_HasInvalidSecond
 *  \retval #CDT_HasInvalidMillisecond
 */

int CDT_ValidTime( int hour, int minute, int second, int millisecond )
{
    if (  hour < 0 || hour > 23 )
    {
        return( CDT_HasInvalidHour );
    }
    if ( minute < 0 || minute > 59 )
    {
        return( CDT_HasInvalidMinute );
    }
    else if ( second < 0 || second > 59 )
    {
        return( CDT_HasInvalidSecond );
    }
    else if ( millisecond < 0 || millisecond > 999 )
    {
        return( CDT_HasInvalidMillisecond );
    }
    return( CDT_HasValidTime );
}

/*----------------------------------------------------------------------------*/
/*  End of cdtlib.c                                                           */
/*----------------------------------------------------------------------------*/

