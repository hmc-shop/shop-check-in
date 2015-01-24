/*
Richard Piersall - rpiersall@g.hmc.edu
Taylor Peterson  - tpeterson@g.hmc.edu

Arduino code to communicate over USB serial with computer
Read in strings and write to LCD
Read in switch states and write out to serial

*/


// Include the LiquidCrystal Library for writing to LCD:
#include <LiquidCrystal.h>

// Pinout
#define SIG0  0 // Signal Input Pins
#define SIG1  1 
#define SEL0  5 // Signal Select Pins
#define SEL1  4
#define SEL2  3
#define SEL3  2
#define PWR   7 // Power Switch Pin
#define in0   8 // Button Pins
#define in1   9
#define in2  10
#define in3  11
#define in4  12 

// Constants
const int STRLEN = 16;      // Character-width of LCD row
const int INITDELAY = 1000; // Multiplexer post-reset delay
const int SHIFTTIME = 62;   // Time between multiplexer shifts

// Initializing Variables
int lcdCursor;
int buttonState[5];
int buttonRead[5];
int switchRead;
int switchState;
int matrixState[32];
int selectPins;
boolean mux0, mux1;
unsigned long nextShift;
unsigned long currentTime;
unsigned long difference;

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(18, 19, 20, 21, 22, 23);



void setup() {
  Serial.begin(115200);

  initSwitch();
  initButtons();
  initMuxer();  
  resetMuxer();
  lcd.begin(STRLEN, 2);
  
  nextShift = micros() + INITDELAY;  
  currentTime = micros();
}

void loop() {
  readSwitch();
  readButtons();
  readMuxer();
  lcdScroll();
}
  



void initSwitch(){
  pinMode(PWR, INPUT_PULLUP);
}

void readSwitch(){
  switchRead = digitalRead(PWR);
  if (switchState != switchRead){
    Serial.print("S");
    Serial.println(switchRead);
    switchState = switchRead;
  }
}
  


void initButtons(){
  pinMode(in0, INPUT_PULLUP);
  pinMode(in1, INPUT_PULLUP);
  pinMode(in2, INPUT_PULLUP);
  pinMode(in3, INPUT_PULLUP);
  pinMode(in4, INPUT_PULLUP);
}



void readButtons(){
  buttonRead[0] = !digitalRead(in0);
  buttonRead[1] = !digitalRead(in1);
  buttonRead[2] = !digitalRead(in2);
  buttonRead[3] = !digitalRead(in3);
  buttonRead[4] = !digitalRead(in4);  
  
  for(int i=0; i<5; i++){
    if (buttonRead[i] != buttonState[i]){
      Serial.print("B");
      Serial.println(i);
      buttonState[i] = buttonRead[i];
    }
  }
}



void initMuxer(){
  pinMode(SIG0, INPUT);
  pinMode(SIG1, INPUT);
  pinMode(SEL0, OUTPUT);
  pinMode(SEL1, OUTPUT);
  pinMode(SEL2, OUTPUT);
  pinMode(SEL3, OUTPUT);
}



void resetMuxer(){
  selectPins = 0;
  digitalWrite(SEL0, LOW);
  digitalWrite(SEL1, LOW);
  digitalWrite(SEL2, LOW);
  digitalWrite(SEL3, LOW);
}



void readMuxer(){
  currentTime = micros();
  difference = currentTime - nextShift;
  if (difference > SHIFTTIME && difference < 10*SHIFTTIME){
    mux0 = digitalRead(SIG0);
    mux1 = digitalRead(SIG1);
    
    if (mux0 != matrixState[selectPins]){
      Serial.print("M");
      Serial.print(mux0);
      Serial.print(selectPins);
    }      
    if (mux1 != matrixState[selectPins + 16]){
      Serial.print("M");
      Serial.print(mux1);
      Serial.print(selectPins + 16);
    }         
      
    matrixState[selectPins]      = mux0;
    matrixState[selectPins + 16] = mux1;
    selectPins = (selectPins + 1) % 16;
    
    digitalWrite(SEL0, (selectPins & 1) >> 0);
    digitalWrite(SEL1, (selectPins & 2) >> 1);
    digitalWrite(SEL2, (selectPins & 4) >> 2);
    digitalWrite(SEL3, (selectPins & 8) >> 3);
          
    nextShift = nextShift+SHIFTTIME;   
  } 
}



void lcdScroll(){
  if (Serial.available()) {
    if (lcdCursor > 2*STRLEN-1) {
      lcdCursor = 0;
      lcd.clear();
    }
    else lcd.setCursor(lcdCursor%STRLEN, lcdCursor/STRLEN);
    
    char c = Serial.read();
    if      (c == '\0') lcdCursor = 2*STRLEN;
    else if (c == '\n') lcdCursor = lcdCursor + STRLEN;
    else if (c == '\r') lcdCursor = (lcdCursor/STRLEN)*STRLEN;
    else {
      lcd.write(c);
      lcdCursor++;  
    }
  }
}



