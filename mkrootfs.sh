#!/bin/sh
# Wii-Linux Switch Companion build script
# Copyright (C) 2024-2025 Michael "Techflash" Garofalo
#
# Licensed under the GNU GPLv2.

set -o pipefail
set -e

# download info
alpine_rel="3.22.0"
alpine_rel_short="v3.22"
rootfs_filename="alpine-minirootfs-$alpine_rel-aarch64.tar.gz"
dl_url="https://dl-cdn.alpinelinux.org/alpine/$alpine_rel_short/releases/aarch64/$rootfs_filename"

# packages to install
apk_packages=""
add_pkg() {
	apk_packages="$apk_packages $*"
}

# necessary to have a functional init and services
add_pkg openrc

# networking
add_pkg networkmanager networkmanager-cli networkmanager-tui networkmanager-wifi wpa_supplicant

# firmware necessary for the networking
add_pkg linux-firmware-brcm

# have a getty capable of autologin
add_pkg agetty

# to be able to setup udevd
add_pkg alpine-conf

# for compatiblity with Tegra BSP
add_pkg gcompat

# for debugging
add_pkg evtest

base="$(dirname -- "$( readlink -f -- "$0"; )")"
echo "Base directory: $base"
cd "$base"

# we must be root to properly extract the rootfs
if [ "$(id -u)" -ne 0 ]; then
	echo "This script must be run as root!"
	exit 1
fi

# Ensure download folder exists
mkdir -p downloads

# ensure userspace is built (and ARM cross compiler is downloaded)
./build-userspace.sh || {
	echo "Failed to build userspace!"
	exit 1
}

# Download rootfs hash
if ! [ -f "downloads/${rootfs_filename}.sha256" ]; then
	wget "${dl_url}.sha256" -P downloads || {
		echo "Failed to download rootfs hash - check your internet connection!"
		exit 1
	}
fi

# Download rootfs
if ! [ -f "downloads/${rootfs_filename}" ]; then
	wget "${dl_url}" -P downloads || {
		echo "Failed to download rootfs - check your internet connection!"
		rm -f "downloads/${rootfs_filename}.sha256"
		exit 1
	}
fi

# Check hash
cd downloads
sha256sum -c "${rootfs_filename}.sha256" || {
	echo "Hash check failed - check your internet connection!"
	rm -f "${rootfs_filename}" "${rootfs_filename}.sha256"
	exit 1
}

# make sure the rootfs folder exists
cd "$base"
mkdir -p rootfs

# extract the rootfs using some safe args to ensure that
# all necessary permissions, xattrs, acls, etc, are preserved
tar -xvP --acls --xattrs --same-owner --same-permissions --numeric-owner --sparse -f "${base}/downloads/${rootfs_filename}" -C "${base}/rootfs"

# grab Tegra BSP from gman's Noble repo
# Some of it, like graphics acceleration won't work under Alpine,
# since it requires glibc, and is generally only compatible with Ubuntu,
# or at least other Debian-based distros.  Thankfully, we don't
# need any of that here.
#
# These don't have any hashes, so we just have to assume that they're probably valid.
# Also, due to the fact that these are direct links to the GitHub repo, and the old versions
# are removed from the tree upon a new release, we use permalinks (commit hashes) to ensure
# that the link never dies.
commit_hash="a655ef091afd0f90db3ab8ee5f974d82dd4b49e6"
gman_url="https://github.com/theofficialgman/l4t-debs/raw/$commit_hash/pool"

cd "$base/downloads"
mkdir -p "bsp"
# small wrapper function to download (if necessary) and extract the .debs
deb_dl_extract() {
	if ! [ -f "$(basename "$1")" ]; then
		wget "$gman_url/$1" || {
			echo "Failed to download $gman_url/$1 - check your internet connection!"
			rm -f "$(basename "$1")"
			exit 1
		}
	fi

	ar x "$(basename "$1")"
	tar -xf data.tar.* -C "$base/rootfs"
	rm -f data.tar.* control.tar.* debian-binary
}

# small wrapper function to make this even more streamlined for some packages
nv_l4t_deb_dl_extract() {
	deb_dl_extract "main/n/$1/${1}_32.3.1-20191209225816_arm64.deb"
}
nv_l4t_deb_dl_extract "nvidia-l4t-core"
nv_l4t_deb_dl_extract "nvidia-l4t-init"
nv_l4t_deb_dl_extract "nvidia-l4t-firmware"
nv_l4t_deb_dl_extract "nvidia-l4t-configs"

# get Switch-specific packages
deb_dl_extract "noble/s/switch-l4t-configs/switch-l4t-configs_1.5-18+24.04_all.deb"
deb_dl_extract "noble/s/switch-bsp/switch-bsp_5.1.2-26+24.04_all.deb"

# hack up the Noble bootstack for our use
cd "$base/rootfs/opt/switchroot"

# replace the ini
rm L4T-noble.ini
cat << EOF > wii-linux-switch-companion.ini
[Wii-Linux Switch Companion]
l4t=1
boot_prefixes=/switchroot/wii-linux-switch-companion/
r2p_action=self
icon=switchroot/wii-linux-switch-companion/icon_wii-linux_hue.bmp
logopath=switchroot/wii-linux-switch-companion/bootlogo_wii-linux.bmp
bootargs_extra="fbcon=rotate:3 ro quiet"
uart_port=3
EOF

cd bootstack

# delete the old icon and logo
rm ./*_ubuntu*.bmp

# replace them with new ones
cp "$base/files/switchroot/wii-linux-switch-companion/"* ./



# hack x2 - repacking the script with these changes somehow makes it
# not work.....
#
# hack the boot.scr to only ever allow booting from the WL-SWCOMP id
# rather than falling back to /dev/mmcblk0p2

# HACK!  `dumpimage` doesn't seem to want to work with this type
# of Image, so just strip out the headers by hand...
tail -c+73 boot.scr > boot.scr.txt
tail -c+65 initramfs > initramfs.cpio.gz

# patch the boot script
patch --no-backup -p1 < "$base/patches/boot.scr.patch" || {
	echo "failed to apply patch - outdated?"
	exit 1
}


# extract the initramfs
mkdir -p extracted
bsdtar -xf initramfs.cpio.gz -C extracted

# patch the initramfs
patch --no-backup -p1 < "$base/patches/initramfs.patch" || {
	echo "failed to apply patch - outdated?"
	exit 1
}

# remove boot logo (makes the console look weird)
rm extracted/logo*

# repack the initramfs
cd extracted
find . | cpio -H newc -o | gzip -9 > ../initramfs.cpio.gz
cd ..
mkimage -A arm64 -T ramdisk -O linux -C gzip -d initramfs.cpio.gz initramfs

# repack the boot script
mkimage -A arm -T script -O linux -C none -d boot.scr.txt boot.scr

# delete temp files
rm -rf boot.scr.txt boot.scr.txt.gz boot.scr.txt.orig extracted initramfs.cpio.gz

# copy the modules tarball out, then nuke it (we're putting it back later)
mkdir -p "$base/out"
cp ../modules.tar.gz "$base/out/"
rm -f "../modules.tar.gz"

# clean up unnecessary stuff
cd "$base"
rm -rf "rootfs/usr/src/linux"*      # headers
rm -rf "rootfs/etc/X11"             # X11
rm -rf "rootfs/etc/xdg"
rm -rf "rootfs/etc/NetworkManager"  # NetworkManager
rm -rf "rootfs/etc/update-manager"  # Ubuntu update manager
mv "rootfs/etc/systemd/"*.sh "rootfs/etc/nv/"

# patch nvidia scripts
cd "rootfs/etc/nv"
rm -f nvfirstboot nvfb* nvweston.sh nvmemwarning.sh nvgetty.sh nvwifibt*
patch -p1 < "$base/patches/nvidia_scripts.patch" || {
	echo "failed to apply patch - outdated?"
	exit 1
}
cd -

# set up some sane defaults out of the chroot

# copy setup-alpine answers
# includes hostname and dns server
cp "$base/files/setup-alpine_answers.txt" "rootfs/"

# set a temporary DNS server so we can download packages
echo 'nameserver 1.1.1.1' > rootfs/etc/resolv.conf

# add modules
echo 'brcmfmac' >> rootfs/etc/modules

# enable tty1 and serial ports autologin
sed -i 's|tty1::respawn:/sbin/getty 38400 tty1|tty1::respawn:/sbin/agetty --autologin root 38400 tty1|' rootfs/etc/inittab
sed -i 's|\#ttyS0.*|ttyS0::respawn:/sbin/agetty --autologin root 115200 ttyS0 vt100\
ttyS1::respawn:/sbin/agetty --autologin root 115200 ttyS1 vt100\
ttyS2::respawn:/sbin/agetty --autologin root 115200 ttyS2 vt100|' rootfs/etc/inittab

# drop the setup script to run in the chroot
cat << EOF > rootfs/setup.sh
#!/bin/sh
# correct PATH
export PATH="/bin:/usr/bin:/sbin:/usr/sbin"

# update
apk --no-interactive update
apk --no-interactive upgrade

# set the password
echo 'root:alpine' | chpasswd

# install packages
apk --no-interactive add $apk_packages

# run setup-alpine (but disable setting the root password)
setup-alpine -ef /setup-alpine_answers.txt

# setup udev
setup-devd udev

# clean the cache
apk --no-interactive cache --purge

# enable services like NetworkManager
rc-update add networkmanager default
rc-update add hostname default
rc-update add modules default
rc-update add local default

EOF
chmod +x rootfs/setup.sh

# run it
chroot rootfs /setup.sh

# clean up after ourselves
rm -f rootfs/setup-alpine_answers.txt

# set up fstab
cat << EOF > rootfs/etc/fstab
/dev/mmcblk0p1	/boot	vfat	defaults	0 0
LABEL=WL-SWCOMP	/	ext4	defaults,noatime,nodiratime,ro	0 0
tmpfs	/tmp	tmpfs	size=3G	0 0
EOF

# we can do this now (local.d exists)
cat << EOF > rootfs/etc/local.d/nvidia.start
#!/bin/bash

exec /etc/nv/nv.sh
EOF

chmod +x rootfs/etc/local.d/nvidia.start

# set up stuff to happen on autologin
cp "$base/userspace-utils/bin/menu" "rootfs/usr/bin/wl-swcomp_menu"
cp -r "$base/userspace-utils/src/usb" "rootfs/usr/libexec/"
chmod +x "rootfs/usr/bin/wl-swcomp_menu"
cp "$base/files/profile" "rootfs/root/.profile"

# clean up login screen
rm -f "rootfs/etc/motd"
echo "Loading..." > "rootfs/etc/issue"

# copy over all USB scripts
mkdir -p "rootfs/usr/libexec/wl-swcomp"
cp -a "$base/userspace-utils/src/usb" "rootfs/usr/libexec/wl-swcomp/"

# image creation code is modified from https://github.com/theofficialgman/l4t-image-buildscripts/blob/master/scripts/create_image.sh
mkdir -p out
cd out

bytes="$(du -sb ../rootfs | awk '{ print $1 }')"
echo "creating disk image, please wait..."

# buffer space of 64MB
dd if=/dev/zero of=l4t.00 bs=4194304 count=$((bytes / 4194304 + 16))
sync

# create and label fs

# disable orphan file so that
mkfs.ext4 -b 4096 l4t.00 -O ^orphan_file
e2label l4t.00 WL-SWCOMP

mkdir -p mounted_ext4
chown 0:0 mounted_ext4
chmod 0777 mounted_ext4

mount l4t.00 mounted_ext4/
echo "Copying files"

cp -a ../rootfs/* mounted_ext4/ || {
	echo "Failed to copy files - out of disk space?"
	umount mounted_ext4
	exit 1
}
sync
# otherwise it aborts with target is busy
sleep 1
fuser -v mounted_ext4

umount mounted_ext4
rmdir mounted_ext4

echo "Cleaning up free space"
zerofree l4t.00
sync

echo "image is ready at $base/out/l4t.00"
echo "making hekate-compatible 7z..."

mkdir -p bootloader/ini
mkdir -p switchroot/install
mkdir -p switchroot/wii-linux-switch-companion

# copy config
cp "$base/rootfs/opt/switchroot/wii-linux-switch-companion.ini" bootloader/ini/

# copy boot stack
cp "$base/rootfs/opt/switchroot/bootstack/"* switchroot/wii-linux-switch-companion/

# copy the modules in (the initramfs will handle it)
mv "modules.tar.gz" switchroot/wii-linux-switch-companion/

# copy the disk image
cp "$base/out/l4t.00" switchroot/install/

# make a 7z of it
date="$(date +%Y-%m-%d)"
filename="wii-linux-switch-companion__$date.7z"
7z a -r "$filename" switchroot bootloader

echo "DONE!  7z at $base/out/$filename"
