
// define the parameters of our machine.
#define X_STEPS_PER_INCH 400
#define X_STEPS_PER_MM   16.0
#define X_MOTOR_STEPS    200

#define Y_STEPS_PER_INCH 400.0
#define Y_STEPS_PER_MM   16.0
#define Y_MOTOR_STEPS    200

#define Z_STEPS_PER_INCH 400.0
#define Z_STEPS_PER_MM   16.0
#define Z_MOTOR_STEPS    200

//our maximum feedrates
#define FAST_XY_FEEDRATE 15.0
#define FAST_Z_FEEDRATE  15.0

// Units in curve section
#define CURVE_SECTION_INCHES 0.019685
#define CURVE_SECTION_MM 0.5

// Set to one if sensor outputs inverting (ie: 1 means open, 0 means closed)
// RepRap opto endstops are *not* inverting.
#define SENSORS_INVERTING 0

/****************************************************************************************
* digital i/o pin assignment
*
* this uses the undocumented feature of Arduino - pins 14-19 correspond to analog 0-5
****************************************************************************************/

//camera shutter and control pins
#define CAM_SHUTTER_PIN1 11
#define CAM_SHUTTER_PIN2 12
#define CAM_AUX_PIN1 14 // analog 0
#define CAM_AUX_PIN2 15 // analog 1
#define CAM_AUX_PIN3 16 // analog 2
#define CAM_AUX_PIN4 17 // analog 3

// stepper driver pins
#define X_STEP_PIN 2
#define X_DIR_PIN 3
#define X_ENABLE_PIN 4

#define Y_STEP_PIN 5
#define Y_DIR_PIN 6
#define Y_ENABLE_PIN 7

#define Z_STEP_PIN 8
#define Z_DIR_PIN 9
#define Z_ENABLE_PIN 10

// limits not used right now
#define X_MIN_PIN 19
#define X_MAX_PIN 19
#define Y_MIN_PIN 19
#define Y_MAX_PIN 19
#define Z_MIN_PIN 19
#define Z_MAX_PIN 19











