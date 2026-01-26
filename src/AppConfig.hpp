#pragma once
// Global app config for debug flags
// Set to 1 to enable verbose NVS logging in StorageManager
#ifndef MODE_DEBUG
#define MODE_DEBUG 1
#endif

// Encoder configuration (shared across controller and input manager)
#ifndef ENC_MIN
#define ENC_MIN -32768
#endif
#ifndef ENC_MAX
#define ENC_MAX 32767
#endif

// Encoder acceleration thresholds
#ifndef ENCODER_ACCEL_THRESH_SLOW
#define ENCODER_ACCEL_THRESH_SLOW 100
#endif
#ifndef ENCODER_ACCEL_THRESH_FAST
#define ENCODER_ACCEL_THRESH_FAST 40
#endif
