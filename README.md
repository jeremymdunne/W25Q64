This is an interface Library for the W25Q64 Family of Winbond Flash Chips.  

This library provides access to all the available registers and commands from the flash chip, and provides a few additional functions for simple tasks. 

Usage Notes: 
    Every command that results in data being changed on the chip must be preceeded by a WRITE_ENABLE command. This includes erasing and writing data. 
    This library is only tested for the W25Q64FV chips, but should also work on the JV chips. SPI frequencies should be adjusted accordingly. 

Tested Chips: 
    W25Q64FV 

Tested MCUs: 
    STM32F411CE (should work on all stm32 chips using the Arduino framwork)

TODOs: 
    Abstract out all communications (allow for use of other SPI busses) 
    Abstract out the I/O commands (allow for full-speed IO writes) 