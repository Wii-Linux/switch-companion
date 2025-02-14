#!/bin/sh

ARCH="$(uname -m)"

linaroUrl="https://releases.linaro.org/components/toolchain/binaries/latest-7/aarch64-linux-gnu/"
linaroGcc="gcc-linaro-7.5.0-2019.12-${ARCH}_aarch64-linux-gnu"
linaroSysroot="sysroot-glibc-linaro-2.25-2019.12-aarch64-linux-gnu"

base="$(dirname -- "$( readlink -f -- "$0"; )")"
echo "Base directory: $base"
cd "$base"

getSysroot() {
	if [ -d "downloads/$linaroSysroot" ]; then
		return
	fi

	echo "Downloading Linaro sysroot..."
	mkdir -p downloads
	cd downloads
	wget "$linaroUrl/$linaroSysroot.tar.xz" || {
		echo "Failed to download Linaro sysroot - check your internet connection!"
		exit 1
	}

	tar xf "$linaroSysroot.tar.xz"
	rm "$linaroSysroot.tar.xz"
	cd ..
}

checkLinaro() {
	if [ -d "downloads/linaro-$ARCH" ]; then
		return
	fi

	echo "Downloading Linaro toolchain..."
	mkdir -p downloads
	cd downloads
	wget "$linaroUrl/$linaroGcc.tar.xz" || {
		echo "Failed to download Linaro toolchain - check your internet connection!"
		exit 1
	}

	tar xf "$linaroGcc.tar.xz"
	mv "$linaroGcc" "linaro-$ARCH"
	rm "$linaroGcc.tar.xz"
	cd ..

	getSysroot
}

checkPPCToolchain() {
	if command -v aarch64-linux-gnu-gcc && command -v aarch64-linux-gnu-gcc; then
		return
	fi

	# If we aren't on ArchPOWER then we can't do anything useful
	{
		. /etc/os-release
		if ! [ "$ID" = "arch" ] && ! [ "$ID_LIKE" = "arch" ]; then
			echo "Unsupported distro: $ID"
			exit 1
		fi
	}
	echo "Installing ppc toolchain..."
	pacman -S --noconfirm aarch64-linux-gnu-gcc aarch64-linux-gnu-binutils

	getSysroot
}

case $ARCH in
	x86_64|i686)
		checkLinaro
		export PATH="$PATH:$base/downloads/linaro-$ARCH/bin"
		export CC=aarch64-linux-gnu-gcc
		export LD=aarch64-linux-gnu-ld
		export CFLAGS="$CFLAGS --sysroot=$base/downloads/$linaroSysroot"
		;;
	aarch64)
		# no need to check, we are using the host compiler
		export CC=gcc
		export LD=ld
		;;
	ppc)
		# lol someone's actually trying to build this on ppc, likely their Wii
		checkPPCToolchain

		export CC=aarch64-linux-gnu-gcc
		export LD=aarch64-linux-gnu-ld
		export CFLAGS="$CFLAGS --sysroot=$base/downloads/$linaroSysroot"
		;;
	*)
		echo "Unsupported architecture: $ARCH"
		exit 1
		;;
esac

# we have all env necessary, go run make
cd userspace-utils
if [ "$PROD_BUILD" != "true" ]; then
	make "DEBUG_FEATURES=1"
else
	make
fi
