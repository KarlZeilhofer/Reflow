/******************************************************************************
*
* @file       MAX6675.ino
* @author     E.Esquirol, http://www.drotek.fr Copyright (C) 2014.
* @brief      Library for reading temperature from a MAX6675
*
*             Products: http://www.drotek.fr/shop/en/5-controleur-refusion
*             Discussions: http://www.drotek.fr/forum
*
* @license    The GNU Public License (GPL) Version 3
*
* @Changelog  xx-xx-2014 - ...
*
*****************************************************************************/

float temperature = 0.0; 


/****************************************/
/*           MAX6675_init()             */
/****************************************/
void MAX6675_init()
{
  pinMode(SS, OUTPUT);
  pinMode(MISO, INPUT);
  pinMode(SCK, OUTPUT);
  
  digitalWrite(SS, HIGH);
}

/****************************************/
/*        MAX6675_read_temp()           */
/****************************************/
// due to the slwo conversion time (0.2s) multiple aquision and averaging doesn't make sense without any delay!
float MAX6675_read_temp(int samples) // samples-value will be ignored
{
  float value = 0;
  bool error_tc = false;
  float temp = 0;
	
  digitalWrite(SS,LOW); // Enable device

  /* Cycle the clock for dummy bit 15 */
  digitalWrite(SCK,HIGH);
  digitalWrite(SCK,LOW);

  /* Read bits 14-3 from MAX6675 for the Temp 
     Loop for each bit reading the value and 
     storing the final value in 'temp' 
  */
  for (int i=11; i>=0; i--){
    digitalWrite(SCK,HIGH);  // Set Clock to HIGH
    value += digitalRead(MISO) << i;  // Read data and add it to our variable
    digitalWrite(SCK,LOW);  // Set Clock to LOW
  }

  /* Read the TC Input inp to check for TC Errors */
  digitalWrite(SCK,HIGH); // Set Clock to HIGH
  error_tc = digitalRead(MISO); // Read data
  digitalWrite(SCK,LOW);  // Set Clock to LOW

  digitalWrite(SS, HIGH); //Disable Device
   
  /* 
     Keep in mind that the temp that was just read is on the digital scale
     from 0˚C to 1023.75˚C at a resolution of 2^12.  We now need to convert
     to an actual readable temperature. Now multiply by 0.25.  I tried to avoid float math but
     it is tough to do a good conversion to ˚F.  
   */
   
  value = value + OFFSET;  // Apply the offset for Max6675, see defines.h
  
  // °F or °C
  if(UNIT == 0)         temp = ((value*0.25) * (9.0/5.0)) + 32.0;  // Convert value to ˚F (ensure proper floats!)
  else if(UNIT == 1)    temp = (value*0.25);  // Multiply the value by 0.25 to get temp in ˚C
  
  
  /* Output -1 if there is a TC error, otherwise return 'temp' */
  if(error_tc == true) {
    return -1; 
  } else { 
    return temp; 
  }
}

/****************************************/
/*           MAX6675_test()             */
/****************************************/
void MAX6675_test()
{
  GLCD.ClearScreen();
  GLCD.CursorToXY(2,25);
  GLCD.print("Press DOWN button");
  GLCD.CursorToXY(2,35);
  GLCD.print("to test MAX6675");
  while(digitalRead(DOWN) != 0);
  GLCD.ClearScreen();
  
  for (int i=0; i <= 30; i++){
    temperature = MAX6675_read_temp(5);
    GLCD.CursorToXY(2,25);
    GLCD.print("Are you cold? :)");
    GLCD.CursorToXY(2,45);
    GLCD.print(temperature);
    delay(250);
    GLCD.ClearScreen();
  }
}
