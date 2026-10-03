# Mouse Interface

This project is based on [A2Pico](https://github.com/oliverschmidt/a2pico).

With this firmware, the A2Pico emulates a [Mouse Interface](https://mirrors.apple2.org.za/Apple%20II%20Documentation%20Project/Interface%20Cards/Digitizers/Apple%20Mouse%20Interface%20Card/) card that allows to connect any USB mouse.

### Note

On the Apple IIgs ROM 3 (in contrast to ROM 00/01), most 16-bit software - incl. GS/OS - does not work with *any* mouse card. This also applies to the A2Pico with this firmware.

## Usage

Both left and right USB mouse buttons are mapped to the single AppleMouse II button. The middle USB mouse button is is mapped to a (normally non-existent) second Apple II mouse button.

Some adapter is required to connect the USB-A plug of a usual USB mouse to the USB socket of the A2Pico:

### A2Pico with Micro USB socket

* For a wired USB mouse, the [External USB Port](https://jcm-1.com/product/external-usb-port-for-a2pico-usb-micro-to-usb-a/) is a nice solution.
* For a wireless USB mouse, search for "Micro USB OTG" and choose a small, one-piece adapter into which you can plug the USB mouse receiver dongle directly.

### A2Pico with USB-C socket

Search for "USB-A OTG" and choose a small, one-piece adapter.

* For a wired USB mouse, the [External USB-C Port](https://jcm-1.com/product/external-usb-c-port-for-a2pico2-lite/) together with the adapter is a nice solution.
* For a wireless USB mouse, plug the USB mouse receiver dongle directly into the adapter.

Please ensure the A2Pico `USB Pwr` is set to `on` when using this firmware!

## Details

The Mouse Interface emulation uses the unmodified 6502 firmware of the original Mouse Interface card. Therefore it is supposed to work with every Apple II software the original Mouse Interface card works with.
