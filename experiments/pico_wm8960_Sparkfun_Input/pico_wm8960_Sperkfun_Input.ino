/******************************************************************************
  Example_03_INPUT1.ino
  Demonstrates analog audio input (on INPUT1s), sets volume control, and 
  headphone output on the WM8960 Codec.

  Audio should be connected to both the left and right "INPUT1" inputs, 
  they are labeled "RIN1" and "LIN1" on the board.

  This example will pass your audio source through the mixers and gain stages of 
  the codec using all of the analog bypass paths.

  It will output the sound on the headphone outputs. 
  It is setup to do a capless headphone setup, so connect your headphones ground 
  to "OUT3" and this provides a buffered VMID.

  You can now control the volume of the codecs built in headphone buffers using 
  this function:

  codec.setHeadphoneVolumeDB(6.00); Valid inputs are -74.00 (MUTE) up to +6.00, 
  (1.00dB steps).

  Development platform used:
  SparkFun ESP32 IoT RedBoard v10

  HARDWARE CONNECTIONS

  **********************
  ESP32 ------- CODEC
  **********************
  QWIIC ------- QWIIC       *Note this connects GND/3.3V/SDA/SCL
  GND --------- GND         *optional, but not a bad idea
  5V ---------- VIN         *needed to power codec's onboard AVDD (3.3V vreg)

  **********************
  CODEC ------- AUDIO IN
  **********************
  GND --------- TRS INPUT SLEEVE        *ground for line level input
  LINPUT1 ----- TRS INPUT TIP           *left audio
  RINPUT1 ----- TRS INPUT RING1         *right audio

  **********************
  CODEC -------- AUDIO OUT
  **********************
  OUT3 --------- TRS OUTPUT SLEEVE          *buffered "vmid" (aka "HP GND")
  HPL ---------- TRS OUTPUT TIP             *left HP output
  HPR ---------- TRS OUTPUT RING1           *right HP output

  Pete Lewis @ SparkFun Electronics
  October 14th, 2022
  https://github.com/sparkfun/SparkFun_WM8960_Arduino_Library
  
  This code was created using some code by Mike Grusin at SparkFun Electronics
  Included with the LilyPad MP3 example code found here:
  Revision history: version 1.0 2012/07/24 MDG Initial release
  https://github.com/sparkfun/LilyPad_MP3_Player

  Do you like this library? Help support SparkFun. Buy a board!

    SparkFun Audio Codec Breakout - WM8960 (QWIIC)
    https://www.sparkfun.com/products/21250
	
	All functions return 1 if the read/write was successful, and 0
	if there was a communications failure. You can ignore the return value
	if you just don't care anymore.

	For information on the data sent to and received from the CODEC,
	refer to the WM8960 datasheet at:
	https://github.com/sparkfun/SparkFun_Audio_Codec_Breakout_WM8960/blob/main/Documents/WM8960_datasheet_v4.2.pdf

  This code is released under the [MIT License](http://opensource.org/licenses/MIT).
  Please review the LICENSE.md file included with this example. If you have any questions 
  or concerns with licensing, please contact techsupport@sparkfun.com.
  Distributed as-is; no warranty is given.
******************************************************************************/

#include <Wire.h>
#include <SparkFun_WM8960_Arduino_Library.h> 
#include <I2S.h>
// Click here to get the library: http://librarymanager/All#SparkFun_WM8960
WM8960 codec;
 
void setup()
{
 Serial.begin(115200);
    while (!Serial)
    ;
  I2S i2s(INPUT);
  i2s.setBCLK(2);
  //config.pin_ws = 3;
  i2s.setDATA(9);
  i2s.begin(32000);
  i2s.setBitsPerSample(16);

 
  Serial.println("Example 3 - INPUT1");
  
    
 ;

  Wire.setSCL(1);
  Wire.setSDA(0);
  Wire.begin() ;
  //Wire.setClock(10000);





  
  if (codec.begin(Wire) == false) //Begin communication over I2C
  {
    Serial.println("The device did not respond. Please check wiring.");
    //while (1); // Freeze
  }
  Serial.println("Device is connected properly.");

  // General setup needed
  codec.enableVREF();
  codec.enableVMID();

  // Setup signal flow through the analog audio bypass connections

  codec.enableLMIC();
  codec.enableRMIC();
  
  // Connect from INPUT1 to "n" (aka inverting) inputs of PGAs.
  codec.connectLMN1();
  codec.connectRMN1();

  // Disable mutes on PGA inputs (aka INTPUT1)
  codec.disableLINMUTE();
  codec.disableRINMUTE();

  // Set input boosts to get inputs 1 to the boost mixers
  codec.setLMICBOOST(WM8960_MIC_BOOST_GAIN_0DB);
  codec.setRMICBOOST(WM8960_MIC_BOOST_GAIN_0DB);

  codec.connectLMIC2B();
  codec.connectRMIC2B();

  // Enable boost mixers
  codec.enableAINL();
  codec.enableAINR();

  // Connect LB2LO (booster to output mixer (analog bypass)
  codec.enableLB2LO();
  codec.enableRB2RO();

  // Set gainstage between booster mixer and output mixer
  codec.setLB2LOVOL(WM8960_OUTPUT_MIXER_GAIN_0DB); 
  codec.setRB2ROVOL(WM8960_OUTPUT_MIXER_GAIN_0DB); 

  // Enable output mixers
  codec.enableLOMIX();
  codec.enableROMIX();
  
  if (codec.enableHeadphones())
  {Serial.println("enableHeadphones good");}
  codec.enableOUT3MIX(); // Provides VMID as buffer for headphone ground

  
 if ( codec.setHeadphoneVolumeDB(6.00))
 {Serial.println("Volume set to +0dB");}

  Serial.println("Example complete. Listen to INPUT1 on headphone outputs.");
}

void loop()
{
  // Nothing to see here.
}