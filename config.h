#ifndef CONFIG_H_
#define CONFIG_H_

#include <Arduino_CAN.h>

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
    struct {
      // Id of the preceding message to fabricate
      uint16_t id;
      // Dlc of the preceding message to fabricate
      uint8_t dlc;
    } preceding;
    struct {
      // Id of the message of the victim
      uint16_t id;
      // Dlc of the message of the victime
      uint8_t dlc;
    } tampered;
  } messages;
  struct {
    // Number of samples to acquire to obtain period and jitter of the victim
    uint32_t samples;
    // Maximum synchronisation tries to acquire the message
    uint32_t tries;
    // Percentage of the message duration to "go back" during transmission of the preceding message (must be > 100%)
    uint8_t scale;
  } sync;
  // Number of attacks to carry out before terminating
  uint32_t attacks;
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
      }
    },
    .sync = {
      .samples = 5000,
      .tries = 1000,
      .scale = 120 
    },
    .attacks = 100000
};
#endif