# Additional Parts

- [Big Tree Tech HDMI5 Mount](https://www.teamfdm.com/files/file/618-mount-for-bigtreetech-hdmi5-screen-trident-voron-2xxx/)

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
- Ironing type: All top surfaces
