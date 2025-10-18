**Note**
An earlier version of this module is included in Linux kernel 6.10 (TGRIP-16EX and TGRIP-18 only).
Kernel 6.19 is going to have the full version.

# linux-winwing
Linux kernel module for [WinWing](https://winwingsim.com/) Orion2 throttle base with the following grip handles:

  * TGRIP-15E
  * TGRIP-15EX
  * TGRIP-16EX
  * TGRIP-18

On the Orion 2 throttle, numbers 0 .. 63 are reserved for buttons on the throttle grip; the throttle base buttons have numbers 64 .. 110.
Some of the throttle base buttons don't work with Linux because the kernel HID subsystem only supports up to 80 buttons without special kernel modules.

TGRIP-16EX, TGRIP-18: Unused button numbers 33 .. 64 are not mapped.

TGRIIP-15E, TGRIP-15EX: Buttons 17 .. 44 are maped to KEY_MACRO1 .. KEY_MACRO28.

## Setup

Install the header files for currently running kernel. This command is for Debian-based systems (e.g. Ubuntu, Mint):

```
sudo apt install linux-headers-`uname -r`
```

## Build

The following command builds the `winwing` module for currently running kernel.

```
make
```

## Install

The following command copies new module into the module directory of currently running kernel and updates module dependencies.

```
make install
```
