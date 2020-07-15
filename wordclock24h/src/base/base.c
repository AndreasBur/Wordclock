/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * base.c - base routines
 *
 * Copyright (c) 2016-2017 Frank Meyer - frank(at)fli4l.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#include "base.h"
#include "log.h"

const char *                wdays_en[7] = { "Su", "Mo", "Tu", "We", "Th", "Fr", "Sa" };
const char *                wdays_de[7] = { "So", "Mo", "Di", "Mi", "Do", "Fr", "Sa" };

#define SUNDAY               0                                              // first day of a unix week
#define HOURS_PER_DAY       24                                              // hours per day
#define DAYS_PER_WEEK        7                                              // days per week

#define IS_LEAP_YEAR(y)     ((((y) % 4) == 0) && ((((y) % 100) != 0) || (((y) % 400) == 0)))

typedef struct
{
    int     day;
    int     month;
} HOLIDAYS;

static int                  g_days_per_month[]    = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

#define N_HOLIDAYS          9
#define INT(x)              (x)                                             // integer function

static const HOLIDAYS       holiday_table[N_HOLIDAYS] =
{
    {  1,  1 },                                                             // Neujahr
    {  1,  5 },                                                             // Maifeiertag
    {  3, 10 },                                                             // Tag der deutschen Einheit
    { 25, 12 },                                                             // 1. Weihnachtsfeiertag
    { 26, 12 },                                                             // 2. Weihnachtsfeiertag

    { -2,  0 },                                                             // Karfreitag
    {  1,  0 },                                                             // Ostermontag
    { 39,  0 },                                                             // Christi Himmelfahrt
    { 50,  0 },                                                             // Pfingstmontag
#if 0
    { 60,  0 },                                                             // Fronleichnam (kein bundesweiter Feiertag)
#endif
};

static HOLIDAYS             holidays[N_HOLIDAYS];

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * add_days () - add n days
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
add_days (int start_day, int start_month, int start_year, int n_days, int * new_day_p, int * new_month_p)
{
    int days_this_month;

    while (n_days > 0)
    {
        start_day += n_days;

        days_this_month = g_days_per_month[start_month - 1];

        if (start_month == 1 && IS_LEAP_YEAR(start_year))
        {
            days_this_month++;
        }

        if (start_day > days_this_month)
        {
            n_days      = start_day - days_this_month - 1;
            start_day   = 1;
            start_month++;

            if (start_month > 12)
            {
                start_month = 1;
                start_year++;
            }
        }
        else
        {
            n_days = 0;
        }
    }

    while (n_days < 0)
    {
        start_day += n_days;

        if (start_day <= 0)
        {
            n_days = start_day;

            start_month--;

            if (start_month == 0)
            {
                start_month = 12;
                start_year--;
            }

            days_this_month = g_days_per_month[start_month - 1];

            if (start_month == 1 && IS_LEAP_YEAR(start_year))
            {
                days_this_month++;
            }

            start_day = days_this_month;
        }
        else
        {
            n_days = 0;
        }
    }

    *new_day_p      = start_day;
    *new_month_p    = start_month;

    return;
} // add_days (start_day, start_month, start_year, n_days, new_day_p, new_month_p)

/*--------------------------------------------------------------------------------------------------------------------------------------
 * get day of week (0=Sunday, 1=Monday, ... 6=Saturday)
 *
 *  day         - day of month
 *  month       - month beginning with 1
 *  year        - greater than 2000
 *
 *  example:    int rtc = dayofweek (tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900);
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
int
dayofweek (int d, int m, int y)
{
   return (d += m < 3 ? y-- : y - 2 , 23 * m / 9 + d + 4 + y / 4 - y / 100 + y / 400) % 7;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * is_holiday () - check if day is holiday
 *
 *  day         - day of month
 *  month       - month beginning with 1
 *  year        - greater than 2000
 *
 *  example:    int rtc = is_holiday (tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900);
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
int
is_holiday (int day, int month, int year)
{
    static int  last_year;
    static int  ostern_day;
    static int  ostern_month;
    int         a, b, c, d, e;
    int         m, s, M, N, D;
    int         i;
    int         offset;

    if (last_year != year)
    {
        last_year = year;

        a = year % 19;
        b = year % 4;
        c = year % 7;

        m = INT ((8 * INT (year/100) + 13) / 25) - 2;
        s = INT (year/100) - INT (year/400) - 2;

        M = (15 + s - m) % 30;
        N = (6 + s) % 7;

        d = (M + 19 * a) % 30;

        if (d == 29)
        {
            D = 28;
        }
        else if (d == 28 && a >= 11)
        {
            D = 27;
        }
        else
        {
            D = d;
        }

        e = (2 * b + 4 * c + 6 * D + N) % 7;

        offset = D + e + 1;

        ostern_day = 21 + offset;
        ostern_month = 3;

        while (ostern_day > 31)
        {
            ostern_day -= 31;
            ostern_month++;
        }

        for (i = 0; i < N_HOLIDAYS; i++)
        {
            if (holiday_table[i].month == 0)
            {
                add_days (ostern_day, ostern_month, year, holiday_table[i].day, &(holidays[i].day), &(holidays[i].month));
            }
            else
            {
                holidays[i].day     = holiday_table[i].day;
                holidays[i].month   = holiday_table[i].month;
            }
        }

        log_printf ("easter date: %04d-%02d-%02d\r\n", year, ostern_month, ostern_day);
    }

    for (i = 0; i < N_HOLIDAYS; i++)
    {
        if (holidays[i].day == day && holidays[i].month == month)
        {
            return 1;
        }
    }

    return 0;
} // is_holiday (day, month, year)

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * hex to integer
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint16_t
htoi (char * buf, uint8_t max_digits)
{
    uint8_t     i;
    uint8_t     x;
    uint16_t    sum = 0;

    for (i = 0; i < max_digits && *buf; i++)
    {
        x = buf[i];

        if (x >= '0' && x <= '9')
        {
            x -= '0';
        }
        else if (x >= 'A' && x <= 'F')
        {
            x -= 'A' - 10;
        }
        else if (x >= 'a' && x <= 'f')
        {
            x -= 'a' - 10;
        }
        else
        {
            x = 0;
        }
        sum <<= 4;
        sum += x;
    }

    return (sum);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * substitute characters
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
strsubst (char * s, int old, int new)
{
    while (*s)
    {
        if (*s == old)
        {
            *s = new;
        }

        s++;
    }
}
