#include <Arduino.h>
#ifndef INCLUDES_H
    #include <includes.h>
#endif
/* ======= Band Select Functions ======== */
void all_band_relays_off() {
    // clear port A
    mcp23017.writePort(MCP23017Port::A, 0x00);
}

void band80Mtr_Active() {
    all_band_relays_off();
    mcp23017.writePort(MCP23017Port::A, band80Mtr_switch);
}
void band60_40Mtr_Active() {
    all_band_relays_off();
    mcp23017.writePort(MCP23017Port::A, band60_40Mtr_switch);
}
void band30_20Mtr_Active() {
    all_band_relays_off();
    mcp23017.writePort(MCP23017Port::A, band30_20Mtr_switch);
}
void band17_15Mtr_Active() {
    all_band_relays_off();
    mcp23017.writePort(MCP23017Port::A, band17_15Mtr_switch);
}
void band12_10Mtr_Active() {
    all_band_relays_off();
    mcp23017.writePort(MCP23017Port::A, band12_10Mtr_switch);
}
void band6Mtr_Active() {
    all_band_relays_off();
    mcp23017.writePort(MCP23017Port::A, band6Mtr_switch);
}
void band160_4Mtr_Active() {
    all_band_relays_off();
#ifndef DISPLAY160M
    // 4Mtr
    mcp23017.writePort(MCP23017Port::A, band4Mtr_switch);
#else
    // 160mtr
    mcp23017.writePort(MCP23017Port::A, band160Mtr_switch);
#endif
}

void select_band(uint8_t lastByteRX) {
    // check before we read array
    // band80Mtr_Selected = 31
    // lastByteRX = band160_4Mtr_Selected ; // end stop test
    if ((lastByteRX >= (band80Mtr_Selected - 1)) && (lastByteRX <= (band6Mtr_Selected + 1))) {
        // used to read/write array later
        calc_array_swr_offset = (lastByteRX - FIRST_ICON);             // 0-6 one spare 7
        calc_array_lpf_offset = (lastByteRX - FIRST_ICON) + EEPROMROW; // 8-14 one spare 15
        calc_array_drive_offset = (lastByteRX - FIRST_ICON) + DRIVEOFFSET; // 32-37 one spare
        #ifdef MYDEBUG
        Serial.print("calc_array_drive_offset "); Serial.println(calc_array_drive_offset);
        Serial.print("calc_array_swr_offset "); Serial.println(calc_array_swr_offset);
        Serial.print(" calc_array_lpf_offset "); Serial.println( calc_array_lpf_offset);
        #endif

        glo_drive_power = powerCalcArray[calc_array_drive_offset];
    }

    switch (lastByteRX) {
    case band80Mtr_Selected:
        band80Mtr_Active();
        hmi.setVPWord(band_display, (band80Mtr_Selected_Icon)); // display icon numbers 38-44
        break;
    case band60_40Mtr_Selected:
        band60_40Mtr_Active();
        hmi.setVPWord(band_display, (band60_40Mtr_Selected_Icon));
        break;
    case band30_20Mtr_Selected:
        band30_20Mtr_Active();
        hmi.setVPWord(band_display, (band30_20Mtr_Selected_Icon));
        break;
    case band17_15Mtr_Selected:
        band17_15Mtr_Active();
        hmi.setVPWord(band_display, (band17_15Mtr_Selected_Icon));
        break;
    case band12_10Mtr_Selected:
        band12_10Mtr_Active();
        hmi.setVPWord(band_display, (band12_10Mtr_Selected_Icon));
        break;
    case band6Mtr_Selected:
        band6Mtr_Active();
        hmi.setVPWord(band_display, (band6Mtr_Selected_Icon));
        break;
#ifndef DISPLAY160M
    case band4Mtr_Selected:
        band160_4Mtr_Active();
        hmi.setVPWord(band_display, (band4Mtr_Selected));
        break;
#else
    case band160Mtr_Selected:
        band160_4Mtr_Active();
        hmi.setVPWord(band_display, (band160Mtr_Selected_Icon));
        break;
#endif
    }
}

#ifdef AUTO_BAND

float readChannel(ADS1015_MUX channel) {
  float voltage = 0.0;
  adc.setCompareChannels(channel);
  adc.startSingleMeasurement();
  while(adc.isBusy()){delay(0);}
  voltage = adc.getResult_V(); // alternative: getResult_mV for Millivolt
  return voltage;
}

#ifdef YAESU
void auto_band() {

    byte bcd0 = 0;
    byte bcd1 = 0;
    byte bcd2 = 0;
    byte bcd3 = 0;
    byte bcd_result;
    static byte last_result;

    /*  // OLD BCD Band Switch
    if (!digitalRead(BCD_0)) bcd0 = 1;
    if (!digitalRead(BCD_1)) bcd1 = 2;
    if (!digitalRead(BCD_2)) bcd2 = 4;
    if (!digitalRead(BCD_3)) bcd3 = 8;
    */


    if (readChannel(ADS1115_COMP_0_GND) > adc_level) bcd0 = 8;
    if (readChannel(ADS1115_COMP_1_GND) > adc_level) bcd1 = 4;
    if (readChannel(ADS1115_COMP_2_GND) > adc_level) bcd2 = 2;
    if (readChannel(ADS1115_COMP_3_GND) > adc_level) bcd3 = 1;

    bcd_result = bcd0 + bcd1 + bcd2 + bcd3;

//    Serial.print("BCD Result ");
//    Serial.println(bcd_result,BIN);

    //bcd_result = 0b1100;  //TEST
    /* 
      ***************************   
      BCD's from my FT-710 Manual
      ***************************
  */

    if ((bcd_result != last_result) || (band_auto_touched)) {
        last_result = bcd_result;
        //Serial.println(bcd_result,BIN);
        switch (bcd_result) {
        case 0b1000:
            // 160M
#ifdef DISPLAY160M  // alternate icons
            select_band(band160Mtr_Selected);
#endif
            break;
        case 0b0100:
            // 80M
            select_band(band80Mtr_Selected);
            break;
        case 0b1100:
            // 60-40
            select_band(band60_40Mtr_Selected);
            break;
        case 0b0010:
            //case 0b0011: ft-710 manual says this which appears wrong
            // 30M
            select_band(band30_20Mtr_Selected);
            break;
        case 0b1010:
            // 20M
            select_band(band30_20Mtr_Selected);
            break;
        case 0b0110:
            // 17M
            select_band(band17_15Mtr_Selected);
            break;
        case 0b1110:
            // 15M
            select_band(band17_15Mtr_Selected);
            break;
        case 0b0001:
            // 12M
            select_band(band12_10Mtr_Selected);
            break;
        case 0b1001:
            // 10M
            select_band(band12_10Mtr_Selected);
            break;
        case 0b0101:
            // 6M
            select_band(band6Mtr_Selected);
            break;
        case 0b1111:
// 4M
#ifndef DISPLAY160M
            select_band(band4Mtr_Selected);
#endif
            break;
            //default: // unknown
            //
        }
    } // end if
    band_auto_touched = false;
}
#else
void auto_band(){  // not tested ICOM
if (!band_auto_touched) return;
 float adc_result = readChannel(ADS1115_COMP_0_GND);
 adc_result = (adc_result * 2); // make up for resistor split
 
 #ifdef DISPLAY160M  // alternate icons
   if (inRange(adc_result,7.0,8.0)){
   select_band(band160Mtr_Selected);
   }
 #endif
   if (inRange(adc_result,5.6,6.5)){
     select_band(band80Mtr_Selected);
   }
   if (inRange(adc_result,4.2,5.1)){
     select_band(band60_40Mtr_Selected);
    }
    if (inRange(adc_result,0,0.7)){
        select_band(band30_20Mtr_Selected);    
    }
    if (inRange(adc_result,2.8,3.5)){
        select_band(band30_20Mtr_Selected);    
    }
    if (inRange(adc_result,3.5,4.2)){
         select_band(band17_15Mtr_Selected);   
    }
    if (inRange(adc_result,2.0,2.8)){
         select_band(band12_10Mtr_Selected);
    }
    if (inRange(adc_result,2.0,2.8)){
         select_band(band12_10Mtr_Selected);
    }
    if (inRange(adc_result,1.0,2.0)){
        // 4m the same as 6m ?
         select_band(band6Mtr_Selected);
    }

band_auto_touched = false;
}   
#endif // end YAESU


#endif // AUTOBAND


bool inRange(float val, float minimum, float maximum){
  return ((minimum <= val) && (val <= maximum));
}