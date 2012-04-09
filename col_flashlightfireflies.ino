
//#include <DmxInterrupt.h>
#include <DmxSimple.h>


//output pins

//digital pin controlling ringLight
int ringLightPin = 9;
int ringBrightness = 32;
//digital pin controlling example pattern light
int patternLightPin = 8;
//digital pin controlling button light
int buttonLightPin = 10;
int buttonBrightness = 128;

//input pins
//digital pin sensing button presses
int buttonPin = 11;
int patternJumperPin = 12; 


//DMX messages
int dmxChan1 = 400;
int dmxVal1 = 255;
int dmxChan2 = 410;
int dmxVal2 = 255;
long successDelay = 7000;  //how long are deck controls locked out for dmx pattern to play


//button press variables
long buttonChangeOn;
long buttonOn;
long buttonOff;
long buttonLightOn;
long patternLightOn=0;
long patternLightOff=0;
long ringLightOn;
//keeping track of success/failure
int matchState=0;
long matchTime=1;
long failTime=1;
int matchTotal;
int failTotal;
int sucState=1;
int successOn=0;



//difficulty adjustments
  
  float matchRatio =.6; //% of overlap in button/light state require to match
  float failRatio =1;  //%  no of overlap in button/light state required to fail
  int matchTarget=10; // total matches needed to achieve success
  int failTarget=5; //total fails needed to reset to 0


long blinkTime=500;//base time unit for blink pattern

//button variables
int buttonPressed;//is button momentarily pressed?
int buttonState=0;// is the button on? (to debounce)


//button light variables --  off while button is in use, goes on after timeout if not active
int buttonLightState; //is button actively in use?
long buttonLightTime;
long buttonLightTimeout = 2000;

int patternLightState;
int ringLightState;

//patterns times
long pattern1Time[] = {blinkTime,blinkTime,blinkTime,blinkTime,blinkTime,blinkTime,blinkTime,blinkTime};
long pattern2Time[] = {blinkTime/2,blinkTime/2,blinkTime/2,blinkTime,blinkTime/2,blinkTime/2,blinkTime/2,blinkTime};

//patterns on/off rhythms
int pattern1State[] ={1,0,1,0,1,0,1,0};
int pattern2State[] ={1,0,1,0,1,0,1,0};

int patternCounter=0;
int patternTotal=8;


void setup(){


Serial.begin(9600);  

DmxSimple.usePin(2); // use 3 for rev. E DMX Sheilds, 2 for rev. F.
DmxSimple.maxChannel(1); //send 1 channel of dmx

//set pin modes
pinMode(buttonLightPin, OUTPUT);
pinMode(patternLightPin, OUTPUT);
pinMode(ringLightPin, OUTPUT);

pinMode(buttonPin, INPUT);
pinMode(patternJumperPin, INPUT);

buttonLightState=1;
digitalWrite(buttonLightPin, HIGH);

Serial.println("Creatures of Light: Nature's Bioluminescence");
Serial.print("The state of the jumper pin is: ");
Serial.println(digitalRead(patternJumperPin));
}

void loop(){

if(successOn==0){
if(digitalRead(patternJumperPin)==1){
 patternBlink(pattern2Time, pattern2State);
}
else
 //blink pattern light and check for button match
 patternBlink(pattern1Time, pattern1State);

}
//if pattern is a match trigger dmx lights
  if(successOn==1){
 dmxSend();
  }

}

void buttonCheck(){
buttonPressed=digitalRead(buttonPin);

//Serial.println(buttonPressed);
if(buttonPressed==1 && buttonState!=1){

buttonOn=millis();
buttonState=1;

analogWrite(ringLightPin,ringBrightness);
//digitalWrite(ringLightPin, HIGH);

buttonLightTime=millis();
if(buttonLightState==1){
//digitalWrite(buttonLightPin, LOW);
analogWrite(buttonLightPin,0);
buttonLightState=0;
}


}

if(buttonPressed==0 && buttonState!=0){
 
buttonOff=millis();
buttonState=0;
//digitalWrite(ringLightPin, LOW);
analogWrite(ringLightPin,0);


}

//if button light is off and button is inactive for timout period, turn it on & reset the match total
if(buttonState==0){
if(buttonLightState==0 && millis()-buttonLightTime>buttonLightTimeout){
//digitalWrite(buttonLightPin, HIGH);
analogWrite(buttonLightPin,buttonBrightness);
  buttonLightState=1;
  matchTotal=0;
  failTotal=0;
}

}
}


//function to blnk indicator light
void patternBlink(long patternTime[], int pattern[]){

 //check time and advance pattern
 if((millis()-patternLightOn)>patternTime[patternCounter]){
 
   
   if(patternCounter<patternTotal){
   patternCounter++;
   
   digitalWrite(patternLightPin, pattern[patternCounter]);
   patternLightOn = millis();
  // Serial.println(patternCounter);

 
   }
   //loop pattern at the end
  if(patternCounter==patternTotal){patternCounter=0;
  digitalWrite(patternLightPin, pattern[patternCounter]);
   
   patternLightOn = millis();
  }

 }
  buttonCheck();

//check for match & start timing overlap
if(pattern[patternCounter] == buttonState && matchState==0){
  matchState=1;
  matchTime=millis();

}

//check for mismatch & start timing lack of overlap
if(pattern[patternCounter] != buttonState && matchState==1){
  matchState=0;
  failTime=millis();
 
   
}

//add a match to the total if the button/light overlap time exceeded the threshold ratio for this step of the  pattern
if(millis()-matchTime>patternTime[patternCounter]*matchRatio && matchTime!=0 && buttonLightState==0){

matchTime=millis();
matchTotal++;
 //Serial.print("match total"); //for debug
 //Serial.println(matchTotal); //for debug
}

//add a fail to the total if there button/light mismatch exceeded the threshold ration for this step of the  pattern
if(millis()-failTime>patternTime[patternCounter]*failRatio && failTime!=0 && buttonLightState==0){

 failTime=millis();
matchTime=0;
failTotal++;
 //Serial.print("fail total"); //for debug
 //Serial.println(failTotal); //for debug

}
 
 // fail condition -- if fail total surpasses the target, reset match & fail totals
 if(failTotal>=failTarget){
 //   Serial.println("FAIL"); //for debug
 matchTotal=0;
   failTotal=0;
 }
 
 //Success condition -- if match total surpasses the target, trigger dmx lights
 if(matchTotal>matchTarget){
   // Serial.println("SUCCESS"); //for debug
    successOn=1;
   matchTotal=0;
   failTotal=0;
 }
 

}

void dmxSend() {
  
    Serial.println("Success"); //for debug
     Serial.print("Sending ");
     Serial.print(dmxVal1);
     Serial.print(" to channel ");
   
 
   
   if(digitalRead(patternJumperPin)==1){
      DmxSimple.write(dmxChan2, dmxVal1); // Set DMX channel 1 to new value
          Serial.println(dmxChan2);
   } else {
      DmxSimple.write(dmxChan1, dmxVal1); // Set DMX channel 1 to new value
          Serial.println(dmxChan1);
   }
   // DmxSimple.write(dmxChan2, dmxVal2); // Set DMX channel 1 to new value
    digitalWrite(patternLightPin, LOW);
    digitalWrite(ringLightPin, LOW);
    digitalWrite(buttonLightPin, LOW);
    
    
    delay(successDelay);
    Serial.println("setting back to 0.");
      if(digitalRead(patternJumperPin)==1){
    DmxSimple.write(dmxChan2, 0); // Set DMX channel 1 to new value
    
       } else {
     DmxSimple.write(dmxChan1, 0); // Set DMX channel 1 to new value      
       }
  
   // DmxSimple.write(dmxChan2, 0); // Set DMX channel 1 to new value
   
   //reset our variables for good measure
   successOn=0;
   matchTotal=0;
   failTotal=0;
   buttonLightState=1;
   digitalWrite(buttonLightPin, HIGH);
}
   

