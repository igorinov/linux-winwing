# linux-winwing
Linux kernel module for [WinWing](https://winctrl.com/) Orion2 throttle base with the following grip handles:

  * TGRIP-15E
  * TGRIP-15EX
  * TGRIP-16EX
  * TGRIP-18

## Releases
  - Kernel versions 6.10 ‐ 6.18 include an earlier version of this module (TGRIP-16EX and TGRIP-18 only);
  - Kernel versions 6.19 - 7.0 support all throttle grips, but no rumble feedback;
  - Kernel 7.1 will have rumble feedback support.

## Button mapping

On the Orion 2 throttle, numbers 0 .. 63 are reserved for buttons on the throttle grip; the throttle base buttons have numbers 64 .. 110.
Some of the throttle base buttons don't work with Linux because the kernel HID subsystem only supports up to 80 buttons without special kernel modules.

### TGRIP-16EX, TGRIP-18

Unused button numbers 33 .. 64 are not mapped.

### TGRIP-15E, TGRIP-15EX

Buttons 17 .. 44 are maped to KEY_MACRO1 .. KEY_MACRO28.

Rumble: left is "strong" and right is "weak".
Left and right handles probably have the same motors, but rumble feels stronger on the left handle because it's smaller.

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
