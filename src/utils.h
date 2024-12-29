#ifndef UTILS_H_
#define UTILS_H_

#include <math.h>
#include <Arduino.h>
#include <Arduino_CAN.h>

// CAN Receive Error Counter Register
#define RECR 0x4005084E
// CAN Transmit Error Counter Register
#define TECR 0x4005084F
// CAN Message Control Register for Transmit Mailbox 0
#define MCTL_TX0 0x40050820

uint64_t msg_duration_micros(const uint32_t bps, const uint8_t dlc);

template<typename T>
T avg(const T data[], const unsigned int len) {

  if (len == 0) return 0;

  T counter = {};
  for (unsigned int i = 0; i < len; i++) {
    counter += data[i];
  }

  return counter / len;

}



template<typename T>
T stddev(const T data[], const unsigned int len) {

  if (len == 0) return 0;

  auto mu = avg(data, len);

  T var = {};
  for (unsigned int i = 0; i < len; i++) {
    auto delta = data[i] - mu;
    var += delta * delta;
  }
  var /= len;

  return floor(sqrt((var)));

}

bool synchronise(uint16_t can_id, int max_tries);

void attack(CanMsg &preceding, CanMsg &tampered);

template<typename T>
T scalei(T value, uint32_t num, uint32_t den) {
    if (den == 0) return 0;
    return (value * num) / den;
}

template<typename T>
T scaleperci(T value, uint8_t perc) {
  return scalei(value, perc, 100);
}

void log_error_counters();

void send_traffic(const CanMsg messages[], uint32_t len);


#endif