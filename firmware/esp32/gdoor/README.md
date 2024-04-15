# ESP32 Firmware
This firmware is working with the modified hardware.

> [!NOTE]  
> Work in progress.
> The current hardware as shown here has a hardware bug:
> The high/low logic values of the ESP32 on the bus data receiving side are not
> respected. A comparator on Pin28 (GPIO32) is needed.
>
> The current firmware therefore decodes on Pin9 (IO12),
> as a manual hardware modification was added.

## Installation / Flash
You will need PlatformIO (https://platformio.org/).
You can use the tutorial https://docs.platformio.org/en/stable/tutorials/espressif32/espidf_debugging_unit_testing_analysis.html
as a baseline.

## Usage
Currently only the serial port is implemented.
Connect to the port with baudrate 115200.

### Serial Commands
The following serial commands can be send to the bus adapter:
- `*debug`: Activate debug mode with more verbose output
- `*normal`: Switch back to normal mode, with limited output

### Receive bus events/commands
On each bus communication you will receive a JSON formatted string with the following fields:
- action: The action of the bus message like e.g. `DOOR_OPEN`. For details please look into [gdoor_data.cpp](src/gdoor_data.cpp).
- type: The hardware type of the message sender. For details please look into [gdoor_data.cpp](src/gdoor_data.cpp).
- source: Bus address of message sender, 3 Bytes.
- destination: Bus address of message destination, 3 Bytes. This field is not contained in each bus message in such cases it is set to 0.
- parameters: Parameter values depend on action and bus structure. E.g. it can be the ID of which button was pressed, 2 Bytes.
- busdata: Bus data as hex value. You can send this value back to the serial port to duplicate a bus message.
- event_id: Counter value. It counts up for each new bus message.

If you switch on debug mode, you will also receive:
- raw: The raw counting values of each high/low bit. Array of bytes where each byte represents the number of pulses of a individual bit.
- busdata: Bus data as hex value.
- valid: 1 if checksum and parity bits passed the check.

### Send bus events/commands
If you want to send to the bus, just send to the serial port:
- Send hex values
- Each hex value needs to be 2 digit, in case of values < 0x1F, add a leading zero.
  E.g. 0x0F, 0x00, 0x02. Wrong would be: 0xF, 0x0, 0x2.
- Do not use 0x prefix.
- Do not separate the values with whitespace, tabs or commas
- Send the values in one go, e.g.: 02A300452F
  Do not pause the transmission in between characters!

> [!TIP]
> You can use the field `busdata` of a received bus message to duplicate this bus message back to the bus.
> E.g. Useful to easily open the door.

> [!TIP]
> If you use a terminal and want to do it by hand, use copy and paste to send in one go