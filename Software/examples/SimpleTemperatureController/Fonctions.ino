


void intro(int timer) { // Introduction with version of firmware
  GLCD.DrawCircle( 15, 30, 5); 
  GLCD.DrawCircle( 112, 30, 5);
  GLCD.DrawRoundRect(2,45,GLCD.Width-5,15,5);
  GLCD.DrawBitmap(icon, 32,0);
  GLCD.CursorToXY(30,50);
  GLCD.print("Firmware V");
  GLCD.CursorToXY(92,50);
  GLCD.print((int)VERSION);
  GLCD.CursorToXY(29,32);
  GLCD.print("With Arduino");
  GLCD.CursorToXY(0,0);
  delay(timer);
  GLCD.ClearScreen();
  
  GLCD.print("Press OK for Debug");
  long int t0 = millis();
  while(DebugMode == false && millis() < t0+1500){
    if(digitalRead(OK) != 1){
      DebugMode = true;
    }
  }
  GLCD.ClearScreen();
}

void menu(){ // The main MENU
  
  GLCD.DrawLine(0,11,127,11);
  
  if(!DebugMode){
    GLCD.CursorToXY(5,3);
    GLCD.print("SimpleTempController");
  }
  
  GLCD.CursorToXY(6,15);
  sprintf(str, "T_Sensor: %3d.%d C", (int)(T_Sensor+0.05), (int)(T_Sensor*10)%10);
  GLCD.print(str);
  
  GLCD.CursorToXY(12,25);
  sprintf(str, "T_Set:    <%3d C>", (int)T_Set);
  GLCD.print(str);

  GLCD.CursorToXY(12,35);
  sprintf(str, "Reg_Gain: <%3d%%/K>", (int)Reg_Gain);
  GLCD.print(str);
  
  GLCD.CursorToXY(12,45);
  sprintf(str, "Ramp_Lim: <%2d.%dK/s>", (int)(Ramp_Lim+0.05), (int)(Ramp_Lim*10+0.5)%10);
  GLCD.print(str);
  
  GLCD.CursorToXY(12,55);
  if(Enabled){
    GLCD.print("Enabled:  <ON >");
  }else{
    GLCD.print("Enabled:  <OFF>");
  }
}

// option = 1,2,3
void menuP(int option){ // Select the first option in the MENU

  menu();
  GLCD.CursorToXY(2,15+option*10);
  GLCD.print(">");
}

void toggleEnabled(){
  if(Enabled){
    Enabled = false;
    Relay_off();
    RAMtoEEPROM();
  }else{
    Enabled = true;
    T_Ramp = T_Sensor; // save start temperature for limiting the ramp
    Beeped = false;
  }
}

// pause in ms
// call regulate during waiting
void pause(int timer){
  long int t1 = millis() + timer;
  while((long int)millis() < t1){
    regulate();
  }
}
 
// update the printed value of T_Sensor
// and do the regulator-procedure
void regulate(){
  static float dutyCycle=0; // a value between 0 and 100
  static int lastTic = 0;
  static int first = true; // use this to initialize static variables
      



  // generate the temperature-ramp (it is used as the regulator target-trajectory)
  static long int rampIncTime=0;
  if(first)
    rampIncTime = millis();
  if(Enabled && T_Ramp < T_Set){
    T_Ramp += Ramp_Lim*((millis()-rampIncTime)*0.001); // multiply with dt
    T_Ramp = min(T_Ramp, T_Set); // limit to max. T_Set
  }
  rampIncTime = millis();
  
  
  // simple P-Regulator:
  dutyCycle = (T_Ramp - T_Sensor)*Reg_Gain; // calculate output
  dutyCycle = constrain(dutyCycle, 0, 100);
    
  
  
  // realize software PWM:
  static long int timeOn = 0; // remember last turn on time (in milli-seconds)
  if(!Enabled){
    dutyCycle = 0;
    Relay_off();
  }else{
    if(millis() >= (timeOn + 10*dutyCycle)){
      Relay_off();
    }
    if(millis() >= timeOn + 1000){
      Relay_on();
      timeOn = millis();
    }
  }
  
  
  // update LCD twice in a second
  const int dt_ms = 500;
  if(lastTic != millis()/dt_ms){
    static float last_T_Sensor = 0;
    T_Sensor = MAX6675_read_temp(1);
    if(first){
      last_T_Sensor = T_Sensor;
    }
    
    // show some extra information in the first line (instead of the title)
    if(DebugMode){
      static int visits=0;
      visits++;
      if(visits == 2000/dt_ms){
        float actualRamp = (float)(T_Sensor - last_T_Sensor)/2.0;
        GLCD.CursorToXY(0,2);
        float val = abs(actualRamp);
        if(actualRamp >= 0){
          sprintf(str, "+%2d.%dK/s", (int)(val+0.05), (int)(val*10+0.5)%10);
        }else{
          sprintf(str, "-%2d.%dK/s", (int)(val+0.05), (int)(val*10+0.5)%10);
        }
        GLCD.print(str);
        visits=0;
        last_T_Sensor = T_Sensor;
      }
      
      GLCD.CursorToXY(60,2);
      sprintf(str, "%3dC", (int)T_Ramp);
      GLCD.print(str);

      GLCD.CursorToXY(100,2);
      sprintf(str, "%3d%%", (int)dutyCycle);
      GLCD.print(str);
         
    }
    
    GLCD.CursorToXY(6+10*6,15);
    sprintf(str, "%3d.%d", (int)(T_Sensor+0.05), (int)(T_Sensor*10+0.5)%10);
    GLCD.print(str);
    lastTic = millis()/dt_ms;
  }
  
  
  
  // Handle the buzzer:
  static bool beeping = false;
  static long int beepOnTime = 0;

  // turn buzzer on:
  if(!Beeped && !beeping && T_Sensor >= T_Set-3){
    Buzzer_on();
    beeping = true;
    beepOnTime = millis();
  }
  
  // turn buzzer off:
  if(beeping && millis() >= beepOnTime+2000){
    Buzzer_off();
    beeping = false;
    Beeped = true;
  }

  
  first = false;
}



   //include all values pre-saved on EEPROM into RAM  
void EEPROMtoRAM(){
   T_Set = ((int)EEPROM.read(TEMPERATURE_SET_MSB_ADDR)*256) + EEPROM.read(TEMPERATURE_SET_LSB_ADDR);
   Reg_Gain = ((int)EEPROM.read(REG_GAIN_MSB_ADDR)*256) + EEPROM.read(REG_GAIN_LSB_ADDR);
   Ramp_Lim = (float)(((int)EEPROM.read(RAMP_LIM_MSB_ADDR)*256) + EEPROM.read(RAMP_LIM_LSB_ADDR))/10.0;
}
   
   // Save the values into EEPROM
void RAMtoEEPROM(){
  // read values from EEPROM. Save the values only to EEPROM, if they have changed. 
   int T_Set_EE = ((int)EEPROM.read(TEMPERATURE_SET_MSB_ADDR)*256) + EEPROM.read(TEMPERATURE_SET_LSB_ADDR);
   int Reg_Gain_EE = ((int)EEPROM.read(REG_GAIN_MSB_ADDR)*256) + EEPROM.read(REG_GAIN_LSB_ADDR);
   float Ramp_Lim_10_EE = (((int)EEPROM.read(RAMP_LIM_MSB_ADDR)*256) + EEPROM.read(RAMP_LIM_LSB_ADDR));

  // T_Set is 0...500
  if(T_Set_EE != (int)T_Set){
    if(T_Set > 255){
      EEPROM.write(TEMPERATURE_SET_LSB_ADDR,255);
      EEPROM.write(TEMPERATURE_SET_MSB_ADDR,(int)T_Set>>8);
    }
    else{
      EEPROM.write(TEMPERATURE_SET_LSB_ADDR,T_Set);
      EEPROM.write(TEMPERATURE_SET_MSB_ADDR,0);
    }
  }
    
  // Reg_Gain is 0...100
  if(Reg_Gain_EE != (int)Reg_Gain)
  {
    EEPROM.write(REG_GAIN_LSB_ADDR,Reg_Gain);
    EEPROM.write(REG_GAIN_MSB_ADDR,0);
  }
  
  // Ramp_Lim is 0...200
  int Ramp_Lim_10 = Ramp_Lim*10; // convert to int
  if(Ramp_Lim_10_EE != Ramp_Lim_10)
  {
    EEPROM.write(RAMP_LIM_LSB_ADDR,Ramp_Lim_10);
    EEPROM.write(RAMP_LIM_MSB_ADDR,0);
  }
}


void CheckFirstBoot(){  
    if (EEPROM.read(BOOT_ADDR)!= BOOT_VALUE){      
      GLCD.ClearScreen();
      GLCD.CursorToXY(1,10);
      GLCD.print("It's the FirstBoot!!!");
      GLCD.CursorToXY(1,30);
      GLCD.print("Writing default values in EEprom...");
      
      T_Set = TEMPERATURE_SET;
      Reg_Gain = REG_GAIN;
      Ramp_Lim = RAMP_LIM;
    
      RAMtoEEPROM();
    
      EEPROM.write(BOOT_ADDR,BOOT_VALUE);
    
      delay(10000);
    }
}
    

