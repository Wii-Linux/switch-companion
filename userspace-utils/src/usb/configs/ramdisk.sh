mkdir -p functions/mass_storage.usb0
echo 0 > functions/mass_storage.usb0/lun.0/cdrom
echo 0 > functions/mass_storage.usb0/lun.0/ro

ln -s functions/mass_storage.usb0 configs/c.1/

echo "Making disk image..."
if [ -z "$diskSz" ]; then
	diskSz=2G
fi
rm -f /tmp/swapdisk.img 2>/dev/null || true
fallocate -l $diskSz /tmp/swapdisk.img
echo /tmp/swapdisk.img > functions/mass_storage.usb0/lun.0/file
