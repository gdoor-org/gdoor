# Gira Bus Protocol

Gira is a german manufacturer of house appliances, including door systems.
These systems are connected via a proprietary two wire bus system.

GDOOR is a project to collect and document findings about the Gira Door System, all figured out by reverse engineering.


# Table of contents
- [Gira Bus Protocol](#gira-bus-protocol)
- [Table of contents](#table-of-contents)
- [Bus Protocol](#bus-protocol)
  - [Hardware Layer](#hardware-layer)
  - [Digital Signaling](#digital-signaling)
  - [Bits and Bytes](#bits-and-bytes)
  - [Bus protocol / Frame](#bus-protocol--frame)
    - [?Length?](#length)
    - [?Status?](#status)
    - [Action](#action)
    - [Source](#source)
    - [Parameter](#parameter)
    - [Type](#type)
    - [Destination](#destination)
- [Bus Messages](#bus-messages)
  - [Open Door](#open-door)
  - [Ring button](#ring-button)
  - [Request audio](#request-audio)
  - [Request video](#request-video)
  - [End audio/video transmission](#end-audiovideo-transmission)
- [Bus CRC Routine - Dummycode](#bus-crc-routine---dummycode)

# Bus Protocol
## Hardware Layer
The bus is formed by two wires and is single ended, meaning one of the wire carries
power and data and the other wire is ground. For the GDOOR hardware adapter, the polarity doesn't matter.

To power the bus devices, a central device called controller powers the bus with ~26 Vdc.
Bus commands and analog data (audio and video!) are modulated onto this 26V.
The exact electrical scheme for the modulations is not known, but capacitive coupling the signals
on the 26 V seems to work.

Digital bus commands indicate the start and end of analog audio/video transmission,
each participating bus device decides on its own to:
- send audio/video on the bus
- receive audio/video from the bus

The outdoor station can be commanded to send audio/video to the bus, the indoor stations
will send audio only if a user pressed the "call accept" button.

## Digital Signaling
![Example of Bus voltage](busvoltage.png)

The digital bus signal is based on a non standardized modulation scheme, similar to OOK, but not quite OOK.
A ~ +-2V, 60 kHz sine carrier is turned on and off, where the duration of the carrier indicates a one or zero bit value. These pulses (aka bits) are separated with a fixed length pause (with no carrier signal).
So it is similar to OOK, but the bit period depends on the bit value.

All data transmission starts with a start bit which contains ~60 sine periods. A one is signaled by ~12 sine periods and a zero is signaled by ~32 sine periods.

## Bits and Bytes
![Bit stream order](wavedrom-bitstream.png)

Data transmission is LSB first and each Byte has an additional 9th odd parity bit.
The last byte in the transmission is kind of a checksum, literally the sum of all previous bytes.

## Bus protocol / Frame
![Byte Frame](wavedrom-byteframe.png)
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
| 0x12          | Internal call from one to another indoor station - which station is specified in ?Param?|
| 0x13          | Floor bell button pressed|
| 0x20          | End audio/video transmission|
| 0x21          | Request audio|
| 0x28          | Request video|
| 0x31          | Open door|
| 0x41          | Light button pressed|
| 0x42          | Generic button pressed - which button is specified in ?Param?|

see [mapping of actions in the firmware source code](../firmware/esp32/gdoor/src/gdoor_data.cpp).

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

# Bus Messages

## Open Door
| 0x02 | 0x00 | 0x31 | src[0] | src[1] | src[2] | 0x00 | 0x00 | 0xA0 | dst[0] | dst[1] | dst[2] |
| -----|------|------|--------|--------|--------|------|------|------|--------|--------|------- |

The door opener with address `dst` ignores `src` (can be any byte values),
it also ignores the hardware type (0xA0).
The indoor station sets `dst` to the outdoor station if there is an active audio/video transmission. Otherwise it sets the controller as `dst`.

## Ring button
| 0x01 | 0x10 | 0x11 | doorstation[0] | doorstation[1] | doorstation[2] | button | 0xA0 | 0xA0 |
| -----|------|------|----------------|----------------|----------------|--------|------|------|

Doorstation ring button is 0x01, 0x02, 0x03 ...

## Request audio
| 0x02 | 0x00 | 0x21 | indoor[0] | indoor[1] | indoor[2] | 0x00 | 0x00 | 0xA1 | door[0] | door[1] | door[2] |
| -----|------|------|-----------|-----------|-----------|------|------|------|---------|---------|-------- |

Door station ignores indoor byte values and hardware type (0xA1).
As soon as door station receives this command, it sends analog audio onto the bus.

## Request video
| 0x02 | 0x00 | 0x28 | indoor[0] | indoor[1] | indoor[2] | 0x00 | 0x00 | 0xA1 | door[0] | door[1] | door[2] |
| -----|------|------|-----------|-----------|-----------|------|------|------|---------|---------|-------- |

Door station ignores indoor byte values and hardware type (0xA1).
As soon as door station receives this command, it sends analog video onto the bus.

> [!NOTE]  
> The indoor station also sends this command when viewing a missed call or opening the menu.
> The parameters are different in this case.

## End audio/video transmission
| 0x02 | 0x00 | 0x20 | indoor[0] | indoor[1] | indoor[2] | 0x00 | 0x00 | 0xA1 | door[0] | door[1] | door[2] |
| -----|------|------|-----------|-----------|-----------|------|------|------|---------|---------|-------- |

Door station ignores indoor byte values and hardware type (0xA1).
As soon as door station receives this command, it stops analog audio/video onto the bus.

# Bus CRC Routine - Dummycode

```C
uint8_t crc(uint8_t *command, uint8_t len) {
    uint8_t crc = 0;
    for(uint8_t i=0; i<len; i++) {
        crc += command[i];
    }
    return crc;
}
```
