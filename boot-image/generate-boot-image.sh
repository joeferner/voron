#!/bin/bash
set -e
set -u

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

function usage {
  echo "$0 <splash image> <resolution>"
  echo ""
  echo "<splash image>  PNG you would like to display on boot"
  echo "<resolution>    Resolution of screen (eg 800x480)"
}

if [ "$#" -ne 2 ]; then
  usage
  exit 1
fi

LOGO=$1
RESOLUTION=$2

if ! command -v identify &> /dev/null; then
  echo "'identify' not found. Did you install imagemagick?"
  echo "  sudo apt-get install imagemagick"
  exit 1
fi
if ! command -v convert &> /dev/null; then
  echo "'convert' not found. Did you install imagemagick?"
  echo "  sudo apt-get install imagemagick"
  exit 1
fi
if [ ! -f "${SCRIPT_DIR}/bootsplash-packer.c" ]; then
  echo "missing bootsplash-packer.c script"
  exit 1
fi

if [ ! -f "${SCRIPT_DIR}/bootsplash-packer" ]; then
  gcc bootsplash-packer.c -o "${SCRIPT_DIR}/bootsplash-packer"
fi

chmod +x "${SCRIPT_DIR}/bootsplash-packer"

THROBBER=spinner.gif
THROBBER_WIDTH=$(identify $THROBBER | head -1 | cut -d " " -f 3 | cut -d x -f 1)
THROBBER_HEIGHT=$(identify $THROBBER | head -1 | cut -d " " -f 3 | cut -d x -f 2)
convert -resize "${RESOLUTION}" "${LOGO}" /tmp/logo.png
LOGO_WIDTH=$(identify /tmp/logo.png | cut -d " " -f 3 | cut -d x -f 1)
LOGO_HEIGHT=$(identify /tmp/logo.png | cut -d " " -f 3 | cut -d x -f 2)
convert -alpha remove -background "#000000" /tmp/logo.png /tmp/logo.rgb
rm -rf /tmp/throbber*.rgb
convert -alpha remove -background "#000000" "${THROBBER}" /tmp/throbber%02d.rgb
"${SCRIPT_DIR}/bootsplash-packer" \
  --bg_red 0x00 \
  --bg_green 0x00 \
  --bg_blue 0x00 \
  --frame_ms 48 \
  --picture \
  --pic_width "${LOGO_WIDTH}" \
  --pic_height "${LOGO_HEIGHT}" \
  --pic_position 0 \
  --blob /tmp/logo.rgb \
  --picture \
  --pic_width $THROBBER_WIDTH \
  --pic_height $THROBBER_HEIGHT \
  --pic_position 0x05 \
  --pic_position_offset 200 \
  --pic_anim_type 1 \
  --pic_anim_loop 0 \
  $(ls /tmp/throbber*.rgb | sort -n | xargs -I xxx echo "--blob xxx") \
  bootsplash.armbian

echo ""
echo "Complete!"
echo ""
echo "cp /lib/firmware/bootsplash.armbian /lib/firmware/bootsplash.armbian.backup"
echo "sudo cp bootsplash.armbian /lib/firmware/bootsplash.armbian"
echo "sudo update-initramfs -c -k \$(uname -r)"
echo ""

