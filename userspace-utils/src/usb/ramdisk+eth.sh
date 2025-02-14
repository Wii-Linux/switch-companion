configs="RAMDisk, USB Ethernet"
gadget_name="ramdisk+eth"
. usb.sh
. $SCRIPT_BASE/configs/com.sh
. $SCRIPT_BASE/configs/ramdisk.sh
. $SCRIPT_BASE/configs/ethernet.sh

finalize
