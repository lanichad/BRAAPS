#ifndef SUBCORE
#error "Core selection is wrong!!"
#endif

#include <MP.h>

int sensorValue1 = 0;  // variable to store the value coming from the sensor
int sensorValue2 = 0;
int sensorValue3 = 0;
int sensorValue4 = 0;

boolean isStart = 0;
uint32_t *rcv1;
uint32_t *rcv2;
uint32_t *rcv3;
uint32_t *rcv4;
uint32_t *main;

int8_t   rcvid1;
int8_t   rcvid2;
int8_t   rcvid3;
int8_t   rcvid4;
int8_t   mainId;

int radLength;
int countLength;

int timer_start = 0;
int timer_end = 0;
int elapsed_timer = 0;

void setup() {
  MP.begin();
  delay(1000);
  Serial.begin(115200);

  //Receiving Virt2Phys addresses from all other subcores
  //MP.Recv(&rcvid4, &rcv4, 4);
  //MP.Recv(&rcvid3, &rcv3, 3);
  //MP.Recv(&rcvid2, &rcv2, 2);
  //MP.Recv(&rcvid1, &rcv1, 1);

}

void loop() {
  /*
    sensorValue1 = *rcv1; //Detector End Station (Optical) Subject to change
    sensorValue2 = *rcv2; //Detector Inline (Optical)
    sensorValue3 = *rcv3; //Reactor Inline (Optical)
    sensorValue4 = *rcv4; //Reactor (Piezo)
    Serial.print(sensorValue1);
    Serial.print("\t");
    Serial.print(sensorValue2);
    Serial.print("\t");
    Serial.print(sensorValue3);
    Serial.print("\t");
    Serial.print(sensorValue4);
    Serial.print("\t");
    //Serial.print(sensorValue5);
    Serial.print("\n");
  */
  //setting sensor values to the Virt2Phys address
  //sensorValue1 = *rcv1; //Detector End Station (Optical) Subject to change
  //sensorValue2 = *rcv2; //Detector Inline (Optical)
  //sensorValue3 = *rcv3; //Reactor Inline (Optical)
  //sensorValue4 = *rcv4; //Reactor (Piezo)
  
  boolean error = 0;
  uint64_t j = 0;
  int dly = 1;
  uint64_t timeout = 1560000;
  //uint64_t timeout = 7000;
  //Signal recieved from main core to start watching sensors
  MP.Recv(&mainId, &main);
  if(mainId==1){
  MP.Recv(&mainId, &main);

  //Values received from main core are also the irradiation and count times in [sec]
  radLength = mainId * 1000;
  countLength = (int)main * 1000;
  timer_start = millis();
/*
  //Waiting for sensor 1
  while (sensorValue1 == HIGH) {
    j = j + 1;
    delayMicroseconds(1);
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
    MP.Send(8, 5);
    return;
  }
  Serial.println("Sen 1");
*/

  //waiting for sensor 2
  j = 0;
  sensorValue1=digitalRead(7);
  sensorValue2=digitalRead(6);
  sensorValue4=digitalRead(4);
  while (sensorValue2 == HIGH) {
    j++;
    //sensorValue1=digitalRead(7);
    sensorValue2=digitalRead(6);
    sensorValue4=digitalRead(4);
    //delayMicroseconds(1);
      //Serial.print(sensorValue1);
  //Serial.print("\t");
  //Serial.println(sensorValue2);
  //Serial.print("\t");
  //Serial.print(sensorValue3);
  //Serial.print("\t");
  //Serial.print(sensorValue4);
  //Serial.print("\t");
  //Serial.print("\n");
    if (j > timeout) {
      Serial.println("Capsule Timeout");
      error = 1;
      break;
    }
    
   /* if (sensorValue4 == 0) {
      Serial.println("Capsule Broken");
      error = 1;
      break;
    }
    */
    
  }
  if (error == 1) {
    MP.Send(8, 5);
    return;
  }
  
/*
  //Waiting for sensor 3
  j = 0;
  while (sensorValue3 == HIGH) {
    j = j + 1;
    delayMicroseconds(1);
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
    MP.Send(8, 5);
    return;
  }  
*/
timer_end = millis();
elapsed_timer = timer_end - timer_start;
Serial.print(elapsed_timer);
Serial.print(" ");
 //timer_start = millis();
  MP.Send(1, 5); //sends a value to the main core to tell it to slow the sample
  delay(100);
  
  //Waiting for sensor 4
  j = 0;
  
  sensorValue4=digitalRead(4);
  /*
  while (sensorValue4 == HIGH) {
    sensorValue4=digitalRead(4);
    j = j + 1;
    //delayMicroseconds(1);
    if (j > timeout) {
      Serial.println("Capsule Timeout");
      error = 1;
      break;
    }
    //
    //if (sensorValue1 == 0 || sensorValue2 == 0) {
    //  Serial.println("Capsule Broken");
    //  error = 1;
    //  break;
    //}
    //
  }
  */
  
  //delay(700);
  if (error == 1) {
    MP.Send(8, 5);
    return;
  }
  timer_end=millis();
  elapsed_timer=timer_end-timer_start;
  Serial.print(elapsed_timer);
  Serial.print(" ");
  //Serial.println("Sample at Reactor");
  delay(radLength);
  //timer_start = millis();
  MP.Send(2, 10); //sends a value to the main core to tell it to send the sample again

/*
  //Waiting for sensor 4
  j=0;
  while (sensorValue4 == HIGH) {
    j = j + 1;
    delayMicroseconds(1);
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
    MP.Send(8, 5);
    return;
  }

  //Waiting for sensor 3
  j = 0;
  while (sensorValue3 == HIGH) {
    j = j + 1;
    delayMicroseconds(1);
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
    MP.Send(8, 5);
    return;
  }
*/

  //Waiting for sensor 2
  j = 0;
  sensorValue2=digitalRead(6);
  while (sensorValue2 == HIGH) {
    sensorValue2=digitalRead(6);
    j = j + 1;
    //delayMicroseconds(1);
    if (j > timeout) {
      Serial.println("Capsule Timeout");
      error = 1;
      break;
    }
    /*
    if (sensorValue1 == 0 || sensorValue4 == 0) {
      Serial.println("Capsule Broken");
      error = 1;
      break;
    }
    */
  }
  if (error == 1) {
    MP.Send(8, 5);
    return;
  }
  timer_end = millis();
  elapsed_timer = timer_end - timer_start;
  Serial.print(elapsed_timer);
  Serial.print(" ");
  
  //timer_start = millis();
  MP.Send(3, 15); //sends a value to the main core to tell it to slow the sample
 
  delay(50);
  
  //Waiting for sensor 1
  j = 0;
  sensorValue1=digitalRead(7);
  while (sensorValue1 == HIGH) {
    j = j + 1;
    sensorValue1=digitalRead(7);
    //delayMicroseconds(1);
    if (j > timeout) {
      Serial.println("Capsule Timeout");
      error = 1;
      break;
    }
    /*
    if (sensorValue4 == 0 || sensorValue2 == 0) {
      Serial.println("Capsule Broken");
      error = 1;
      break;
    }
    */
  }
  if (error == 1) {
    MP.Send(8, 5);
    return;
  }
  timer_end = millis();
  elapsed_timer = timer_end - timer_start;
  Serial.println(elapsed_timer);
  //Serial.println("Sample at Detector");
  delay(countLength);

  MP.Send(4, 20); //sends to main core to acknowledge cycle complete
  }
  else{
    MP.Recv(&mainId, &main);

  //Values received from main core are also the irradiation and count times in [sec]
  radLength = mainId * 1000;
  countLength = (int)main * 1000;
    //Waiting for sensor 2
  j = 0;
  sensorValue2=digitalRead(6);
  while (sensorValue2 == HIGH) {
    sensorValue2=digitalRead(6);
    j = j + 1;
    //delayMicroseconds(1);
    if (j > timeout) {
      Serial.println("Capsule Timeout");
      error = 1;
      break;
    }
    /*
    if (sensorValue1 == 0 || sensorValue4 == 0) {
      Serial.println("Capsule Broken");
      error = 1;
      break;
    }
    */
  }
  if (error == 1) {
    MP.Send(8, 5);
    return;
  }

  MP.Send(3, 15); //sends a value to the main core to tell it to slow the sample
  delay(100);
  
  //Waiting for sensor 1
  j = 0;
  sensorValue1=digitalRead(7);
  while (sensorValue1 == HIGH) {
    j = j + 1;
    sensorValue1=digitalRead(7);
    //delayMicroseconds(1);
    if (j > timeout) {
      Serial.println("Capsule Timeout");
      error = 1;
      break;
    }
    /*
    if (sensorValue4 == 0 || sensorValue2 == 0) {
      Serial.println("Capsule Broken");
      error = 1;
      break;
    }
    */
  }
  if (error == 1) {
    MP.Send(8, 5);
    return;
  }
  //Serial.println("Sample at Detector");
  delay(countLength);

  MP.Send(4, 20); //sends to main core to acknowledge cycle complete
  }
  
}
