//RX - Teensy 4.0 @ 600MHz
//


#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <driver/i2s.h>
#include <RingBuf.h>

#define audiobuffersize 128  // one block in audio library has 128 values of 2 byte
#define CE_PIN 9
#define CSN_PIN 10
#define MISO_PIN 12
#define MOSI_PIN 11
#define SCK_PIN 13

 RF24 radio;

SPIClass *hspi_class = NULL;


#define I2S_BCLK_PIN 4
#define I2S_LRCLK_PIN 5
#define I2S_DATA_PIN 18

#define DEBUG_PIN 1

#define IRQ_PIN 38



// I2S configuration
const i2s_config_t i2s_config = {
  .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
  .sample_rate = 32000,
  .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
  .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
  .communication_format = I2S_COMM_FORMAT_I2S,
  .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
  .dma_buf_count = 4,
  .dma_buf_len = 256,
  .use_apll = false,
  .tx_desc_auto_clear = true,
  .fixed_mclk = 0

};

// I2S port
const i2s_port_t i2s_port = I2S_NUM_0;


//const byte IRQ_Pin = 0;

const byte pipeaddress[][6] = { "1Ad", "2Ad", "3Ad" };
uint8_t pipeNr;



uint8_t bufr[audiobuffersize * 2];  //buffer to hold 256 byte of audio data
uint8_t bufcount;                   //received nrf24l01+ packet counter
uint8_t databufr[32];
uint8_t transferbuffer[audiobuffersize * 2];

int k = 0;  //zeitmessung, debug


void setup() {
  Serial.begin(9600);
  bufcount = 0;

  
    pinMode(DEBUG_PIN, OUTPUT);
    digitalWrite(DEBUG_PIN, LOW);

  hspi_class = new SPIClass();

  hspi_class->begin(SCK_PIN, MISO_PIN, MOSI_PIN, CSN_PIN);

  pinMode(IRQ_PIN, OUTPUT);  // Setzt den Digitalpin 13 als Outputpin
  i2s_driver_install(i2s_port, &i2s_config, 0, NULL);
  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCLK_PIN,
    .ws_io_num = I2S_LRCLK_PIN,
    .data_out_num = I2S_DATA_PIN,
    .data_in_num = I2S_PIN_NO_CHANGE
  };
  i2s_set_pin(i2s_port, &pin_config);





  radio.begin(hspi_class, CE_PIN, CSN_PIN);

  if (!radio.isChipConnected()) {
    Serial.println("chip not connected.");
  };

  radio.setPayloadSize(32);  //this is the maximum payload size for a packet transfer of nrf24
  radio.setAutoAck(false);   //no Ack
  radio.disableCRC();        //no CRC
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_2MBPS);
  radio.setChannel(125);
  delay(100);

  radio.openReadingPipe(1, pipeaddress[1]);  // Open a reading pipe
  delay(50);

  radio.startListening();
  radio.printPrettyDetails();
  Serial.begin(230400);

  //pinMode(IRQ_Pin, INPUT_PULLUP);
  //attachInterrupt(IRQ_Pin,rfRead,FALLING);
}
size_t bytes_written;
RingBuf<byte[32], 4> aBuffer;
int16_t *sixteen = (short *)bufr;
void loop() {
  if (radio.available(&pipeNr)) {



    //Serial.println("audiorecieved");
    if ((bufcount >= 0) && (bufcount < 8)) {
      digitalWrite(DEBUG_PIN, HIGH);
      radio.read(&bufr[bufcount * 32], 32);

      for (int i = 0; i < 16; i++) {

        //Serial.println(sixteen[bufcount + i]);
      }


      // test if all 8 packets received
      if (bufcount == 7) {
        memcpy(transferbuffer, &bufr[0], (audiobuffersize * 2));

        for (int i = 0; i < 128; i++) {

          //Serial.println(sixteen[i]);
        }
        //digitalWrite(IRQ_PIN, HIGH);

        bufcount = 0;
        i2s_write(i2s_port, transferbuffer, sizeof(transferbuffer), &bytes_written, portMAX_DELAY);
        digitalWrite(DEBUG_PIN, LOW);
        //digitalWrite(IRQ_PIN,HIGH LOW);
        // int16_t *p = queue1.getBuffer();
        // memcpy(p, &bufr[0], (audiobuffersize * 2));
        // queue1.playBuffer();
      } else {
        bufcount++;
      }
    }
  }
}
