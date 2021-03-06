# STM32F4 #

This repo contains bootloader, applications for STM32F407 boards.

## Shell ##
This is a bootloader for STM32F407. It provides a shell console through USB interface, and also implements module support to
allow running test code from USB interface directly.

### Build ###
Clone this git repo with "--recurse-submodules" so that submoulde can be created.
To build the bootloader image, run following commands:

    cd Shell
    make clean
    make

### Flash ###
To flash the bootloader into flash, please do the following:
  - Connect STLINK V2 to STM32F407 board through SWD interface
  - For Windows, install libusb filter driver for the STLINK device
  -
    Run the following:

      cd Shell
      make burnstlink

### Install Drivers ###
On Windows, when connecting the STM32F407 to host through USB, it will show up as a composite USB device.
One interface is for USB console and the other interface is for USB data communication. Drivers need to be installed before using.
Please use [Zadig](https://zadig.akeo.ie/) to install libusb-win32 driver for both of the interfaces.


### Start Shell Console ###
To access Shell console through USB interface, please use Python3. pyusb module needs to be installed for python first.
The following python command will start the Shell console.

    sudo python Tools/stmtalk.py ""

Alternatively, on Windows, to view the USB Shell console, [SuperTerm](https://github.com/microxblue/superterm) can be used.
In SuperTerm configuration page, using "USB Bulk Setting" to select the USB console interface.


