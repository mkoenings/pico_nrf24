#include <Arduino.h>

/**
 *  @brief Raspberry Pico + nRF24 Tx code.
 *          The device should blink onboard LED and send data periodically.
 *  @date 23.Feb.2024.
 *  @warning Line: while (!Serial);
 *              will cause the program to hang (wait) until a serial monitor is connected.
 */

#include <string.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "Wire.h"
#include "es8388.h"
#include <I2S.h>

#include "FreeRTOS.h"
#include "task.h"

const int sampleRate = 8000; // sample rate in Hz
#define EXAMPLE_SAMPLE_RATE     (32000)
#define TESTING_IO_PIN          18

I2S i2s(INPUT);


void testing_pin_toggle(void)
{
    digitalWrite(TESTING_IO_PIN, !digitalRead(TESTING_IO_PIN));
}

void testing_pin_init(void)
{
    pinMode(TESTING_IO_PIN, OUTPUT);
    digitalWrite(TESTING_IO_PIN, LOW);
}
void __not_in_flash_func(test_on_rec)(void)
{
    // for(int i = 0; i<10;i++)
    // {
        int16_t l, r;
        i2s.read16(&l, &r);
        Serial.printf("%d %d\r\n", l, r);
    // }
    // testing_pin_toggle();
}

void audio_board_init(void)
{
    esp_err_t err;
    audio_hal_codec_config_t audio_hal_conf;

    audio_hal_conf.adc_input          = AUDIO_HAL_ADC_INPUT_LINE2;
    audio_hal_conf.dac_output         = AUDIO_HAL_DAC_OUTPUT_ALL;
    audio_hal_conf.codec_mode         = AUDIO_HAL_CODEC_MODE_ENCODE;
    audio_hal_conf.i2s_iface.mode     = AUDIO_HAL_MODE_SLAVE;
    audio_hal_conf.i2s_iface.fmt      = AUDIO_HAL_I2S_LEFT;
    audio_hal_conf.i2s_iface.samples  = AUDIO_HAL_32K_SAMPLES;
    audio_hal_conf.i2s_iface.bits     = AUDIO_HAL_BIT_LENGTH_16BITS;

    err = es8388_init(&audio_hal_conf);
    if(err != ESP_OK)
    {
        Serial.printf("es8388_init error %d\r\n", err);
    }
    else{
        Serial.printf("es8388_init OK %d\r\n", err);
    }

    err = es8388_config_i2s(audio_hal_conf.codec_mode, &audio_hal_conf.i2s_iface);
    if(err != ESP_OK)
    {
        Serial.println("es8388_config_i2s error");
    }
    else{
        Serial.printf("es8388_init OK %d\r\n", err);
    }

    err = es8388_set_voice_volume(80);
    if(err != ESP_OK)
    {
        Serial.println("es8388_set_voice_volume error");
    }
    else{
        Serial.printf("es8388_init OK %d\r\n", err);
    }

    i2s.setDATA(22);
    i2s.setBCLK(27); // Note: LRCLK = BCLK + 1
    i2s.setMCLK(26); // Note: LRCLK = BCLK + 1
    i2s.setBitsPerSample(16);
    i2s.setFrequency(32000);
    // i2s.onReceive(test_on_rec);
    // i2s.setBuffers(3, 10, 0);
    i2s.begin();

    err = es8388_ctrl_state(audio_hal_conf.codec_mode, AUDIO_HAL_CTRL_START);
    if(err != ESP_OK)
    {
        Serial.println("es8388_ctrl_state error");
    }
}

void setup()
{
    Serial.begin(115200);
    while (!Serial);    /*! @warning This program will wait at this line until a serial monitor is connected! */
    Serial.printf("Startin Pico Tx\n");
    testing_pin_init();

    pinMode(LED_BUILTIN, OUTPUT);

    audio_board_init();
}


void loop()
{
    // digitalWrite(LED_BUILTIN, HIGH);
    // delay(500);
    // digitalWrite(LED_BUILTIN, LOW);
    // delay(500);
    vTaskDelete(NULL);
}
