#include <gtest/gtest.h>

#include "L76X.h"
#include <float.h>
#include <stdbool.h>

TEST(NMEA_checksumTest, calculateChecksum) {
    char buf[] = "GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W";
    uint8_t check = NMEAChecksum(buf, 64);
    ASSERT_EQ(check, 106);

    char buf2[] = "GPRMC,083559.00,A,4717.11437,N,00833.91522,E,0.004,77.52,091202,,,A,V";
    check = NMEAChecksum(buf2, 69);
    ASSERT_EQ(check, 45);
}

TEST(NMEA_checksumTest, validateChecksum) {
    char buf[] = "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A";
    bool status = validateChecksum(buf);
    ASSERT_EQ(status, true);

    char buf2[] = "$GPRMC,083559.00,A,4717.11437,N,00833.91522,E,0.004,77.52,091202,,,A,V*57\r\n";
    status = validateChecksum(buf2);
    ASSERT_EQ(status, false);

    char buf3[] = "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*8A";
    status = validateChecksum(buf3);
    ASSERT_EQ(status, false);

}

TEST(NMEA_test, RMC_parsing) {
    GPSdata_t GPS;
    strncpy(buff_t, "$GPRMC,083559.00,A,4717.11437,N,00833.91522,E,0.004,77.52,091202,,,A,V*2D\r\n", BUFFSIZE);
    ASSERT_EQ(findMessageId("RMC"), 0);
    ASSERT_EQ(buff_t[0], '$');

    L76X_Extract_RMC(&GPS);

    ASSERT_EQ(GPS.Time.h, 8);
    ASSERT_EQ(GPS.Time.m, 35);
    ASSERT_EQ(GPS.Time.s, 59);
    ASSERT_EQ(GPS.Time.ms, 0);
    ASSERT_EQ(GPS.Status, 1);
    EXPECT_FLOAT_EQ(GPS.Lat, 4717.11437);
    ASSERT_EQ((char)GPS.Lat_area, 'N');
    EXPECT_FLOAT_EQ(GPS.Lon, 833.91522);
    ASSERT_EQ((char)GPS.Lon_area, 'E');
    EXPECT_FLOAT_EQ(GPS.Speed, 0.007408);
    ASSERT_EQ(GPS.Date.d, 9);
    ASSERT_EQ(GPS.Date.m, 12);
    ASSERT_EQ(GPS.Date.y, 2002);
}

TEST(NMEA_convertersTest, ParseAsHex) {
    bool status;
    uint8_t hex_val{0};

    status = parseAsHex("00", 2, &hex_val);
    ASSERT_EQ(status, true);
    ASSERT_EQ(hex_val, 0);

    status = parseAsHex("6A", 2, &hex_val);
    ASSERT_EQ(status, true);
    ASSERT_EQ(hex_val, 106);

    status = parseAsHex("ff", 2, &hex_val);
    ASSERT_EQ(status, true);
    ASSERT_EQ(hex_val, 255);

    status = parseAsHex("7", 1, &hex_val);
    ASSERT_EQ(status, true);
    ASSERT_EQ(hex_val, 7);

    status = parseAsHex("GG", 2, &hex_val);
    ASSERT_EQ(status, false);

    status = parseAsHex("1G", 2, &hex_val);
    ASSERT_EQ(status, false);

    status = parseAsHex("100", 3, &hex_val);
    ASSERT_EQ(status, false);
}

TEST(NMEA_convertersTest, ParseAsInteger) {
    bool status;
    uint64_t int_val{0};

    status = parseAsInteger("12345", 5, &int_val);
    ASSERT_EQ(int_val, 12345);
    ASSERT_EQ(status, true);
    
    status = parseAsInteger("12f45", 5, &int_val);
    ASSERT_EQ(status, false);
}

TEST(NMEA_convertersTest, ParseAsDouble) {
    bool status;
    double dbl_val{0};

    // positive
    status = parseAsDouble("123.45", 6, &dbl_val);
    EXPECT_DOUBLE_EQ(dbl_val, 123.45);
    ASSERT_EQ(status, true);

    status = parseAsDouble("0.12345", 7, &dbl_val);
    EXPECT_DOUBLE_EQ(dbl_val, 0.12345);
    ASSERT_EQ(status, true);

    status = parseAsDouble("01234.5", 7, &dbl_val);
    EXPECT_DOUBLE_EQ(dbl_val, 01234.5);
    ASSERT_EQ(status, true);

    status = parseAsDouble("0.0", 3, &dbl_val);
    EXPECT_DOUBLE_EQ(dbl_val, 0.0);
    ASSERT_EQ(status, true);

    status = parseAsDouble("12345.0", 7, &dbl_val);
    EXPECT_DOUBLE_EQ(dbl_val, 12345.0);
    ASSERT_EQ(status, true);

    status = parseAsDouble("123.d45", 7, &dbl_val);
    ASSERT_EQ(status, false);

    // negative
    status = parseAsDouble("-123.45", 7, &dbl_val);
    EXPECT_DOUBLE_EQ(dbl_val, -123.45);
    ASSERT_EQ(status, true);

    status = parseAsDouble("-0.12345", 8, &dbl_val);
    EXPECT_DOUBLE_EQ(dbl_val, -0.12345);
    ASSERT_EQ(status, true);

    status = parseAsDouble("-01234.5", 8, &dbl_val);
    EXPECT_DOUBLE_EQ(dbl_val, -01234.5);
    ASSERT_EQ(status, true);

    status = parseAsDouble("-0.0", 4, &dbl_val);
    EXPECT_DOUBLE_EQ(dbl_val, -0.0);
    ASSERT_EQ(status, true);

    status = parseAsDouble("-12345.0", 8, &dbl_val);
    EXPECT_DOUBLE_EQ(dbl_val, -12345.0);
    ASSERT_EQ(status, true);

    status = parseAsDouble("-123.d45", 8, &dbl_val);
    ASSERT_EQ(status, false);
}

TEST(NMEA_convertersTest, Convert2Kmph) {
    /* convert2Kmph */
    EXPECT_DOUBLE_EQ(convert2Kmph(10.0), 18.52);
}