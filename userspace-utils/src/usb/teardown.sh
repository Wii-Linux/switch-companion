# Wii-Linux Switch Companion - USB Gadget teardown
# Version 0.1
# Copyright (C) 2024-2025 Techflash

# remove anything keeping the gadget up
echo "Cleaning up USB state..."
cd /sys/kernel/config/usb_gadget/* || exit 0
find configs -type l -exec rm {} \;
find configs -name 'strings' -exec rmdir {}/0x409 \;
ls -d configs/* | xargs rmdir
ls -d strings/* | xargs rmdir
ls -d functions/* | xargs rmdir
cd /
rmdir /sys/kernel/config/usb_gadget/*

# clean up ramdisk stuff
rm -f /tmp/swapdisk.img 2>/dev/null || true
