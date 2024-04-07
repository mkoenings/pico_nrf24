#include <Arduino.h>
#include "i2c_bus.h"
#include "Wire.h"
#include "types.h"

#define AUDIO_BOARD_I2C_SCL     21
#define AUDIO_BOARD_I2C_SDA     20

#define END true

TwoWire *p_wire = &Wire;


// scan the Wire interfaces for devices
void scan_Wire(unsigned int SDA, unsigned int SCL, TwoWire &Wire)
{
    byte error, address;
    unsigned int nDevices;
    unsigned int wireN = 0;

    p_wire->setSDA(SDA);
    p_wire->setSCL(SCL);
    p_wire->begin();

    Serial.print("Wire");
    Serial.print(wireN % 2);
    Serial.print(" SDA: ");
    Serial.print(SDA);
    Serial.print(" SCL: ");
    Serial.println(SCL);
    for(address = 1; address < 127; address++ ) 
    {
        // The i2c_scanner uses the return value of
        // the Write.endTransmisstion to see if
        // a device did acknowledge to the address.
        p_wire->beginTransmission(address);
        error = p_wire->endTransmission();

        if (error == 0)
        {
            Serial.print("I2C device found at address 0x");
            if (address<16) 
              Serial.print("0");
            Serial.println(address,HEX);
            nDevices++;
        }
        else if (error==4) 
        {
            Serial.print("Unknown error at address 0x");
            if (address<16)
            { 
                Serial.print("0");
            }
            Serial.println(address,HEX);
        }
        else
        {
            Serial.printf("error scan %d\r\n", error);
        }
    }
    if (nDevices == 0)
    {    
        Serial.println("No I2C devices found\n");
    }
    else
    {
      Serial.println("Scan Complete\n");
    }
    p_wire->end();
    wireN++;
    // End of Wire Scanning Section

}


void i2c_bus_create(void)
{
    // scan_Wire(AUDIO_BOARD_I2C_SDA, AUDIO_BOARD_I2C_SCL, Wire);
    // return;
    bool ret = p_wire->setSDA(AUDIO_BOARD_I2C_SDA);
    if(ret != true)
    {
        Serial.println("Wire SDA pin set failed");
    }
    ret = p_wire->setSCL(AUDIO_BOARD_I2C_SCL);
    if(ret != true)
    {
        Serial.println("Wire SCL pin set failed");
    }
    // p_wire->setClock(100000);
    p_wire->begin();
}

esp_err_t i2c_bus_write_bytes(i2c_bus_handle_t bus, int addr, uint8_t* reg, int reglen, uint8_t* data, int datalen)
{
    assert(reglen == 1);
    assert(datalen == 1);

    int result = ESP_OK;
    p_wire->beginTransmission(addr >> 1);
    p_wire->write(reg[0]);
    p_wire->write(data[0]);
    int rc = p_wire->endTransmission(END);
    if (rc != 0) {
        Serial.printf("[i2c_bus_write_bytes]p_wire->endTransmission: %d\n", rc);
        result = ESP_FAIL;
    }
    return result;
}

esp_err_t i2c_bus_read_bytes(i2c_bus_handle_t bus, int addr, uint8_t* reg, int reglen, uint8_t* outdata, int datalen)
{
    assert(reglen == 1);
    assert(datalen == 1);

    outdata[0] = 0;
    int result = ESP_OK;

    p_wire->beginTransmission(addr >> 1);
    p_wire->write(reg[0]);
    int rc = p_wire->endTransmission();
    if (rc!=0){
        Serial.printf("[i2c_bus_read_bytes]p_wire->endTransmission: %d\n", rc);
    }

    uint8_t result_len = p_wire->requestFrom((uint16_t)(addr >> 1), (uint8_t)1, true);
    if (result_len > 0) {
        result_len = p_wire->readBytes(outdata, datalen);
    } else {
        Serial.printf("[i2c_bus_read_bytes2]p_wire->requestFrom %d->%d\n", datalen, result_len);
        result = ESP_FAIL;
    }
    return result;
}

esp_err_t i2c_bus_delete(void)
{
    p_wire->end();
    return ESP_OK;
}
