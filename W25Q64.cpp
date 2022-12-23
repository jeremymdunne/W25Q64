/**
 * @file W25Q64.cpp
 * @author Jeremy Dunne 
 * @brief Implementation of the Winbond W25Q64 interface library 
 * @version 0.1
 * @date 2022-12-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "W25Q64.hpp"

W25Q64_status_t W25Q64::init(int cs_pin){
    // setup 
    _cs_pin = cs_pin; 
    pinMode(_cs_pin, OUTPUT); 
    digitalWrite(_cs_pin, HIGH); 
    SPI.begin(); 
    // _spi_settings = SPISettings(W25Q64_SPI_SPEED, W25Q64_SPI_DATA_ORDER, W25Q64_SPI_MODE);

    // read the device ID 
    byte manufacturer_id, device_id; 
    _status = readManufacturerId(&manufacturer_id, &device_id); 
    // check the expected IDs 
    if(manufacturer_id != 0xEF){
        return W25Q64_UNKOWN_MANUFACTURER_ID; 
    }
    else if(device_id != 0x16){
        return W25Q64_UNKOWN_DEVICE_ID; 
    }

    // report success  
    return W25Q64_OK; 
}

bool W25Q64::busy(){
    // busy bit is in status register one 
    byte status_reg_1;
    readStatusRegister1(&status_reg_1); // read_status_register_1(); 
    if(status_reg_1&0b00000001) return true;
    return false; 
}

W25Q64_status_t W25Q64::reset(){
    // check status 
    if(busy()) return W25Q64_BUSY; 
    // enable a reset 
    enableReset(); 
    // immediately reset 
    resetDevice(); 
    return W25Q64_OK; 
}

// chip instructions \\ 

W25Q64_status_t W25Q64::writeEnable(){
    _select(); 
    SPI.transfer(W25Q64_WRITE_ENABLE); 
    _release(); 
    return W25Q64_OK; 
}

W25Q64_status_t W25Q64::volatileWriteEnable(){
    _select(); 
    SPI.transfer(W25Q64_VOLATILE_WRITE_ENABLE); 
    _release(); 
    return W25Q64_OK; 
}

W25Q64_status_t W25Q64::writeDisable(){
    _select();
    SPI.transfer(W25Q64_WRITE_DISABLE);
    _release(); 
    return W25Q64_OK; 
}

W25Q64_status_t W25Q64::releasePowerDown(){
    _select();
    SPI.transfer(W25Q64_RELEASE_POWER_DOWN); 
    _release(); 
    return W25Q64_OK; 
}

W25Q64_status_t W25Q64::readManufacturerId(byte *manufacturer_id, byte *device_id){
    // read the data 
    byte buff[5]; 
    _select(); 
    SPI.transfer(W25Q64_MANUFACTURER_ID); 
    for(int i = 0; i < 5; i ++){
        buff[i] = SPI.transfer(0); 
    }
    *manufacturer_id = buff[3]; 
    *device_id = buff[4]; 
    _release(); 
    return W25Q64_OK; 
}

W25Q64_status_t W25Q64::readJedecId(byte *manufacturer_id, byte *memory_type, byte *capacity){
    _select(); 
    SPI.transfer(W25Q64_JEDEC_ID); 
    *manufacturer_id = SPI.transfer(0); 
    *memory_type = SPI.transfer(0); 
    *capacity = SPI.transfer(0); 
    _release(); 
    return W25Q64_OK; 
}

W25Q64_status_t W25Q64::readUniqueId(byte *unique_id){
    _select();
    SPI.transfer(W25Q64_READ_UNIQUE_ID); 
    // transfer 4 dummy bytes 
    for(int i = 0; i < 4; i ++){
        SPI.transfer(0); 
    }
    // read 8 bytes for the unique ID 
    for(int i = 0; i < 8; i ++){
        *unique_id = SPI.transfer(0); 
        *unique_id ++; 
    }
    return W25Q64_OK; 
}

W25Q64_status_t W25Q64::readData(unsigned int addr, byte* buff, unsigned int len){
    // check if busy 
    if(busy()) return W25Q64_BUSY; 
    // transaction 
    _select(); 
    // change the transaction settings to the lower frequency 
    SPI.beginTransaction(SPISettings(W25Q64_READ_DATA_SPI_SPEED, W25Q64_SPI_DATA_ORDER, W25Q64_SPI_MODE));
    SPI.transfer(W25Q64_READ_DATA); 
    // send the 24-bit address 
    for(int i = 0; i < 3; i ++){
        SPI.transfer((byte)(addr >> ((2-i)*8))); 
    }
    // read sequentially 
    while(len > 0){
        *buff = SPI.transfer(0); 
        *buff ++; 
        len --; 
    }
    _release();  
    // return OK
    return W25Q64_OK;  
}

W25Q64_status_t W25Q64::fastRead(unsigned int addr, byte* buff, unsigned int len){
    // check if busy 
    if(busy()) return W25Q64_BUSY; 
    // transaction 
    _select(); 
    SPI.transfer(W25Q64_FAST_READ); 
    // send the 24-bit address 
    for(int i = 0; i < 3; i ++){
        SPI.transfer((byte)(addr >> ((2-i)*8))); 
    }
    // send a dummy byte 
    SPI.transfer(0); 
    // read sequentially 
    while(len > 0){
        *buff = SPI.transfer(0); 
        *buff ++; 
        len --; 
    }
    _release();  
    // return OK
    return W25Q64_OK;  
}

W25Q64_status_t W25Q64::pageProgram(unsigned int addr, byte* buff, unsigned int len){
    // check if busy 
    if(busy()) return W25Q64_BUSY; 
    // assume that a write enable command has already been issued 
    // assume no security lockouts 
    _select(); 
    SPI.transfer(W25Q64_PAGE_PROGRAM); 
    // send the 24-bit start address 
    for(int i = 0; i < 3; i ++){
        SPI.transfer((byte)(addr >> ((2-i)*8))); 
    }
    // send the buffer 
    while(len > 0){
        SPI.transfer(*buff); 
        *buff ++; 
        len --; 
    }
    _release(); 
    return W25Q64_OK;
}

W25Q64_status_t W25Q64::sectorErase(unsigned int addr){
    // check if busy 
    if(busy()) return W25Q64_BUSY; 
    // assume that a write enable command has already been issued 
    // assume no security lockouts 
    _select(); 
    SPI.transfer(W25Q64_SECTOR_ERASE); 
    // send the 24-bit start address 
    for(int i = 0; i < 3; i ++){
        SPI.transfer((byte)(addr >> ((2-i)*8))); 
    }
    _release(); 
    return W25Q64_OK;
}

W25Q64_status_t W25Q64::block32Erase(unsigned int addr){
    // check if busy 
    if(busy()) return W25Q64_BUSY; 
    // assume that a write enable command has already been issued 
    // assume no security lockouts 
    _select(); 
    SPI.transfer(W25Q64_BLOCK_32_ERASE); 
    // send the 24-bit start address 
    for(int i = 0; i < 3; i ++){
        SPI.transfer((byte)(addr >> ((2-i)*8))); 
    }
    _release(); 
    return W25Q64_OK;
}

W25Q64_status_t W25Q64::block64Erase(unsigned int addr){
    // check if busy 
    if(busy()) return W25Q64_BUSY; 
    // assume that a write enable command has already been issued 
    // assume no security lockouts 
    _select(); 
    SPI.transfer(W25Q64_BLOCK_64_ERASE); 
    // send the 24-bit start address 
    for(int i = 0; i < 3; i ++){
        SPI.transfer((byte)(addr >> ((2-i)*8))); 
    }
    _release(); 
    return W25Q64_OK;
}

W25Q64_status_t W25Q64::chipErase(){
    // check if busy 
    if(busy()) return W25Q64_BUSY; 
    // assume that a write enable command has already been issued 
    // assume no security lockouts 
    _select(); 
    SPI.transfer(W25Q64_CHIP_ERASE); 
    _release(); 
    return W25Q64_OK;
}

W25Q64_status_t W25Q64::readStatusRegister1(byte* reg){
    _select(); 
    SPI.transfer(W25Q64_READ_STATUS_REGISTER_1); 
    *reg = SPI.transfer(0); 
    _release(); 
    return W25Q64_OK;
}

W25Q64_status_t W25Q64::writeStatusRegister1(byte reg){
    // check if busy 
    if(busy()) return W25Q64_BUSY; 
    // assume that a write enable command has already been issued 
    _select(); 
    SPI.transfer(W25Q64_WRITE_STATUS_REGISTER_1); 
    SPI.transfer(reg); 
    _release(); 
    return W25Q64_OK;
}

W25Q64_status_t W25Q64::readStatusRegister2(byte* reg){
    _select(); 
    SPI.transfer(W25Q64_READ_STATUS_REGISTER_2); 
    *reg = SPI.transfer(0); 
    _release(); 
    return W25Q64_OK;
}

W25Q64_status_t W25Q64::writeStatusRegister2(byte reg){
    // check if busy 
    if(busy()) return W25Q64_BUSY; 
    // assume that a write enable command has already been issued 
    _select(); 
    SPI.transfer(W25Q64_WRITE_STATUS_REGISTER_2); 
    SPI.transfer(reg); 
    _release(); 
    return W25Q64_OK;
}

W25Q64_status_t W25Q64::readStatusRegister3(byte* reg){
    _select(); 
    SPI.transfer(W25Q64_READ_STATUS_REGISTER_3); 
    *reg = SPI.transfer(0); 
    _release(); 
    return W25Q64_OK;
}

W25Q64_status_t W25Q64::writeStatusRegister3(byte reg){
    // check if busy 
    if(busy()) return W25Q64_BUSY; 
    // assume that a write enable command has already been issued 
    _select(); 
    SPI.transfer(W25Q64_WRITE_STATUS_REGISTER_3); 
    SPI.transfer(reg); 
    _release(); 
    return W25Q64_OK;
}

W25Q64_status_t W25Q64::readSFDPRegister(unsigned int addr, byte* buff, unsigned int len){
    // check if busy 
    if(busy()) return W25Q64_BUSY; 
    // transaction 
    _select(); 
    SPI.transfer(W25Q64_READ_SFDP_REGISTER); 
    // send the 24-bit address 
    for(int i = 0; i < 3; i ++){
        SPI.transfer((byte)(addr >> ((2-i)*8))); 
    }
    // send a dummy byte 
    SPI.transfer(0); 
    // read sequentially 
    while(len > 0){
        *buff = SPI.transfer(0); 
        *buff ++; 
        len --; 
    }
    _release();  
    // return OK
    return W25Q64_OK;  
}

W25Q64_status_t W25Q64::eraseSecurityRegister(unsigned int addr){
    // check if busy 
    if(busy()) return W25Q64_BUSY; 
    // assume that a write enable command has already been issued 
    _select(); 
    SPI.transfer(W25Q64_ERASE_SECURITY_REGISTER); 
    // send the 24-bit address 
    for(int i = 0; i < 3; i ++){
        SPI.transfer((byte)(addr >> ((2-i)*8))); 
    }
    _release(); 
    return W25Q64_OK;
}

W25Q64_status_t W25Q64::programSecurityRegister(unsigned int addr, byte* buff, unsigned int len){
    // check if busy 
    if(busy()) return W25Q64_BUSY; 
    // assume that a write enable command has already been issued
    // transaction 
    _select(); 
    SPI.transfer(W25Q64_PROGRAM_SECURITY_REGISTER); 
    // send the 24-bit address 
    for(int i = 0; i < 3; i ++){
        SPI.transfer((byte)(addr >> ((2-i)*8))); 
    } 
    // write sequentially 
    while(len > 0){
        SPI.transfer(*buff); 
        *buff ++; 
        len --; 
    }
    _release();  
    // return OK
    return W25Q64_OK;  
}

W25Q64_status_t W25Q64::readSecurityRegister(unsigned int addr, byte* buff, unsigned int len){
    // check if busy 
    if(busy()) return W25Q64_BUSY; 
    // transaction 
    _select(); 
    SPI.transfer(W25Q64_READ_SECURITY_REGISTER); 
    // send the 24-bit address 
    for(int i = 0; i < 3; i ++){
        SPI.transfer((byte)(addr >> ((2-i)*8))); 
    }
    // send a dummy byte 
    SPI.transfer(0); 
    // read sequentially 
    while(len > 0){
        *buff = SPI.transfer(0); 
        *buff ++; 
        len --; 
    }
    _release();  
    // return OK
    return W25Q64_OK;  
}

W25Q64_status_t W25Q64::eraseProgramSuspend(){
    _select(); 
    SPI.transfer(W25Q64_ERASE_PROGRAM_SUSPEND); 
    _release(); 
    return W25Q64_OK; 
}

W25Q64_status_t W25Q64::eraseProgramResume(){
    _select(); 
    SPI.transfer(W25Q64_ERASE_PROGRAM_RESUME); 
    _release(); 
    return W25Q64_OK; 
}

W25Q64_status_t W25Q64::powerDown(){
    _select(); 
    SPI.transfer(W25Q64_POWER_DOWN); 
    _release(); 
    return W25Q64_OK; 
}

W25Q64_status_t W25Q64::enableReset(){
    _select(); 
    SPI.transfer(W25Q64_ENABLE_RESET); 
    _release(); 
    return W25Q64_OK; 
}

W25Q64_status_t W25Q64::resetDevice(){
    _select(); 
    SPI.transfer(W25Q64_RESET_DEVICE); 
    _release(); 
    return W25Q64_OK; 
}
