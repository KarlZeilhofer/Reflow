/******************************************************************************
*
* @file       SimpleTemperatureController.ino
* @author     Karl Zeilhofer, http://www.zeilhofer.co.at
* @brief      Regulates a heating-element to a user-defined temperature. 
*             The fan-output is not used. 
*
*             Features:
*               simple P-Regulator, Regulator-Gain is user settable
*               temperature ramp limitation (maximum °C/s)
*               Buzzer sounds, when temperature reached
*               keeps the temperature, when finished (user has to manually turn off the controller)
*               user settings are stored in EEPROM (written every time, the controller is disabled by the user)
*               additional informations available, instead of the title-string (DebugMode, press OK on power-up):
*                 [measured T-ramp in K/s; internal target temperature in °C; PWM duty-cycle in %]
*               
*
*             Products: http://www.drotek.fr/shop/en/5-controleur-refusion
*             Discussions: http://www.drotek.fr/forum
*
* @license    The GNU Public License (GPL) Version 3
*
* @Changelog  this software is bases on the "SoftReflow", Oct. 2014
*             2014-10-25: started "SimpleTemperatureController"
*
*****************************************************************************/


/***************** LCD Layout **************************
* SimpleTempController
* T_Sensor: 23.1°C
* > T_set: < 100°C >
*   Reg_Gain: < 5.3 %/K >
*   Ramp_Lim: < 2.5 K/s >
*   Enabled:   < ON >
*
********************************************************/

/****************************************/
/*        Include Header Files          */
/****************************************/
#include <glcd.h>
#include <glcd_Buildinfo.h>
#include <glcd_Config.h>
#include "fonts/allFonts.h"
#include "bitmaps/allBitmaps.h" 
#include <EEPROM.h>
#include "defines.h"


/****************************************/
/*              Variables               */
/****************************************/
//======GLCD=======
Image_t icon;
      

//=====VARIANTS======
char str[20]; // global helper string for printing a line with sprintf()

float T_Sensor = 0;
float T_Set=0;
float Reg_Gain=0;
float Ramp_Lim=0;
float T_Ramp=0; // save the start temperature for limiting the ramp, and increment this value in the regulate() every second
bool Enabled=false;
bool Beeped=true; // flag, if buzzer has beeped since last Enabled. It beeps when the T_Set-3°C is reached. 
int currentOpt = 1; // selected option on main screen

bool DebugMode = false;

/****************************************/
/*                SETUP                 */
/****************************************/

void setup(){
  
  /* Open a serial communication */ 
  Serial.begin(9600);
  
  /* Initialization of all devices */  
  MAX6675_init();
  Button_init();
  Relay_init();
  Fan_init();
  Buzzer_init();  
  GLCD.Init();
  
  /* Configuration of LCD */ 
  icon = ArduinoIcon64x64;
  GLCD.ClearScreen(); 
  GLCD.SelectFont(System5x7, BLACK);
   
  // Check if it's the first boot  
  CheckFirstBoot();

  //Transferts EEPROM values to RAM
  EEPROMtoRAM();
    
  GLCD.ClearScreen();
  intro(5000); // Introduction
}


/****************************************/
/*             Main Loop                */
/****************************************/
void loop(){
    // ============== MAIN_MENU==============
option1: // T_Set
    currentOpt = 1;
    GLCD.ClearScreen();
    menuP(currentOpt);
    pause(DT_SLOW);
option1_fast:
    menuP(currentOpt);
    pause(DT_FAST);
    
    do{
      regulate();
    }while (digitalRead(OK) != 0 && digitalRead(DOWN) != 0 && digitalRead(LEFT) != 0 && digitalRead(RIGHT) != 0);
    
    if (digitalRead(OK) != 1){
      toggleEnabled();
      goto option1;
    }else if (digitalRead(OK) != 1){
      goto option2;
    }else if (digitalRead(LEFT) != 1){
      if(T_Set > 0){
        T_Set-=2;
      }
      goto option1_fast;
    }else if (digitalRead(RIGHT) != 1){
      if(T_Set < 500){
        T_Set+=2;
      }
      goto option1_fast;
    }

option2: // Reg_Gain
    currentOpt = 2;
    GLCD.ClearScreen();
    menuP(currentOpt);
    pause(DT_SLOW);
option2_fast:
    menuP(currentOpt);
    pause(DT_FAST);
    do{
      regulate();
    }while(digitalRead(OK) != 0 && digitalRead(UP) != 0 && digitalRead(DOWN) != 0 && digitalRead(LEFT) != 0 && digitalRead(RIGHT) != 0);
    
    if (digitalRead(UP) != 1){
      goto option1;
    }else if(digitalRead(DOWN) != 1){
      goto option3;
    }else if(digitalRead(OK) != 1){
      toggleEnabled();
      goto option2;
    }else if (digitalRead(LEFT) != 1){
      if(Reg_Gain > 0){
        Reg_Gain--;
      }
      goto option2_fast;
    }else if (digitalRead(RIGHT) != 1){
      if(Reg_Gain < 30){
        Reg_Gain++;
      }
      goto option2_fast;
    }
    
option3: // Ramp_Lim
    currentOpt = 3;
    GLCD.ClearScreen();
    menuP(currentOpt);
    pause(DT_SLOW);
option3_fast:
    menuP(currentOpt);
    pause(DT_FAST);
    
    do{
      regulate();
    }while(digitalRead(OK) != 0 && digitalRead(UP) != 0 && digitalRead(DOWN) != 0 && digitalRead(LEFT) != 0 && digitalRead(RIGHT) != 0);
    
    if (digitalRead(UP) != 1){
      goto option2;
    }else if(digitalRead(DOWN) != 1){
      goto option4;
    }else if(digitalRead(OK) != 1){
      toggleEnabled();
      goto option3;
    }else if (digitalRead(LEFT) != 1){
      if(Ramp_Lim > 0){
        Ramp_Lim -= 0.1;
      }
      goto option3_fast;
    }else if (digitalRead(RIGHT) != 1){
      if(Ramp_Lim < 20){
        Ramp_Lim += 0.1;
      }
      goto option3_fast;
    }
    
option4: // Enabled
    currentOpt = 4;
    GLCD.ClearScreen();
    menuP(currentOpt);
    pause(DT_SLOW);


    do{
      regulate();
    }while(digitalRead(OK) != 0 && digitalRead(UP) && digitalRead(LEFT) != 0 && digitalRead(RIGHT) != 0);
    
    if (digitalRead(UP) != 1){
      goto option3;
    }else if(digitalRead(OK) != 1 || digitalRead(LEFT) != 1 || digitalRead(RIGHT) != 1){
      toggleEnabled();
      goto option4;
    } 
}


