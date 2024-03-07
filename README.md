# Welcome to GDOOR
<img src="https://raw.githubusercontent.com/gdoor-org/gdoor/main/doc/esp32-pcb.png" alt="3D Render of ESP32 adapterboard" width=300>
<img src="https://raw.githubusercontent.com/gdoor-org/gdoor/main/doc/esp32-schem.png" alt="Schematic of ESP32 adapterboard" width=300>

GDOOR is a project to collect and document findings about the Gira Door System.
Gira is a german manufacturer of house appliances, including door systems.
These systems are connected via a proprietary two wire bus system.

Contributions are welcome!

# Table of contents
- [Bus Adapter Hardware](#bus-adapter-hardware)
- [Bus Protocol](#bus-protocol)
  - [Hardware Layer](#hardware-layer)
  - [Digital Signaling](#digital-signaling)
  - [Bits and Bytes](#bits-and-bytes)
  - [Bus protocol / Frame](#bus-protocol--frame)
- [Messages](#messages)
  - [Open Door](#open-door)
  - [Call / Call button](#call--call-button)
  - [Accept call](#accept-call)
  - [Close call](#close-call)
- [CRC Routine - Dummycode](#crc-routine---dummycode)

# Bus Adapter Hardware
Currently work is ongoing to release
a bus ESP32 based adapter, which translates the bus messages to MQTT and vice versa.

Hardware can be found in the `hardware` subfolder,
corresponding firmware in the `firmware` subfolder.

Work is not finished yet.

# Bus Protocol
## Hardware Layer
The bus is formed by two wires and is single ended, meaning one of the wire carries
power and data and the other wire is ground.

To power the bus devices, a central device called controller powers the bus with ~26 Vdc.
Bus commands and analog data (voice and video!) are modulated onto this 26V.
The exact electrical scheme for the modulations is not known, but capacitive coupling the signals
on the 26 V seems to work.

Digital bus commands indicate the start and end of analog audio transmission,
each participating bus device decides on its own to:
- send audio on the bus
- receive audio from the bus

The outdoor station can be commanded to send audio to the bus, the indoor stations
will do this only if a user pressed the "call accept" button.

## Digital Signaling
![Example of Bus voltage](https://raw.githubusercontent.com/gdoor-org/gdoor/main/doc/busvoltage.png)

The digital bus signal is based on a non standardized modulation scheme, similar to OOK, but not quite OOK.
A ~ +-2V, 60 kHz sine carrier is turned on and off, where the duration of the carrier indicates a one or zero bit value. These pulses (aka bits) are separated with a fixed length pause (with no carrier signal).
So it is similar to OOK, but the bit period depends on the bit value.

All data transmission starts with a start bit which contains ~60 sine periods. A one is signaled by ~12 sine periods and a zero is signaled by ~32 sine periods.

## Bits and Bytes
![Bit stream order](https://raw.githubusercontent.com/gdoor-org/gdoor/main/doc/wavedrom-bitstream.png)

Data transmission is LSB first and each Byte has an additional 9th odd parity bit.
The last byte in the transmission is kind of a checksum, literally the sum of all previous bytes.

## Bus protocol / Frame
![Byte Frame](https://raw.githubusercontent.com/gdoor-org/gdoor/main/doc/wavedrom-byteframe.png)
The exact meaning of the bytes is unknown, but certain clues can be made by observing the bus and sending
to the bus and observing device behavior:

| Byte          | Description   |
| ------------- | ------------- |
| ?Length?      | Maybe frame length. Content is 0x01 if no destination fields in frame, otherwise 0x02  |
| ?Status?      | Unknown, fixed values for different commands  |
| Action        | The real command, like "open door", "accept call" etc.  |
| Source        | 3 Byte value with device bus address, unique per device!  |
| Parameter     | 2 Byte value which e.g. specifies pressed button  |
| Device Type   | Fixed value for each hardware device type  |
| Destination   | 3 Byte value with device bus address, unique per device!  |
| CRC           | Sum of all previous byte (8 bit, without parity bit) values |

### ?Length?
| Byte-Value    | Description   |
| ------------- | ------------- |
| 0x01          | Frame with 9 Bytes, no destination bytes |
| 0x02          | Frame with 12 Bytes, incl. destination bytes |

### ?Status?
| Byte-Value    | Description   |
| ------------- | ------------- |
| 0x00          | Often in combination with ?Length? = 0x02 |
| 0x10          | Often in combination with ?Length? = 0x01 |

### Action
| Byte-Value    | Description   |
| ------------- | ------------- |
| 0x00          | Programming mode - Stop|
| 0x01          | Programming mode - Start|
| 0x02          | Door opener programming - Stop|
| 0x03          | Door opener programming - Start|
| 0x04          | Learn doorbell button|
| 0x05          | Confirm learned doorbell button|
| 0x08          | Reset device configuration (announcement by device itself)|
| 0x0F          | Confirm learned door opener|
| 0x11          | Door bell button pressed - which button is specified in ?Param?|
| 0x13          | Floor bell button pressed|
| 0x20          | Close call|
| 0x21          | Open call|
| 0x31          | Open door|
| 0x41          | Generic button pressed|
| 0x42          | Unlearned button pressed|

### Source
3 Byte of device address

### Parameter
2 Bytes with parameters for the Action field.
E.g. a door station with multiple buttons encodes the pressed key number

### Type
| Byte-Value    | Description   |
| ------------- | ------------- |
| 0xA0          | Door station |
| 0xA1          | Indoor station|
| 0xA3          | Controller|

### Destination
3 Byte of device address

# Messages

## Open Door
0x02 0x00 0x31 src[0] src[1] src[2] 0x00 0x00 0xA0 dst[0] dst[1] dst[2]

The door opener with address dst ignores src (can be any byte values),
it also ignores the hardware type (0xA0).

## Call / Call button
0x01 0x10 0x11 doorstation[0] doorstation[1] doorstation[2] button 0xA0 0xA0

Doorstation button is 0x01, 0x02, 0x03 ...

## Accept call
0x02 0x00 0x21 indoor[0] indoor[1] indoor[2] 0x00 0x00 0xA1 doorstation[0] doorstation[1] doorstation[2]

Door station ignores indoor byte values and hardware type (0xA1).
As soon as door station receives this command, it sends analog audio onto the bus.

## Close call
0x02 0x00 0x20 indoor[0] indoor[1] indoor[2] 0x00 0x00 0xA1 doorstation[0] doorstation[1] doorstation[2]

Door station ignores indoor byte values and hardware type (0xA1).
As soon as door station receives this command, it stops analog audio onto the bus.

# CRC Routine - Dummycode

```C
uint8_t crc(uint8_t *command, uint8_t len) {
    uint8_t crc = 0;
    for(uint8_t i=0; i<len; i++) {
        crc += command[i];
    }
    return crc;
}
```
