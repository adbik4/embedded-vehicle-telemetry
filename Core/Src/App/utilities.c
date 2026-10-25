#include "utilities.h"

#include <math.h>

void Quaternion2Euler(float *q, float *out)
{
    // Roll (x-axis rotation)
    out[0] = atan2(2 * (q[3] * q[0] + q[1] * q[2]), 1 - 2 * (q[0]*q[0] + q[1]*q[1]));
    
    // Pitch (y-axis rotation)
    out[1] = asin(2 * (q[3] * q[1] - q[2] * q[0]));
    
    // Yaw (z-axis rotation)
    out[2] = atan2(2 * (q[3] * q[2] + q[0] * q[1]), 1 - 2 * (q[1]*q[1] + q[2]*q[2]));
}