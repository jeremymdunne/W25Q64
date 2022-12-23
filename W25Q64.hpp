/**
 * @file W25Q64.hpp
 * @author Jeremy Dunne
 * @brief Interface library for the Winbond W25Q64 family of flash chips 
 * @version 0.1
 * @date 2022-12-20
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#ifndef _W25Q64_HPP_
#define _W25Q64_HPP_ 


// imports 
#include <Arduino.h>
#include <SPI.h> 


// register definitions  
#define W25Q64_WRITE_ENABLE                 0x06
#define W25Q64_VOLATILE_WRITE_ENABLE        0x50 
#define W25Q64_WRITE_DISABLE                0x04 
#define W25Q64_RELEASE_POWER_DOWN           0xAB 
#define W25Q64_MANUFACTURER_ID              0x90 
#define W25Q64_JEDEC_ID                     0x9F 
#define W25Q64_READ_UNIQUE_ID               0x4B 
#define W25Q64_READ_DATA                    0x03 
#define W25Q64_FAST_READ                    0x0B 
#define W25Q64_PAGE_PROGRAM                 0x02 
#define W25Q64_SECTOR_ERASE                 0x20 
#define W25Q64_BLOCK_32_ERASE               0x52 
#define W25Q64_BLOCK_64_ERASE               0xD8 
#define W25Q64_CHIP_ERASE                   0xC7 
#define W25Q64_READ_STATUS_REGISTER_1       0x05 
#define W25Q64_WRITE_STATUS_REGISTER_1      0x01 
#define W25Q64_READ_STATUS_REGISTER_2       0x35
#define W25Q64_WRITE_STATUS_REGISTER_2      0x31 
#define W25Q64_READ_STATUS_REGISTER_3       0x15
#define W25Q64_WRITE_STATUS_REGISTER_3      0x11
#define W25Q64_READ_SFDP_REGISTER           0x5A
#define W25Q64_ERASE_SECURITY_REGISTER      0x44
#define W25Q64_PROGRAM_SECURITY_REGISTER    0x42 
#define W25Q64_READ_SECURITY_REGISTER       0x48 
#define W25Q64_ERASE_PROGRAM_SUSPEND        0x75 
#define W25Q64_ERASE_PROGRAM_RESUME         0x7A 
#define W25Q64_POWER_DOWN                   0xB9
#define W25Q64_ENABLE_RESET                 0x66
#define W25Q64_RESET_DEVICE                 0x99 

// SPI Settings 
#define W25Q64_SPI_SPEED                    50000000 // Hz, needs to be testsed. FV supports 104 MHz, JV 130
#define W25Q64_READ_DATA_SPI_SPEED          50000000 // Kz, needs to be testsed. This is the max speed for the read data command, the only exception for the default max speed 
#define W25Q64_SPI_MODE                     SPI_MODE_0 
#define W25Q64_SPI_DATA_ORDER               MSBFIRST 

// Chip Settings 
#define W25Q64_MAX_ADDRESS                  0x7FFFFFL // Max address, 8M-bit 

// extraneous chip commands/settings/registers 

// standard return enum 
typedef enum{
    W25Q64_OK = 0, 
    W25Q64_BUSY, 
    W25Q64_UNKOWN_MANUFACTURER_ID,
    W25Q64_UNKOWN_DEVICE_ID

} W25Q64_status_t; 

/**
 * @brief Handler class for the W25Q64 family of FLASH chips 
 * 
 * Provides an access level for all chip-avialable functions with a couple of additional functions. Currently only supports the 
 *  JV and FV families. Tested only on FV variants 
 * 
 */
class W25Q64{
public: 
    // standard interface instructions \\ 

    /**
     * @brief initialize the flash chip 
     * 
     * checks communication and ID of the flash chip 
     * 
     * @param cs_pin chip select pin for the flash chip 
     * @return W25Q64_status_t standard return type 
     */
    W25Q64_status_t init(int cs_pin); 

    /**
     * @brief check if the chip is busy 
     * 
     * @return bool true if busy, false otherwise  
     */
    bool busy(); 

    /**
     * @brief reset the device
     * 
     * Checks for busy, executes the RESET ENABLE and then the RESET command sequentially 
     * 
     * @return W25Q64_status_t standard return type 
     */
    W25Q64_status_t reset();  
        
    // chip instructions \\ 

    /**
     * @brief enable writing to the chip 
     * 
     * @return W25Q64_status_t standard return type 
     */
    W25Q64_status_t writeEnable(); 

    /**
     * @brief enable writing to the volatile status register 
     * 
     * @return W25Q64_status_t standard return type 
     */
    W25Q64_status_t volatileWriteEnable(); 

    /**
     * @brief disable writing to the device 
     * 
     * @return W25Q64_status_t standard return type 
     */
    W25Q64_status_t writeDisable(); 

    /**
     * @brief release the power-down state of the device 
     * 
     * Will take time to come out of the power down state. This only initiates that process 
     * 
     * @return W25Q64_status_t standard return type 
     */
    W25Q64_status_t releasePowerDown(); 

    /**
     * @brief read the manufacturing and device ID
     * 
     * @param manufacturer_id byte of the manufacturer ID. Should be 0xEF 
     * @param device_id byte of the device ID. Should be 0x16 
     * @return W25Q64_status_t standard return type 
     */
    W25Q64_status_t readManufacturerId(byte *manufacturer_id, byte *device_id);  

    /**
     * @brief read the JEDEC ID 
     * 
     * @param manufacturer_id manufacturing ID byte, should by 0xEF  
     * @param memory_type memory type byte  
     * @param capacity capacity byte 
     * @return W25Q64_status_t standard return type 
     */
    W25Q64_status_t readJedecId(byte *manufacturer_id, byte *memory_type, byte *capacity);  

    /**
     * @brief read the unique id of the chip 
     * 
     * Reads the 64-bit unique id 
     * 
     * @param unique_id pointer to a 64-bit buffer to store the unique id in 
     * @return W25Q64_status_t standard return type 
     */
    W25Q64_status_t readUniqueId(byte *unique_id); 

    /**
     * @brief read a stream of data from the chip 
     * 
     * Reads sequentially, can read the entire contents. NOTE: this is locked into a lower read frequency for the course of this transaction. 
     * 
     * @param addr 24-bit address to read from 
     * @param buff byte buffer to read into 
     * @param len number of bytes to read 
     * @return W25Q64_status_t standard return type 
     */
    W25Q64_status_t readData(unsigned int addr, byte* buff, unsigned int len); 

   /**
     * @brief perform a fast read of data from the chip 
     * 
     * Reads sequentially, can read the entire contents. Takes a 1 byte dummy read but operates at the chip's max speed 
     * @param addr 24-bit address to read from 
     * @param buff byte buffer to read into 
     * @param len number of bytes to read 
     * @return W25Q64_status_t standard return type 
     */
    W25Q64_status_t fastRead(unsigned int addr, byte* buff, unsigned int len); 

    /**
     * @brief program a page on the flash chip 
     * 
     * Write up to 256 bytes to the chip. A Write Enable command must preceed this command. 
     * 
     * @param addr 24-bit address to write to 
     * @param buff byte buffer to write 
     * @param len length to write 
     * @return W25Q64_status_t standard return type 
     */
    W25Q64_status_t pageProgram(unsigned int addr, byte* buff, unsigned int len); 

    /**
     * @brief erase a 4K-byte sector (4096 bytes).
     * 
     * Erase the sector at the sector address. addr must point to the start of a sector. Write Enable command must preceed this command. Chip will be busy 
     *  following the erase command.  
     * 
     * @param addr 24-bit sector address to erase (last 12 bits must be 0's)
     * @return W25Q64_status_t standard return type 
     */
    W25Q64_status_t sectorErase(unsigned int addr); 

    /**
     * @brief erase a 32K-byte sector (32768 bytes).
     * 
     * Erase the 32k-byte block at the block address. addr must point to the start of a 32k-byte block. Write Enable command must preceed this command. Chip will be busy 
     *  following the erase command.  
     * 
     * @param addr 24-bit block address to erase (last 15 bits must be 0's)
     * @return W25Q64_status_t standard return type 
     */
    W25Q64_status_t block32Erase(unsigned int addr); 

    /**
     * @brief erase a 64K-byte sector (65536 bytes).
     * 
     * Erase the 64k-byte block at the block address. addr must point to the start of a 64k-byte block. Write Enable command must preceed this command. Chip will be busy 
     *  following the erase command.  
     * 
     * @param addr 24-bit block address to erase (last 16 bits must be 0's)
     * @return W25Q64_status_t standard return type 
     */
    W25Q64_status_t block64Erase(unsigned int addr); 

    /**
     * @brief erase the chip 
     * 
     * Performs a full chip erase. Write Enable command must preceed this command. Chip will be busy following this erase command for some time. 
     * 
     * @return W25Q64_status_t standard return type 
     */
    W25Q64_status_t chipErase(); 

    /**
     * @brief read status register 1 
     * 
     * @param reg byte to store the register value into 
     * @return W25Q64_status_t standard return type 
     */
    W25Q64_status_t readStatusRegister1(byte* reg); 

    /**
     * @brief write to status register 1 
     * 
     * @param reg value to write into the status register 
     * @return W25Q64_status_t standard return type 
     */
    W25Q64_status_t writeStatusRegister1(byte reg); 

    /**
     * @brief read status register 2
     * 
     * @param reg byte to store the register value into 
     * @return W25Q64_status_t standard return type 
     */
    W25Q64_status_t readStatusRegister2(byte* reg); 

    /**
     * @brief write to status register 2
     * 
     * @param reg value to write into the status register 
     * @return W25Q64_status_t standard return type 
     */
    W25Q64_status_t writeStatusRegister2(byte reg); 

    /**
     * @brief read status register 3 
     * 
     * @param reg byte to store the register value into 
     * @return W25Q64_status_t standard return type 
     */
    W25Q64_status_t readStatusRegister3(byte* reg); 

    /**
     * @brief write to status register 3
     * 
     * @param reg value to write into the status register 
     * @return W25Q64_status_t standard return type 
     */
    W25Q64_status_t writeStatusRegister3(byte reg); 

    /**
     * @brief read the SFDP registers 
     * 
     * @param addr address to read from 
     * @param buff buffer to read data into 
     * @param len length of the data to read 
     * @return W25Q64_status_t standard return type 
     */
    W25Q64_status_t readSFDPRegister(unsigned int addr, byte* buff, unsigned int len); 

    /**
     * @brief erase the security register 
     * 
     * @param addr address of the register to erase 
     * @return W25Q64_status_t 
     */
    W25Q64_status_t eraseSecurityRegister(unsigned int addr); 
    
    /**
     * @brief program the security register
     * 
     * @param addr address of security register to program 
     * @param buff buffer of data to write 
     * @param len length of data to write 
     * @return W25Q64_status_t standard return type 
     */
    W25Q64_status_t programSecurityRegister(unsigned int addr, byte* buff, unsigned int len); 

     /**
     * @brief read the security registers 
     * 
     * @param addr address to read from 
     * @param buff buffer to read data into 
     * @param len length of the data to read 
     * @return W25Q64_status_t standard return type 
     */
    W25Q64_status_t readSecurityRegister(unsigned int addr, byte* buff, unsigned int len); 

    /**
     * @brief suspend an erase or program operation 
     * 
     * Allows for read & program commands to be completed when paused 
     * 
     * @return W25Q64_status_t standard return type 
     */
    W25Q64_status_t eraseProgramSuspend(); 

    /**
     * @brief resume an erase or program operation that was suspended  
     * 
     * @return W25Q64_status_t standard return type 
     */
    W25Q64_status_t eraseProgramResume(); 

    /**
     * @brief power down the device 
     * 
     * @return W25Q64_status_t standard return type 
     */
    W25Q64_status_t powerDown(); 

    /**
     * @brief enable a reset 
     * 
     * @return W25Q64_status_t standard return type 
     */
    W25Q64_status_t enableReset(); 

    /**
     * @brief reset the device 
     * 
     * @return W25Q64_status_t standard return type 
     */
    W25Q64_status_t resetDevice(); 
    

private: 
    SPISettings _spi_settings = SPISettings(W25Q64_SPI_SPEED, W25Q64_SPI_DATA_ORDER, W25Q64_SPI_MODE); ///< spi settings for the flash chip  
    int _cs_pin; ///< chip select pin for the flash chip 
    W25Q64_status_t _status;  ///< general status for chip functions 




    /**
     * @brief select the flash chip 
     * 
     * pulls cs-pin low and implements a SPI transaction 
     * 
     */
    void _select(){
        // select the cs_pin 
        digitalWrite(_cs_pin, LOW); 
        // begin transaction with settings 
        SPI.beginTransaction(_spi_settings); 
    };

    /**
     * @brief release the flash chip 
     * 
     * pulls cs-pin high and releases the SPI transaction 
     * 
     */
    void _release(){
        // release cs_pin 
        digitalWrite(_cs_pin, HIGH); 
        // release transaction 
        SPI.endTransaction(); 
    };  






}; 

#endif 
