# Wii-Linux Switch Companion - USB Gadget setup
# Version 0.2
# Copyright (C) 2024-2025 Techflash

SCRIPT_BASE=$PWD
export SCRIPT_BASE

echo "Making gadget..."
mkdir -p /sys/kernel/config/usb_gadget/$gadget_name
cd /sys/kernel/config/usb_gadget/$gadget_name

echo "Setting ids..."
echo 0x1d6b > idVendor
echo 0x0104 > idProduct
echo 0x0100 > bcdDevice
echo 0x0200 > bcdUSB

echo "Setting strings..."
mkdir -p strings/0x409
cat /sys/firmware/devicetree/base/serial-number > strings/0x409/serialnumber
echo "Techflash" > strings/0x409/manufacturer
echo "Wii-Linux Switch Companion ($configs)" > strings/0x409/product

echo "Setting configs..."
mkdir -p configs/c.1/strings/0x409
echo "Config 1: Multifunction composite gadget" > configs/c.1/strings/0x409/configuration
echo 500 > configs/c.1/MaxPower

echo "Making function..."

finalize() {
	time=1
	echo "Applying..."
	ls /sys/class/udc > UDC
	sleep 1

	while [ "$(cat UDC)" = "" ]; do
		echo "Failed, trying again..."
		echo "$(ls /sys/class/udc)" > UDC
		sleep $time
		time=$((time * 2))
		echo "Will retry in $time sec"
	done

	echo "Successfully bound to $(cat UDC)"
}
