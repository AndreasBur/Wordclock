/*----------------------------------------------------------------------------------------------------------------------------------------
 * http.cpp - http server
 *
 * Copyright (c) 2016-2017 Frank Meyer - frank(at)fli4l.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>
#include <WString.h>
#include <FS.h>
#include "base.h"
#include "vars.h"
#include "wifi.h"
#include "version.h"
#include "http.h"
#include "httpclient.h"
#include "stm32flash.h"

#define WCLOCK24H   1

const char *                wdays_en[7] = { "Su", "Mo", "Tu", "We", "Th", "Fr", "Sa" };
const char *                wdays_de[7] = { "So", "Mo", "Di", "Mi", "Do", "Fr", "Sa" };

WiFiServer                  http_server(80);                                        // create an instance of the server on Port 80
static WiFiClient           http_client;

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * firmware update parameters
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#define DEFAULT_UPDATE_HOST                         "uclock.de"
#define DEFAULT_UPDATE_PATH                         "update"

#define ESP_WORDCLOCK_TXT                           "ESP-WordClock.txt"             // avaliable version of ESP8266 firmware
#define ESP_WORDCLOCK_BIN                           "ESP-WordClock-4M.bin"          // name of ES8266 firmware bin file

#define RELEASENOTE_HTML                            "releasenote.html"              // release notes
#define WC_TXT                                      "wc.txt"                        // avaliable version of STM32 firmware
#define WC_LIST_TXT                                 "wc-list.txt"                   // list of available STM32 firmware files

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * http parameters
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#define MAX_LINE_LEN                                256                             // max. length of line_buffer
#define MAX_PATH_LEN                                20                              // max. length of path
#define MAX_HTTP_PARAMS                             16                              // max. number of http parameters

typedef struct
{
    char *  name;
    char *  value;
} HTTP_PARAMETERS;

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * globals
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static HTTP_PARAMETERS                              http_parameters[MAX_HTTP_PARAMS];
static int                                          bgcolor_cnt;

#define MAX_KEY_LEN                                 64
#define MAX_IP_LEN                                  15
#define MAX_TIMEZONE_LEN                            3
#define MAX_DATE_LEN                                10
#define MAX_TIME_LEN                                5
#define MAX_BRIGHTNESS_LEN                          2
#define MAX_COLOR_VALUE_LEN                         2
#define MAX_TEMP_CORR_LEN                           2
#define MAX_MINUTE_INTERVAL_LEN                     2
#define MAX_TICKER_DECELERATION_LEN                 3
#define MAX_RAINBOW_DECELERATION_LEN                3
#define MAX_RAW_VALUE_LEN                           5
#define MAX_ANIMATION_DECELERATION_LEN              2
#define MAX_COLOR_ANIMATION_DECELERATION_LEN        2
#define MAX_AMBILIGHT_MODE_DECELERATION_LEN         2

#define MAIN_HEADER_COLS                            2
#define DATETIME_HEADER_COLS                        6
#define TICKER_HEADER_COLS                          2
#define NETWORK_HEADER_COLS                         4
#define NETWORK_HEADER_COLS2                        3
#define WEATHER_HEADER_COLS                         3
#define DISPLAY_HEADER_COLS                         3
#define OVERLAY_HEADER_COLS                         3
#define ANIMATION_HEADER_COLS                       3
#define ANIMATION_DECELERATION_HEADER_COLS          5
#define COLOR_ANIMATION_DECELERATION_HEADER_COLS    4
#define AMBILIGHT_MODE_DECELERATION_HEADER_COLS     4
#define TIMERS_HEADER_COLS                          8
#define UPDATE_HEADER_COLS                          2

#define MAX_HTTP_RESPONSE_LEN                       1024
static char     http_response[MAX_HTTP_RESPONSE_LEN + 1];
static int      http_response_len = 0;
/*-------------------------------------------------------------------------------------------------------------------------------------------
 * display flags:
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#define DISPLAY_FLAGS_NONE                          0x00                    // no display flag
#define DISPLAY_FLAGS_PERMANENT_IT_IS               0x01                    // show "ES IST" permanently
#define DISPLAY_FLAGS_SYNC_AMBILIGHT                0x02                    // synchronize display and ambilight

/*--------------------------------------------------------------------------------------------------------------------------------------
 * possible modes of ESP8266
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
#define ESP8266_CLIENT_MODE                         0
#define ESP8266_AP_MODE                             1

#define ESP8266_MAX_FIRMWARE_SIZE                   16
#define ESP8266_MAX_ACCESSPOINT_SIZE                32
#define ESP8266_MAX_IPADDRESS_SIZE                  16
#define ESP8266_MAX_HTTP_GET_PARAM_SIZE             256
#define ESP8266_MAX_CMD_SIZE                        32
#define ESP8266_MAX_TIME_SIZE                       16

static struct tm tm;

#define MAX_COLOR_STEPS                             64

/*--------------------------------------------------------------------------------------------------------------------------------------
 * max update filename len
 *--------------------------------------------------------------------------------------------------------------------------------------
 */
#define MAX_UPDATE_FILENAME_LEN                     64

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * flush output buffer
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
http_flush (void)
{
    if (http_response_len > 0)
    {
        http_client.print (http_response);
        http_client.flush ();
        http_response[0] = '\0';
        http_response_len = 0;
    }
}

static void
http_concat_response (const char * s, int len)
{
    strncpy (http_response + http_response_len, s, len);
    http_response_len += len;
    http_response[http_response_len] = '\0';
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * send string
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
http_send (const char * s)
{
    int len = strlen (s);

    while (http_response_len + len > MAX_HTTP_RESPONSE_LEN)
    {
        int rest = MAX_HTTP_RESPONSE_LEN - http_response_len;

        http_concat_response (s, rest);
        http_flush ();

        len -= rest;
        s += rest;
    }

    http_concat_response (s, len);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * send string
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
http_send (String s)
{
    http_send (s.c_str());
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * set parameters from list
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
http_set_params (char * paramlist)
{
    char *  p;                                              // ap=access&pw=secret&action=saveap
    int     idx = 0;

    if (paramlist && *paramlist)
    {
        http_parameters[idx].name = paramlist;

        for (p = paramlist; idx < MAX_HTTP_PARAMS - 1 && *p; p++)
        {
            if (*p == '=')
            {
                *p = '\0';
                http_parameters[idx].value = p + 1;
            }
            else if (*p == '&')
            {
                *p = '\0';
                idx++;
                http_parameters[idx].name = p + 1;
            }
        }
        idx++;
    }

    while (idx < MAX_HTTP_PARAMS)
    {
        http_parameters[idx].name = (char *) 0;
        idx++;
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * get a parameter
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static char *
http_get_param (const char * name)
{
    static char empty[] = "";
    int     idx;

    for (idx = 0; idx < MAX_HTTP_PARAMS && http_parameters[idx].name != (char *) 0; idx++)
    {
        if (! strcmp (http_parameters[idx].name, name))
        {
            return http_parameters[idx].value;
        }
    }

    return empty;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * get a parameter by index
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static char *
http_get_param_by_idx (const char * name, int idx)
{
    char    name_buf[16];
    char *  rtc;

    sprintf (name_buf, "%s%d", name, idx);
    rtc = http_get_param (name_buf);

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * get a checkbox parameter
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static int
http_get_checkbox_param (const char * name)
{
    char *  value = http_get_param (name);
    int     rtc;

    if (! strcmp (value, "active"))
    {
        rtc = 1;
    }
    else
    {
        rtc = 0;
    }

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * get a checkbox parameter by index
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static int
http_get_checkbox_param_by_idx (const char * name, int idx)
{
    char *  value = http_get_param_by_idx (name, idx);
    int     rtc;

    if (! strcmp (value, "active"))
    {
        rtc = 1;
    }
    else
    {
        rtc = 0;
    }

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * send http and html header
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
http_header (const char * title, const char * refresh, const char * url)
{
    http_send_FS ("HTTP/1.0 200 OK\r\n\r\n<html><head><title>");
    http_send (title);
    http_send_FS ("</title>");

    if (refresh)
    {
        http_send_FS ("<meta http-equiv=\"refresh\" content=\"");
        http_send (refresh);
        http_send_FS ("; URL=");
        http_send (url);
        http_send_FS ("\">");
    }

    http_send_FS ("</head><body><H1>");
    http_send (title);
    http_send_FS ("</H1>\r\n");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * send html trailer
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
http_trailer (void)
{
    http_send_FS ("</body></html>\r\n");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * send table header columns
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
table_header (const char ** columns, int cols)
{
    int i;

    http_send_FS ("<table border=0><tr>");

    for (i = 0; i < cols; i++)
    {
        http_send_FS ("<th><font color=blue>");
        http_send (columns[i]);
        http_send_FS ("</font></th>");
    }

    http_send_FS ("</tr>\r\n");
    bgcolor_cnt = 0;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * begin form
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
begin_form (const char * page)
{
    http_send_FS ("<form method=\"GET\" action=\"/");
    http_send (page);
    http_send_FS ("\">\r\n");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * end form
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
void
end_form (void)
{
    http_send_FS ("</form>\r\n");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * begin table row
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
begin_table_row (void)
{
    bgcolor_cnt++;

    if (bgcolor_cnt & 0x01)
    {
        http_send_FS ("<tr bgcolor=#f0f0f0>\r\n");
    }
    else
    {
        http_send_FS ("<tr>\r\n");
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * begin table row as form
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
begin_table_row_form (const char * page)
{
    begin_table_row ();
    begin_form (page);
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * end table row
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
end_table_row (void)
{
    http_send_FS ("</tr>\r\n");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * end table row as form
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
end_table_row_form (void)
{
    end_form ();
    end_table_row ();
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * checkbox field
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
checkbox_field (const char * id, const char * desc, int checked)
{
    http_send (desc);
    http_send_FS ("&nbsp;<input type=\"checkbox\" name=\"");
    http_send (id);
    http_send_FS ("\" value=\"active\" ");

    if (checked)
    {
        http_send_FS ("checked");
    }

    http_send_FS (">");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * checkbox column
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
checkbox_column (const char * id, const char * desc, int checked)
{
    http_send_FS ("<td>");
    checkbox_field (id, desc, checked);
    http_send_FS ("</td>");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * text column
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
text_column (const char * text)
{
    http_send_FS ("<td>");
    http_send (text);
    http_send_FS ("</td>");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * input field
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
input_field (const char * id, const char * desc, const char * value, int maxlength, int maxsize)
{
    char maxlength_buf[8];
    char maxsize_buf[8];

    sprintf (maxlength_buf, "%d", maxlength);
    sprintf (maxsize_buf, "%d", maxsize);

    if (desc && *desc)
    {
        http_send (desc);
        http_send_FS ("&nbsp;");
    }

    http_send_FS ("<input type=\"text\" id=\"");
    http_send (id);
    http_send_FS ("\" name=\"");
    http_send (id);
    http_send_FS ("\" value=\"");
    http_send (value);
    http_send_FS ("\" maxlength=\"");
    http_send (maxlength_buf);
    http_send_FS ("\" size=\"");
    http_send (maxsize_buf);
    http_send_FS ("\">");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * input column
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
input_column (const char * id, const char * desc, const char * value, int maxlength, int maxsize)
{
    http_send_FS ("<td>");
    input_field (id, desc, value, maxlength, maxsize);
    http_send_FS ("</td>");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * select field
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
select_field (const char * id, const char ** text, int selected_value, int max_values)
{
    char    buf[3];
    int     i;

    http_send_FS ("<select id=\"");
    http_send (id);
    http_send_FS ("\" name=\"");
    http_send (id);
    http_send_FS ("\">");

    for (i = 0; i < max_values; i++)
    {
        sprintf (buf, "%d", i);
        http_send_FS ("<option value=\"");
        http_send (buf);
        http_send_FS ("\"");

        if (i == selected_value)
        {
            http_send_FS (" selected");
        }

        http_send_FS (">");
        http_send ((char *) text[i]);
        http_send_FS ("</option>");
    }

    http_send_FS ("</select>\r\n");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * select column
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
select_column (const char * id, const char ** text, int selected_value, int max_values)
{
    http_send_FS ("<td>");
    select_field (id, text, selected_value, max_values);
    http_send_FS ("</td>");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * slider field
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
slider_field (const char * id, const char * text, const char * value, const char * min, const char * max, const char * pixelwidth)
{
    if (text && *text)
    {
        http_send (text);
        http_send_FS ("&nbsp;");
    }

    http_send_FS ("<input type=\"range\" id=\"");
    http_send (id);
    http_send_FS ("\" name=\"");
    http_send (id);
    http_send_FS ("\" value=\"");
    http_send (value);
    http_send_FS ("\" min=\"");
    http_send (min);
    http_send_FS ("\" max=\"");
    http_send (max);

    if (pixelwidth && *pixelwidth)
    {
        http_send_FS ("\" style=\"width:");
        http_send (pixelwidth);
        http_send_FS ("px;");
    }

    http_send_FS ("\" oninput=\"");
    http_send (id);
    http_send_FS ("_output.value=");
    http_send (id);
    http_send_FS (".value\">");
    http_send_FS ("<output name=\"");
    http_send (id);
    http_send_FS ("_output\">");
    http_send (value);
    http_send_FS ("</output>");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * slider column
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
slider_column (const char * id, const char * value, const char * min, const char * max)
{
    http_send_FS ("<td>");
    slider_field (id, "", value, min, max, "");
    http_send_FS ("</td>\r\n");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * submit button field
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
button_field (const char * id, const char * text)
{
    http_send_FS ("<button type=\"submit\" name=\"action\" value=\"");
    http_send (id);
    http_send_FS ("\">");
    http_send (text);
    http_send_FS ("</button>&nbsp;");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * submit column field
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
button_column (const char * id, const char * text)
{
    http_send_FS ("<td>");
    button_field (id, text);
    http_send_FS ("</td>\r\n");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * save column
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
save_column (const char * id)
{
    http_send_FS ("<td><button type=\"submit\" name=\"action\" value=\"save");
    http_send (id);
    http_send_FS ("\">Save</button></td>\r\n");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * table row
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
table_row (const char * col1, const char * col2, const char * col3)
{
    begin_table_row ();
    text_column (col1);
    text_column (col2);
    text_column (col3);
    end_table_row ();
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * table row with input
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
table_row_input (const char * page, int cols, const char * text, const char * id, const char * value, int maxlength)
{
    int cols_used = 1;

    begin_table_row_form (page);
    text_column (text);
    cols_used++;
    input_column (id, "", value, maxlength, maxlength);
    cols_used++;

    while (cols_used < cols)
    {
        http_send_FS ("<td></td>");
        cols_used++;
    }

    save_column (id);
    end_table_row_form ();
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * table row with multiple inputs
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
table_row_inputs (const char * page, const char * id, int n, const char ** ids, const char ** desc, const char ** value, int * maxlength, int * maxsize)
{
    int     i;

    begin_table_row_form (page);

    for (i = 0; i < n; i++)
    {
        http_send_FS ("<td>");
        input_field (ids[i], desc[i], value[i], maxlength[i], maxsize[i]);
        http_send_FS ("</td>");
    }

    save_column (id);
    end_table_row_form ();
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * table row with checkbox
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
table_row_checkbox (const char * page, const char * text, const char * id, const char * desc, int checked)
{
    begin_table_row_form (page);
    text_column (text);
    checkbox_column (id, desc, checked);
    save_column (id);
    end_table_row_form ();
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * table row with selection
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
table_row_select (const char * page, const char * text1, const char * id, const char ** text2, int selected_value, int max_values)
{
    begin_table_row_form (page);
    text_column (text1);
    select_column (id, text2, selected_value, max_values);
    save_column (id);
    end_table_row_form ();
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * table row with slider
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
table_row_slider (const char * page, const char * text1, const char * id, const char * text2, const char * min, const char * max)
{
    begin_table_row_form (page);
    text_column (text1);
    slider_column (id, text2, min, max);
    save_column (id);
    end_table_row_form ();
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * table row with n sliders
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
table_row_sliders (const char * page, const char * text1, const char * id, int n, const char ** ids, const char ** desc, const char * const * text2, const char ** min, const char ** max)
{
    int i;

    begin_table_row_form (page);
    text_column (text1);

    http_send_FS ("<td>");

    for (i = 0; i < n; i++)
    {
        slider_field (ids[i], desc[i], text2[i], min[i], max[i], "64");
        http_send_FS ("&nbsp;&nbsp;&nbsp;\r\n");
    }

    http_send_FS ("</td>\r\n");

    save_column (id);
    end_table_row_form ();
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * table trailer
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
table_trailer (void)
{
    http_send_FS ("</table><P>\r\n");
}

static void
menu_entry (const char * page, const char * entry)
{
    http_send_FS ("<a href=\"/");
    http_send (page);
    http_send_FS ("\">");
    http_send (entry);
    http_send_FS ("</a>&nbsp;&nbsp;");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * menu
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
http_menu (void)
{
    menu_entry ("", "Main");
    menu_entry ("network", "Network");
    menu_entry ("temperature", "Temperature");
    menu_entry ("weather", "Weather");
    menu_entry ("ldr", "LDR");
    menu_entry ("brightness", "Brightness");
    menu_entry ("display", "Display");
    menu_entry ("animations", "Animations");
    menu_entry ("ambilight", "Ambilight");
    menu_entry ("timers", "Timers");
    menu_entry ("atimers", "Ambilight Timers");
    menu_entry ("update", "Update");
    menu_entry ("flash_stm32_local", "Local Update");
    http_send_FS ("\r\n<P>");
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * main page
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
http_main (void)
{
    const char *    thispage = "";
    const char *    header_cols[MAIN_HEADER_COLS]               = { "Name", "Value" };
    const char *    datetime_header_cols[DATETIME_HEADER_COLS]  = { "YYYY", "MM", "DD", "hh", "mm", "Action" };
    const char *    ids[5]                                      = { "year", "month", "day", "hour", "min" };
    const char *    desc[5]                                     = { "", "", "", "", "" };
    int             maxlen[5]                                   = { 4, 2, 2, 2, 2 };
    int             maxsize[5]                                  = { 4, 2, 2, 2, 2 };
    char            year_str[5];
    char            mon_str[3];
    char            day_str[3];
    char            hour_str[3];
    char            minutes_str[3];
    const char *    values[5]                                   = { year_str, mon_str, day_str, hour_str, minutes_str };
    char *          action;
    const char *    message                                     = (const char *) 0;
    STR_VAR *       sv;
    char *          version;
    char *          eeprom_version;
    uint_fast8_t    rtc                                         = 0;
    struct tm *     tmp;
    uint_fast8_t    eeprom_is_up;

    sv              = get_strvar (VERSION_STR_VAR);
    version         = sv->str;

    sv              = get_strvar (EEPROM_VERSION_STR_VAR);
    eeprom_version  = sv->str;

    tmp = get_tm_var (CURRENT_TM_VAR);

    if (tmp->tm_year >= 0 && tmp->tm_mon >= 0 && tmp->tm_mday >= 0 && tmp->tm_hour >= 0 && tmp->tm_min >= 0 &&
        tmp->tm_year <= 1200 && tmp->tm_mon <= 12 && tmp->tm_mday <= 31 && tmp->tm_hour < 24 && tmp->tm_min < 60)
    {                                                               // check values to avoid buffer overflow
        sprintf (year_str,      "%4d",  tmp->tm_year + 1900);
        sprintf (mon_str,       "%02d", tmp->tm_mon + 1);
        sprintf (day_str,       "%02d", tmp->tm_mday);
        sprintf (hour_str,      "%02d", tmp->tm_hour);
        sprintf (minutes_str,   "%02d", tmp->tm_min);
    }
    else
    {
        year_str[0]     = '\0';
        mon_str[0]      = '\0';
        day_str[0]      = '\0';
        hour_str[0]     = '\0';
        minutes_str[0]  = '\0';
    }

    eeprom_is_up = get_numvar (EEPROM_IS_UP_NUM_VAR);
    action = http_get_param ("action");

    if (action)
    {
        if (! strcmp (action, "learnir"))
        {
            message = "Learning IR remote control...";
            rpc (LEARN_IR_RPC_VAR);
        }
        else if (! strcmp (action, "poweron"))
        {
            message = "Switching power on...";
            set_numvar (DISPLAY_POWER_NUM_VAR, 1);
        }
        else if (! strcmp (action, "poweroff"))
        {
            message = "Switching power off...";
            set_numvar (DISPLAY_POWER_NUM_VAR, 0);
        }
        else if (! strcmp (action, "apoweron"))
        {
            message = "Switching ambilight power on...";
            set_numvar (DISPLAY_AMBILIGHT_POWER_NUM_VAR, 1);
        }
        else if (! strcmp (action, "apoweroff"))
        {
            message = "Switching ambilight power off...";
            set_numvar (DISPLAY_AMBILIGHT_POWER_NUM_VAR, 0);
        }
        else if (! strcmp (action, "savedatetime"))
        {
            TM tm;

            int year    = atoi (http_get_param ("year"));
            int month   = atoi (http_get_param ("month"));
            int day     = atoi (http_get_param ("day"));
            int hour    = atoi (http_get_param ("hour"));
            int minutes = atoi (http_get_param ("min"));

            sprintf (year_str,      "%4d",  year);
            sprintf (mon_str,       "%02d", month);
            sprintf (day_str,       "%02d", day);
            sprintf (hour_str,      "%02d", hour);
            sprintf (minutes_str,   "%02d", minutes);

            tm.tm_year  = year - 1900;
            tm.tm_mon   = month - 1;
            tm.tm_mday  = day;
            tm.tm_hour  = hour;
            tm.tm_min   = minutes;
            tm.tm_sec   = 0;
            tm.tm_wday  = dayofweek (day, month, year);

            set_tm_var (CURRENT_TM_VAR, &tm);
        }
        else if (! strcmp (action, "saveticker"))
        {
            char * ticker = http_get_param ("ticker");
            set_strvar (TICKER_TEXT_STR_VAR, ticker);
        }
    }

    http_header ("WordClock", (const char *) NULL, (const char *) NULL);
    http_menu ();

    table_header (header_cols, MAIN_HEADER_COLS);
    table_row ("Version", version, "");
    table_row ("EEPROM", eeprom_is_up ? "online" : "offline", "");
    if (eeprom_is_up)
    {
        table_row ("EEPROM Version", eeprom_version, "");
    }
    table_trailer ();

    table_header (datetime_header_cols, DATETIME_HEADER_COLS);
    table_row_inputs (thispage, "datetime", 5, ids, desc, values, maxlen, maxsize);
    table_trailer ();

    table_header (header_cols, TICKER_HEADER_COLS);
    table_row_input (thispage, 3, "Ticker", "ticker", "", MAX_TICKER_TEXT_LEN);
    table_trailer ();

    begin_form (thispage);
    button_field ("poweron", "Power On");
    button_field ("poweroff", "Power Off");
    button_field ("apoweron", "Ambilight Power On");
    button_field ("apoweroff", "Ambilight Power Off");
    button_field ("learnir", "Learn IR remote control");
    button_field ("eepromdump", "EEPROM dump");
    end_form ();

    if (! strcmp (action, "eepromdump"))
    {
        message = "Currently not implemented";
        // http_eeprom_dump ();
    }

    if (message)
    {
        http_send_FS ("<P><font color=green>");
        http_send (message);
        http_send_FS ("</font>\r\n");
    }

    http_trailer ();
    http_flush ();

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * network page
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
http_network (void)
{
    const char *                thispage = "network";
    const char *                header_cols[NETWORK_HEADER_COLS] = { "Name", "Value", "Key", "Action" };
    const char *                header_cols2[NETWORK_HEADER_COLS2] = { "Name", "Value", "Action" };
    char *                      action;
    const char *                message = (const char *) 0;
    const char *                alert_message = (const char *) 0;
    const char *                esp_firmware_version;
    char                        timezone_str[MAX_TIMEZONE_LEN+1];
    int                         networks;
    int                         idx;
    uint_fast16_t               utz;
    int_fast16_t                tz;
    STR_VAR *                   sv;
    uint_fast8_t                rtc             = 0;

    esp_firmware_version        = ESP_VERSION;

    utz = get_numvar (TIMEZONE_NUM_VAR);

    tz = utz & 0xFF;

    if (utz & 0x100)
    {
        tz = -tz;
    }

    action = http_get_param ("action");

    if (action)
    {
        if (! strcmp (action, "savewlanlist"))
        {
            char * ssid = http_get_param ("ssidlist");
            char * key  = http_get_param ("keylist");

            http_header ("WordClock Network", (const char *) NULL, (const char *) NULL);
            http_send_FS ("<B>Connecting to Access Point, try again later.</B>");
            http_trailer ();
            http_flush ();

            wifi_connect (ssid, key);
            return 0;
        }
        else if (! strcmp (action, "savewlan"))
        {
            char * ssid = http_get_param ("ssid");
            char * key  = http_get_param ("key");

            if (strlen (key) < 10)
            {
                alert_message = "Minimum length of key is 10!";
            }
            else
            {
                http_header ("WordClock Network", (const char *) NULL, (const char *) NULL);

                http_send_FS ("<B>Restarting as Access Point, try again later.</B>");
                http_trailer ();
                http_flush ();

                wifi_ap (ssid, key);
                return 0;
            }
        }
        else if (! strcmp (action, "savetimeserver"))
        {
            char * newtimeserver = http_get_param ("timeserver");

            set_strvar (TIMESERVER_STR_VAR, newtimeserver);
            message = "Timeserver successfully changed.";
        }
        else if (! strcmp (action, "savetimezone"))
        {
            tz = atoi (http_get_param ("timezone"));

            if (tz < 0)
            {
                utz = -tz;
                utz |= 0x100;
            }
            else
            {
                utz = tz;
            }

            set_numvar (TIMEZONE_NUM_VAR, utz);
            message = "Timezone successfully changed.";
        }
        else if (! strcmp (action, "nettime"))
        {
            message = "Getting net time";
            rpc (GET_NET_TIME_RPC_VAR);
        }
        else if (! strcmp (action, "wps"))
        {
            http_header ("WordClock Network", (const char *) NULL, (const char *) NULL);

            http_send_FS ("<B>Connecting to AP via WPS, try again later.</B>");
            http_trailer ();
            http_flush ();

            wifi_wps ();
            return 0;
        }
    }

    sprintf (timezone_str, "%d", tz);

    http_header ("WordClock Network", (const char *) NULL, (const char *) NULL);
    http_menu ();

    http_send_FS ("<table><tr><td>ESP8266 firmware</td><td>");
    http_send (esp_firmware_version);
    http_send_FS ("</td></tr>\r\n");

    http_send_FS ("<tr><td>IP address</td><td>");
    http_send (wifi_ip_address);
    http_send_FS ("</td></tr></table><P>\r\n");

    table_header (header_cols, NETWORK_HEADER_COLS);

    begin_table_row_form (thispage);
    text_column ("WLAN Client");
    http_send_FS ("<td>");
    networks = WiFi.scanNetworks();
    http_send_FS ("<select id=\"ssidlist\" name=\"ssidlist\">");

    for (idx = 0; idx < networks; ++idx)
    {
        String tmp_ssid = WiFi.SSID(idx);

        http_send_FS ("<option value=\"");
        http_send (tmp_ssid);

        if (! strcmp (wifi_ssid, tmp_ssid.c_str()))
        {
            http_send_FS ("\" selected>");
        }
        else
        {
            http_send_FS ("\">");
        }

        http_send (tmp_ssid);
        http_send_FS ("</option>\r\n");
    }

    http_send_FS ("</select></td><td>");
    input_field ("keylist", "", "", MAX_KEY_LEN, 20);
    http_send_FS ("</td>");
    save_column ("wlanlist");
    end_table_row_form ();

    begin_table_row_form (thispage);
    text_column ("WLAN AP");
    http_send_FS ("<td>");
    input_field ("ssid", "", "", WIFI_MAX_SSID_LEN, 20);
    http_send_FS ("</td><td>");
    input_field ("key", "", "", MAX_KEY_LEN, 20);
    http_send_FS ("</td>");
    save_column ("wlan");
    end_table_row_form ();
    table_trailer ();

    begin_form (thispage);
    button_field ("wps", "WPS");
    end_form ();
    http_send_FS ("<P>");

    table_header (header_cols2, NETWORK_HEADER_COLS2);
    sv = get_strvar (TIMESERVER_STR_VAR);
    table_row_input (thispage, 4, "Time server", "timeserver", sv->str, MAX_IP_LEN);
    table_row_input (thispage, 4, "Time zone (GMT +)", "timezone", timezone_str, MAX_TIMEZONE_LEN);
    table_trailer ();

    begin_form (thispage);
    button_field ("nettime", "Get net time");
    end_form ();
    http_send_FS ("<P>");

    if (alert_message)
    {
        http_send_FS ("<P><font color=red><B>");
        http_send (alert_message);
        http_send_FS ("</B></font>\r\n");
    }
    else if (message)
    {
        http_send_FS ("<P>\r\n<font color=green>");
        http_send (message);
        http_send_FS ("</font>\r\n");
    }

    http_trailer ();
    http_flush ();

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * temperature page
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
http_temperature (void)
{
    const char *    thispage = "temperature";
    const char *    header_cols[MAIN_HEADER_COLS]               = { "Name", "Value" };
    char *          action;
    const char *    message                                     = (const char *) 0;
    uint_fast8_t    rtc                                         = 0;
    char            rtc_temp[16];
    char            temperature_correction_str[4];
    char            rtc_temperature_correction_str[4];
    uint_fast8_t    rtc_temperature_correction;
    uint_fast8_t    temperature_correction;
    char            ds18xx_temp[16];
    uint_fast8_t    temp_index;
    uint_fast8_t    rtc_is_up;

    action = http_get_param ("action");

    if (action)
    {
        if (! strcmp (action, "savetcorrrtc"))
        {
            temp_index                  = get_numvar (RTC_TEMP_INDEX_NUM_VAR);
            uint_fast8_t old_correction = get_numvar (RTC_TEMP_CORRECTION_NUM_VAR);
            int temp_corr               = atoi (http_get_param ("tcorrrtc"));

            if (temp_corr < 0)
            {
                temp_corr = -temp_corr;
            }

            rtc_temperature_correction = temp_corr;

            if (old_correction > rtc_temperature_correction)                // correct immediately here
            {
                temp_index += (old_correction - rtc_temperature_correction);
            }
            else
            {
                temp_index -= (rtc_temperature_correction - old_correction);
            }

            set_numvar (RTC_TEMP_INDEX_NUM_VAR, temp_index);
            set_numvar (RTC_TEMP_CORRECTION_NUM_VAR, rtc_temperature_correction);
        }
        else if (! strcmp (action, "savetcorrds18xx"))
        {
            temp_index                  = get_numvar (DS18XX_TEMP_INDEX_NUM_VAR);
            uint_fast8_t old_correction = get_numvar (DS18XX_TEMP_CORRECTION_NUM_VAR);
            int temp_corr               = atoi (http_get_param ("tcorrds18xx"));

            if (temp_corr < 0)
            {
                temp_corr = -temp_corr;
            }

            temperature_correction = temp_corr;

            if (old_correction > temperature_correction)                // correct immediately here
            {
                temp_index += (old_correction - temperature_correction);
            }
            else
            {
                temp_index -= (temperature_correction - old_correction);
            }

            set_numvar (DS18XX_TEMP_INDEX_NUM_VAR, temp_index);
            set_numvar (DS18XX_TEMP_CORRECTION_NUM_VAR, temperature_correction);
        }
        else if (! strcmp (action, "displaytemperature"))
        {
            message = "Displaying temperature...";
            rpc (DISPLAY_TEMPERATURE_RPC_VAR);
        }
    }

    rtc_is_up = get_numvar (RTC_IS_UP_NUM_VAR);

    if (rtc_is_up)
    {
        temp_index = get_numvar (RTC_TEMP_INDEX_NUM_VAR);
        sprintf (rtc_temp, "%d", temp_index / 2);

        if (temp_index % 2)
        {
            strcat (rtc_temp, ".5");
        }

        strcat (rtc_temp, "&deg;C");
    }
    else
    {
        strcpy (rtc_temp, "offline");
    }

    uint_fast8_t ds18xx_is_up = get_numvar (DS18XX_IS_UP_NUM_VAR);

    if (ds18xx_is_up)
    {
        temp_index = get_numvar (DS18XX_TEMP_INDEX_NUM_VAR);
        sprintf (ds18xx_temp, "%d", temp_index / 2);

        if (temp_index % 2)
        {
            strcat (ds18xx_temp, ".5");
        }

        strcat (ds18xx_temp, "&deg;C");
    }
    else
    {
        strcpy (ds18xx_temp, "offline");
    }

    rtc_temperature_correction = get_numvar (RTC_TEMP_CORRECTION_NUM_VAR);

    if (rtc_temperature_correction)
    {
        sprintf (rtc_temperature_correction_str, "%d", rtc_temperature_correction);
    }
    else
    {
        rtc_temperature_correction_str[0] = '\0';
    }

    temperature_correction = get_numvar (DS18XX_TEMP_CORRECTION_NUM_VAR);

    if (temperature_correction)
    {
        sprintf (temperature_correction_str, "%d", temperature_correction);
    }
    else
    {
        temperature_correction_str[0] = '\0';
    }

    http_header ("WordClock Temperature", (const char *) NULL, (const char *) NULL);
    http_menu ();

    table_header (header_cols, MAIN_HEADER_COLS);
    table_row ("RTC temperature", rtc_temp, "");
    table_row ("DS18xx", ds18xx_temp, "");
    table_row_input (thispage, 3, "Temp correction RTC (units of 0.5&deg;C)", "tcorrrtc", rtc_temperature_correction_str, MAX_TEMP_CORR_LEN);
    table_row_input (thispage, 3, "Temp correction DS18xx (units of 0.5&deg;C)", "tcorrds18xx", temperature_correction_str, MAX_TEMP_CORR_LEN);
    table_trailer ();

    begin_form (thispage);
    button_field ("displaytemperature", "Display temperature");
    end_form ();

    if (message)
    {
        http_send_FS ("<P><font color=green>");
        http_send (message);
        http_send_FS ("</font>\r\n");
    }

    http_trailer ();
    http_flush ();

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * weather page
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
http_weather (void)
{
    const char *                thispage = "weather";
    const char *                header_cols[WEATHER_HEADER_COLS] = { "Name", "Value", "Action" };
    char *                      action;
    const char *                message = (const char *) 0;
    const char *                alert_message = (const char *) 0;
    STR_VAR *                   sv;
    uint_fast8_t                rtc     = 0;

    action = http_get_param ("action");

    if (action)
    {
        if (! strcmp (action, "saveappid"))
        {
            char * newappid = http_get_param ("appid");

            set_strvar (WEATHER_APPID_STR_VAR, newappid);
            message = "AppID successfully changed.";
        }
        else if (! strcmp (action, "savecity"))
        {
            char * newcity = http_get_param ("city");

            set_strvar (WEATHER_CITY_STR_VAR, newcity);
            message = "City successfully changed.";
        }
        else if (! strcmp (action, "savelonlat"))
        {
            char * newlon = http_get_param ("lon");
            char * newlat = http_get_param ("lat");

            strsubst (newlon, ',', '.');
            strsubst (newlat, ',', '.');

            set_strvar (WEATHER_LON_STR_VAR, newlon);
            set_strvar (WEATHER_LAT_STR_VAR, newlat);
            message = "Coordinates successfully changed.";
        }
        else if (! strcmp (action, "getweather"))
        {
            message = "Getting weather";
            rpc (GET_WEATHER_RPC_VAR);
        }
    }

    http_header ("WordClock Weather", (const char *) NULL, (const char *) NULL);
    http_menu ();
    table_header (header_cols, WEATHER_HEADER_COLS);

    sv = get_strvar (WEATHER_APPID_STR_VAR);
    table_row_input (thispage, 3, "APPID", "appid", sv->str, MAX_WEATHER_APPID_LEN);
    sv = get_strvar (WEATHER_CITY_STR_VAR);
    table_row_input (thispage, 3, "City", "city", sv->str, MAX_WEATHER_CITY_LEN);

    const char *    ids[2]                                      = { "lon", "lat" };
    const char *    desc[2]                                     = { "LON", "LAT" };
    int             maxlen[2]                                   = { MAX_WEATHER_LON_LEN, MAX_WEATHER_LAT_LEN };
    int             maxsize[2]                                  = { MAX_WEATHER_LON_LEN, MAX_WEATHER_LAT_LEN };
    const char *    values[2];

    sv = get_strvar (WEATHER_LON_STR_VAR);
    values[0] = sv->str;
    sv = get_strvar (WEATHER_LAT_STR_VAR);
    values[1] = sv->str;

    table_row_inputs (thispage, "lonlat", 2, ids, desc, values, maxlen, maxsize);

    table_trailer ();

    begin_form (thispage);
    button_field ("getweather", "Get weather");
    end_form ();

    if (alert_message)
    {
        http_send_FS ("<P><font color=red><B>");
        http_send (alert_message);
        http_send_FS ("</B></font>\r\n");
    }
    else if (message)
    {
        http_send_FS ("<P><font color=green>");
        http_send (message);
        http_send_FS ("</font>\r\n");
    }

    http_trailer ();
    http_flush ();

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * ldr page
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
http_ldr (void)
{
    const char *    thispage = "ldr";
    const char *    header_cols[MAIN_HEADER_COLS]               = { "Name", "Value" };
    char *          action;
    const char *    message                                     = (char *) 0;
    uint_fast8_t    rtc                                         = 0;
    uint16_t        raw_value;
    uint16_t        min_value;
    uint16_t        max_value;
    uint_fast8_t    auto_brightness_active;
    char            raw_buf[MAX_RAW_VALUE_LEN + 1];
    char            min_buf[MAX_RAW_VALUE_LEN + 1];
    char            max_buf[MAX_RAW_VALUE_LEN + 1];

    auto_brightness_active  = get_numvar (DISPLAY_AUTOMATIC_BRIGHTNESS_ACTIVE_NUM_VAR);

    action = http_get_param ("action");

    if (action)
    {
        if (! strcmp (action, "ldrmin"))
        {
            rpc (LDR_MIN_VALUE_RPC_VAR);
            message = "Stored minimum value";
        }
        else if (! strcmp (action, "ldrmax"))
        {
            rpc (LDR_MAX_VALUE_RPC_VAR);
            message = "Stored maximum value";
        }
        else if (! strcmp (action, "saveauto"))
        {
            if (http_get_checkbox_param ("auto"))
            {
                auto_brightness_active = 1;
            }
            else
            {
                auto_brightness_active = 0;
            }

            set_numvar (DISPLAY_AUTOMATIC_BRIGHTNESS_ACTIVE_NUM_VAR, auto_brightness_active);
        }
    }

    http_header ("WordClock LDR", (const char *) NULL, (const char *) NULL);
    http_menu ();

    table_header (header_cols, MAIN_HEADER_COLS);

    if (auto_brightness_active)
    {
        raw_value = get_numvar (LDR_RAW_VALUE_NUM_VAR);
        min_value = get_numvar (LDR_MIN_VALUE_NUM_VAR);
        max_value = get_numvar (LDR_MAX_VALUE_NUM_VAR);

        sprintf (raw_buf, "%u", raw_value);
        sprintf (min_buf, "%u", min_value);
        sprintf (max_buf, "%u", max_value);

        table_row ("LDR", raw_buf, "");
        table_row ("Min", min_buf, "");
        table_row ("Max", max_buf, "");
    }

    table_row_checkbox (thispage, "LDR", "auto", "Automatic brightness", auto_brightness_active);

    table_trailer ();

    if (auto_brightness_active)
    {
        begin_form (thispage);
        button_field ("ldrmin", "Set as minimum value");
        button_field ("ldrmax", "Set as maximum value");
        end_form ();
    }

    if (message)
    {
        http_send_FS ("<P><font color=green>");
        http_send (message);
        http_send_FS ("</font>\r\n");
    }

    http_trailer ();
    http_flush ();

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * brightness page
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
http_brightness (void)
{
    const char *    thispage = "brightness";
    const char *    header_cols[MAIN_HEADER_COLS]               = { "Brightness", "Value" };
    char            txtbuf[32];
    char            idbuf[32];
    char            valbuf[32];
    uint_fast8_t    val;
    char *          action;
    int             idx;
    uint_fast8_t    rtc                                         = 0;

    action = http_get_param ("action");

    if (action)
    {
        if (! strncmp (action, "savedim", 7))
        {
            idx = atoi (action + 7);
            sprintf (idbuf, "dim%d", idx);
            val = atoi (http_get_param (idbuf));
            set_num_array (DISPLAY_DIMMED_COLORS, idx, val);
        }
    }

    http_header ("WordClock Brightness", (const char *) NULL, (const char *) NULL);
    http_menu ();

    table_header (header_cols, MAIN_HEADER_COLS);

    for (idx = 0; idx < MAX_BRIGHTNESS; idx++)
    {
        val = get_num_array (DISPLAY_DIMMED_COLORS, idx);

        sprintf (txtbuf, "%2d", idx + 1);
        sprintf (idbuf, "dim%d", idx);
        sprintf (valbuf, "%d", val);
        table_row_slider (thispage, txtbuf, idbuf, valbuf, "0", "15");
    }

    table_trailer ();

    http_trailer ();
    http_flush ();

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * display page
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
http_display (void)
{
    const char *        thispage = "display";
    const char *        header_cols[DISPLAY_HEADER_COLS] = { "Name", "Value", "Action" };
    const char *        overlay_header_cols[OVERLAY_HEADER_COLS] = { "Overlay", "Interval", "Action" };
    static int          already_called  = 0;
    char *              action;
    const char *        message         = (const char *) 0;
    uint_fast8_t        use_rgbw        = get_numvar (DISPLAY_USE_RGBW_NUM_VAR);
    DSP_COLORS          rgbw;
    const char *        ids[4]          = { "red", "green", "blue", "white" };
    const char *        desc[4]         = { "R", "G", "B", "W" };
    char *              rgbw_buf[4];
    const char *        minval[4]       = { "0",   "0",  "0",  "0" };
    const char *        maxval[4]       = { "63", "63", "63", "63" };
    const char *        display_mode_names[MAX_DISPLAY_MODE_VARIABLES];
    int                 max_display_modes;
    int                 color_animation_mode;
    uint_fast8_t        auto_brightness_active;
    uint_fast8_t        display_brightness;
    char                brbuf[MAX_BRIGHTNESS_LEN + 1];
    char                red_buf[MAX_COLOR_VALUE_LEN + 1];
    char                green_buf[MAX_COLOR_VALUE_LEN + 1];
    char                blue_buf[MAX_COLOR_VALUE_LEN + 1];
    char                white_buf[MAX_COLOR_VALUE_LEN + 1];
    char                display_temperature_interval_str[4];
    char                display_heart_interval_str[4];
    char                display_xmas_tree_interval_str[4];
    char                display_date_interval_str[4];
    char                display_fireworks_interval_str[4];
    char                ticker_deceleration_str[4];
    int                 display_mode;
    uint_fast8_t        display_flags;
    uint_fast8_t        permanent_display_of_it_is;
    uint_fast8_t        display_temperature_interval;
    uint_fast8_t        display_heart_interval;
    uint_fast8_t        display_xmas_tree_interval;
    uint_fast8_t        display_date_interval;
    uint_fast8_t        display_fireworks_interval;
    uint_fast8_t        ticker_deceleration;
    uint_fast8_t        idx;
    uint_fast8_t        rtc = 0;

    display_mode                    = get_numvar (DISPLAY_MODE_NUM_VAR);
    max_display_modes               = get_numvar (MAX_DISPLAY_MODES_NUM_VAR);
    display_flags                   = get_numvar (DISPLAY_FLAGS_NUM_VAR);
    color_animation_mode            = get_numvar (COLOR_ANIMATION_MODE_NUM_VAR);
    display_brightness              = get_numvar (DISPLAY_BRIGHTNESS_NUM_VAR);
    auto_brightness_active          = get_numvar (DISPLAY_AUTOMATIC_BRIGHTNESS_ACTIVE_NUM_VAR);
    display_temperature_interval    = get_numvar (DISPLAY_TEMPERATURE_INTERVAL_NUM_VAR);
    display_heart_interval          = get_numvar (DISPLAY_HEART_INTERVAL_NUM_VAR);
    display_xmas_tree_interval      = get_numvar (DISPLAY_XMAS_TREE_INTERVAL_NUM_VAR);
    display_date_interval           = get_numvar (DISPLAY_DATE_INTERVAL_NUM_VAR);
    display_fireworks_interval      = get_numvar (DISPLAY_FIREWORKS_INTERVAL_NUM_VAR);
    ticker_deceleration             = get_numvar (TICKER_DECELRATION_NUM_VAR);

    get_dsp_color_var (DISPLAY_DSP_COLOR_VAR, &rgbw);

    permanent_display_of_it_is      = (display_flags & DISPLAY_FLAGS_PERMANENT_IT_IS) ? 1 : 0;

    for (idx = 0; idx < max_display_modes; idx++)
    {
        DISPLAY_MODE * dm = get_display_mode_var ((DISPLAY_MODE_VARIABLE) idx);
        display_mode_names[idx] = dm->name;
    }

    action = http_get_param ("action");

    if (action)
    {
        if (! strcmp (action, "saveitis"))
        {
            if (http_get_checkbox_param ("itis"))
            {
                permanent_display_of_it_is = 1;
                display_flags |= DISPLAY_FLAGS_PERMANENT_IT_IS;
            }
            else
            {
                permanent_display_of_it_is = 0;
                display_flags &= ~DISPLAY_FLAGS_PERMANENT_IT_IS;
            }

            set_numvar (DISPLAY_FLAGS_NUM_VAR, display_flags);
        }
        else if (! strcmp (action, "savebrightness"))
        {
            display_brightness = atoi (http_get_param ("brightness"));
            set_numvar (DISPLAY_BRIGHTNESS_NUM_VAR, display_brightness);
        }
        else if (! strcmp (action, "savecolors"))
        {
            rgbw.red     = atoi (http_get_param ("red"));
            rgbw.green   = atoi (http_get_param ("green"));
            rgbw.blue    = atoi (http_get_param ("blue"));

            if (use_rgbw)
            {
                rgbw.white = atoi (http_get_param ("white"));
            }
            else
            {
                rgbw.white = 0;
            }

            set_dsp_color_var (DISPLAY_DSP_COLOR_VAR, &rgbw, use_rgbw);
        }
        else if (! strcmp (action, "savedisplaymode"))
        {
            display_mode = atoi (http_get_param ("displaymode"));
            set_numvar (DISPLAY_MODE_NUM_VAR, display_mode);
        }
        else if (! strcmp (action, "savetinterval"))
        {
            display_temperature_interval = atoi (http_get_param ("tinterval"));
            set_numvar (DISPLAY_TEMPERATURE_INTERVAL_NUM_VAR, display_temperature_interval);
        }
        else if (! strcmp (action, "savehinterval"))
        {
            display_heart_interval = atoi (http_get_param ("hinterval"));
            set_numvar (DISPLAY_HEART_INTERVAL_NUM_VAR, display_heart_interval);
        }
        else if (! strcmp (action, "savexinterval"))
        {
            display_xmas_tree_interval = atoi (http_get_param ("xinterval"));
            set_numvar (DISPLAY_XMAS_TREE_INTERVAL_NUM_VAR, display_xmas_tree_interval);
        }
        else if (! strcmp (action, "savedinterval"))
        {
            display_date_interval = atoi (http_get_param ("dinterval"));
            set_numvar (DISPLAY_DATE_INTERVAL_NUM_VAR, display_date_interval);
        }
        else if (! strcmp (action, "savefinterval"))
        {
            display_fireworks_interval = atoi (http_get_param ("finterval"));
            set_numvar (DISPLAY_FIREWORKS_INTERVAL_NUM_VAR, display_fireworks_interval);
        }
        else if (! strcmp (action, "savetickerdec"))
        {
            ticker_deceleration = atoi (http_get_param ("tickerdec"));
            set_numvar (TICKER_DECELRATION_NUM_VAR, ticker_deceleration);
        }
        else if (! strcmp (action, "testdisplay"))
        {
            message = "Testing display...";
            rpc (TEST_DISPLAY_RPC_VAR);
        }
        else if (! strcmp (action, "poweron"))
        {
            message = "Switching power on...";
            set_numvar (DISPLAY_POWER_NUM_VAR, 1);
        }
        else if (! strcmp (action, "poweroff"))
        {
            message = "Switching power off...";
            set_numvar (DISPLAY_POWER_NUM_VAR, 0);
        }
        else if (! strcmp (action, "apoweron"))
        {
            message = "Switching ambilight power on...";
            set_numvar (DISPLAY_AMBILIGHT_POWER_NUM_VAR, 1);
        }
        else if (! strcmp (action, "apoweroff"))
        {
            message = "Switching ambilight power off...";
            set_numvar (DISPLAY_AMBILIGHT_POWER_NUM_VAR, 0);
        }
    }

    sprintf (brbuf,     "%d", display_brightness);
    sprintf (red_buf,   "%d", rgbw.red);
    sprintf (green_buf, "%d", rgbw.green);
    sprintf (blue_buf,  "%d", rgbw.blue);

    if (use_rgbw)
    {
        sprintf (white_buf, "%d", rgbw.white);
    }
    else
    {
        white_buf[0] = '0';
        white_buf[1] = '\0';
    }

    rgbw_buf[0] = red_buf;
    rgbw_buf[1] = green_buf;
    rgbw_buf[2] = blue_buf;
    rgbw_buf[3] = white_buf;

    if (display_temperature_interval)
    {
        sprintf (display_temperature_interval_str, "%d", display_temperature_interval);
    }
    else
    {
        display_temperature_interval_str[0] = '\0';
    }

    if (display_heart_interval)
    {
        sprintf (display_heart_interval_str, "%d", display_heart_interval);
    }
    else
    {
        display_heart_interval_str[0] = '\0';
    }

    if (display_xmas_tree_interval)
    {
        sprintf (display_xmas_tree_interval_str, "%d", display_xmas_tree_interval);
    }
    else
    {
        display_xmas_tree_interval_str[0] = '\0';
    }

    if (display_date_interval)
    {
        sprintf (display_date_interval_str, "%d", display_date_interval);
    }
    else
    {
        display_date_interval_str[0] = '\0';
    }

    if (display_fireworks_interval)
    {
        sprintf (display_fireworks_interval_str, "%d", display_fireworks_interval);
    }
    else
    {
        display_fireworks_interval_str[0] = '\0';
    }

    sprintf (ticker_deceleration_str, "%d", ticker_deceleration);

    http_header ("WordClock Display", (const char *) NULL, (const char *) NULL);
    http_menu ();
    table_header (header_cols, DISPLAY_HEADER_COLS);

    table_row_checkbox (thispage, "ES IST", "itis", "Permanent display of \"ES IST\"", permanent_display_of_it_is);
    table_row_select (thispage, "Display Mode", "displaymode", display_mode_names, display_mode, max_display_modes);

    if (! auto_brightness_active)
    {
        table_row_slider (thispage, "Brightness (1-15)", "brightness", brbuf, "0", "15");
    }

    if (color_animation_mode == COLOR_ANIMATION_MODE_NONE)
    {
        uint_fast8_t    n_colors;

        if (use_rgbw)
        {
            n_colors = 4;
        }
        else
        {
            n_colors = 3;
        }

        table_row_sliders (thispage, "Colors", "colors", n_colors, ids, desc, rgbw_buf, minval, maxval);
    }

    table_row_input (thispage, 3, "Ticker deceleration", "tickerdec", ticker_deceleration_str, MAX_TICKER_DECELERATION_LEN);

    table_trailer ();

    http_send_FS ("<H3>Overlays</H3>\r\n");
    table_header (overlay_header_cols, OVERLAY_HEADER_COLS);
    table_row_input (thispage, 3, "Temperature", "tinterval", display_temperature_interval_str, MAX_MINUTE_INTERVAL_LEN);
    table_row_input (thispage, 3, "Date", "dinterval", display_date_interval_str, MAX_MINUTE_INTERVAL_LEN);
    table_row_input (thispage, 3, "Heart", "hinterval", display_heart_interval_str, MAX_MINUTE_INTERVAL_LEN);
    table_row_input (thispage, 3, "XMas tree", "xinterval", display_xmas_tree_interval_str, MAX_MINUTE_INTERVAL_LEN);
    table_row_input (thispage, 3, "Fireworks", "finterval", display_fireworks_interval_str, MAX_MINUTE_INTERVAL_LEN);
    table_trailer ();

    begin_form (thispage);
    button_field ("poweron", "Power On");
    button_field ("poweroff", "Power Off");
    button_field ("apoweron", "Ambilight Power On");
    button_field ("apoweroff", "Ambilight Power Off");
    button_field ("testdisplay", "Test display");
    end_form ();

    if (message)
    {
        http_send_FS ("<P><font color=green>");
        http_send (message);
        http_send_FS ("</font>\r\n");
    }

    http_trailer ();
    http_flush ();

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * animations page
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
http_animations (void)
{
    const char *        thispage = "animations";
    const char *        header_cols[ANIMATION_HEADER_COLS] = { "Name", "Value", "Action" };
    const char *        dec_cols[ANIMATION_DECELERATION_HEADER_COLS] = { "Name", "Deceleration", "Default", "Favourite", "Action" };
    const char *        color_dec_cols[COLOR_ANIMATION_DECELERATION_HEADER_COLS] = { "Name", "Deceleration", "Default", "Action" };
    static int          already_called  = 0;
    static const char * animation_mode_names[MAX_DISPLAY_ANIMATION_VARIABLES];
    static const char * color_animation_mode_names[MAX_COLOR_ANIMATION_VARIABLES];
    const char *        message         = (const char *) 0;
    char *              action;
    unsigned int        animation_mode;
    int                 color_animation_mode;
    char                animidbuf[8];
    char                decidbuf[8];
    char                defaultidbuf[8];
    char                favidbuf[8];
    char                color_animidbuf[8];
    char                color_decidbuf[8];
    char                color_defaultidbuf[8];
    char                color_decbuf[MAX_COLOR_ANIMATION_DECELERATION_LEN + 1];

    char                decbuf[MAX_ANIMATION_DECELERATION_LEN + 1];
    uint_fast8_t        idx;
    uint_fast8_t        rtc = 0;

    if (! already_called)
    {
        for (idx = 0; idx < MAX_DISPLAY_ANIMATION_VARIABLES; idx++)
        {
            DISPLAY_ANIMATION * da = get_display_animation_var ((DISPLAY_ANIMATION_VARIABLE) idx);
            animation_mode_names[idx] = da->name;
        }

        for (idx = 0; idx < MAX_COLOR_ANIMATION_VARIABLES; idx++)
        {
            COLOR_ANIMATION * ca = get_color_animation_var ((COLOR_ANIMATION_VARIABLE) idx);
            color_animation_mode_names[idx] = ca->name;
        }

        already_called = 1;
    }

    animation_mode          = get_numvar (ANIMATION_MODE_NUM_VAR);
    color_animation_mode    = get_numvar (COLOR_ANIMATION_MODE_NUM_VAR);

    action = http_get_param ("action");

    if (action)
    {
        if (! strcmp (action, "saveanimation"))
        {
            animation_mode = atoi (http_get_param ("animation"));
            set_numvar (ANIMATION_MODE_NUM_VAR, animation_mode);
        }
        else if (! strcmp (action, "savecoloranimation"))
        {
            color_animation_mode = atoi (http_get_param ("coloranimation"));
            set_numvar (COLOR_ANIMATION_MODE_NUM_VAR, color_animation_mode);
        }

        else if (! strncmp (action, "def", 3))
        {
            uint_fast8_t    animation_idx;

            animation_idx = atoi (action + 3);

            if (animation_idx < MAX_DISPLAY_ANIMATION_VARIABLES)
            {
                DISPLAY_ANIMATION * da = get_display_animation_var ((DISPLAY_ANIMATION_VARIABLE) animation_idx);
                set_display_animation_deceleration ((DISPLAY_ANIMATION_VARIABLE) animation_idx, da->default_deceleration);
            }
        }
        else if (! strncmp (action, "savean", 6))
        {
            uint_fast8_t    animation_idx;
            uint_fast8_t    animation_deceleration;
            uint_fast8_t    animation_favourite;
            animation_idx = atoi (action + 6);

            if (animation_idx < MAX_DISPLAY_ANIMATION_VARIABLES)
            {
                DISPLAY_ANIMATION * da = get_display_animation_var ((DISPLAY_ANIMATION_VARIABLE) animation_idx);
                sprintf (decidbuf, "sp%d", animation_idx);
                sprintf (favidbuf, "fav%d", animation_idx);

                animation_deceleration = atoi (http_get_param (decidbuf));

                if (animation_deceleration >= ANIMATION_MIN_DECELERATION && animation_deceleration <= ANIMATION_MAX_DECELERATION)
                {
                    set_display_animation_deceleration ((DISPLAY_ANIMATION_VARIABLE) animation_idx, animation_deceleration);
                }

                animation_favourite = http_get_checkbox_param (favidbuf);

                if (animation_favourite)
                {
                    set_display_animation_flags ((DISPLAY_ANIMATION_VARIABLE) animation_idx, da->flags | ANIMATION_FLAG_FAVOURITE);
                }
                else
                {
                    set_display_animation_flags ((DISPLAY_ANIMATION_VARIABLE) animation_idx, da->flags & ~ANIMATION_FLAG_FAVOURITE);
                }
            }
        }
        else if (! strncmp (action, "savecan", 7))
        {
            uint_fast8_t    color_animation_idx;
            uint_fast8_t    color_animation_deceleration;

            color_animation_idx = atoi (action + 7);

            if (color_animation_idx < MAX_COLOR_ANIMATION_VARIABLES)
            {
                sprintf (color_decidbuf, "csp%d", color_animation_idx);

                color_animation_deceleration = atoi (http_get_param (color_decidbuf));

                if (color_animation_deceleration <= COLOR_ANIMATION_MAX_DECELERATION)
                {
                    set_color_animation_deceleration ((COLOR_ANIMATION_VARIABLE) color_animation_idx, color_animation_deceleration);
                }
            }
        }
        else if (! strncmp (action, "cdef", 4))
        {
            COLOR_ANIMATION_VARIABLE    color_animation_idx;

            color_animation_idx = (COLOR_ANIMATION_VARIABLE) atoi (action + 4);

            if (color_animation_idx < MAX_COLOR_ANIMATION_VARIABLES)
            {
                COLOR_ANIMATION * ca = get_color_animation_var (color_animation_idx);
                set_color_animation_deceleration (color_animation_idx, ca->default_deceleration);
            }
        }
    }

    http_header ("WordClock Animations", (const char *) NULL, (const char *) NULL);
    http_menu ();

    table_header (header_cols, ANIMATION_HEADER_COLS);
    table_row_select (thispage, "Animation", "animation", animation_mode_names, animation_mode, MAX_DISPLAY_ANIMATION_VARIABLES);
    table_row_select (thispage, "Color Animation", "coloranimation", color_animation_mode_names, color_animation_mode, MAX_COLOR_ANIMATION_VARIABLES);
    table_trailer ();

    table_header (dec_cols, ANIMATION_DECELERATION_HEADER_COLS);

    for (idx = 0; idx < MAX_DISPLAY_ANIMATION_VARIABLES; idx++)
    {
        DISPLAY_ANIMATION * da = get_display_animation_var ((DISPLAY_ANIMATION_VARIABLE) idx);

        if (da->flags & ANIMATION_FLAG_CONFIGURABLE)
        {
            sprintf (animidbuf, "an%d", idx);
            sprintf (decidbuf, "sp%d", idx);
            sprintf (defaultidbuf, "def%d", idx);
            sprintf (favidbuf, "fav%d", idx);
            sprintf (decbuf, "%d", da->deceleration);

            begin_table_row_form (thispage);
            text_column (da->name);
            slider_column (decidbuf, decbuf, "1", "15");
            button_column (defaultidbuf, "Default");
            checkbox_column (favidbuf, "", (da->flags & ANIMATION_FLAG_FAVOURITE) ? 1 : 0);
            save_column (animidbuf);
            end_table_row_form ();
        }
    }

    table_trailer ();

    table_header (color_dec_cols, COLOR_ANIMATION_DECELERATION_HEADER_COLS);

    for (idx = 0; idx < MAX_COLOR_ANIMATION_VARIABLES; idx++)
    {
        COLOR_ANIMATION * color_animation;
        color_animation = get_color_animation_var ((COLOR_ANIMATION_VARIABLE) idx);

        if (color_animation->flags & COLOR_ANIMATION_FLAG_CONFIGURABLE)
        {
            sprintf (color_animidbuf, "can%d", idx);
            sprintf (color_decidbuf, "csp%d", idx);
            sprintf (color_defaultidbuf, "cdef%d", idx);
            sprintf (color_decbuf, "%d", color_animation->deceleration);

            begin_table_row_form (thispage);
            text_column (color_animation->name);
            slider_column (color_decidbuf, color_decbuf, "0", "15");
            button_column (color_defaultidbuf, "Default");
            save_column (color_animidbuf);
            end_table_row_form ();
        }
    }

    table_trailer ();


    begin_form (thispage);
    end_form ();

    if (message)
    {
        http_send_FS ("<P><font color=green>");
        http_send (message);
        http_send_FS ("</font>\r\n");
    }

    http_trailer ();
    http_flush ();

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * ambilight page
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
http_ambilight (void)
{
    const char *        thispage = "ambilight";
    const char *        header_cols[DISPLAY_HEADER_COLS] = { "Name", "Value", "Action" };
    const char *        ambimode_dec_cols[AMBILIGHT_MODE_DECELERATION_HEADER_COLS] = { "Name", "Deceleration", "Default", "Action" };
    static const char * ambilight_mode_names[MAX_AMBILIGHT_MODE_VARIABLES];
    static uint_fast8_t already_called;
    char *              action;
    const char *        message         = (const char *) 0;
    uint_fast8_t        use_rgbw        = get_numvar (DISPLAY_USE_RGBW_NUM_VAR);
    DSP_COLORS          rgbw;
    const char *        ids[4]          = { "red", "green", "blue", "white" };
    const char *        desc[4]         = { "R", "G", "B", "W" };
    char *              rgbw_buf[4];
    const char *        minval[4]       = { "0",   "0",  "0",  "0" };
    const char *        maxval[4]       = { "63", "63", "63", "63" };
    char                rbdecbuf[MAX_RAINBOW_DECELERATION_LEN + 1];
    AMBILIGHT_MODE *     am;
    char                ambilight_leds_buf[5];
    char                ambilight_offset_buf[5];
    uint_fast8_t        ambilight_brightness;
    uint_fast8_t        auto_brightness_active;
    char                brbuf[MAX_BRIGHTNESS_LEN + 1];
    char                red_buf[MAX_COLOR_VALUE_LEN + 1];
    char                green_buf[MAX_COLOR_VALUE_LEN + 1];
    char                blue_buf[MAX_COLOR_VALUE_LEN + 1];
    char                white_buf[MAX_COLOR_VALUE_LEN + 1];
    char                ambimode_idbuf[8];
    char                ambimode_decidbuf[8];
    char                ambimode_defaultidbuf[8];
    char                ambimode_decbuf[MAX_AMBILIGHT_MODE_DECELERATION_LEN + 1];
    int                 ambilight_mode;
    int                 ambilight_leds;
    int                 ambilight_offset;
    int                    ambilight_markers = 0;
    uint_fast8_t        display_flags;
    uint_fast8_t        sync_ambilight;
    uint_fast8_t        idx;
    uint_fast8_t        rtc             = 0;

    if (! already_called)
    {
        for (idx = 0; idx < MAX_AMBILIGHT_MODE_VARIABLES; idx++)
        {
            AMBILIGHT_MODE * am = get_ambilight_mode_var ((AMBILIGHT_MODE_VARIABLE) idx);
            ambilight_mode_names[idx] = am->name;
        }
        already_called = 1;
    }

    ambilight_mode          = get_numvar (AMBILIGHT_MODE_NUM_VAR);
    ambilight_leds          = get_numvar (AMBILIGHT_LEDS_NUM_VAR);
    ambilight_offset        = get_numvar (AMBILIGHT_OFFSET_NUM_VAR);
    display_flags           = get_numvar (DISPLAY_FLAGS_NUM_VAR);
    sync_ambilight          = (display_flags & DISPLAY_FLAGS_SYNC_AMBILIGHT) ? 1 : 0;
    ambilight_brightness    = get_numvar (AMBILIGHT_BRIGHTNESS_NUM_VAR);
    auto_brightness_active  = get_numvar (DISPLAY_AUTOMATIC_BRIGHTNESS_ACTIVE_NUM_VAR);
    get_dsp_color_var (AMBILIGHT_DSP_COLOR_VAR, &rgbw);

    action = http_get_param ("action");

    if (action)
    {
        if (! strcmp (action, "savesyncambi"))
        {
            if (http_get_checkbox_param ("syncambi"))
            {
                sync_ambilight = 1;
                display_flags |= DISPLAY_FLAGS_SYNC_AMBILIGHT;
            }
            else
            {
                sync_ambilight = 0;
                display_flags &= ~DISPLAY_FLAGS_SYNC_AMBILIGHT;
            }

            set_numvar (DISPLAY_FLAGS_NUM_VAR, display_flags);
        }
        else if (! strcmp (action, "savebrightness"))
        {
            ambilight_brightness = atoi (http_get_param ("brightness"));
            set_numvar (AMBILIGHT_BRIGHTNESS_NUM_VAR, ambilight_brightness);
        }
        else if (! strcmp (action, "savecolors"))
        {
            rgbw.red     = atoi (http_get_param ("red"));
            rgbw.green   = atoi (http_get_param ("green"));
            rgbw.blue    = atoi (http_get_param ("blue"));

            if (use_rgbw)
            {
                rgbw.white = atoi (http_get_param ("white"));
            }
            else
            {
                rgbw.white = 0;
            }

            set_dsp_color_var (AMBILIGHT_DSP_COLOR_VAR, &rgbw, use_rgbw);
        }
        else if (! strcmp (action, "saveambimode"))
        {
            ambilight_mode = atoi (http_get_param ("ambimode"));
            set_numvar (AMBILIGHT_MODE_NUM_VAR, ambilight_mode);
        }
        else if (! strcmp (action, "saveambileds"))
        {
            ambilight_leds = atoi (http_get_param ("ambileds"));
            set_numvar (AMBILIGHT_LEDS_NUM_VAR, ambilight_leds);
        }
        else if (! strcmp (action, "saveambioffset"))
        {
            ambilight_offset = atoi (http_get_param ("ambioffset"));
            set_numvar (AMBILIGHT_OFFSET_NUM_VAR, ambilight_offset);
        }
        else if (! strncmp (action, "saveaan", 7))
        {
            uint_fast8_t    ambilight_mode_idx;
            uint_fast8_t    ambilight_mode_deceleration;

            ambilight_mode_idx = atoi (action + 7);

            if (ambilight_mode_idx < MAX_AMBILIGHT_MODE_VARIABLES)
            {
                sprintf (ambimode_decidbuf, "asp%d", ambilight_mode_idx);

                ambilight_mode_deceleration = atoi (http_get_param (ambimode_decidbuf));

                if (ambilight_mode_deceleration <= AMBILIGHT_MODE_MAX_DECELERATION)
                {
                    set_ambilight_mode_deceleration ((AMBILIGHT_MODE_VARIABLE) ambilight_mode_idx, ambilight_mode_deceleration);
                }
            }
        }
        else if (! strncmp (action, "adef", 4))
        {
            uint_fast8_t    ambilight_mode_idx;

            ambilight_mode_idx = atoi (action + 4);

            if (ambilight_mode_idx < MAX_AMBILIGHT_MODE_VARIABLES)
            {
                am = get_ambilight_mode_var ((AMBILIGHT_MODE_VARIABLE) ambilight_mode_idx);
                set_ambilight_mode_deceleration ((AMBILIGHT_MODE_VARIABLE) ambilight_mode_idx, am->default_deceleration);
            }
        }
        else if (! strcmp (action, "savemarkers"))
        {
            am = get_ambilight_mode_var (CLOCK_AMBILIGHT_MODE_VAR);
            int flags = am->flags;

            if (http_get_checkbox_param ("markers"))
            {
                flags |= AMBILIGHT_FLAG_SECONDS_MARKER;
            }
            else
            {
                flags &= ~AMBILIGHT_FLAG_SECONDS_MARKER;
            }

            set_ambilight_mode_flags (CLOCK_AMBILIGHT_MODE_VAR, flags);
        }
    }

    am = get_ambilight_mode_var (CLOCK_AMBILIGHT_MODE_VAR);

    if (am->flags & AMBILIGHT_FLAG_SECONDS_MARKER)
    {
        ambilight_markers = 1;
    }

    am = get_ambilight_mode_var (RAINBOW_AMBILIGHT_MODE_VAR);

    sprintf (rbdecbuf,              "%d", am->deceleration);

    sprintf (ambilight_leds_buf,    "%d", ambilight_leds);
    sprintf (ambilight_offset_buf,  "%d", ambilight_offset);

    sprintf (brbuf,                 "%d", ambilight_brightness);

    sprintf (red_buf,               "%d", rgbw.red);
    sprintf (green_buf,             "%d", rgbw.green);
    sprintf (blue_buf,              "%d", rgbw.blue);

    if (use_rgbw)
    {
        sprintf (white_buf, "%d", rgbw.white);
    }
    else
    {
        white_buf[0] = '0';
        white_buf[1] = '\0';
    }

    rgbw_buf[0] = red_buf;
    rgbw_buf[1] = green_buf;
    rgbw_buf[2] = blue_buf;
    rgbw_buf[3] = white_buf;

    http_header ("WordClock Ambilight", (const char *) NULL, (const char *) NULL);
    http_menu ();
    table_header (header_cols, DISPLAY_HEADER_COLS);

    table_row_input (thispage, 3, "#LEDs", "ambileds", ambilight_leds_buf, 3);
    table_row_input (thispage, 3, "Offset of second = 0", "ambioffset", ambilight_offset_buf, 3);

    table_row_checkbox (thispage, "Ambilight", "syncambi", "Use display colors", sync_ambilight);

    if (! sync_ambilight)
    {
        uint_fast8_t    n_colors;

        if (! auto_brightness_active)
        {
            table_row_slider (thispage, "Brightness (1-15)", "brightness", brbuf, "0", "15");
        }

        if (use_rgbw)
        {
            n_colors = 4;
        }
        else
        {
            n_colors = 3;
        }

        table_row_sliders (thispage, "Colors", "colors", n_colors, ids, desc, rgbw_buf, minval, maxval);
    }

    table_row_select (thispage, "Ambilight Mode", "ambimode", ambilight_mode_names, ambilight_mode, MAX_AMBILIGHT_MODE_VARIABLES);
    table_trailer ();

    table_header (ambimode_dec_cols, AMBILIGHT_MODE_DECELERATION_HEADER_COLS);

    for (idx = 0; idx < MAX_AMBILIGHT_MODE_VARIABLES; idx++)
    {
        AMBILIGHT_MODE * am = get_ambilight_mode_var ((AMBILIGHT_MODE_VARIABLE) idx);

        if (am->flags & AMBILIGHT_FLAG_CONFIGURABLE)
        {
            sprintf (ambimode_idbuf, "aan%d", idx);
            sprintf (ambimode_decidbuf, "asp%d", idx);
            sprintf (ambimode_defaultidbuf, "adef%d", idx);
            sprintf (ambimode_decbuf, "%d", am->deceleration);

            begin_table_row_form (thispage);
            text_column (am->name);
            slider_column (ambimode_decidbuf, ambimode_decbuf, "0", "15");
            button_column (ambimode_defaultidbuf, "Default");
            save_column (ambimode_idbuf);
            end_table_row_form ();
        }
    }

    table_row_checkbox (thispage, "CLOCK", "markers", "Enable 5-second markers", ambilight_markers);

    table_trailer ();
    begin_form (thispage);
    end_form ();

    if (message)
    {
        http_send_FS ("<P><font color=green>");
        http_send (message);
        http_send_FS ("</font>\r\n");
    }

    http_trailer ();
    http_flush ();

    return rtc;
}

static void
table_row_timers (const char * page, uint_fast8_t is_ambilight, int idx)
{
    char            id[8];
    char            idx_buf[3];
    char            hour_buf[3];
    char            minute_buf[3];
    char            act_id[8];
    char            on_id[8];
    char            hour_id[8];
    char            min_id[8];
    char            day_id[8];
    NIGHT_TIME *    nt;

    nt = get_night_time_var (is_ambilight, (NIGHT_TIME_VARIABLE) idx);

    if (nt)
    {
        sprintf (hour_buf,   "%02d", nt->minutes / 60);
        sprintf (minute_buf, "%02d", nt->minutes % 60);

        begin_table_row_form (page);

        sprintf (idx_buf, "%d",   idx);
        sprintf (id,      "id%d", idx);
        sprintf (act_id,  "a%d",  idx);
        sprintf (on_id,   "o%d",  idx);
        sprintf (hour_id, "h%d",  idx);
        sprintf (min_id,  "m%d",  idx);

        text_column (idx_buf);

        checkbox_column (act_id, "", (nt->flags & NIGHT_TIME_FLAG_ACTIVE) ? 1 : 0);
        checkbox_column (on_id, "", (nt->flags & NIGHT_TIME_FLAG_SWITCH_ON) ? 1 : 0);

        sprintf (day_id, "f%d", idx);
        select_column (day_id, wdays_en, (nt->flags & NIGHT_TIME_FROM_DAY_MASK) >> 3, 7);

        sprintf (day_id, "t%d", idx);
        select_column (day_id, wdays_en, (nt->flags & NIGHT_TIME_TO_DAY_MASK), 7);

        input_column (hour_id, "",  hour_buf, 2, 2);
        input_column (min_id, "",  minute_buf, 2, 2);

        save_column (id);
        end_table_row_form ();
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * timers page
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
http_timers (uint_fast8_t is_ambilight)
{
    const char *        header_cols[TIMERS_HEADER_COLS] = { "Slot", "Active", "On", "From", "To", "Hour", "Min", "Action" };
    const char *        thispage;
    char *              action;
    char *              title;
    uint_fast8_t        idx;
    uint_fast8_t        rtc = 0;

    action = http_get_param ("action");

    if (action)
    {
        if (! strncmp (action, "saveid", 6))
        {
            char id[16];
            uint_fast8_t idx = atoi (action + 6);

            if (idx < MAX_NIGHT_TIME_VARIABLES)
            {
                NIGHT_TIME *    nt;
                uint_fast8_t    from_day;
                uint_fast8_t    to_day;
                uint_fast16_t   minutes;
                uint_fast8_t    flags;

                nt = get_night_time_var (is_ambilight, (NIGHT_TIME_VARIABLE) idx);

                flags = nt->flags;

                if (http_get_checkbox_param_by_idx ("a", idx))
                {
                    flags |= NIGHT_TIME_FLAG_ACTIVE;
                }
                else
                {
                    flags &= ~NIGHT_TIME_FLAG_ACTIVE;
                }

                if (http_get_checkbox_param_by_idx ("o", idx))
                {
                    flags |= NIGHT_TIME_FLAG_SWITCH_ON;
                }
                else
                {
                    flags &= ~NIGHT_TIME_FLAG_SWITCH_ON;
                }

                sprintf (id, "f%d", idx);
                from_day = atoi (http_get_param (id));
                sprintf (id, "t%d", idx);
                to_day = atoi (http_get_param (id));

                flags &= ~(NIGHT_TIME_FROM_DAY_MASK | NIGHT_TIME_TO_DAY_MASK);
                flags |= NIGHT_TIME_FROM_DAY_MASK & (from_day << 3);
                flags |= NIGHT_TIME_TO_DAY_MASK & (to_day);

                minutes = atoi (http_get_param_by_idx ("h", idx)) * 60 + atoi (http_get_param_by_idx ("m", idx));

                set_night_time_var (is_ambilight, (NIGHT_TIME_VARIABLE) idx, minutes, flags);
            }
        }
    }

    if (is_ambilight)
    {
        title = "WordClock Ambilight Timers";
        thispage = "atimers";
    }
    else
    {
        title = "WordClock Timers";
        thispage = "timers";
    }

    http_header (title, (const char *) NULL, (const char *) NULL);
    http_menu ();
    table_header (header_cols, TIMERS_HEADER_COLS);

    for (idx = 0; idx < MAX_NIGHT_TIME_VARIABLES; idx++)
    {
        table_row_timers (thispage, is_ambilight, idx);
    }

    table_trailer ();

    http_trailer ();
    http_flush ();

    return rtc;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * HTTP update
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint_fast8_t
http_update (void)
{
    const char *        thispage = "update";
    const char *        header_cols[MAIN_HEADER_COLS]               = { "Name", "Value" };
    int                 i;
    char *              action;
    char                flash_stm32_filename[MAX_UPDATE_FILENAME_LEN];
    char                stm32_default_filename[MAX_UPDATE_FILENAME_LEN];
    int                 do_update = 0;
    int                 do_reset = 0;
    STR_VAR *           sv;
    char *              version;
    char *              update_host;
    char *              update_path;
    int                 hardware_configuration;
    char                flashsizebuf[32];
    uint32_t            flashsize;
    uint_fast8_t        rtc = 0;

    sv = get_strvar (UPDATE_HOST_VAR);
    update_host = sv->str;

    if (! update_host[0])
    {
        set_strvar (UPDATE_HOST_VAR, DEFAULT_UPDATE_HOST);
    }

    sv = get_strvar (UPDATE_PATH_VAR);
    update_path = sv->str;

    if (! update_path[0])
    {
        set_strvar (UPDATE_PATH_VAR, DEFAULT_UPDATE_PATH);
    }

    flash_stm32_filename[0] = '\0';

    sv              = get_strvar (VERSION_STR_VAR);
    version         = sv->str;

    hardware_configuration = get_numvar (HARDWARE_CONFIGURATION_NUM_VAR);

    flashsize = ESP.getFlashChipRealSize ();
    sprintf (flashsizebuf, "%ld", flashsize);

    action = http_get_param ("action");

    if (action)
    {
        if (! strcmp (action, "update"))
        {
            do_update = 1;
        }
        else if (! strcmp (action, "saveuphost"))
        {
            set_strvar (UPDATE_HOST_VAR, http_get_param ("uphost"));
        }
        else if (! strcmp (action, "saveuppath"))
        {
            set_strvar (UPDATE_PATH_VAR, http_get_param ("uppath"));
        }
        else if (! strcmp (action, "flash"))
        {
            strncpy (flash_stm32_filename, http_get_param ("stm32_filenames"), 63);
            flash_stm32_filename[63] = '\0';
        }
        else if (! strcmp (action, "reset"))
        {
            do_reset = 1;
        }
    }

    if (do_update)
    {
        http_header ("WordClock Update", "40", "/update");
    }
    else if (do_reset)
    {
        http_header ("WordClock Update", "20", "/");
    }
    else
    {
        http_header ("WordClock Update", (const char *) NULL, (const char *) NULL);
    }

    http_menu ();

    if (flashsize >= 4194304)
    {
        if (flash_stm32_filename[0])
        {
            int i;

            for (i = 0; i < 100; i++)
            {                                                   // send 1000 spaces to force Browser to begin rendering
                http_send ("          "); 
            }

            http_send_FS ("\r\n<P><B>Updating STM32 firmware...<BR>\r\n");
            http_flush ();

            Serial.print ("Flash STM32: http://");
            Serial.print (update_host);
            Serial.print ("/");
            Serial.print (update_path);
            Serial.print ("/");
            Serial.println (flash_stm32_filename);

            delay (200);
            stm32_flash_from_server (update_host, update_path, flash_stm32_filename, 1);
            delay (200);
            Serial.println ("End of flashmode\r\n");
            Serial.flush ();

            http_send_FS ("Done. <font color=red>Please Reset your STM32 now!</font></B><BR>\r\n"
                          "<form method=\"GET\" action=\"/update\">\r\n"
                          "<button type=\"submit\" name=\"action\" value=\"reset\">Reset STM32</button>"
                          "</form>"
                          "<P>\r\n");
            http_trailer ();
            http_flush ();
        }
        else if (do_reset)
        {
            http_send_FS ("<P><B>Resetting STM32, reconnecting in 15 seconds ...</B><BR>\r\n");
            http_trailer ();
            http_flush ();
            delay (200);
            stm32_reset ();
        }
        else
        {
            if (do_update)
            {
                char url[MAX_UPDATE_HOST_LEN + MAX_UPDATE_PATH_LEN + MAX_UPDATE_FILENAME_LEN + 3];
    
                sprintf (url, "http://%s/%s/%s", update_host, update_path, ESP_WORDCLOCK_BIN);
    
                http_send_FS ("<P><B>Updating ESP firmware, reconnecting in 40 seconds ...</B></BR>\r\n");
    
                http_trailer ();
                http_flush ();
                http_client.stop();
                delay (200);
    
                t_httpUpdate_return ret = ESPhttpUpdate.update (url);
    
                switch(ret)
                {
                    case HTTP_UPDATE_FAILED:
                        Serial.println("HTTP update: failed");
                        break;
    
                    case HTTP_UPDATE_NO_UPDATES:
                        Serial.println("HTTP update: no updates");
                        break;
    
                    case HTTP_UPDATE_OK:
                        Serial.println("HTTP update: ok");    // will be never called, because we are rebooting
                        break;
                }
            }
            else
            {
                char    new_esp_version[16];
                char    new_wc_version[16];
                int     len;
    
                new_esp_version[0] = '\0';
                new_wc_version[0] = '\0';
    
                len = httpclient (update_host, update_path, ESP_WORDCLOCK_TXT);
    
                if (len > 0)
                {
                    int ch;
                    int l = 0;
    
                    while (len > 0)
                    {
                        ch = httpclient_read (&len);
    
                        if (ch != '\r' && ch != '\n' && l < 16 - 1)
                        {
                            new_esp_version[l++] = ch;
                        }
                    }
    
                    httpclient_stop ();
                    new_esp_version[l] = '\0';
                }
                else
                {
                   Serial.print("Error in HTTP request: ");
                   Serial.println(ESP_WORDCLOCK_TXT);
                }
    
                len = httpclient (update_host, update_path, RELEASENOTE_HTML);
    
                if (len > 0)
                {
                    char linebuf[128];
                    int l = 0;
                    int ch;
    
                    while (len > 0)
                    {
                        ch = httpclient_read (&len);
    
                        if (l < 128 - 1 && ch > 0)
                        {
                            linebuf[l++] = ch;
                        }
    
                        if (ch == '\n')
                        {
                            linebuf[l] = '\0';
                            l = 0;
                            http_send (linebuf);
                        }
                    }
    
                    httpclient_stop ();
                }
                else
                {
                   Serial.print("Error in HTTP request: ");
                   Serial.println(RELEASENOTE_HTML);
                }
    
                len = httpclient (update_host, update_path, WC_TXT);
    
                if (len > 0)
                {
                    int ch;
                    int l = 0;
    
                    while (len > 0)
                    {
                        ch = httpclient_read (&len);
    
                        if (ch != '\r' && ch != '\n' && l < 16 - 1)
                        {
                            new_wc_version[l++] = ch;
                        }
                    }
    
                    httpclient_stop ();
                    new_wc_version[l] = '\0';
                }
                else
                {
                   Serial.print("Error in HTTP request: ");
                   Serial.println(WC_TXT);
                }
    
                table_header (header_cols, UPDATE_HEADER_COLS);
                table_row_input (thispage, 3, "Update Host", "uphost", update_host, MAX_UPDATE_HOST_LEN);
                table_row_input (thispage, 3, "Update Path", "uppath", update_path, MAX_UPDATE_PATH_LEN);
                table_trailer ();
    
                http_send_FS ("<table><tr><td>ESP8266 flash size</td><td>");
                http_send (flashsizebuf);
                http_send_FS ("</td></tr><tr><td>ESP firmware version</td><td>");
                http_send (ESP_VERSION);
                http_send_FS ("</td></tr>\r\n"
                              "<tr><td>ESP firmware available</td><td>");
                http_send (new_esp_version);
                http_send_FS ("</td></tr></table>\r\n");
    
                http_send_FS ("<form method=\"GET\" action=\"/update\">\r\n"
                              "<button type=\"submit\" name=\"action\" value=\"update\">Update ESP Firmware</button>"
                              "</form>"
                              "<P>"
                              "<table>"
                              "<tr>"
                              "<td>WordClock firmware version</td>"
                              "<td>");
                http_send (version);
                http_send_FS ("</td>"
                              "</tr>"
                              "<tr>"
                              "<td>WordClock firmware available</td>"
                              "<td>");
                http_send (new_wc_version);
                http_send_FS ("</td>"
                              "</tr>"
                              "</table>"
                              "<form method=\"GET\" action=\"/update\">"
                              "<select id=\"stm32_filenames\" name=\"stm32_filenames\">");
    
                len = httpclient (update_host, update_path, WC_LIST_TXT);
    
                if (len > 0)
                {
                    char fname[MAX_UPDATE_FILENAME_LEN];
                    char * filter = NULL;
                    int ch;
                    int l = 0;
    
                    stm32_default_filename[0] = '\0';

                    if (hardware_configuration != 0xFFFF)
                    {
                        switch (hardware_configuration & HW_WC_MASK)
                        {
                            case HW_WC_24H:                 strcat (stm32_default_filename, "wc24h-");          break;
                            case HW_WC_12H:                 strcat (stm32_default_filename, "wc12h-");          break;
                        }
        
                        switch (hardware_configuration & HW_STM32_MASK)
                        {
                            case HW_STM32_F103:             strcat (stm32_default_filename, "stm32f103-");      filter = "stm32f103"; break;
                            case HW_STM32_F401:             strcat (stm32_default_filename, "stm32f401-");      filter = "stm32f401"; break;
                            case HW_STM32_F411:             strcat (stm32_default_filename, "stm32f411-");      filter = "stm32f411"; break;
                        }
        
                        switch (hardware_configuration & HW_LED_MASK)
                        {
                            case HW_LED_WS2812_GRB_LED:     strcat (stm32_default_filename, "ws2812-grb.hex");  break;
                            case HW_LED_WS2812_RGB_LED:     strcat (stm32_default_filename, "ws2812-rgb.hex");  break;
                            case HW_LED_APA102_RGB_LED:     strcat (stm32_default_filename, "apa102-grb.hex");  break;
                            case HW_LED_SK6812_RGB_LED:     strcat (stm32_default_filename, "sk6812-rgb.hex");  break;
                            case HW_LED_SK6812_RGBW_LED:    strcat (stm32_default_filename, "sk6812-rgbw.hex"); break;
                        }
                    }

                    if (hardware_configuration == 0xFFFF)
                    {
                        http_send_FS ("<option value=\"\" selected>&lt;unknown&gt;</option>\r\n");
                    }

                    while (len > 0)
                    {
                        ch = httpclient_read (&len);
    
                        if (ch != '\r' && ch != '\n' && ch != ' ' && ch != '\t' && l < MAX_UPDATE_FILENAME_LEN - 1)
                        {
                            fname[l++] = ch;
                        }
                        else if (ch == '\n')
                        {
                            int show_option;

                            fname[l] = '\0';
                            l = 0;

                            show_option = 1;

                            if (filter)
                            {
                                if (strncmp (fname + 6, filter, strlen (filter)) != 0)
                                {
                                    show_option = 0;
                                }
                            }

                            if (show_option)
                            {
                                http_send_FS ("<option value=\"");
                                http_send (fname);
        
                                if (! strcmp (fname, stm32_default_filename))
                                {
                                    http_send_FS ("\" selected>");
                                }
                                else
                                {
                                    http_send_FS ("\">");
                                }
        
                                http_send (fname);
                                http_send_FS ("</option>\r\n");
                            }
                        }
                    }
    
                    httpclient_stop ();
                    http_flush ();
                }
                else
                {
                   Serial.print("Error in HTTP request: ");
                   Serial.println(WC_LIST_TXT);
                }
                http_send_FS ("</select>\r\n");
    
                http_send_FS ("<button type=\"submit\" name=\"action\" value=\"flash\">Flash STM32</button>"
                              "</form>"
                              "<P>\r\n");
            }

            http_trailer ();
            http_flush ();
        }
    }
    else
    {
        http_send_FS ("<font color=red><B>Flash size of ESP8266 is too small for Update over OTA</B></font><BR>\r\n");
        http_trailer ();
        http_flush ();
    }

    return rtc;
}

static bool
read_line (String& line)
{
    int c;
    ulong start_millis = 0;
    ulong ellapsed_millis = 0;

    line = "";

    do
    {
        start_millis = millis();
        c = http_client.read();
        ellapsed_millis = (millis() - start_millis);

        if (c >= 0 && c != '\n' && c != '\r')
        {
            line += (char) c;
        }
    } while (c >= 0 && c != '\r' && (ellapsed_millis < 1000));

    yield();
    return (c >= 0) && (ellapsed_millis < 1000);
}

String
sanitize_xml_string(const String& str)
{
    String result;
    for (int i = 0; i < str.length(); i++)
    {
        switch (str[i])
        {
        case '&':
            result += "&amp;";
            break;
        case '<':
            result += "&lt;";
            break;
        case '>':
            result += "&gt;";
            break;
        default:
            result += str[i];
            break;
        }
    }
    return result;
}

static uint_fast8_t
http_get_settings(const char* param)
{
    char buff[255];
    http_send(FS("HTTP/1.0 200 OK\r\n\r\n"));
    http_send(FS("<settings>"));
    // num vars
    for ( int i = 0; i < MAX_NUM_VARIABLES; i++)
    {
        sprintf(buff, FS("<numvar idx=\"%d\" value=\"%d\" />"), i, numvars[i]);
        http_send(buff);
    }
    // string vars
    for (int i = 0; i < MAX_STR_VARIABLES; i++)
    {
        sprintf(buff, FS("<strvar idx=\"%d\" value=\"%s\" />"), i, sanitize_xml_string(strvars[i].str).c_str());
        http_send(buff);
    }
    // colors
    for (int i = 0; i < MAX_DSP_COLOR_VARIABLES; i++)
    {
        sprintf(buff, FS("<dspcolor idx=\"%d\" red=\"%d\" green=\"%d\" blue=\"%d\" white=\"%d\" />"), i, dspcolorvars[i].red, dspcolorvars[i].green, dspcolorvars[i].blue, dspcolorvars[i].white);
        http_send(buff);
    }
    // display modes
    for (int i = 0; i < MAX_DISPLAY_MODE_VARIABLES; i++)
    {
        sprintf(buff, FS("<dispmode idx=\"%d\" name=\"%s\" />"), i, sanitize_xml_string(displaymodevars[i].name).c_str());
        http_send(buff);
    }
    // display animations
    for (int i = 0; i < MAX_DISPLAY_ANIMATION_VARIABLES; i++)
    {
        sprintf(buff, FS("<dispanim idx=\"%d\" name=\"%s\" dcl=\"%d\" def_dcl=\"%d\" flags=\"%d\"/>"),
            i,
            sanitize_xml_string(displayanimationvars[i].name).c_str(),
            displayanimationvars[i].deceleration,
            displayanimationvars[i].default_deceleration,
            displayanimationvars[i].flags);
        http_send(buff);
    }
    // color animations
    for (int i = 0; i < MAX_COLOR_ANIMATION_VARIABLES; i++)
    {
        sprintf(buff, FS("<coloranim idx=\"%d\" name=\"%s\" dcl=\"%d\" def_dcl=\"%d\" flags=\"%d\"/>"),
            i,
            sanitize_xml_string(coloranimationvars[i].name).c_str(),
            coloranimationvars[i].deceleration,
            coloranimationvars[i].default_deceleration,
            coloranimationvars[i].flags);
        http_send(buff);
    }
    // ambilight modes
    for (int i = 0; i < MAX_AMBILIGHT_MODE_VARIABLES; i++)
    {
        sprintf(buff, FS("<almode idx=\"%d\" name=\"%s\" dcl=\"%d\" def_dcl=\"%d\" flags=\"%d\"/>"),
            i,
            sanitize_xml_string(ambilightmodevars[i].name).c_str(),
            ambilightmodevars[i].deceleration,
            ambilightmodevars[i].default_deceleration,
            ambilightmodevars[i].flags);
        http_send(buff);
    }
    // night times
    for (int i = 0; i < MAX_NIGHT_TIME_VARIABLES; i++)
    {
        sprintf(buff, FS("<nighttime idx=\"%d\" minutes=\"%d\" flags=\"%d\"/>"), i, nighttimevars[i].minutes, nighttimevars[i].flags);
        http_send(buff);
    }

    http_send(FS("</settings>"));

    http_flush();
}

static uint_fast8_t
flash_stm32_local ( const char* param, bool post = false )
{
    char *              action;
    int                 do_reset = 0;
    uint32_t            flashsize;

    flashsize = ESP.getFlashChipRealSize ();

    action = http_get_param ("action");

    if (action)
    {
        if (! strcmp (action, "reset"))
        {
            do_reset = 1;
        }
    }

    if (do_reset)
    {
        http_header ("WordClock Local Update", "20", "/");
    }
    else
    {
        http_header("WordClock Local Update", NULL, NULL);
    }

    http_menu();

    if (flashsize == 4194304)
    {
        if (post)
        {
            String line;
            SPIFFS.begin();
            File f = SPIFFS.open("stm32.hex", "w+");

            // find the line with content type and boundary string
            String boundary;

            while (read_line(line))
            {
                if (line.startsWith("Content-Type:"))
                {
                    int n = line.indexOf("boundary=");
                    if (n > 0)
                    {
                        boundary = "--" + line.substring(n + strlen("boundary="));
                    }
                    break;
                }
            }

            // no boundary? something is wrong
            if (boundary == "")
            {
                http_send("<font color='red'>Wrong http header</font><br/>");
            }
            else
            {
                int i;

                // read until a boundary has been found
                while(read_line(line) && line != boundary)
                {
                    ;
                }

                // read the empty line
                while (read_line(line) && line != "")
                {
                    ;
                }

                // read and save file content until the boundary has been found
                while (read_line(line))
                {
                    // boundary found, end of file
                    if (line == boundary)
                    {
                        break;
                    }
                    f.write((const unsigned char*)line.c_str(), line.length());
                    f.write('\n');
                }

                f.close();

                for (i = 0; i < 100; i++)
                {                                                   // send 1000 spaces to force Browser to begin rendering
                    http_send ("          "); 
                }

                stm32_flash_from_local();
                SPIFFS.remove("stm32.hex");
                SPIFFS.end();
                http_send_FS ("Done. <font color=red>Please Reset your STM32 now!</font></B><BR>\r\n");
            }
        }
        else if (do_reset)
        {
            http_send ("<P><B>Resetting STM32, reconnecting in 20 seconds ...</B><BR>\r\n");
            http_trailer ();
            http_flush ();
            delay (200);
            stm32_reset ();
            return 0;
        }

        http_send_FS (
                "<br/>Please select the STM32 firmware file<br/>"
                "<form method='post' action='flash_stm32_local' name='submit' enctype='multipart/form-data' style=\"display:inline\">"
                "<input type='file' name='fileField'><br /><br />"
                "<input type='submit' name='submit' value='Update STM32'>"
                "</form>"
                );
        http_send_FS ("<form method=\"GET\" action=\"/flash_stm32_local\" style=\"display:inline\">"
                      "<button type=\"submit\" name=\"action\" value=\"reset\">Reset STM32</button>"
                      "</form>\r\n");
    }
    else
    {
        http_send_FS ("<font color=red><B>Flash size of ESP8266 is too small for Local Update over OTA</B></font><BR>\r\n");
    }

    http_trailer();
    http_flush ();
    return 0;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * http server
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
http (const char * path, const char * const_param)
{
    char param[256];

    strncpy (param, const_param, 255);
    param[255] = '\0';

    char *          p;
    char *          s;
    char *          t;
    int             rtc = 0;

    // log_printf ("http path: '%s'\r\n", path);

    for (p = param; *p; p++)
    {
        if (*p == '%')
        {
            *p = htoi (p + 1, 2);

            t = p + 1;
            s = p + 3;

            while (*s)
            {
                *t++ = *s++;
            }
            *t = '\0';
        }
        else if (*p == '+')                                     // plus must be mapped to space if GET method
        {
            *p = ' ';
        }
    }

    // log_printf ("http parameters: '%s'\r\n", param);

    if (param)
    {
        http_set_params (param);
    }
    else
    {
        http_set_params ((char *) NULL);
    }

    if (! strcmp (path, "/"))
    {
        rtc = http_main ();
    }
    else if (! strcmp (path, "/network"))
    {
        rtc = http_network ();
    }
    else if (! strcmp (path, "/temperature"))
    {
        rtc = http_temperature ();
    }
    else if (! strcmp (path, "/weather"))
    {
        rtc = http_weather ();
    }
    else if (! strcmp (path, "/ldr"))
    {
        rtc = http_ldr ();
    }
    else if (! strcmp (path, "/brightness"))
    {
        rtc = http_brightness ();
    }
    else if (! strcmp (path, "/display"))
    {
        rtc = http_display ();
    }
    else if (! strcmp (path, "/animations"))
    {
        rtc = http_animations ();
    }
    else if (! strcmp (path, "/ambilight"))
    {
        rtc = http_ambilight ();
    }
    else if (! strcmp (path, "/timers"))
    {
        rtc = http_timers (0);
    }
    else if (! strcmp (path, "/atimers"))
    {
        rtc = http_timers (1);
    }
    else if (! strcmp (path, "/update"))
    {
        rtc = http_update ();
    }
    else if (! strcmp (path, "/flash_stm32_local"))
    {
        rtc = flash_stm32_local (param);
    }
    else if (! strcmp (path, "/get_settings"))
    {
        rtc = http_get_settings (param);
    }
    else
    {
        const char * p = "HTTP/1.0 404 Not Found";
        http_send (p);
        http_send_FS ("\r\n\r\n404 Not Found\r\n");
        http_flush ();
    }

    return rtc;
}

void
http_post(const String& sPath, const String& sParam)
{
    if (sPath == "/flash_stm32_local")
    {
        flash_stm32_local (sParam.c_str(), true);
    }
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * http server
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
void
http_server_loop (void)
{
    String  sPath       = "";
    String  sParam      = "";
    String  sCmd        = "";
    String  sGetstart   = "GET ";
    String  sPoststart  = "POST ";
    String  sResponse   = "";
    int     start_position;
    int     end_position_space;
    int     end_position_question;
    int     i = 0;

    http_client = http_server.available();                                      // check if a client has connected

    if (!http_client)                                                           // wait until the client sends some data
    {
      return;
    }

    Serial.println("- new client");
    Serial.flush ();

    unsigned long ultimeout = millis() + 250;

    while (! http_client.available() && (millis() < ultimeout) )
    {
        delay(1);
    }

    if (millis() > ultimeout)
    {
        Serial.println ("- client connection time-out!");
        Serial.flush ();
        return;
    }
    http_client.setNoDelay(1);

    String sRequest = http_client.readStringUntil ('\r');                   // read the first line of the request

    if (sRequest == "")                                                     // stop client, if request is empty
    {
        Serial.println ("- empty http request");
        Serial.flush ();
        http_client.stop();
        return;
    }

    // POST
    start_position = sRequest.indexOf(sPoststart);

    if (start_position == 0)
    {
        start_position+=+sPoststart.length ();
        end_position_space = sRequest.indexOf (" ", start_position);
        if (end_position_question > 0)
        {                                                               // yes
            sPath  = sRequest.substring(start_position,end_position_question);
            sParam = sRequest.substring(end_position_question + 1,end_position_space);
        }
        else
        {                                                               // no
            sPath  = sRequest.substring(start_position,end_position_space);
        }
        http_post (sPath, sParam);
    }
    else // GET
    {
        start_position = sRequest.indexOf(sGetstart);

        if (start_position >= 0)
        {
            start_position+=+sGetstart.length ();
            end_position_space = sRequest.indexOf (" ", start_position);
            end_position_question = sRequest.indexOf ("?", start_position);

            if (end_position_space > 0)                                         // parameters?
            {
                if (end_position_question > 0)
                {                                                               // yes
                    sPath  = sRequest.substring(start_position,end_position_question);
                    sParam = sRequest.substring(end_position_question + 1,end_position_space);
                }
                else
                {                                                               // no
                    sPath  = sRequest.substring(start_position,end_position_space);
                }
            }
        }

        http (sPath.c_str(), sParam.c_str());
    }

    http_client.flush();
    http_client.stop();                                                     // stop client
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * http server begin
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
void
http_server_begin (void)
{
    http_server.begin();
}
