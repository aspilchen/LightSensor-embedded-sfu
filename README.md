# Embedded Light Sensor
A project from CMPT433 (Embedded Systems) at SFU.

Embedded linux program for the Beaglebone micro computer. Reads current light levels in a room and records a short history of the number of dips
below the running average have occured for up to approx 10 seconds. Control over the size of history (determines how long the dip count is stored)
can be controlled using a dial on the Zen Cape. And uses a small segmented LED screen (like an alarm clock) to display the number of dips in history.

Interfacing with the device is done over UDP.

The dial and LED display are controlled directly via the circuits GPIO pins.

The light sensor is an external circuit containing a photoresistor (light sensitive resistor) wired to the Beaglebones A2D pins.

Images will be uploaded at some point.

## Please consider the following
Given that this is all for the sake of learning, I take advantage of course work to experiment with lots of different techniques/concepts 
to learn as much as I can during the process. However this can often result in some unfortunately messy/awkward code when it goes poorly.
And then I just do what I have to to make it perform the majority of required tasks since that is the main focus of SFU courses (quality is rarely considered).

I am trying to take full advantage of my current sitatution to do this, because it lets me learn a lot about what does/doesn't work in different scenarios and why.
But as I said, it can also result in some awkward/unsafe or just outright hacky code when it goes wrong. Which is great because I can look back at it and see directly
why things did/didn't work well. So please keep that in mind when reading through it.

I will be putting together some personal projects where I intend to be more careful with quality, and hopefully safer regarding bugs and such.

## Dependancies
- ARM compiler.
- Beaglbone with Zen Cape attechmant.

## Build Instructions
### Executable
- Run make to build the executable.
- Copy the LightSensor executable to the target device.

### Light sensing circuit
This will be updated at a later time.

## How to use
Once the light sensing circuit is made and connected. Simply run the program on the target device.

Interfacing with the device requires a UDP connection using port 12345. Once connected, it will begin printing updates of the current readings.

Controlling the device is done using the following commands over UDP.

| command   | description                                                   |
| ---       | ---                                                           |
| count     | display total number of samples taken.                        |
| length    | display number of samples in history (both max, and current). |
| history   | display the full sample history being saved.                  |
| get N     | display the N most recent history values.                     |
| dips      | display the number of dips.                                   |
| stop      | cause the server program to end.                              |
| \<enter\> | repeat last command.                                          |
| help      | display command menu.                                         |