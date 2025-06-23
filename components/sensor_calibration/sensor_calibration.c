#include "sensor_calibration.h"

const sensor_calibration_t calibration = {
    .accel = {
        .scale_x = 1.0f,
        .scale_y = 1.0f,
        .scale_z = 1.0f,
        .offset_x = 0.0f,
        .offset_y = 0.0f,
        .offset_z = 0.0f
    },
    .gyro = {
      .scale_x = 1.0f,
      .scale_y = 1.0f,
      .scale_z = 1.0f,
      .offset_x = 0.00f,
      .offset_y = 0.00f,
      .offset_z = 0.00f
    }
};