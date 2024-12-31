# WeepingCAN
An implementation of the WeepingCAN attack (_Bloom, Gedare. "WeepingCAN: A stealthy CAN bus-off attack." Workshop on Automotive and Autonomous Vehicle Security. 2021., [link](https://par.nsf.gov/servlets/purl/10283597)_) on a Formula Student car from the Università degli Studi di Padova.

The branch [busoff](https://github.com/tommasoprandin/WeepingCAN/tree/busoff) contains the code for the bus-off attack, while the [weeping-can](https://github.com/tommasoprandin/WeepingCAN/tree/weeping-can) contains the code for the WeepingCAN attack.

## Hardware

The hardware used is a PC running the serial logger and monitor from the [Arduino 2 IDE](https://www.arduino.cc/en/software), and the IDE itself for programming the boards.

The car is the RGe-07 from the Formula Student Team of the Università degli Studi di Padova (RaceUP team).

## Setup

The following is the setup used to demonstrate the attack:
![setup-diagram](diagrams/svg/configuration.svg)

## Application

The application architecture is split into three parts:

1. Synchronization
2. Frame crafting
3. Attack

### Synchronisation

In this section the program will try to synchonise with the preceding message specified by the configuration file. In this way it is possible to send the poisoned message at the right time (same of the victim), and thus increasing dramatically the chance of success.

It is supposed that the `dbc` file describing the car's bus messages is available, in this case finding the preceding message is trivial.

### Frame crafting

Now it is possible to craft the message so to carry out the attack

In the case of the traditional _bus-off_ attack it is necessary to send the same message of the victim up until a certain randomic bit that should be dominant on the malicious message, while recessive on the victim's one. We can do it naively by setting all the data bits to zero, making the attack highly detectable, or in a more sophisticated way by predicting the message and randomically chosing a position to make the change.

For the _WeepingCAN_ attack instead we need to do the contrary, so instead the malicious message should be identical to the victim's one, except for a random position where the attacker's one is recessive instead of dominant (1 instead of 0). This is a crucial step to vastly reduce the detectability of the attack.

### Attack

The final attack step is aimed at putting the victim in the _bus-off_ state, thus preventing it from receiving and sending frames.

In the _bus-off_ scheme the attacker sends the malicious message crafted before, synchronising with the victim's one. By construction of the message this will trigger a transmission bit error on both sides and a subsequent chain of retransmissions. After 16 transmission they both will be in _error-passive_. On the next retransmission the attacker will win since it has a dominant bit and, since the victim will be in _error-passive_ its error flag will be overwritten. At this point the attacker will succeed with the transmission, decreasing its `TEC`, while the victim's will keep increasing until _bus-off_.

On the _WeepingCAN_ attack, in order to decrease the detectability, there are more steps: after crafting the malicious frame we need to disable frame retransmission on the CAN interface, this will prevent the chain of retransmission seen on the simple attack, thus making it stealthier. Then the crafted message is sent, causing a transmission error on both the attacker and the victim. The victim then will successfully retry the transmission. The attack will work only if we can decrease the `TEC` of the attacker fast enough to send the victim in _bus-off_ before of it. This is only possible by sending in between attacks many frames and/or skipping some attack frames.

