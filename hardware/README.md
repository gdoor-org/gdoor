# Content
Receiving is most easiest: AC couple the bus to an GPIO, clamp the voltages with diodes to not stress the poor GPIO.
Afterwards just decode the pulse-trains.

## stm32-ugly
This folder contains my very ugly hardware schematic.
Following bugs:
1. The power from bus feature is not working, do not solder in R15 and just plug in a USB cable to power the board.
2. The Crystal osc. pinout is wrong!

In contrast to the more simpler solution this uses bandpass filters,
which work very well and reliable.
