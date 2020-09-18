#include <Wire.h>
#include <Arduino.h>

#define LTC2485_I2C_GLOBAL_ADDRESS              0x24

#define I2C_CHECK(expr)                         ((expr) == 0)

/*! @name Mode Configuration
  @{
  See Table 1 of datasheet.
*/
#define LTC2485_SPEED_1X                        0x00
#define LTC2485_SPEED_2X                        0x01
#define LTC2485_INTERNAL_TEMP                   0x08

// Select rejection frequency - 50 and 60, 50, or 60Hz
#define LTC2485_R50                             0x02
#define LTC2485_R60                             0x04
#define LTC2485_R50_R60                         0x00

union LT_union_int32_4bytes
{
    int32_t LT_int32;       //!< 32-bit signed integer to be converted to four bytes
    uint32_t LT_uint32;     //!< 32-bit unsigned integer to be converted to four bytes
    uint8_t LT_byte[4];     //!< 4 bytes (unsigned 8-bit integers) to be converted to a 32-bit signed or unsigned integer
};

int32_t i2c_read(uint8_t address, uint8_t *data, int32_t size) {
    // __disable_irq();

    int32_t index = size - 1;

    auto rv = Wire.requestFrom(address, size, true);
    while (Wire.available()) {
        uint8_t value = Wire.read();
        data[index] = value;
        index--;
        Serial.print(value, HEX);
        Serial.print(" ");
    }

    // __enable_irq();

    delay(100);

    if (rv == 0) {
        Serial.println("no bytes");
        return 1; // fail
    }

    return 0;
}

int32_t i2c_write(uint8_t address, const void *data, int32_t size) {
    // __disable_irq();

    Wire.beginTransmission(address);
    auto ptr = (uint8_t *)data;
    for (auto i = 0; i < size; ++i) {
        Wire.write((uint8_t)*ptr++);
    }
    auto rv = Wire.endTransmission();

    // __enable_irq();

    return rv;
}


int8_t i2c_read_block_data(uint8_t address, uint8_t command, uint8_t length, uint8_t *values) {
    int32_t rv;

    rv = i2c_write(address, &command, sizeof(command));
    if (!I2C_CHECK(rv)) {
        return rv;
    }

    if (true) {
        // Gives 192 192
        delay(300); // Critical.
    } else {
        while (true) {
            Wire.beginTransmission(address);
            auto rv = Wire.endTransmission();
            if (rv == 0) {
                Serial.println("BRK");
                break;
            }
        }
    }

    rv = i2c_read(address, values, length);
    if (!I2C_CHECK(rv)) {
        return rv;
    }

    return 0;
}

int8_t LTC2485_read(uint8_t i2c_address, uint8_t adc_command, int32_t *adc_code, uint16_t eoc_timeout) {
    LT_union_int32_4bytes data;
    uint16_t timer_count = 0;
    int8_t ack;

    while (true) {
        ack = i2c_read_block_data(i2c_address, adc_command, 4, data.LT_byte);
        if (!ack) {
            break; // !ack indicates success
        }
        if (timer_count++ > eoc_timeout) {
            return 1;
        }
        else {
            delay(1);
        }
    }

    if (data.LT_byte[3] == 0xC0) {
        *adc_code = 2147483647; // Positive Overflow
        return ack;
    }

    if (data.LT_byte[3] == 0x3F) {
        *adc_code = -2147483648; // Negative Overflow
        return ack;
    }

    Serial.print("ADC: ");
    Serial.print(data.LT_int32);

    data.LT_byte[3] = data.LT_byte[3] & 0x7F; // Remove sign bit

    Serial.print(" ");
    Serial.print(data.LT_int32);

    data.LT_int32 = data.LT_int32 << 1; // shift left by one bit to restore two's complement

    Serial.print(" ");
    Serial.print(data.LT_int32);

    data.LT_int32 /= 256;  // Convert back to 24 bit value from 32 bits.
    *adc_code = data.LT_int32;

    Serial.print(" ");
    Serial.print(data.LT_int32);

    Serial.println();

    return ack; // Success
}

void setup() {
    Serial.begin(9600);

    while (!Serial) {
        delay(100);
    }

    auto value = 2200;

    analogWrite(A1, value);

    Serial.println("hey there");

    Wire.begin();

    while (true) {
        auto found = false;

        Serial.println("scanning bus");

        for (uint8_t address = 0; address < 128; ++address) {
            Wire.beginTransmission(address);
            uint8_t error = Wire.endTransmission();
            if (error == 0) {
                Serial.print("device found: 0x");
                Serial.println(address, HEX);
                found = true;
            }
            else if (error == 4) {
                Serial.print("device error: 0x");
                Serial.println(address, HEX);
            }
        }

        delay(100);

        analogWrite(A1, value);

        value += 100;

        if (value > 4000) {
            value = 1000;
        }

        if (found) {
            uint8_t config = LTC2485_R50 | LTC2485_SPEED_1X;
            uint16_t timeout = 150;
            int32_t adc = 0;

            int8_t rv = LTC2485_read(LTC2485_I2C_GLOBAL_ADDRESS, config, &adc, timeout);

            Serial.print("adc: rv=");
            Serial.print(rv);
            Serial.print(" adc=");
            Serial.print(adc);
            Serial.println();
        }

        delay(1000);

        Serial.println();
    }
}

void loop() {
}
