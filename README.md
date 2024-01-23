# Additional Parts

- [Big Tree Tech HDMI5 Mount](https://www.teamfdm.com/files/file/618-mount-for-bigtreetech-hdmi5-screen-trident-voron-2xxx/)

# Z Calibration

using 0.15mm feeler gauge

```
G28
BED_MESH_CLEAR 
QUAD_GANTRY_LEVEL
G28
G1 X150 Y150 Z1 F6000
Z_ENDSTOP_CALIBRATE
```

# Input Shaping

- https://mellow.klipper.cn/#/advanced/usb_adxl
- https://github.com/VoronDesign/Voron-Stealthburner/blob/main/STLs/Stealthburner/ADXL345_Mounts/sb_adxl_mount_generic_15.5mm_c_c.stl

# Skew Calibration

- https://www.thingiverse.com/thing:2972743/files
- https://www.klipper3d.org/Skew_Correction.html

# Upgrade Klipper Firmware

```
sudo service klipper stop
cd ~/klipper
make clean
make menuconfig
make
make flash FLASH_DEVICE=/dev/serial/by-id/$(ls /dev/serial/by-id/ | grep stm32f446)
sudo service klipper start
```

# Info

- Voron parts were printed with ABS+Glass Fiber filament from Phaetus

# Notifications

- [Mooncord](https://eliteschw31n.gitbook.io/mooncord/)

# Print Settings

- Printer -> Machine G-code
  - Matchine start G-code (add): `PRINT_START EXTRUDER=[nozzle_temperature_initial_layer] BED=[bed_temperature_initial_layer_single] Chamber=[chamber_temperature] PRINT_MIN={first_layer_print_min[0]},{first_layer_print_min[1]} PRINT_MAX={first_layer_print_max[0]},{first_layer_print_max[1]}`
  - Layer change G-code: `M117 Layer {layer_num+1}/[total_layer_count] : {filament_settings_id[0]}`
- Layer height: 0.2 mm
- Line width -> Default: 0.4 mm
- Infill percentage: 40%
- Infill type: grid
- Wall count: 4
- Solid top/bottom layers: 5
- Supports: NONE

# Troubleshooting

## Klipper connection

```
journalctl -u klipper.service
# if c_helper.so
rm -rf /home/biqu/klipper/klippy/chelper/c_helper.so
```
