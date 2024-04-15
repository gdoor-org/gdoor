# Integrate your Gira door system with Home Assistant

The following allows you to e.g.
- open the door via Home Assistant
- trigger notifications on mobile devices when a door bell button is pressed
- use any actions sent on the Gira bus to trigger automations

## Connect Gdoor with Home Assistant

There are two possibilities:

### USB / Serial connection

1. connect the Gdoor adapter to a USB port of the machine where Home Assistant is running
2. in `configuration.yaml`, add
    ```
    sensors:
      - platform: serial
        name: "Gdoor"
        serial_port: /dev/ttyUSB0 # change device if needed
        baudrate: 115200
    ```
   - if you're unsure under which device path the serial port got registered, go to Settings -> Logs and choose "Supervisor". Look for `[supervisor.hardware.monitor] Detecting add hardware /dev/ttyUSB`...
3. restart Home Assistant
4. press the door bell
5. press the "open door" button on your indoor station
6. open Logbook and search for "Gdoor", now you should see state changes like
   ```
   Gdoor changed to {"action": "BUTTON_RING", "parameters": "0360", "source": "A286FD", "destination": "000000", "type": "OUTDOOR", "busdata": "011011A286FD0360A04A"}
   ```
   Note the value of the `parameters` field: this is the unique value of your door bell which you can use to identify your door bell in automations.
7. look for the `DOOR_OPEN` action, copy the value of `busdata` and add the following shell command in `configuration.yaml` to be able to open the door programmatically. Replace `<busdata>` with the copied value.
    ```
    shell_command:
      gdoor_open_door: echo -e '<busdata>' > /dev/ttyUSB0 # change busdata and device if needed
    ```
8. restart Home Assistant


### MQTT

Work in progress


## Examples for typical use cases

### Open door via Home Assistant dashboard

Simply call the above defined service `shell_command.open_house_door` in e.g. a button card:

```
show_name: true
show_icon: true
type: button
tap_action:
  action: call-service
  service: shell_command.open_house_door
name: Haustür öffnen
icon: mdi:door-open
```

### Send notification to mobile devices on door/floor bell

Example automation:

```
alias: Türklingel
trigger:
  - platform: state
    entity_id:
      - sensor.gdoor
action:
  - choose:
      - conditions:
          - condition: state
            entity_id: sensor.gdoor
            attribute: action
            state: BUTTON_RING
          - condition: state
            entity_id: sensor.gdoor
            attribute: parameters
            state: "0360" # check in Logbook which parameter matches your door bell
        sequence:
          - service: notify.all_smartphones # adjust to your notification group/device
            data:
              data:
                push:
                  interruption-level: time-sensitive
              message: Türklingel (außen)
      - conditions:
          - condition: state
            entity_id: sensor.gdoor
            attribute: action
            state: BUTTON_FLOOR
          - condition: state
            entity_id: sensor.gdoor
            attribute: parameters
            state: "FF6F" # check in Logbook which parameter matches your floor bell
        sequence:
          - service: notify.all_smartphones # adjust to your notification group/device
            data:
              data:
                push:
                  interruption-level: time-sensitive
              message: Türklingel (innen)
mode: single
```