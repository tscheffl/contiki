RPL Border Router for AN Solutions Boards
===========

This modification provides support for [AN Solutions Boards](http://an-solutions.de) and is based on the Contiki `avr-zigbit` platform.
There are a few changes in the `platform/avr-zigbit` code, such as the UART port and the baudrate. 

Most changes are kept in the `project-conf.h` file. We had to comment-out the references to button-sensors, since the board does not have any.

The default IPv6 address is: `fe80::11:22ff:fe33:4455`
and there runs a small HTTP server on the board that shows information about neighboring nodes. 


The last Octet can be changed by setting the `MY_NODE_ID` variable in the Makefile like this:

    CFLAGS += -DMY_NODE_ID=10
(**Remember**: the variable value is decimal, IPv6 addresses are hexadecimal, so `10` translates to `0a`)

###Building the binary

The binary can be build in the usual way:

    make clean
    make
    
It also needs to be flashed to the board (using avrdude and JTAGICE 3 here):

    sudo avrdude -c jtag3 -p m1281 -P usb -B2 -U flash:w:border-router.avr-zigbit
    sudo avrdude -c jtag3 -p m1281 -P usb -B2 -U eeprom:w:border-router.avr-zigbit

You can start using the RPL Border Router under Linux by compiling and running the `tunslip6` utility in the `tools` directory: 

    sudo ./tunslip6 -L -v5 -d20 -s /dev/ttyUSB0 -B 38400 aaaa::1/64

More information can be found at: [https://wiki.ipv6lab.beuth-hochschule.de/contiki/a-n-solutions-module/contiki3]()
