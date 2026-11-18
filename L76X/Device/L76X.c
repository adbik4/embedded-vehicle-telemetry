#include <stdint.h>
#ifndef UNIT_TEST
#include "L76X_DEV_Config.h"
#endif

#include "L76X.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <stdbool.h>

char const Temp[16]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

static const double pi = 3.14159265358979324;
static const double a = 6378245.0;
static const double ee = 0.00669342162296594323;
static const double x_pi = 3.14159265358979324 * 3000.0 / 180.0;

#ifdef UNIT_TEST
char buff_t[BUFFSIZE] = {0};
#else
static char buff_t[BUFFSIZE]={0};
#endif

/******************************************************************************
function:	
	Find start of a NMEA Message identificator sentence inside the buffer. (Must be 3 letters)
    returns: An index to the start of the entry. Or -1 if it's not found
******************************************************************************/
int findMessageId(const char* id)
{   
    if (strlen(id) != 3) {
        return -1;
    }

    int offset = -1;
    for (UWORD i = 0; i + 5 < BUFFSIZE; ++i) {
        if (buff_t[i] == '$' && buff_t[i+1] == 'G' &&
            buff_t[i+3] == id[0] && buff_t[i+4] == id[1] && buff_t[i+5] == id[2]
        ){
            offset = i;
            break;
        }
        if (buff_t[i+5] == '\0') {
            break; /* no full sentence */
        }
    }
    return offset;
}

/******************************************************************************
function:	
	Latitude conversion
******************************************************************************/
static double transformLat(double x,double y)
{
	double ret = -100.0 + 2.0 * x + 3.0 * y + 0.2 * y * y + 0.1 * x * y + 0.2 *sqrt(abs(x));
    ret += (20.0 * sin(6.0 * x * pi) + 20.0 * sin(2.0 * x * pi)) * 2.0 / 3.0;
    ret += (20.0 * sin(y * pi) + 40.0 * sin(y / 3.0 * pi)) * 2.0 / 3.0;
    ret += (160.0 * sin(y / 12.0 * pi) + 320 * sin(y * pi / 30.0)) * 2.0 / 3.0;
    return ret;
}

/******************************************************************************
function:	
	Longitude conversion
******************************************************************************/
static double transformLon(double x,double y)
{
	double ret = 300.0 + x + 2.0 * y + 0.1 * x * x + 0.1 * x * y + 0.1 * sqrt(abs(x));
    ret += (20.0 * sin(6.0 * x * pi) + 20.0 * sin(2.0 * x * pi)) * 2.0 / 3.0;
    ret += (20.0 * sin(x * pi) + 40.0 * sin(x / 3.0 * pi)) * 2.0 / 3.0;
    ret += (150.0 * sin(x / 12.0 * pi) + 300.0 * sin(x / 30.0 * pi)) * 2.0 / 3.0;
    return ret;
}

/******************************************************************************
function:	
	GCJ-02 international standard converted to Baidu map BD-09 standard
******************************************************************************/
static Coordinates bd_encrypt(Coordinates gg)
{
	Coordinates bd;
    double x = gg.Lon, y = gg.Lat;
	double z = sqrt(x * x + y * y) + 0.00002 * sin(y * x_pi);
	double theta = atan2(y, x) + 0.000003 * cos(x * x_pi);
	bd.Lon = z * cos(theta) + 0.0065;
	bd.Lat = z * sin(theta) + 0.006;
	return bd;
}

/******************************************************************************
function:	
	GPS's WGS-84 standard is converted into GCJ-02 international standard
******************************************************************************/
static Coordinates transform(Coordinates gps)
{
	Coordinates gg;
    double dLat = transformLat(gps.Lon - 105.0, gps.Lat - 35.0);
    double dLon = transformLon(gps.Lon - 105.0, gps.Lat - 35.0);
    double radLat = gps.Lat / 180.0 * pi;
    double magic = sin(radLat);
    magic = 1 - ee * magic * magic;
    double sqrtMagic = sqrt(magic);
    dLat = (dLat * 180.0) / ((a * (1 - ee)) / (magic * sqrtMagic) * pi);
    dLon = (dLon * 180.0) / (a / sqrtMagic * cos(radLat) * pi);
    gg.Lat = gps.Lat + dLat;
    gg.Lon = gps.Lon + dLon;
	return gg;
}

#ifndef UNIT_TEST
/******************************************************************************
function:	
	Send a command to the L76X，Automatic calculation of the code
parameter:
    data ：The end of the command ends with ‘\0’ or it will go wrong, 
           no need to increase the validation code.
******************************************************************************/
void L76X_Send_Command(char *data)
{
    char Check = data[1], Check_char[3]={0};
    UBYTE i = 0;
    DEV_Uart_SendByte('\r');
    DEV_Uart_SendByte('\n');
    
    //printf(" 1i = %d Check =%x \n", i, Check);
    for(i=2; data[i] != '\0'; i++){
        Check ^= data[i];       //Calculate the check value
    }
    //printf(" i = %d Check =%x \n", i, Check);
    Check_char[0] = Temp[Check/16%16];
    Check_char[1] = Temp[Check%16];
		Check_char[2] = '\0';
   

    DEV_Uart_SendString(data);
    DEV_Uart_SendByte('*');
    DEV_Uart_SendString(Check_char);
    DEV_Uart_SendByte('\r');
    DEV_Uart_SendByte('\n');
}

void L76X_Exit_BackupMode()
{
    DEV_Set_GPIOMode(L76X_RST_GPIO, 0);
    
    DEV_Digital_Write(L76X_RST_GPIO, 0);
    DEV_Delay_ms(1000);
    DEV_Digital_Write(L76X_RST_GPIO, 1);
    
    DEV_Set_GPIOMode(L76X_RST_GPIO, 1);
}
#endif

/******************************************************************************
function:
    Calculates the checksum of a NMEA payload.
    Input: only the part between $ and *
******************************************************************************/
#ifndef UNIT_TEST
static uint8_t NMEAChecksum(const char* start, const UWORD len) {
#else
uint8_t NMEAChecksum(const char* start, const UWORD len) {
#endif
    uint8_t sum = 0;
    for (UWORD i = 0; i < len; ++i) {
        sum ^= start[i];
    }
    return sum;
}

/******************************************************************************
function:
    Parses a hexadecimal value.
******************************************************************************/
#ifndef UNIT_TEST
static bool parseAsHex(const char* str, const UWORD len, uint8_t* result) {
#else
bool parseAsHex(const char* str, const UWORD len, uint8_t* result) {
#endif
    if (len == 0 || len > 2 || str == NULL || result == NULL) {
        return false;
    }

    uint8_t value = 0;
    for (UWORD i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)str[i];
        uint8_t digit;

        if (c >= '0' && c <= '9') {
            digit = (uint8_t)(c - '0');
        } else if (c >= 'A' && c <= 'F') {
            digit = (uint8_t)(c - 'A' + 10);
        } else if (c >= 'a' && c <= 'f') {
            digit = (uint8_t)(c - 'a' + 10);
        } else {
            return false;
        }

        value = (uint8_t)((value << 4) | digit);
    }

    *result = value;
    return true;
}

/******************************************************************************
function:
    Verifies the checksum of one NMEA phrase.
    Input: A pointer to the first character of the entire message
******************************************************************************/
#ifndef UNIT_TEST
static bool validateChecksum(const char* payload) {
#else
bool validateChecksum(const char* payload) {
#endif
    UWORD star = 0;
    UWORD length = 0;
    uint8_t check = 0;
    while (length < BUFFSIZE && payload[length] != '\0') {
        ++length;
    }

    for (UWORD i = 0; i < length; ++i) {
        if (payload[i] == '*') {
            star = i;
            break;
        }
    }

    if (star == 0 || star + 2 >= length) {
        return false;
    }

    if (!parseAsHex(payload + star + 1, 2, &check)) {
        return false;
    }

    return check == NMEAChecksum(payload + 1, star - 1);
}

/******************************************************************************
function:
    Remove '.' and parse as integer. Returns UINT64_MAX if invalid
******************************************************************************/
#ifndef UNIT_TEST
static bool parseAsInteger(const char* str, const UWORD len, uint64_t* result) {
#else
bool parseAsInteger(const char* str, const UWORD len, uint64_t* result) {
#endif
    uint64_t sum = 0;
    for (UWORD k = 0; k < len; ++k) {
        char c = str[k];
        if (c == '.' ) { continue; }
        if (c < '0' || c > '9') { return false; };
        sum = sum * 10 + (c - '0');
    }

    *result = sum;
    return true;
}

/******************************************************************************
function:
    Detect '.' and parse as double. Returns DBL_MAX if invalid
******************************************************************************/
#ifndef UNIT_TEST
static bool parseAsDouble(const char* str, const UWORD len, double* result) {
#else
bool parseAsDouble(const char* str, const UWORD len, double* result) {
#endif
    double sum = 0;
    int decimal_pos = 0;
    uint8_t is_negative = 0;
    for (UWORD k = 0; k < len; ++k) {
        char c = str[k];
        if (c == '-' && k == 0) { is_negative = 1; continue; }
        if (c == '.' ) { decimal_pos = len - k - 1; continue; }
        if (c < '0' || c > '9') { return false; }
        sum = sum * 10 + (c - '0');
    }

    if (is_negative) {
        *result = -1.0 * sum / pow(10, decimal_pos);
    } else {
        *result = sum / pow(10, decimal_pos);
    }
    return true;
}

/******************************************************************************
function:
    Convert from knots to kilometers per hour
******************************************************************************/
#ifndef UNIT_TEST
static double convert2Kmph(const double sog) {
#else
double convert2Kmph(const double sog) {
#endif
    return sog * 1.852;
}

static bool verifyTime(const double value, UTC_Time_t* time) {
    if (value < 0) {
        return false;
    }

    int64_t int_val = floor(value);
    UBYTE hour = (UBYTE)(int_val / 10000.0);
    UBYTE minute = (UWORD)(int_val / 100.0) % 100;
    UBYTE second = int_val % 100;

    if (hour > 23 || minute > 59 || second > 59) {
        return false;
    }

    time->h = hour;
    time->m = minute;
    time->s = second;
    time->ms = (value - int_val) * 1000;
    return true;
}

static bool verifyLat(const double value) {
    int degrees = (int)(value / 100.0);
    double minutes = value - degrees * 100.0;

    return value >= 0 && degrees <= 90 && minutes < 60 &&
        (degrees < 90 || minutes == 0);
}

static bool verifyLon(const double value) {
    int degrees = (int)(value / 100.0);
    double minutes = value - degrees * 100.0;

    return value >= 0 && degrees <= 180 && minutes < 60 &&
        (degrees < 180 || minutes == 0);
}

/******************************************************************************
function:	
	Analyze GNRMC data in L76x, latitude and longitude, time
******************************************************************************/
void L76X_Extract_RMC(GPSdata_t* result) {
    UWORD i = 0;
    memset(result, 0, sizeof(GPSdata_t));

    int start = findMessageId("RMC");
    if (start < 0) {
        return; /* no GNRMC sentence found */
    }
    if (!validateChecksum(buff_t + start)) {
        return;
    }

    /* Parse fields separated by commas. We only need up to field index 9. */
    int field = 0;
    for (i = (UWORD)(start + 6); ((field <= 9) && (i < BUFFSIZE) && (buff_t[i] != '\0')); ++i) {
        if (buff_t[i] == ',') {
            ++field;
            const char *f = &buff_t[i+1];

            /* Find length of this field */
            UWORD len = 0;
            while ((i+1+len) < BUFFSIZE && f[len] != ',' && f[len] != '\0') {
                ++len;
            }

            if (field == 1 && len > 0) {
                /* POSITION FIX UTC (TIME):
                hh: Hours (00–23)
                mm: Minutes (00–59)
                ss: Seconds (00–59)
                sss: Decimal fraction of seconds */
                double dbl_val = 0;
                if (!parseAsDouble(f, len, &dbl_val)) {
                    continue;
                }
                if (!verifyTime(dbl_val, &result->Time)) {
                    continue;
                }

            } else if (field == 2 && len > 0) {
                /* Status: A = ACTIVE (valid), V = VOID (invalid) */
                result->Status = (f[0] == 'A') ? 1 : 0;

            } else if (field == 3 && len > 0) {
                /* LATITUDE: (ddmm.mmmmmm)
                dd: Degrees (00–90)
                mm: Minutes (00–59)
                mmmmmm: Decimal fraction of minutes */
                double dbl_val = 0;
                if (!parseAsDouble(f, len, &dbl_val)) {
                    continue;
                }
                if (!verifyLat(dbl_val)) {
                    continue;
                }
                result->Lat = dbl_val;

            } else if (field == 4 && len > 0) {
                /* LATITUDE DIRECTION: N = North, S = South */
                result->Lat_area = f[0];

            } else if (field == 5 && len > 0) {
                /* LONGITUDE:
                ddd: Degrees (000–180)
                mm: Minutes (00–59)
                mmmmmm: Decimal fraction of minutes */
                double dbl_val = 0;
                if (!parseAsDouble(f, len, &dbl_val)) {
                    continue;
                }
                if (!verifyLon(dbl_val)) {
                    continue;
                }
                result->Lon = dbl_val;

            } else if (field == 6 && len > 0) {
                /* LONGITUDE DIRECTION: W = West, E = East */
                result->Lon_area = f[0];
            
            } else if (field == 7 && len > 0) {
                /* SPEED OVER GROUND: (knots)*/
                double dbl_val = 0;
                if (!parseAsDouble(f, len, &dbl_val)) {
                    continue;
                }
                result->Speed = convert2Kmph(dbl_val);

            } else if (field == 9 && len > 0) {
                /* DATE: (ddmmyy)
                dd: Day (01-31)
                mm: Month (01-12)
                yy: Year (00-99) */
                uint64_t int_val = 0;
                if (!parseAsInteger(f, len, &int_val)) {
                    continue;
                }

                UBYTE day = (UBYTE)(int_val / 10000.0);
                UBYTE month = (UWORD)(int_val / 100.0) % 100;
                UWORD year = (UWORD)(int_val % 100 + 2000);

                if(day < 1 || day > 31 || month < 1 || month > 12 || year < 2000 || year > 2099) {
                    continue;
                }
                result->Date.d = day;
                result->Date.m = month;
                result->Date.y = year;
            }
        }
    }
}

void L76X_Receive_NMEA(GPSdata_t* GPS)
{
    #ifndef UNIT_TEST
    DEV_Uart_ReceiveString(buff_t, BUFFSIZE);
    #endif

    L76X_Extract_RMC(GPS);
}

/******************************************************************************
function:	
	Convert GPS latitude and longitude into Baidu map coordinates
******************************************************************************/
Coordinates L76X_Baidu_Coordinates(const GPSdata_t* data)
{
    Coordinates temp;
    temp.Lat =((int)(data->Lat)) + (data->Lat - ((int)(data->Lat)))*100 / 60;
    temp.Lon =((int)(data->Lon)) + (data->Lon - ((int)(data->Lon)))*100 / 60;
    temp = transform(temp);
    temp = bd_encrypt(temp);
    return temp;
}

/******************************************************************************
function:	
	Convert GPS latitude and longitude into Google Maps coordinates
******************************************************************************/
Coordinates L76X_Google_Coordinates(const GPSdata_t* data)
{
    Coordinates temp;
    temp.Lat = ((int)(data->Lat)) + (data->Lat - ((int)(data->Lat)))*100 / 60;
    temp.Lon =((int)(data->Lon)) + (data->Lon - ((int)(data->Lon)))*100 / 60;
    return temp;
}

/******************************************************************************
function:
	Convert Google Maps coordinates into human-readable
    Degrees Minutes Seconds format
******************************************************************************/
DMS_Coordinates Google2DMS(Coordinates coors, char lat_area, char lon_area)
{
    DMS_Coordinates result = {};
    result.Lat.Degrees = (uint16_t)(coors.Lat / 100.0);
    result.Lat.Minutes = (uint16_t)(coors.Lat - result.Lat.Degrees * 100);
    result.Lat.Seconds = (coors.Lat - result.Lat.Degrees * 100 - result.Lat.Minutes) * 60;
    result.Lat.Area = lat_area;

    result.Lon.Degrees = (uint16_t)(coors.Lon / 100.0);
    result.Lon.Minutes = (uint16_t)(coors.Lon - result.Lon.Degrees * 100);
    result.Lon.Seconds = (coors.Lon - result.Lon.Degrees * 100 - result.Lon.Minutes) * 60;
    result.Lon.Area = lon_area;
    return result;
}