#ifndef SENSOR_CALIBRATION_H
#define SENSOR_CALIBRATION_H

typedef struct {
    float scale_x;
    float scale_y;
    float scale_z;
    float offset_x;
    float offset_y;
    float offset_z;
} sensor_axis_calibration_t;

typedef struct {
    sensor_axis_calibration_t accel;
    sensor_axis_calibration_t gyro;
} sensor_calibration_t;

extern const sensor_calibration_t calibration;


#endif // SENSOR_CALIBRATION_H