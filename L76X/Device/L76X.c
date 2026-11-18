#ifndef UNIT_TEST
#include "L76X_DEV_Config.h"
#endif

#include "L76X.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

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
    Remove '.' and parse as integer
******************************************************************************/
static uint64_t parseAsInteger(const char* str, const UWORD len) {
    uint64_t result = 0;
    for (UWORD k = 0; k < len; ++k) {
        char c = str[k];
        if (c == '.' ) continue;
        if (c < '0' || c > '9') break;
        result = result * 10 + (c - '0');
    }
    return result;
}

/******************************************************************************
function:	
	Analyze GNRMC data in L76x, latitude and longitude, time
******************************************************************************/
void L76X_Extract_RMC(GPSdata_t* result) {
    // TODO: Add Checksum verification
    UWORD i = 0;
    memset(result, 0, sizeof(GPSdata_t));

    int start = findMessageId("RMC");
    if (start < 0) {
        return; /* no GNRMC sentence found */
    }

    /* Parse fields separated by commas. We only need up to field index 9. */
    int field = 0;
    for (i = (UWORD)(start + 6); field <= 9 && buff_t[i] != '\0' && i < BUFFSIZE; ++i) {
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
                uint64_t t = parseAsInteger(f, len);
                result->Time.h = (UBYTE)(t / 1000000);
                result->Time.m = (UBYTE)((t / 10000) % 100);
                result->Time.s = (UBYTE)((t / 100) % 100);
                result->Time.ms = (UBYTE)(t % 100);

            } else if (field == 2 && len > 0) {
                /* Status: A = ACTIVE (valid), V = VOID (invalid) */
                result->Status = (f[0] == 'A') ? 1 : 0;

            } else if (field == 3 && len > 0) {
                /* LATITUDE: (ddmm.mmmmmm)
                dd: Degrees (00–90)
                mm: Minutes (00–59)
                mmmmmm: Decimal fraction of minutes */
                uint64_t v = parseAsInteger(f, len);
                result->Lat = (double)v / 10000000.0;

            } else if (field == 4 && len > 0) {
                /* LATITUDE DIRECTION: N = North, S = South */
                result->Lat_area = f[0];

            } else if (field == 5 && len > 0) {
                /* LONGITUDE:
                ddd: Degrees (000–180)
                mm: Minutes (00–59)
                mmmmmm: Decimal fraction of minutes */
                uint64_t v = parseAsInteger(f, len);
                result->Lon = (double)v / 10000000.0;

            } else if (field == 6 && len > 0) {
                /* LONGITUDE DIRECTION: W = West, E = East */
                result->Lon_area = f[0];
            
            } else if (field == 7 && len > 0) {
                /* SPEED OVER GROUND: (knots)*/
                uint64_t s = parseAsInteger(f, len);
                result->Speed = (double)s / 1000.0; 

            } else if (field == 9 && len > 0) {
                /* DATE: (ddmmyy)
                dd: Day (00-31)
                mm: Month (01-12)
                yy: Year (00-99) */
                uint64_t d = parseAsInteger(f, len);
                result->Date.d = (UBYTE)(d / 10000.0);
                result->Date.m = (UBYTE)((int)(d / 100.0) % 100);
                result->Date.y = (UWORD)(d % 100 + 2000);
            }
        }
    }
}

GPSdata_t L76X_Receive_NMEA()
{
    GPSdata_t GPS;

    #ifndef UNIT_TEST
    DEV_Uart_ReceiveString(buff_t, BUFFSIZE);
    #endif

    L76X_Extract_RMC(&GPS);

    return GPS;
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