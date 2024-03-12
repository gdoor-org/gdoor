# Content
> [!IMPORTANT]  
> Work in progress.
> The current hardware as shown here has a hardware bug:
> The high/low logic values of the ESP32 on the bus data receiving side are not
> respected. A comparator on Pin28 (GPIO32) is needed.
>
> The current firmware therefore decodes on Pin9 (IO12),
> as a manual hardware modification was added.

Simple adapter board for ESP32mini.
There are production files to get nearly fully assembled boards from http://jlcpcb.com under `esp32/production`.
A very rough guide how to order on http://jlcpcb.com is under https://www.youtube.com/watch?v=Y2G-SiKUJPk.

Additionally you will need:
- ESP32 Mini. E.g. from https://www.az-delivery.de/products/esp32-d1-mini
- 2x Stiftleiste 2,54mm 2x10 to solder into the adapter and connect the adapter PCB to the ESP32 Mini. E.g. https://www.reichelt.de/stiftleisten-2-54-mm-2x10-gerade-mpe-087-2-020-p119898.html

![Schematic, PDF version in esp32 subfolder](https://raw.githubusercontent.com/gdoor-org/gdoor/main/doc/esp32-schem.png)

![3D Render of ESP32 adapterboard](https://raw.githubusercontent.com/gdoor-org/gdoor/main/doc/esp32-pcb.png)

# Installation / Flash
You will need PlatformIO (https://platformio.org/).
You can use the tutorial https://docs.platformio.org/en/stable/tutorials/espressif32/espidf_debugging_unit_testing_analysis.html
as a baseline.
