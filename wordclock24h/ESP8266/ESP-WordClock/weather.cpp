/*----------------------------------------------------------------------------------------------------------------------------------------
 * weather.cpp - weather stuff
 *
 * Copyright (c) 2016 Frank Meyer - frank(at)fli4l.de
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
#include <ESP8266WiFi.h>
#include "weather.h"
#include "base.h"

WiFiClient openweather_client;

typedef struct
{
    char *  coord_lon;                              // City geo location, longitude
    char *  coord_lat;                              // City geo location, latitude
    char *  weather_id;                             // Weather condition id
    char *  weather_main;                           // Group of weather parameters (Rain, Snow, Extreme etc.)
    char *  weather_description;                    // Weather condition within the group
    char *  weather_icon;                           // Weather icon id
    char *  base;                                   // Internal parameter
    char *  main_temp;                              // Temperature. Unit Default: Kelvin, Metric: Celsius, Imperial: Fahrenheit.
    char *  main_pressure;                          // Atmospheric pressure (on the sea level, if there is no sea_level or grnd_level data), hPa
    char *  main_humidity;                          // Humidity, %
    char *  main_temp_min;                          // Minimum temperature at the moment.
    char *  main_temp_max;                          // Maximum temperature at the moment
    char *  wind_speed;                             // Wind speed. Unit Default: meter/sec, Metric: meter/sec, Imperial: miles/hour.
    char *  clouds_all;                             // Cloudiness, %
    char *  dt;                                     // Time of data calculation, unix, UTC
    char *  sys_type;                               // Internal parameter
    char *  sys_id;                                 // Internal parameter
    char *  sys_message;                            // Internal parameter
    char *  sys_country;                            // Country code (GB, JP etc.)
    char *  sys_sunrise;                            // Sunrise time, unix, UTC
    char *  sys_sunset;                             // Sunset time, unix, UTC
    char *  id;                                     // City ID
    char *  name;                                   // City name
    char *  cod;                                    // Internal parameter, 200 = ok
} WEATHER;

static WEATHER weather;

/*----------------------------------------------------------------------------------------------------------------------------------------
 * strip_quotes() - strip quotes
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
static char *
strip_quotes (char * p)
{
    char *  pp;

    if (*p == '"')
    {
        p++;

        for (pp = p; *pp; pp++)
        {
            if (*pp == '"')
            {
                *pp = '\0';
                break;
            }
        }
    }
    return p;
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * set_weather_value () - store a value in weather struct
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
static void
set_weather_value (char * name, char * subname, char * value)
{
    if (! strcmp (name, "coord"))
    {
        if (! strcmp (subname, "lon"))
        {
            weather.coord_lon = value;
        }
        else if (! strcmp (subname, "lat"))
        {
            weather.coord_lat = value;
        }
    }
    else if (! strcmp (name, "weather"))
    {
        if (! strcmp (subname, "id"))
        {
            weather.weather_id = value;
        }
        else if (! strcmp (subname, "main"))
        {
            weather.weather_main = value;
        }
        else if (! strcmp (subname, "description"))
        {
            convert_utf8_to_iso8859 ((unsigned char *) value);
            weather.weather_description = value;
        }
        else if (! strcmp (subname, "icon"))
        {
            weather.weather_icon = value;
        }
    }
    else if (! strcmp (name, "base"))
    {
        weather.base = value;
    }
    else if (! strcmp (name, "main"))
    {
        if (! strcmp (subname, "temp"))
        {
            weather.main_temp = value;
        }
        else if (! strcmp (subname, "pressure"))
        {
            weather.main_pressure = value;
        }
        else if (! strcmp (subname, "humidity"))
        {
            weather.main_humidity = value;
        }
        else if (! strcmp (subname, "temp_min"))
        {
            weather.main_temp_min = value;
        }
        else if (! strcmp (subname, "temp_max"))
        {
            weather.main_temp_max = value;
        }
    }
    else if (! strcmp (name, "wind"))
    {
        if (! strcmp (subname, "speed"))
        {
            weather.wind_speed = value;
        }
    }
    else if (! strcmp (name, "clouds"))
    {
        if (! strcmp (subname, "all"))
        {
            weather.clouds_all = value;
        }
    }
    else if (! strcmp (name, "dt"))
    {
        weather.dt = value;
    }
    else if (! strcmp (name, "sys"))
    {
        if (! strcmp (subname, "type"))
        {
            weather.sys_type = value;
        }
        else if (! strcmp (subname, "id"))
        {
            weather.sys_id = value;
        }
        else if (! strcmp (subname, "message"))
        {
            weather.sys_message = value;
        }
        else if (! strcmp (subname, "country"))
        {
            weather.sys_country = value;
        }
        else if (! strcmp (subname, "sunrise"))
        {
            weather.sys_sunrise = value;
        }
        else if (! strcmp (subname, "sunset"))
        {
            weather.sys_sunset = value;
        }
    }
    else if (! strcmp (name, "id"))
    {
        weather.id = value;
    }
    else if (! strcmp (name, "name"))
    {
        weather.name = value;
    }
    else if (! strcmp (name, "cod"))
    {
        weather.cod = value;
    }
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * set_weather_values () - store all values in weather struct
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
static void
set_weather_values (char * name, char * value)
{
    char * subname;
    char * p;

    if (*value == '[')
    {
        value++;
        *(value + strlen (value) - 1) = '\0';
    }

    if (*value == '{')
    {
        value++;

        while (*value && *value != '}')
        {
            p = value;

            while (1)
            {
                if (*p == ':')
                {
                    subname = value;
                    *p = '\0';
                    value = p + 1;
                }
                else if (*p == ',' || *p == '}')
                {
                    *p++ = '\0';

                    subname = strip_quotes (subname);
                    value = strip_quotes (value);

                    set_weather_value (name, subname, value);
                    value = p;
                    break;
                }
                else if (! *p)
                {
                    break;
                }
                p++;
            }
        }
    }
    else
    {
        value = strip_quotes (value);
        set_weather_value (name, (char *) 0, value);
    }
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * get_weather_value () - get a weather value
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
static char *
get_weather_value (char * name, char * p)
{
    int cnt = 0;

    char * value = p;

    while (*p && (*p != ',' || cnt > 0))
    {
        if (*p == '{')
        {
            cnt++;
            p++;
        }
        else if (*p == '}')
        {
            cnt--;
            p++;
        }
        else
        {
            p++;
        }
    }
    *p++ = '\0';

    set_weather_values (name, value);

    return p;
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * get_weather_item () - get an item
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
static char *
get_weather_item (char * p)
{
    char *  name;

    if (*p == '"')
    {
        p++;
        name = p;

        while (*p && *p != '"')
        {
            p++;
        }

        *p++ = '\0';

        if (*p == ':')
        {
            p = get_weather_value (name, p + 1);
        }
    }
    else
    {
        p = (char *) 0;
    }

    return p;
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * strip_whitespaces () - strip whitespaces at end of string
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
static void
strip_whitespaces (char * p)
{
    int len = strlen (p);

    while (len > 0 && (*(p + len - 1) == '\r' || *(p + len - 1) == '\n'))
    {
        *(p + len - 1) = '\0';
        len--;
    }
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * fill_weather () - fill weather struct
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
static void
fill_weather (const char * answer)
{
    static char     buf[1024];                                      // must be static, weather items point to it!
    char *          p;

    memset (&weather, 0, sizeof (weather));

    strncpy (buf, answer, 1024);
    strip_whitespaces (buf);

    p = buf;

    if (*p == '{')
    {
        p++;

        while (*p == ' ')
        {
            p++;
        }

        *(p + strlen (p) - 1) = '\0';

        do
        {
            p = get_weather_item (p);
        } while (p && *p);
    }
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * query_weather () - query weather for a coordinate or city
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
static void
query_weather (char * appid, char * lon, char * lat, char * city)
{
    const char *    hostname = "api.openweathermap.org";
    String          url;

    if (city)
    {
        char * pp;

        url = (String) "/data/2.5/weather?q=";

        do
        {
            pp = strchr (city, ' ');

            if (pp)
            {
                *pp = '\0';
                url += city;
                url += "%20";
                city = pp + 1;
            }
        } while (pp);

        url += (String) city + "&lang=de&units=metric&APPID=" + appid;
    }
    else
    {
        url = (String) "/data/2.5/weather?lon=" + lon + "&lat=" + lat + "&lang=de&units=metric&APPID=" + appid;
    }

    if (openweather_client.connect(hostname, 80))
    {
        debugmsg ("Connected to server");
        openweather_client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + hostname + "\r\n" + "Connection: close\r\n\r\n");
        debugmsg (String("GET ") + url + " HTTP/1.1<CR><LF>" + "Host: " + hostname + "<CR><LF>" + "Connection: close<CR><LF><CR><LF>");

        delay (200);    // fm??? 100 results in timeout sometimes?

        if (openweather_client.available())
        {
            String          line;
            int             cnt = 0;
            const char *    p;

            do
            {
                line = openweather_client.readStringUntil('\n');
                p = line.c_str();

                if (p)
                {
                    while (*p == '\r' || *p == '\n')
                    {
                        p++;
                    }

                    // debugmsg ("answer", p);

                    if (! *p)
                    {
                        // debugmsg ("empty answer line found");
                        break;
                    }
                    cnt++;
                }
                else
                {
                    debugmsg ("null");
                }
            } while (openweather_client.available() && cnt < 20);

            if (openweather_client.available())
            {
                line = openweather_client.readStringUntil('\n');
                p = line.c_str();
                // debugmsg ("data0", p);

                if (p && strlen (p) < 10 && openweather_client.available())       // 1st data line is length of following line in Hex, e.g. "1da"
                {
                    line = openweather_client.readStringUntil('\n');            // read next data line
                    p = line.c_str();
                    // debugmsg ("data1", p);
                }

                if (p)
                {
                    fill_weather (p);

#if 0
                    debugmsg ("coord_lon", weather.coord_lon);                        delay (20);
                    debugmsg ("coord_lat", weather.coord_lat);                        delay (20);
                    debugmsg ("weather_id", weather.weather_id);                      delay (20);
                    debugmsg ("weather_main", weather.weather_main);                  delay (20);
                    debugmsg ("weather_description", weather.weather_description);    delay (20);
                    debugmsg ("weather_icon", weather.weather_icon);                  delay (20);
                    debugmsg ("base", weather.base);                                  delay (20);
                    debugmsg ("main_temp", weather.main_temp);                        delay (20);
                    debugmsg ("main_pressure", weather.main_pressure);                delay (20);
                    debugmsg ("main_humidity", weather.main_humidity);                delay (20);
                    debugmsg ("main_temp_min", weather.main_temp_min);                delay (20);
                    debugmsg ("main_temp_max", weather.main_temp_max);                delay (20);
                    debugmsg ("wind_speed", weather.wind_speed);                      delay (20);
                    debugmsg ("clouds_all", weather.clouds_all);                      delay (20);
                    debugmsg ("dt", weather.dt);                                      delay (20);
                    debugmsg ("sys_type", weather.sys_type);                          delay (20);
                    debugmsg ("sys_id", weather.sys_id);                              delay (20);
                    debugmsg ("sys_message", weather.sys_message);                    delay (20);
                    debugmsg ("sys_country", weather.sys_country);                    delay (20);
                    debugmsg ("sys_sunrise", weather.sys_sunrise);                    delay (20);
                    debugmsg ("sys_sunset", weather.sys_sunset);                      delay (20);
                    debugmsg ("id", weather.id);                                      delay (20);
                    debugmsg ("name", weather.name);                                  delay (20);
                    debugmsg ("cod", weather.cod);                                    delay (20);
#endif

                    if (weather.weather_description)
                    {
                        Serial.print ("WEATHER ");
                        Serial.print ("Wetter: ");
                        Serial.print (weather.main_temp);
                        Serial.print (" Grad, ");
                        Serial.println (weather.weather_description);
                    }
                    else if (weather.cod)
                    {
                        Serial.print("WEATHER Code ");
                        Serial.println (weather.cod);
                    }
                    else
                    {
                        Serial.println("WEATHER Wetter: unbekannt");
                    }
                }
            }
        }

        openweather_client.stop ();
    }
    else
    {
        Serial.println(String ("ERROR connection to ") + hostname + " failed");
    }
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * get_weather () - get weather for a coordinate
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
void
get_weather (char * appid, char * lon, char * lat)
{
    query_weather (appid, lon, lat, (char *) NULL);
}

/*----------------------------------------------------------------------------------------------------------------------------------------
 * get_weather () - get weather for a city
 *----------------------------------------------------------------------------------------------------------------------------------------
 */
void
get_weather (char * appid, char * city)
{
    query_weather (appid, (char *) NULL, (char *) NULL, city);
}


