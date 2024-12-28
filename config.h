#ifndef CONFIG_H_
#define CONFIG_H_

#include <Arduino_CAN.h>

#define TRAFFIC_MSGS 4

struct MessageData {
  uint16_t id;
  uint8_t dlc;
};

struct Config {
  struct {
    // Serial baudrate in baud/s
    uint32_t baud;
  } serial;
  struct { 
    // Can bitrate enum
    CanBitRate bitrate;
  } can;
  struct {
    MessageData preceding;
    MessageData tampered;
    MessageData traffic[TRAFFIC_MSGS];
    uint8_t traffic_msgs;
  } messages;
  struct {
    // Number of samples to acquire to obtain period and jitter of the victim
    uint32_t samples;
    // Maximum synchronisation tries to acquire the message
    uint32_t tries;
    // Percentage of the message duration to "go back" during transmission of the preceding message (must be > 100%)
    uint8_t scale;
  } sync;
  // Number of iterations to carry out before terminating
  uint32_t iter;
  // Iterations skipped in attack loop (higher increases attack's chances but also the time required)
  uint32_t skip;
  // Transmission period of "normal" messages (to reduce TEC) in ms
  uint32_t period;
};

constexpr Config CONFIG = {
    .serial = {
      .baud = 115200
    },
    .can = {
      .bitrate = CanBitRate::BR_500k
    },
    .messages = {
      .preceding = {
        .id = 258,
        .dlc = 3
      },
      .tampered = {
        .id = 260,
        .dlc = 3
      },
      .traffic = {
        { .id = 0x10, .dlc = 4},
        { .id = 0x11, .dlc = 4},
        { .id = 0x12, .dlc = 4},
        { .id = 0x13, .dlc = 4},
      },
      .traffic_msgs = TRAFFIC_MSGS
    },
    .sync = {
      .samples = 5000,
      .tries = 1000,
      .scale = 120 
    },
    .iter = 1000000,
    .skip = 100,
    .period = 1
};
#endif