//Authors: Chad Lani and Nicholas Evich
//Purpose: This code is to run a cyclic pneumatic transfer system tied to a data acqusition system that would allow
//for the detection of shortlived radio-isotopes following irradiation.
//---------------------------------------------------------------------------------- -
#ifdef SUBCORE
#error "Core selection is wrong!!"
#endif

#include <MP.h>

//---------------------------------------------------------------------------------- -
// Input Parameters
// Change only these, upload script, and press the button to begin
int cycleCount = 500;
uint8_t radLength = 3; // in seconds
uint32_t countLength = 120; // in seconds
//
//
//

const byte numChars = 32;
char receivedChars[numChars];
String tempString = "";
boolean newData = false;
char check;
int t = 0;

int sensorValue1 = 0;  // variable to store the value coming from the sensor
int sensorValue2 = 0;
int sensorValue3 = 0;
int sensorValue4 = 0;

//parameters used for core communication
uint32_t *rcv1;
uint32_t *rcv2;
uint32_t *rcv3;
uint32_t *rcv4;

int8_t   rcvid1;
int8_t   rcvid2;
int8_t   rcvid3;
int8_t   rcvid4;

uint32_t *rcv5;
int8_t   rcvid5;

//parameters for sensors stepper motor
int sensorThresh = 100;
int ret;
int steps = 60;  //full open is 60

void beginCycles();
void cycleOnce();



// To do:
// Make this code robust so that the microsteps per revolution do not matter and are coded into a variable
// then a scaling factor can be applied anywhere a number of steps is referenced

#define MICROSTEPS 400

//This class defines all of the functions of the stepper motor such as sending and returning the sample
class StepMotor {

  public:
    StepMotor() {
      pinMode(getPulsePin(), OUTPUT);
      pinMode(getEnabPin(), OUTPUT);
      pinMode(getDirPin(), OUTPUT);
      pinMode(getRelayPin(), OUTPUT);
      //pinMode(getSendPin(), INPUT);
      //pinMode(getReturnPin(), INPUT);
      //pinMode(getHomePin(), INPUT);

      for (int i = 8; i <= 13; i++) {
        digitalWrite(i, LOW);
      }
      
      randomPos();
      homeValve();
      
      turnOn();
    }

    // The purpose of this function is to start the valve at a random (not homed) position
    //    so that the homing function works consistently each time
    // If the valve starts at a "homed" position (of which there are many), then consistency
    //    will not exist
    void randomPos() {
      Serial.print("Generating a random starting position before homing sequence executes\n");
            if (digitalRead(getDirPin()) == HIGH){
        digitalWrite(getDirPin(),LOW);
      }
      else{
        digitalWrite(getDirPin(),HIGH);
      }
      for (int i = 0; i < random(20, 100); i++) {
        digitalWrite(getPulsePin(), HIGH);
        delay(25);
        digitalWrite(getPulsePin(), LOW);
        delay(25);
      }
      delay(1000);
    }

    void microstep(float deg) {
      int steper = static_cast<int>(MICROSTEPS * (deg / 360.0));
      //Serial.print("Stepping now\n");

      for (int stepCounter = 0; stepCounter < steper; stepCounter++) {
        digitalWrite(getPulsePin(), HIGH);
        delay(3);
        digitalWrite(getPulsePin(), LOW);
        delayMicroseconds(5);
        //delay(1);
        //Serial.print("Step\n");
      }
    }

    void homeValve() {
      Serial.print("Starting to home valve\n");
      if (digitalRead(getDirPin()) == HIGH){
        digitalWrite(getDirPin(),LOW);
      }
      else{
        digitalWrite(getDirPin(),HIGH);
      }
      while (analogRead(getHomePin()) < 800) {    //was 75, altered for testing purposes
        digitalWrite(getPulsePin(), HIGH);
        delay(50);
        digitalWrite(getPulsePin(), LOW);
        delay(50);
      }

      // Hardcoded fix for valve fabrication imperfections
      microstep(8);

      Serial.print("Homing of DP Valve Complete\n");
      ret = MP.begin(4);
    if (ret < 0) {
      printf("MP.begin(%d) error = %d\n", 4, ret);
    }
      //----- For Testing Purposes Only ---------------
      // digitalWrite(getRelayPin(), HIGH);
    }

    void sendSample() {
      //Serial.print("Sending sample to reactor\n");

      // The HIGH direction corresponds to clockwise
      digitalWrite(getDirPin(), HIGH);

      // Step the motor 67.5 degrees to send the sample
      microstep(steps);
    }

    void returnSample() {
      //Serial.print("Returning sample to detector\n");

      // The LOW direction corresponds to counterclockwise
      digitalWrite(getDirPin(), LOW); // note the direction

      // Step the motor 67.5 degrees to return the sample
      microstep(steps);
    }

    void slowSample() {
      //steps the motor so the sample slows
      if (digitalRead(getDirPin()) == HIGH) {
        digitalWrite(getDirPin(), LOW);
      }
      else {
        digitalWrite(getDirPin(), HIGH);
      }
     // Serial.println("Slowing the sample");

      microstep(steps - 10);

    }

    void turnOff() {
      ret=MP.end(4);
      Serial.println("Turning Pump off");
      digitalWrite(getRelayPin(), LOW);
    }

    void turnOn() {
      digitalWrite(getRelayPin(), HIGH);
      Serial.println("Turning pump on");
      delay(5000);
    }

    // Getters
    byte getPulsePin() {
      return PULSE_PIN;
    }
    byte getEnabPin() {
      return ENAB_PIN;
    }
    byte getDirPin() {
      return DIR_PIN;
    }
    byte getRelayPin() {
      return VACUUM_RELAY;
    }
    byte getSendPin() {
      return BUTTON_SEND;
    }
    byte getReturnPin() {
      return BUTTON_RETURN;
    }
    byte getHomePin() {
      return HOMING_SENSOR;
    }

    ~StepMotor() {

    }

  private:
    // Output pin for pulsing the stepper motor
    const byte PULSE_PIN = 9;
    // Output pin for setting the direction of rotation for the stepper motor
    const byte DIR_PIN = 8;
    // Output pin for enabling the stepper motor
    const byte ENAB_PIN = 11;
    // Output pin for switching the solid-state relay
    const byte VACUUM_RELAY = 10;
    // Input pin for sending samples down into the Fast Neutron Irradiator (Black)
    const byte BUTTON_SEND = 30;
    // Input pin for returning samples from the Fast Neutron Irradiator (Brown)
    const byte BUTTON_RETURN = 32;
    // Input pin for the photoelectric amplifier
    const byte HOMING_SENSOR = A5;
};
//--------------------------------------------------------------------
//---------------------------------------------------------------------------------- -
void setup() {
  Serial.begin(115200);
  pinMode(2, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  pinMode(6, INPUT);
  pinMode(7, INPUT);
  for (int i = 0; i <= 13; i++) {
    digitalWrite(i, LOW);
  }

  int subid;
  int ret;
  /* Boot SubCore */
  for (subid = 5; subid >= 5; subid--) {
    ret = MP.begin(subid);
    if (ret < 0) {
      printf("MP.begin(%d) error = %d\n", subid, ret);
    }
    //delay(2000);
  }

  //    MP.Recv(&rcvid4, &rcv4, 4);
  //    MP.Recv(&rcvid3, &rcv3, 3);
  //    MP.Recv(&rcvid2, &rcv2, 2);
  //    MP.Recv(&rcvid1, &rcv1, 1);

  //pinMode(0, OUTPUT);
  //pinMode(1, OUTPUT);
  //pinMode(2, OUTPUT);
  //pinMode(3, OUTPUT);
  //set the digital pins controlling solenoids to low
  //digitalWrite(1, LOW); //Det Pressure
  //digitalWrite(2, LOW); //Gen Pressure
  //digitalWrite(3, LOW); //Gen Vent
  //digitalWrite(0, LOW); //Det Vent
  delay(2000);
}


//---------------------------------------------------------------------------------- -
void loop() {
  sensorValue1 = digitalRead(2);

  if (sensorValue1 == HIGH) {
    delay(2000);
    sensorValue1 = digitalRead(2);
    if (sensorValue1 == HIGH) {
      Serial.println("Starting Begin Cycles");
      delay(2000);
      beginCycles();

    }
    else {
      Serial.println("Return Capsule");
      delay(2000);
      cycleOnce();
    }
  }
}

//---------------------------------------------------------------------------------- -
void cycleOnce() { //return
  /*
  sensorValue1 = digitalRead(7); //Detector End Station (Optical) Subject to change
  //sensorValue2 = analogRead(A5); //Detector Inline (Optical)
  //sensorValue3 = analogRead(A3); //Reactor Inline (Optical)
  sensorValue4 = digitalRead(4); //Reactor (Piezo)

  StepMotor* Motor = new StepMotor;
  int time1 = millis();
  Motor->sendSample();
  /*
    while(sensorValue4 == 1){
    delayMicroseconds(10);
    sensorValue3 = digitalRead(4);
    }
    //slows the sample after a delay
    int time2=millis();
    Serial.println(time2-time1);
  //
  delay(350);
  Motor->slowSample();

  delay(1000);
  Serial.println("Sample at Reactor");
  delay(2000);
  time1 = millis();
  
  Motor->microstep(10);
  
  Motor->returnSample();
  /*
    while(sensorValue1 ==1){
    delayMicroseconds(10);
    sensorValue2 = digitalRead(7);
    }
    time2=millis();
    Serial.println(time2-time1);
  
  delay(5000);
  Motor->slowSample();

  delay(2000);
  Serial.println("Sample at Detector");
  delay(1000);
  
  Motor->microstep(10);
  
  Motor->turnOff();
  delay(2000);
  //Motor->randomPos();
  //Motor->homeValve();
  Serial.println("End of cycle");
  */
    int i = 1;
  boolean error = 0;
   // int timer_start = 0;
   // int timer_end = 0;
   // int elapsed_timer = 0;
    int err;
    StepMotor* Motor = new StepMotor;
    MP.RecvTimeout(MP_RECV_BLOCKING);
    
    err = MP.Send(0, countLength, 5);
      if (err < 0) {
        Serial.println("code is fucked");

      }
      err = MP.Send(radLength, countLength, 5);
      if (err < 0) {
        Serial.println("code is fucked");

      }
  Motor->returnSample();

      MP.Recv(&rcvid5, &rcv5, 5);
      if (rcvid5 != 3) {
        error = 1;
        Serial.println("Send/Recv Error before detector slow");
      }
      //timer_end = millis();
      //elapsed_timer = timer_end - timer_start;
      //Serial.print(elapsed_timer);
      //Serial.print(" ");
      //timer_start = millis();
      Motor->slowSample();

      MP.Recv(&rcvid5, &rcv5, 5);
      if (rcvid5 != 4) {
        error = 1;
        Serial.println("Send/Recv Error after detector slow");
      }
      //timer_end = millis() - countLength * 1000;
      //elapsed_timer = timer_end - timer_start;
      //Serial.println(elapsed_timer);
  //    
      Motor->microstep(10);
  //    
      i ++;
    
    Motor->turnOff();
    Serial.println("End of return");
}

//---------------------------------------------------------------------------------- -
void beginCycles() {
  int i = 1;
  boolean error = 0;
  if (radLength <= 0 & countLength <= 0) {
    Serial.println("Please enter valid times");
  }
  /*
    else {
    StepMotor* Motor = new StepMotor;

    //setting sensor values to the Virt2Phys address
    sensorValue1 = *rcv1; //Detector End Station (Optical) Subject to change
    sensorValue2 = *rcv2; //Detector Inline (Optical)
    sensorValue3 = *rcv3; //Reactor Inline (Optical)
    sensorValue4 = *rcv4; //Reactor (Piezo)

    while (i <= cycleCount) {

      int j = 0;
      int dly = 1;
      int timeout = 1000000;
      Motor->sendSample();
      //Waiting for sensor 1
      while (sensorValue1 != 0) {

        j = j + 1;
        if (j > timeout) {
          Serial.println("Capsule Timeout");
          error = 1;
          break;
        }
        if (sensorValue2 == 0 || sensorValue3 == 0 || sensorValue4 == 0) {
          Serial.println("Capsule Broken");
          error = 1;
          break;
        }
      }
      if (error == 1) {
        break;
      }
      Serial.println("Sen 1");

      //waiting for sensor 2
      j = 0;
      while (sensorValue2 != 0) {
        j = j + 1;
        if (j > timeout) {
          Serial.println("Capsule Timeout");
          error = 1;
          break;
        }
        if (sensorValue1 == 0 || sensorValue3 == 0 || sensorValue4 == 0) {
          Serial.println("Capsule Broken");
          error = 1;
          break;
        }
      }
      if (error == 1) {
        break;
      }

      //Waiting for sensor 3
      j = 0;
      while (sensorValue3 != 0) {
        j = j + 1;
        if (j > timeout) {
          Serial.println("Capsule Timeout");
          error = 1;
          break;
        }
        if (sensorValue1 == 0 || sensorValue2 == 0 || sensorValue4 == 0) {
          Serial.println("Capsule Broken");
          error = 1;
          break;
        }
      }
      if (error == 1) {
        break;
      }

      Motor->slowSample(); //sends a value to the main core to tell it to slow the sample

      //Waiting for sensor 4
      j = 0;
      while (sensorValue4 != 0) {
        j = j + 1;
        if (j > timeout) {
          Serial.println("Capsule Timeout");
          error = 1;
          break;
        }
        if (sensorValue1 == 0 || sensorValue2 == 0 || sensorValue3 == 0) {
          Serial.println("Capsule Broken");
          error = 1;
          break;
        }
      }
      if (error == 1) {
        break;
      }
      Serial.println("Sample at Reactor");
      delay(radLength);

      Motor->returnSample(); //sends a value to the main core to tell it to send the sample again

      //Waiting for sensor 4
      j = 0;
      while (sensorValue4 != 0) {
        j = j + 1;
        if (j > timeout) {
          Serial.println("Capsule Timeout");
          error = 1;
          break;
        }
        if (sensorValue2 == 0 || sensorValue3 == 0 || sensorValue1 == 0) {
          Serial.println("Capsule Broken");
          error = 1;
          break;
        }
      }
      if (error == 1) {
        break;
      }

      //Waiting for sensor 3
      j = 0;
      while (sensorValue3 != 0) {
        j = j + 1;
        if (j > timeout) {
          Serial.println("Capsule Timeout");
          error = 1;
          break;
        }
        if (sensorValue1 == 0 || sensorValue2 == 0 || sensorValue4 == 0) {
          Serial.println("Capsule Broken");
          error = 1;
          break;
        }
      }
      if (error == 1) {
        break;
      }

      //Waiting for sensor 2
      j = 0;
      while (sensorValue2 != 0) {
        j = j + 1;
        if (j > timeout) {
          Serial.println("Capsule Timeout");
          error = 1;
          break;
        }
        if (sensorValue1 == 0 || sensorValue3 == 0 || sensorValue4 == 0) {
          Serial.println("Capsule Broken");
          error = 1;
          break;
        }
      }
      if (error == 1) {
        break;
      }

      Motor->slowSample(); // slow the sample

      //Waiting for sensor 1
      j = 0;
      while (sensorValue1 != 0) {
        j = j + 1;
        if (j > timeout) {
          Serial.println("Capsule Timeout");
          error = 1;
          break;
        }
        if (sensorValue4 == 0 || sensorValue2 == 0 || sensorValue3 == 0) {
          Serial.println("Capsule Broken");
          error = 1;
          break;
        }
      }
      if (error == 1) {
        break;
      }
      Serial.println("Sample at Detector");
      delay(countLength);
    }
    Motor->turnOff();
    Serial.println("End of run");
    if (error == 1) {
      Serial.print("Ended on cycle ");
      Serial.print(i);
      Serial.print(" of ");
      Serial.print(cycleCount);
      Serial.print(". \n");
    }
    else {
      Serial.println("Cycles completed sucessfully");
    }



    }
  */
  //If subcore 5 is implemented

  else {
    int timer_start = 0;
    int timer_end = 0;
    int elapsed_timer = 0;
    int err;
    StepMotor* Motor = new StepMotor;
    MP.RecvTimeout(MP_RECV_BLOCKING);
    while (i <= cycleCount) {
    err = MP.Send(1, countLength, 5);
      if (err < 0) {
        Serial.println("code is fucked");
        break;
      }
      err = MP.Send(radLength, countLength, 5);
      if (err < 0) {
        Serial.println("code is fucked");
        break;
      }
      //delay(200);
    //  timer_start = millis();
      Motor->sendSample();

      MP.Recv(&rcvid5, &rcv5, 5);
      if (rcvid5 != 1) {
        error = 1;
        Serial.println("Send/Recv Error before reactor slow");
        break;
      }
      //timer_end = millis();
      //elapsed_timer = timer_end - timer_start;
      //Serial.print(elapsed_timer);
      //Serial.print(" ");
      //timer_start = millis();
      delay(100);
      Motor->slowSample();


      MP.Recv(&rcvid5, &rcv5, 5);
      if (rcvid5 != 2) {
        error = 1;
        Serial.println("Send/Recv Error after reactor slow");
        break;
      }
      //timer_end = millis() - radLength * 1000;
      //elapsed_timer = timer_end - timer_start;
      //Serial.print(elapsed_timer);
      //Serial.print(" ");
 //     
      Motor->microstep(10);
 //     
      //timer_start = millis();
      Motor->returnSample();

      MP.Recv(&rcvid5, &rcv5, 5);
      if (rcvid5 != 3) {
        error = 1;
        Serial.println("Send/Recv Error before detector slow");
        break;
      }
      //timer_end = millis();
      //elapsed_timer = timer_end - timer_start;
      //Serial.print(elapsed_timer);
      //Serial.print(" ");
      //timer_start = timer_end;
      Motor->slowSample();

      MP.Recv(&rcvid5, &rcv5, 5);
      if (rcvid5 != 4) {
        error = 1;
        Serial.println("Send/Recv Error after detector slow");
        break;
      }
      //timer_end = millis() - countLength * 1000;
      //elapsed_timer = timer_end - timer_start;
      //Serial.println(elapsed_timer);
  //    
      Motor->microstep(10);
  //    
      i ++;
    }
    Motor->turnOff();
    Serial.println("End of run");
    if (error == 1) {
      Serial.print("Ended on cycle ");
      Serial.print(i);
      Serial.print(" of ");
      Serial.print(cycleCount);
      Serial.print(". \n");
    }
    else {
      Serial.println("Cycles completed sucessfully");
    }



  }

}
/*
*/
