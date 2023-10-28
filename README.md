# linux-winwing
Linux kernel module for [WinWing](https://winwingsim.com/) Orion2 throttle base with F/A-18 Hornet grip

On the Orion 2 throttle, numbers 0 .. 63 are reserved for buttons on the throttle grip; the throttle base buttons have numbers 64 .. 110.
Some of the throttle base buttons don't work with Linux because the kernel HID subsystem only supports up to 80 buttons.

This kernel module remaps throttle base buttons to numbers 32 .. 78, reserving only numbers 0 .. 31 for buttons on the throttle grip.

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
