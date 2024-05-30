**Note**
This module is already included in Linux kernel 6.10 (and possibly later).

# linux-winwing
Linux kernel module for [WinWing](https://winwingsim.com/) Orion2 throttle base with the following grip handles:

  * TGRIP-18
  * TGRIP-16EX

On the Orion 2 throttle, numbers 0 .. 63 are reserved for buttons on the throttle grip; the throttle base buttons have numbers 64 .. 110.
Some of the throttle base buttons don't work with Linux because the kernel HID subsystem only supports up to 80 buttons.

This kernel module remaps throttle base buttons to numbers 32 .. 78, reserving only numbers 0 .. 31 for buttons on the throttle grip.
Other grip handles do have more than 32 codes (every position of a switch has its own button code).

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
