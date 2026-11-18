#include <gtest/gtest.h>

#include "L76X.h"

TEST(NMEA_test, RMC_parsing) {
    GPSdata_t GPS;
    strncpy(buff_t, "$GPRMC,083559.00,A,4717.11437,N,00833.91522,E,0.004,77.52,091202,,,A,V*57\r\n", BUFFSIZE);
    ASSERT_EQ(findMessageId("RMC"), 0);
    ASSERT_EQ(buff_t[0], '$');

    L76X_Extract_RMC(&GPS);

    ASSERT_EQ(GPS.Time.h, 8);
    ASSERT_EQ(GPS.Time.m, 35);
    ASSERT_EQ(GPS.Time.s, 59);
    ASSERT_EQ(GPS.Time.ms, 0);
    ASSERT_EQ(GPS.Status, 1);
    EXPECT_FLOAT_EQ(GPS.Lat, 47.1711437);
    ASSERT_EQ((char)GPS.Lat_area, 'N');
    EXPECT_FLOAT_EQ(GPS.Lon, 8.3391522);
    ASSERT_EQ((char)GPS.Lon_area, 'E');
    EXPECT_FLOAT_EQ(GPS.Speed, 0.004);
    ASSERT_EQ(GPS.Date.d, 9);
    ASSERT_EQ(GPS.Date.m, 12);
    ASSERT_EQ(GPS.Date.y, 2002);
}