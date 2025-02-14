mkdir -p functions/ecm.usb0

# need MAC addresses, use the serial number
serial=$(cat /sys/firmware/devicetree/base/serial-number)
serial_pt1=$(echo "$serial" | cut -d- -f2)
serial_pt2=$(echo "$serial" | cut -d- -f3)

# serial part 1 is 8 characters, 4 hex digits all packed
# serial part 2 is 6 characters, 3 hex digits all packed

host_mac=$(printf "02:00:%02x:%02x:%02x:%02x" \
	$((0x${serial_pt1:0:2})) \
	$((0x${serial_pt1:2:2})) \
	$((0x${serial_pt1:4:2})) \
	$((0x${serial_pt1:6:2})))

dev_mac=$(printf "02:00:%02x:%02x:%02x:%02x" \
	$((0x${serial_pt2:0:2})) \
	$((0x${serial_pt2:2:2})) \
	$((0x${serial_pt2:4:2})) \
	$((0x${serial_pt2:6:2})))

echo $host_mac > functions/ecm.usb0/host_addr
echo $dev_mac > functions/ecm.usb0/dev_addr

ln -s functions/ecm.usb0 configs/c.1/
