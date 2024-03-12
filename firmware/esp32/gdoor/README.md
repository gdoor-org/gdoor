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

On each bus communication you will receive a JSON formatted string with the following fields:
- data: The bus data received. Array of bytes.
- raw: The raw counting values of each high/low bit. Array of bytes where each byte represents the number of pulses of a individual bit.
- hexvalue: This is the same value as data, but formatted, so that you can copy and paste it to the serial terminal,
  to send this bus data back on the bus.

If you want to send to the bus, just send to the serial port:
- Send hex values
- Each hex value needs to be 2 digit, in case of values < 0x1F, add a leading zero.
  E.g. 0x0F, 0x00, 0x02. Wrong would be: 0xF, 0x0, 0x2.
- Do not use 0x prefix.
- Do not separate the values with whitespaces, tabs or commas
- Send the values in one go, e.g.: 02A300452F

> [!TIP]
> If you use a terminal and want to do it by hand, use copy and paste to send in one go