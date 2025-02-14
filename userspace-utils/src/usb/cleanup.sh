name=ramdisk

# clean up usb ethernet
killall dnsmasq
iptables -t nat -F
iptables -F


cd /sys/kernel/config/usb_gadget/ramdisk
echo "" > /sys/kernel/config/usb_gadget/$name/UDC
find configs -type l -exec rm -v {} \;
find configs -name 'strings' -exec rmdir -v {}/0x409 \;
ls -d configs/* | xargs rmdir -v
ls -d strings/* | xargs rmdir -v
ls -d functions/* | xargs rmdir -v
cd ~
rmdir /sys/kernel/config/usb_gadget/$name


