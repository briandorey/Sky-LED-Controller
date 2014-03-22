/*
 * Control system for sky led controller
 * Conrtols 5 channels of RGB leds setting colour and brightness to match a selected time of day
 * System is controllable via RS485 or the onboard button and pot.
 */

#include <Wire.h>
#include <EEPROM.h>


// Define registers values from datasheet
const byte addr = 0x40;
const byte MODE1 = 0x00;
const byte MODE2 = 0x01;
const byte SUBADR1 = 0x02;
const byte SUBADR2 = 0x03;
const byte SUBADR3 = 0x04;
const byte ALLCALLADR = 0x05;
const byte LED0_ON_L = 0x06;
const byte LED0_ON_H = 0x07;
const byte LED0_OFF_L = 0x08;
const byte LED0_OFF_H = 0x09;
const byte ALL_LED_ON = 0xFA;
const byte ALL_LED_ON_L = 0xFA;
const byte ALL_LED_ON_H = 0xFB;
const byte ALL_LED_OFF_L = 0xFC;
const byte ALL_LED_OFF = 0xFC;
const byte ALL_LED_OFF_H = 0xFD;
const byte PRE_SCALE = 0xFE;

// Define LED channel registers
const byte LED_RED_CHANNEL_1 = 0x1E;
const byte LED_GREEN_CHANNEL_1 = 0x22;
const byte LED_BLUE_CHANNEL_1 = 0x1A;
const byte LED_RED_CHANNEL_2 = 0x12;
const byte LED_GREEN_CHANNEL_2 = 0x16;
const byte LED_BLUE_CHANNEL_2 = 0x0E;
const byte LED_RED_CHANNEL_3 = 0x06;
const byte LED_GREEN_CHANNEL_3 = 0x0A;
const byte LED_BLUE_CHANNEL_3 = 0x3E;
const byte LED_RED_CHANNEL_4 = 0x36;
const byte LED_GREEN_CHANNEL_4 = 0x3A;
const byte LED_BLUE_CHANNEL_4 = 0x32;
const byte LED_RED_CHANNEL_5 = 0x2A;
const byte LED_GREEN_CHANNEL_5 = 0x2E;
const byte LED_BLUE_CHANNEL_5 = 0x26;


// Define Pin addresses
const int POT_PIN = 1;
const int BUTTON_PIN = 8;

// Define Serial constants
const byte SB_ADDRESS = 0x02; 
const byte MAX_MESSAGE_LENGTH = 6;

// global variables
int val = 0;
int prev_val = 0;
byte SB_BUFFER[100];
int timer1_counter;

char system_mode = 0;  // 0 = off, 1 = local control, 2 = all leds on full
byte time = 0;
byte oldtime = 0;
word brightness = 0;
word oldbrightness = 0;

// led colour variables

word channel_1_Red_Value = 0;
word channel_1_Green_Value = 0;
word channel_1_Blue_Value = 0;

word channel_2_Red_Value = 0;
word channel_2_Green_Value = 0;
word channel_2_Blue_Value = 0;

word channel_3_Red_Value = 0;
word channel_3_Green_Value = 0;
word channel_3_Blue_Value = 0;

word channel_4_Red_Value = 0;
word channel_4_Green_Value = 0;
word channel_4_Blue_Value = 0;

word channel_5_Red_Value = 0;
word channel_5_Green_Value = 0;
word channel_5_Blue_Value = 0;

word channel_1_Red_Target = 0;
word channel_1_Green_Target = 0;
word channel_1_Blue_Target = 0;

word channel_2_Red_Target = 0;
word channel_2_Green_Target = 0;
word channel_2_Blue_Target = 0;

word channel_3_Red_Target = 0;
word channel_3_Green_Target = 0;
word channel_3_Blue_Target = 0;

word channel_4_Red_Target = 0;
word channel_4_Green_Target = 0;
word channel_4_Blue_Target = 0;

word channel_5_Red_Target = 0;
word channel_5_Green_Target = 0;
word channel_5_Blue_Target = 0;

char updateChannel_1 = 0;
char updateChannel_2 = 0;
char updateChannel_3 = 0;
char updateChannel_4 = 0;
char updateChannel_5 = 0;

// main code
void setup()
{
  Serial.begin(115200); // set up serial
  Serial.flush();
  pinMode(A0, OUTPUT); // pin for output enable
  pinMode(A1, INPUT); // pin for variable pot
  pinMode(BUTTON_PIN, INPUT); // pin for push button

  Wire.begin();           // Wire must be started!

  system_mode = EEPROM.read(0x01);

  initPWM();  
  setAllLEDOff();
  outputEnable();   

}

void loop(){
  // system modes 0 = off, 1 = local control, 2 = all leds on full, 3 = remote control

  // check if the pushbutton is pressed.
  // if it is, the buttonState is HIGH:

  if (digitalRead(BUTTON_PIN) == HIGH) {    

    while (digitalRead(BUTTON_PIN) == HIGH){  // wait until the button is depressed     
      delay(100); 
    }

    system_mode++;
    if (system_mode >= 3){ 
      system_mode = 0; 
    } // increment system_mode and reset if rolls over to 4

    if (system_mode == 0){ // flash red and then turn off the leds
      setAllLEDOff(); 
      setLED_RGB(5, 200, 0, 0); 
      delay(300); 
      setAllLEDOff(); 
      outputDisable();
      EEPROM.write(0x01, 0);
    } 
    if (system_mode == 1){ // flash green and then go to local control
      outputEnable(); 
      setAllLEDOff(); 
      setLED_RGB(5, 0, 200, 0); 
      delay(300); 
      setAllLEDOff(); 
      EEPROM.write(0x01, 1);
    }  
    if (system_mode == 2){ // turn on the leds and adjust brighness using pot
      outputEnable();       
      EEPROM.write(0x01, 2);
    } 
  }


  if (system_mode == 1){
    val = analogRead(POT_PIN);
        if (val < 42) time = 0;
        if (val >= 42 && val < 84) time = 1;
        if (val >= 84 && val < 1416) time = 2;
        if (val >= 126 && val < 168) time = 3;
        if (val >= 168 && val < 210) time = 4;
        if (val >= 210 && val < 252) time = 5;
        if (val >= 252 && val < 294) time = 6;
        if (val >= 294 && val < 336) time = 7;
        if (val >= 336 && val < 378) time = 8;
        if (val >= 378 && val < 420) time = 9;
        if (val >= 420 && val < 462) time = 10;
        if (val >= 462 && val < 504) time = 11;
        if (val >= 504 && val < 546) time = 12;
        if (val >= 546 && val < 588) time = 13;
        if (val >= 588 && val < 630) time = 14;
        if (val >= 630 && val < 672) time = 15;
        if (val >= 672 && val < 714) time = 16;
        if (val >= 714 && val < 756) time = 17;
        if (val >= 756 && val < 798) time = 18;
        if (val >= 798 && val < 840) time = 19;
        if (val >= 840 && val < 882) time = 20;
        if (val >= 882 && val < 924) time = 21;
        if (val >= 924 && val < 966) time = 22;
        if (val >= 966 && val < 1008) time = 23;
        if (val >= 1008) time = 0;
    
    if (oldtime != time){
      oldtime = time;
      setTimeOfDay(time);
    }
    
    
  }
  
  if (system_mode == 2){ // turn on the leds and adjust brighness using pot
      
        val = analogRead(POT_PIN);
        brightness = val * 2;
    if ((prev_val > val + 20) || (prev_val < val - 20)){
      prev_val = val;
      setLED_RGB(1, brightness, brightness, brightness);
      setLED_RGB(2, brightness, brightness, brightness);
      setLED_RGB(3, brightness, brightness, brightness);
      setLED_RGB(4, brightness, brightness, brightness);
      setLED_RGB(5, brightness, brightness, brightness);
    }

   }
 
//if (system_mode == 1 || system_mode >= 3){
//    updateDisplay();  
//}

}

// Serial Events ===========================================
void serialEvent(){
    delay(1);
    receive_request();
}

int receive_request() 
{

        byte bytes_received = 0;
        byte current_byte = 0;
        byte packet_start = 0;
        byte packet_length = 0;
        
        system_mode = 3; // set program to be remote control

        while (Serial.available()) {            
                current_byte = byte(Serial.read());
                if (current_byte == SB_ADDRESS && packet_start == 0){ // ignore bytes until an address byte is found                  
                  packet_start = 1;
                }
                if (bytes_received == 1){
                  packet_length = current_byte;
                }
                if (packet_start == 1){
                    SB_BUFFER[bytes_received] = current_byte;
                    bytes_received++;
                }                                               
        }
 
       if (SB_BUFFER[packet_length] == 0xFF){
          processSerial();
       }
}

void processSerial(){
 //flashled();
  if (SB_BUFFER[0] == 0x02){ // serial is meant for this device     
    switch (SB_BUFFER[2]){
    case 0x01: // turn all leds on full             
      outputEnable(); 
      setAllLEDOn();
      break;
    case 0x02: // flash red and then turn off the leds 
      setAllLEDOff(); 
      setLED_RGB(5, 200, 0, 0); 
      delay(300); 
      setAllLEDOff(); 
      outputDisable();
      break;

    case 0x03: // set led channel colours
      outputEnable(); 
      setLED_RGB(byte(SB_BUFFER[3]), word(SB_BUFFER[4],SB_BUFFER[5]), word(SB_BUFFER[6],SB_BUFFER[7]), word(SB_BUFFER[8],SB_BUFFER[9]));
      break;

    case 0x04: // set time of day
      outputEnable(); 
      setTimeOfDay(SB_BUFFER[3]);
      break;   

    default:
      break; 
    }


  }

}

// LED Control Events =========================================================

void flashled(){
       setAllLEDOff(); 
      setLED_RGB(5, 200, 0, 0); 
      delay(200); 
      setAllLEDOff(); 
}


void setTimeOfDay(byte time){
  switch (time){
  case 0: // 12am
    setLED_RGB(1, 0, 100, 300);
    setLED_RGB(2, 0, 100, 300);
    setLED_RGB(3, 0, 100, 300);
    setLED_RGB(4, 0, 100, 300);
    setLED_RGB(5, 0, 100, 300);
    break;
  case 1: // 1am
    setLED_RGB(1, 0, 100, 300);
    setLED_RGB(2, 0, 100, 300);
    setLED_RGB(3, 0, 100, 300);
    setLED_RGB(4, 0, 100, 300);
    setLED_RGB(5, 0, 100, 300);
    break;
  case 2: // 2am
    setLED_RGB(1, 0, 544, 416);
    setLED_RGB(2, 0, 544, 416);
    setLED_RGB(3, 0, 544, 416);
    setLED_RGB(4, 0, 544, 416);
    setLED_RGB(5, 0, 544, 416);
    break;
  case 3: // 3am
    setLED_RGB(1, 0, 784, 1296);
    setLED_RGB(2, 0, 784, 1296);
    setLED_RGB(3, 0, 784, 1296);
    setLED_RGB(4, 0, 784, 1296);
    setLED_RGB(5, 0, 784, 1296);
    break;
  case 4: // 4am
    setLED_RGB(1, 0, 784, 1296);
    setLED_RGB(2, 0, 784, 1296);
    setLED_RGB(3, 0, 784, 1296);
    setLED_RGB(4, 0, 784, 1296);
    setLED_RGB(5, 0, 784, 1296);
    break;
  case 5: // 5am
    setLED_RGB(1, 4080, 2688, 0);
    setLED_RGB(2, 0, 784, 1296);
    setLED_RGB(3, 0, 784, 1296);
    setLED_RGB(4, 0, 784, 1296);
    setLED_RGB(5, 0, 784, 1296);
    break;
  case 6: // 6am
    setLED_RGB(1, 2688, 2688, 2688);
    setLED_RGB(2, 4080, 2688, 0);
    setLED_RGB(3, 0, 2496, 4080);
    setLED_RGB(4, 0, 2496, 4080);
    setLED_RGB(5, 0, 2496, 4080);
    break;
  case 7: // 7am
    setLED_RGB(1, 4080, 3920, 3568);
    setLED_RGB(2, 4080, 3920, 3568);
    setLED_RGB(3, 4080, 3920, 3568);
    setLED_RGB(4, 4080, 3920, 3568);
    setLED_RGB(5, 2880, 3248, 3472);
    break;
  case 8: // 8am
    setLED_RGB(1, 4080, 4080, 4080);
    setLED_RGB(2, 3888, 3888, 3888);
    setLED_RGB(3, 3888, 3888, 3888);
    setLED_RGB(4, 3888, 3888, 3888);
    setLED_RGB(5, 3536, 3536, 3536);
    break;
  case 9: // 9am
    setLED_RGB(1, 3888, 3888, 3888);
    setLED_RGB(2, 4080, 4080, 4080);
    setLED_RGB(3, 4080, 4080, 4080);
    setLED_RGB(4, 3888, 3888, 3888);
    setLED_RGB(5, 3888, 3888, 3888);
    break;
  case 10: // 10am
    setLED_RGB(1, 3888, 3888, 3888);
    setLED_RGB(2, 4080, 4080, 4080);
    setLED_RGB(3, 4080, 4080, 4080);
    setLED_RGB(4, 3888, 3888, 3888);
    setLED_RGB(5, 3888, 3888, 3888);
    break;
  case 11: // 11am
    setLED_RGB(1, 3888, 3888, 3888);
    setLED_RGB(2, 4080, 4080, 4080);
    setLED_RGB(3, 4080, 4080, 4080);
    setLED_RGB(4, 4080, 4080, 4080);
    setLED_RGB(5, 3888, 3888, 3888);
    break;
  case 12: // 12pm
    setLED_RGB(1, 4095, 4095, 4095);
    setLED_RGB(2, 4095, 4095, 4095);
    setLED_RGB(3, 4095, 4095, 4095);
    setLED_RGB(4, 4095, 4095, 4095);
    setLED_RGB(5, 4095, 4095, 4095);
    break;
  case 13: // 1pm
    setLED_RGB(1, 4095, 4095, 4095);
    setLED_RGB(2, 4095, 4095, 4095);
    setLED_RGB(3, 4095, 4095, 4095);
    setLED_RGB(4, 4095, 4095, 4095);
    setLED_RGB(5, 4095, 4095, 4095);
    break;
  case 14: // 2pm
    setLED_RGB(1, 3888, 3888, 3888);
    setLED_RGB(2, 4080, 4080, 4080);
    setLED_RGB(3, 4080, 4080, 4080);
    setLED_RGB(4, 4080, 4080, 4080);
    setLED_RGB(5, 3888, 3888, 3888);
    break;
  case 15: // 3pm
    setLED_RGB(1, 3888, 3888, 3888);
    setLED_RGB(2, 3888, 3888, 3888);
    setLED_RGB(3, 4080, 4080, 4080);
    setLED_RGB(4, 4080, 4080, 4080);
    setLED_RGB(5, 3888, 3888, 3888);
    break;
  case 16: // 4pm
    setLED_RGB(1, 3888, 3888, 3888);
    setLED_RGB(2, 3888, 3888, 3888);
    setLED_RGB(3, 4080, 4080, 4080);
    setLED_RGB(4, 4080, 4080, 4080);
    setLED_RGB(5, 3888, 3888, 3888);
    break;
  case 17: // 5pm
    setLED_RGB(1, 3536, 3536, 3536);
    setLED_RGB(2, 3888, 3888, 3888);
    setLED_RGB(3, 3888, 3888, 3888);
    setLED_RGB(4, 3888, 3888, 3888);
    setLED_RGB(5, 4080, 4080, 4080);
    break;
  case 18: // 6pm
    setLED_RGB(1, 2880, 3248, 3472);
    setLED_RGB(2, 4080, 3920, 3568);
    setLED_RGB(3, 4080, 3920, 3568);
    setLED_RGB(4, 4080, 3920, 3568);
    setLED_RGB(5, 4080, 3920, 3568);
    break;
  case 19: // 7pm
    setLED_RGB(1, 0, 2496, 4095);
    setLED_RGB(2, 0, 2496, 4095);
    setLED_RGB(3, 0, 2496, 4095);
    setLED_RGB(4, 4080, 2688, 0);
    setLED_RGB(5, 2688, 2688, 2688);
    break;
  case 20: // 8pm
    setLED_RGB(1, 0, 784, 1296);
    setLED_RGB(2, 0, 784, 1296);
    setLED_RGB(3, 0, 784, 1296);
    setLED_RGB(4, 0, 2496, 4080);
    setLED_RGB(5, 4080, 2688, 0);
    break;
  case 21: // 9pm
    setLED_RGB(1, 0, 784, 1296);
    setLED_RGB(2, 0, 784, 1296);
    setLED_RGB(3, 0, 784, 1296);
    setLED_RGB(4, 0, 784, 1296);
    setLED_RGB(5, 0, 2496, 4080);
    break;
  case 22: // 10pm
    setLED_RGB(1, 0, 784, 1296);
    setLED_RGB(2, 0, 784, 1296);
    setLED_RGB(3, 0, 784, 1296);
    setLED_RGB(4, 0, 784, 1296);
    setLED_RGB(5, 0, 784, 1296);
    break;
  case 23: // 11pm
    setLED_RGB(1, 0, 544, 416);
    setLED_RGB(2, 0, 544, 416);
    setLED_RGB(3, 0, 544, 416);
    setLED_RGB(4, 0, 544, 416);
    setLED_RGB(5, 0, 544, 416);
    break;
  } 


}



bool initPWM() {

  delay(10);
  writeRegister(MODE1, (byte)0x01);	// reset the device

  delay(10);
  bool isOnline;
  if (byte(readRegister(MODE1))==0x01)	{
    isOnline = true;

  } 
  else {
    isOnline = false;

  }
  setPWMFreq(1000);

  writeRegister(MODE1, (byte)B10100000);	// set up for auto increment
  writeRegister(MODE2, (byte)B00001100);	// set to output

  return isOnline;
}

void setLEDOn(byte ledNumber) {
  writeLED(ledNumber, 0x1000, 0);
}

void setLEDOff(byte ledNumber) {
  writeLED(ledNumber, 0, 0x1000);
}

void setAllLEDOn() {        
    setLED_RGB(1, 4095, 4095, 4095);
    setLED_RGB(2, 4095, 4095, 4095);
    setLED_RGB(3, 4095, 4095, 4095);
    setLED_RGB(4, 4095, 4095, 4095);
    setLED_RGB(5, 4095, 4095, 4095);
}

void setAllLEDOff() {
  writeAllLED(0, 0x1000);
}


void setLED_RGB(byte channel, word red, word green, word blue){
  word red_off = red;
  word red_on = 4095 - red_off;
  word green_off = green;
  word green_on = 4095 - green_off;      
  word blue_off = blue;
  word blue_on = 4095 - blue_off;   

  switch (channel){
  case 1:
    writeLED(LED_RED_CHANNEL_1, red_on, red_off);
    writeLED(LED_GREEN_CHANNEL_1, green_on, green_off);      
    writeLED(LED_BLUE_CHANNEL_1, blue_on, blue_off);
    break;
  case 2:
    writeLED(LED_RED_CHANNEL_2, red_on, red_off);
    writeLED(LED_GREEN_CHANNEL_2, green_on, green_off);      
    writeLED(LED_BLUE_CHANNEL_2, blue_on, blue_off);
    break;
  case 3:
    writeLED(LED_RED_CHANNEL_3, red_on, red_off);
    writeLED(LED_GREEN_CHANNEL_3, green_on, green_off);      
    writeLED(LED_BLUE_CHANNEL_3, blue_on, blue_off);
    break;
  case 4:
    writeLED(LED_RED_CHANNEL_4, red_on, red_off);
    writeLED(LED_GREEN_CHANNEL_4, green_on, green_off);      
    writeLED(LED_BLUE_CHANNEL_4, blue_on, blue_off);
    break;
  case 5:
    writeLED(LED_RED_CHANNEL_5, red_on, red_off);
    writeLED(LED_GREEN_CHANNEL_5, green_on, green_off);      
    writeLED(LED_BLUE_CHANNEL_5, blue_on, blue_off);
    break; 
  }    
}

void writeLED(byte ledNumber, word LED_ON, word LED_OFF) {	// LED_ON and LED_OFF are 12bit values (0-4095);
    Wire.beginTransmission(addr);
    Wire.write(ledNumber);
    
    Wire.write(lowByte(LED_ON));
    Wire.write(highByte(LED_ON));
    Wire.write(lowByte(LED_OFF));
    Wire.write(highByte(LED_OFF));

    Wire.endTransmission();
}

void writeAllLED(word LED_ON, word LED_OFF) {	// LED_ON and LED_OFF are 12bit values (0-4095);
  Wire.beginTransmission(addr);
  Wire.write(ALL_LED_ON);
  Wire.write(lowByte(LED_ON));
  Wire.write(highByte(LED_ON));
  Wire.write(lowByte(LED_OFF));
  Wire.write(highByte(LED_OFF));
  Wire.endTransmission();
}

void writeRegister(int regAddress, byte data) {
  Wire.beginTransmission(addr);
  Wire.write(regAddress);
  Wire.write(data);
  Wire.endTransmission();
}

word readRegister(int regAddress) {
  word returnword = 0x00;
  Wire.beginTransmission(addr);
  Wire.write(regAddress);
  Wire.endTransmission();
  Wire.requestFrom((int)addr, 1);
}

void setPWMFreq(float freq) {

  float prescaleval = 25000000;
  prescaleval /= 4096;
  prescaleval /= freq;
  prescaleval -= 1;
  uint8_t prescale = floor(prescaleval + 0.5);

  uint8_t oldmode = readRegister(MODE1);
  //printf("oldmode = %d\n", oldmode);// = 1 for resetDev(fh) in main()

  uint8_t newmode = (oldmode&0x7F) | 0x10; // sleep

  writeRegister(MODE1, newmode); // go to sleep

  writeRegister(PRE_SCALE, prescale); // set the prescaler

  writeRegister(MODE1, oldmode);

  writeRegister(MODE1, oldmode | 0xa1);// MODE1 to auto increment
}



// disable output via OE pin
void outputDisable(){
  digitalWrite(A0, HIGH);
}
// enable output via OE pin
void outputEnable(){
  digitalWrite(A0, LOW); 
}





