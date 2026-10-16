#include <Arduino.h>
#ifndef INCLUDES_H
    #include <includes.h>
#endif


/* ======= Setup DWIN Display on Startup ======== */
void setupDisplay() {
    String TextStartPage;
    hmi.initSerial(Serial2, DGUS_BAUD);
    hmi.hmiCallBack(onHMIEvent);  //set callback
    hmi.echoEnabled(false);  // dont want to see all the display transactions
    hmi.ackDisabled(true);   //for our no ack kernel
#ifdef SCREENROTATE
    /* ======= Screen rotate should be set in .cfg config file not here ======== */
    const byte rotate0 = 0; const byte rotate90 = 1;
    const byte rotate180 = 2; const byte rotate270 = 3;
    setScreenRotate(rotate90);  // some display set to portrait when new!
#endif
    hmi.setPage(startPage);
    // Clear the icons
    which_swr = true; //select swr1 new mod
    hmi.setVPByte(swr_error, GREEN_ICON); hmi.setVPByte(swr2_error, GREEN_ICON);
    hmi.setVPByte(current_error, GREEN_ICON); hmi.setVPByte(volt_error, GREEN_ICON);
    hmi.setVPByte(temp_error, GREEN_ICON); hmi.setVPByte(overdrive_error, GREEN_ICON);
    hmi.setVPByte(external_trip_error, GREEN_ICON);
    hmi.setFloatValue(current_display, 0);
    hmi.setFloatValue(swr_digits, 1.00);  hmi.setVPWord(swr_graph, 100);   // 100 - 200
    hmi.setVPWord(band_manual_display, (BAND_MANUAL + band_auto));
    hmi.setVPWord(antenna_switch_display, (ANTENNA_ONE + which_antenna));
    which_antenna ? mcp23017.digitalWrite(ANTSWITCH, true) : mcp23017.digitalWrite(ANTSWITCH, false);
    hmi.setVPWord(swr_meter_switch_display, 26);
    hmi.setVPByte(case_image,UPC_ICON);

    safeStringFromEEPROM(eeprom_text1_address,30, displayText);
     hmi.setText(main_text1,displayText.c_str());
    safeStringFromEEPROM(eeprom_text2_address,30, displayText);
    hmi.setText(main_text2, displayText.c_str());
    hmi.setTextColor(main_text1_sp, 3, intSettingsArray[TEXT1COLOR]);
    hmi.setTextColor(main_text2_sp, 3, intSettingsArray[TEXT2COLOR]);
    delay(500);
    hmi.setTPBeep(false); // turn off dwin touch beep use ours
    usebeep ? hmi.beepHMI(BEEP_YES) : hmi.playSound(STARTING3);
    hmi.writeLongValue(LCDBackLightAddress, LCDBackLightLow);
    // 0x08 is the offset for the max value location
    hmi.setVPWord((swr_graph_sp + 0x08), swrRangeMax);
    hmi.setVPWord((power_graph_sp + 0x08), powerRangeMax);
#ifdef DISPLAY160M
    select_band(band160Mtr_Selected);     // make sure we get the right icon set
#else
    select_band(band4Mtr_Selected);
#endif
    delay(500);     // debug
    select_band(band_select); //from eeprom
    if (glo_drive_power > 100) glo_drive_power = 50;  // set drive power display page2 if out of range
}

/* ======= MCP Initial Settings ======== */
void mcp23017_setup() {
    mcp23017.init();
    //Port A as output
    //direction (IODIRA) - set direction of all lpf pins as outputs
    mcp23017.portMode(MCP23017Port::A, 0);
    // Configure PORTB (switches)
    // unused pins should be set as outputs
    mcp23017.portMode(MCP23017Port::B,
                      //0b00001100,   // direction (IODIRB) - set direction of pins
                      //0b00001100,   // pull-up (GPPUB) - enable internal pull-up resistors on inputs
                      //0b00001100);  // polarity (IPOLB) - invert logic polarity for inputs
                      // All set for outputs at the moment we dont want floating inputs
                      0b00000000,   // direction (IODIRB) - set direction of pins
                      0b00000000,   // pull-up (GPPUB) - enable internal pull-up resistors on inputs
                      0b00000000);  // polarity (IPOLB) - invert logic polarity for inputs
    // Reset MCP23017 ports
    mcp23017.writeRegister(MCP23017Register::GPIO_A, 0x00);
    mcp23017.writeRegister(MCP23017Register::GPIO_B, 0x00);
    mcp23017.digitalWrite(SWR_TRIP, false);
}


/* ======= Set Pin Mode on Startup ======== */
void setupPins() {
    pinMode(FANPWM, OUTPUT);
    pinMode(TEMPSENSOR,INPUT);
    pinMode(OPTOUT1, OUTPUT);      //13 blinkLED or a600_bias
    pinMode(VIN50V, INPUT);
    pinMode(SWR1, INPUT);
    pinMode(SWR1_INT, INPUT);
    pinMode(REF1, INPUT);
    pinMode(SWR2, INPUT);
    pinMode(SWR2_INT, INPUT);
    pinMode(REF2, INPUT);
    pinMode(ID_IN, INPUT);
    pinMode(RFSENSE, INPUT);
    pinMode(PTT, INPUT);
    pinMode(LED_I, INPUT); // dxworld_errors
    pinMode(LED_PO, INPUT);
    pinMode(LED_SWR, INPUT);
    pinMode(AUX1_IN_OUT, OUTPUT);
    pinMode(AUX2_IN_OUT, OUTPUT);
    pinMode(SPARE1,INPUT);
    pinMode(TRIP50v,OUTPUT);
    //  pinMode(BCD_0, INPUT);  //old
    //  pinMode(BCD_1, INPUT);
    //  pinMode(BCD_2, INPUT);
    //  pinMode(BCD_3, INPUT);
    pinMode(INTB, INPUT_PULLUP);

}
/* ======= Some Ticker Resets ======== */
void tempIDTickerReset() {
    temp_id_reset = true;
}
void peakHoldReset() {
    peak_hold_reset = true;
}
void powerSWRTickerReset() {
    power_swr_reset = true;
}


void setup_dallas_sensors() {
    new(&ow) OneWireNg_CurrentPlatform(OW_PIN, false);
#if SENSOR_COUNT > 1
    sensor_array[0].sensor_our_name = sensor0;
    sensor_array[1].sensor_our_name = sensor1;
    sensor_array[2].sensor_our_name = sensor2;
    sensor_array[3].sensor_our_name = sensor3;
#endif
    // read with long
    readTemp(true);
}


//rotate the screen  eg. setScreenRotate(rotate270); // flip the screen
void setScreenRotate(byte angle) {
    hmi.setVPWord(0x0081, ((hmi.readVP(0x0081) & 0xFFFC) + angle));
    hmi.setVPWord(0x0080, 0x5A00);  // set it.
}


void configureInterrupts() {
    mcp23017.interruptMode(MCP23017InterruptMode::Separated);
    mcp23017.interrupt(MCP23017Port::B, CHANGE);  // Port B change int.
    mcp23017.clearInterrupts();  // reset ints
    attachInterrupt(digitalPinToInterrupt(INTB), mcp23017ChangeOnPortB, FALLING);  //active LOW

}

void mcp23017ChangeOnPortB() {
// todo not needed on this
}

void swr1IntRising(){
 Serial.println("SWR1 Int Count "); Serial.println(swr1IntCount);    
if(digitalRead(PTT) == LOW){    // PTT is active
 swr1IntCount ++; 
}
if (swr1IntCount > interrupt_Count_trigger){
   swr1IntActiveHigh = true;
   #ifdef EXT_CURRENT_TRIP
        digitalWrite(TRIP50v,HIGH);
   #endif
}
}

void swr2IntRising(){    
Serial.println("SWR2 Int Count "); Serial.println(swr2IntCount);    
if(digitalRead(PTT) == LOW){
    swr2IntCount ++;
}  

if (swr2IntCount > interrupt_Count_trigger){
    swr2IntActiveHigh = true;
    #ifdef EXT_CURRENT_TRIP
        digitalWrite(TRIP50v,HIGH);
    #endif
}

}

void clearSWRInterruptFlags(){   
swr1IntActiveHigh = false;
swr2IntActiveHigh = false;
}    




