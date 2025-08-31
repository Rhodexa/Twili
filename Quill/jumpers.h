#ifndef JUMPERS_H
#define JUMPERS_H

/*
  Jumpers can be used between these pins to set up an address tp listen to.
*/

const int JUMPER_BIT_0 = 35;
const int JUMPER_BIT_1 = 34;
const int JUMPER_BIT_2 = 39;
const int JUMPER_BIT_3 = 36;

void jumpers_begin(){
  pinMode(JUMPER_BIT_0, INPUT);
  pinMode(JUMPER_BIT_1, INPUT);
  pinMode(JUMPER_BIT_2, INPUT);
  pinMode(JUMPER_BIT_3, INPUT);
}

int jumpers_getValue(){
  int result = 0;
  if(digitalRead(JUMPER_BIT_0)) result += 1;
  if(digitalRead(JUMPER_BIT_1)) result += 2;
  if(digitalRead(JUMPER_BIT_2)) result += 4;
  if(digitalRead(JUMPER_BIT_3)) result += 8;
  return result;
}

#endif