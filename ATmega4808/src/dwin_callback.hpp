#include <Arduino.h>
#ifndef INCLUDES_H
    #include <includes.h>
#endif


// Event Occurs when response comes from HMI
void onHMIEvent(String address, int lastByte, String message, String response) {  // start tag onHMIEvent
#ifdef DISPLAYDEBUG
    Serial.println("OnEvent : [ A : " + address + " | D : " + String(lastByte, HEX) + " | M : " + message + " | R : " + response + " ]");
#endif

  char* endPtr;
    uint16_t displayVP = (uint16_t)strtol(address.c_str(), &endPtr, 16);
    if (*endPtr != '\0') {
    Serial.println("Bad VP address: " + address);
    return;
    }

    static uint16_t savePowerValue;  // for cancel buttons
    static uint16_t saveDrive;
    static uint16_t saveSWR;
    static uint16_t saveVolt;
    static uint16_t saveCurrent;
    static String saveText;
    static uint8_t whichText; // which main page text box to change
    static uint16_t colorSave;
    static bool which_case; // false = caps


  switch (displayVP) {

    case band_switch: {
    // if ((!band_auto)){   // Test
        if ((!tx_status) && (!band_auto)) {        //dont change band on tx/band auto
            band_select = lastByte;
            select_band(lastByte);
            EEPROM.update(eeprom_band, band_select);
            usebeep ? hmi.beepHMI(BEEP_YES) : hmi.playSound(YES);
        } else {
            usebeep ? hmi.beepHMI(BEEP_CANCEL) : hmi.playSound(BEEPERROR);
        }
    
        }
    break;
      
    case band_manual_switch: {
      if(!tx_status){
         if (band_auto) {
            band_auto = false;
            hmi.setVPWord(band_manual_display, BAND_MANUAL); //23 etc icon numbers
        } else {
            band_auto = true;
            hmi.setVPWord(band_manual_display, BAND_AUTO);
        }
        EEPROM.update(eeprom_auto, band_auto);
        usebeep ? hmi.beepHMI(BEEP_YES) : hmi.playSound(YES);
      }else{  // tx_status
        usebeep ? hmi.beepHMI(BEEP_CANCEL) : hmi.playSound(BEEPERROR);
      }
        }
    break;

// ======= Antenna Switch ======== 
    case antenna_switch: { 
      if (!tx_status){
        if (which_antenna) {
            which_antenna = 0;
            hmi.setVPWord(antenna_switch_display, ANTENNA_ONE);
            mcp23017.digitalWrite(ANTSWITCH, false);
        } else {
            which_antenna = 1;
            hmi.setVPWord(antenna_switch_display, ANTENNA_TWO);
            mcp23017.digitalWrite(ANTSWITCH, true);
        }
        EEPROM.update(eeprom_antenna, which_antenna);
        usebeep ? hmi.beepHMI(BEEP_YES) : hmi.playSound(YES);
      }else{
          usebeep ? hmi.beepHMI(BEEP_CANCEL) : hmi.playSound(BEEPERROR);
      }
    
      }
    break;

// ========== Text strings change select keyboard page ======    
    case top_bottom_text_touch: {
      whichText = lastByte;
      hmi.beepHMI(BEEP_YES);
      hmi.setPage(keyboardPage);
      displayText.clear();
      hmi.setText(tmpTextDisplay,displayText.c_str()); // clear display
      hmi.setVPByte(case_image,UPC_ICON);
      which_case = 0;  // Upper Case set
    } 
    break;

// ======= error reset relay or fet switch for dxworld ======== 
    case touch_reset: { 
        usebeep ? hmi.beepHMI(BEEP_YES) : hmi.playSound(YES);
        error_od_status_stop = false;
    //   swr_soft_trip = false; // reset swr software trip if in use
        error_i_status = false;
        error_vo_status = false;
        clearSWRInterruptFlags();  // ints set from LM393 on high swr
        swr2IntCount = 0; swr1IntCount = 0;
        #ifdef EXT_CURRENT_TRIP
          digitalWrite(TRIP50v,LOW);
          error_50v_status = false;
          enable50v();
        #endif
      }
    break; 

// ======= SWR Display input swr/lpf  ======== 
    case swr_meter_change: { 
      peak_hold_reset = true;  // reset peak on swr change
      if (which_swr) {
            which_swr = false;
            hmi.setVPWord(swr_meter_switch_display, SWR_DISPLAY);
            swrOffset = 0;
            
        } else {
            which_swr = true;
            hmi.setVPWord(swr_meter_switch_display, LPF_DISPLAY);
            swrOffset = EEPROMROW;    //select lpf filter location in eeprom
        }
        usebeep ? hmi.beepHMI(BEEP_YES) : hmi.playSound(YES);
      }
    break;

// ======= Display Power Calc Settings ======== 
    case calc_power_touch: { 
    uint16_t check_Set_Point;

    check_Set_Point = intSettingsArray[POWERSETPOINT];
    if (check_Set_Point > 800 || check_Set_Point < 60) check_Set_Point = 200;
    hmi.setVPWord(display_power_set_point, check_Set_Point);
    usebeep ? hmi.beepHMI(BEEP_YES) : hmi.playSound(YES);
    hmi.setPage(powerSetPage);
    setting_power_calc = true;
        
  //      const uint16_t arraySize = (sizeof(powerCalcArray));
  //      for(int j = 0; j < arraySize/sizeof(uint16_t); j++)
   //     {
  //       Serial.print(" ");
  //       Serial.print(powerCalcArray[j],HEX);
  //       Serial.print(" ");
  //       Serial.print(j);
  //      }
         
        savePowerValue = powerCalcArray[calc_array_swr_offset + swrOffset];
        hmi.setText(startPage_band_text, bandStrings(calc_array_swr_offset));
        if (which_swr) {
            hmi.setText(startPage_swr_text, stringSWR1);
        } else {
            hmi.setText(startPage_swr_text, stringSWR2);
        }   
    
      }
   break;

// ======= save button page 1 power set ======== 
      case save_power_calc: { 
        setting_power_calc = false;
        tx_status ? hmi.setPage(txPage) : hmi.setPage(startPage);
        usebeep ? hmi.beepHMI(BEEP_YES) : hmi.playSound(YES);
        eeprom_write_power_calc_values();
        intSettingsArray[POWERSETPOINT] = hmi.readVP(display_power_set_point);
        eeprom_write_intSetting_values();
        eeprom_read_power_calc_values();   
      }
    break; 
 // ======= page 1 power set cancel button  ======== 
    case startPage_cancel: { 
        powerCalcArray[calc_array_swr_offset + swrOffset] = savePowerValue;
        usebeep ? hmi.beepHMI(BEEP_CANCEL) : hmi.playSound(BEEPERROR);
        setting_power_calc = false;
        tx_status ? hmi.setPage(txPage) : hmi.setPage(startPage);  
      }
    break;
// ======= test power calc button page 1 power set  ======== 
    case test_power_calc: { 
        uint16_t powerCalcSet;
        uint16_t powerSetValue;
        float Voltfwd;
        if (which_swr) {
            Voltfwd = fwd1Voltage();
        } else {
            Voltfwd = fwd2Voltage();
        }

        Voltfwd = Voltfwd + diodeLossMV;
        powerSetValue = hmi.readVP(display_power_set_point);

        powerCalcSet = ((Voltfwd * Voltfwd) / (float)powerSetValue / MAXAMPPOWERCALC);   // power_set_value normally 100/200 watt
        powerCalcSet = map(powerCalcSet, 0, 250, 250, 0);  // reverse it.
        delay(200);
        powerCalcArray[calc_array_swr_offset + swrOffset] = powerCalcSet;

        usebeep ? hmi.beepHMI(BEEP_YES) : hmi.playSound(YES);
    
      }
    break; 
// ======= Drive level Display and Calc. ======== 
    // select page2 drive set
    case calc_drive_touch: { 
    // need if txstatus etc.
        hmi.setPage(driveSetPage);
        setting_power_calc = true;
        usebeep ? hmi.beepHMI(BEEP_YES) : hmi.playSound(YES);
        saveDrive = powerCalcArray[calc_array_drive_offset];
        hmi.setVPWord(power_eeprom_display2, saveDrive);
        hmi.setFloatValue(power_display_page2, driveWatts());
        // use same vp as start page for band text display
        hmi.setText(startPage_band_text, bandStrings(calc_array_swr_offset));   
      }
    break;
 // Save button page2 drive set     
    case save_drive_calc: { 
        setting_power_calc = false;
        tx_status ? hmi.setPage(txPage) : hmi.setPage(startPage);   
        glo_drive_power = hmi.readVP(power_eeprom_display2);
        powerCalcArray[calc_array_drive_offset] = glo_drive_power;
        // EEPROM.update(eeprom_drive,glo_drive_power);
        usebeep ? hmi.beepHMI(BEEP_YES) : hmi.playSound(YES);
        eeprom_write_power_calc_values();
      }
    break; 
// Test button Page 2 drive set
    case test_drive_calc: { 
        glo_drive_power = hmi.readVP(power_eeprom_display2);
        delay(200);
        hmi.setFloatValue(power_display_page2, driveWatts());
        usebeep ? hmi.beepHMI(BEEP_YES) : hmi.playSound(YES);
      }
    break; 
// page 2 drive set cancel button
    case page2_cancel: { 
        glo_drive_power = saveDrive;
        setting_power_calc = false;
        tx_status ? hmi.setPage(txPage) : hmi.setPage(startPage);
        usebeep ? hmi.beepHMI(BEEP_CANCEL) : hmi.playSound(BEEPERROR);  
      }
    break; 
// ======= SWR Level Calc ======== 
    case calc_swr_touch: { 
        hmi.setPage(swrSetPage);
        setting_swr_calc = true;
        hmi.setVPWord(swr_calc_display, powerCalcArray[calc_array_swr_offset + swrOffset + (EEPROMROW * 2)]);
        saveSWR = powerCalcArray[calc_array_swr_offset + swrOffset + (EEPROMROW * 2)];
        //Serial.print("calc_array_swr_offset ");
        //Serial.println(calc_array_swr_offset+swrOffset+(EEPROMROW*2));
        usebeep ? hmi.beepHMI(BEEP_YES) : hmi.playSound(YES);
        hmi.setFloatValue(swr_display_glo_swr, glo_swr_display);
        if (which_swr) {
            hmi.setText(startPage_swr_text, stringSWR1);
        } else {
            hmi.setText(startPage_swr_text, stringSWR2);
        }   
      }
    break; 
// swr calc cancel
    case swr_calc_cancel_control: { 
        powerCalcArray[calc_array_swr_offset + swrOffset + (EEPROMROW * 2)] = saveSWR;
        usebeep ? hmi.beepHMI(BEEP_CANCEL) : hmi.playSound(BEEPERROR);
        setting_swr_calc = false;
        tx_status ? hmi.setPage(txPage) : hmi.setPage(startPage);   
      }
    break; 
// swr calc save
    case swr_calc_save_control: { 
        tx_status ? hmi.setPage(txPage) : hmi.setPage(startPage);
        usebeep ? hmi.beepHMI(BEEP_YES) : hmi.playSound(YES);
        eeprom_write_power_calc_values();
        setting_swr_calc = false;
        eeprom_read_power_calc_values(); 
      }
    break; 
// swr test control
    case swr_calc_test_control: { 
        powerCalcArray[calc_array_swr_offset + swrOffset + (EEPROMROW * 2)] = hmi.readVP(swr_calc_display);
        delay(200);
        usebeep ? hmi.beepHMI(BEEP_YES) : hmi.playSound(YES);
      }
    break; 
// ======= Page 3 Volt calc ======== //
    case calc_volt_touch: { 
        hmi.setPage(voltSetPage);
        hmi.setVPWord(volt_calc_display, glo_volt_setting);
        saveVolt = glo_volt_setting;
        setting_volt_calc = true;
        usebeep ? hmi.beepHMI(BEEP_YES) : hmi.playSound(YES);
      }
    break; 
// volt calc save
        case volt_calc_save_control: {
        tx_status ? hmi.setPage(txPage) : hmi.setPage(startPage);  
        setting_volt_calc = false;
        EEPROM.update(eeprom_volt, glo_volt_setting);
        usebeep ? hmi.beepHMI(BEEP_YES) : hmi.playSound(YES);
      }
    break; 
// Volt calc cancel
    case volt_calc_cancel_control: { 
        setting_volt_calc = false;
        glo_volt_setting = saveVolt;
        usebeep ? hmi.beepHMI(BEEP_CANCEL) : hmi.playSound(BEEPERROR);
        tx_status ? hmi.setPage(txPage) : hmi.setPage(startPage);
      }
    break;
// Volt test
    case volt_calc_test_control: { 
        // hmi.setFloatValue(volt_calc_volt_display,glo_volt_display);
        glo_volt_setting = hmi.readVP(volt_calc_display);
        usebeep ? hmi.beepHMI(BEEP_YES) : hmi.playSound(YES);
        read_volt();   
      }
    break; 
// ======= Page 5 curent calc ======== 
    case calc_current_touch: { 
        hmi.setPage(currentSetPage);
        saveCurrent = glo_current_setting;
        hmi.setVPWord(current_calc_display, glo_current_setting);
        setting_current_calc = true;
        usebeep ? hmi.beepHMI(BEEP_YES) : hmi.playSound(YES); 
      }
    break;

    case current_calc_save_control: { 
      setting_current_calc = false;
         usebeep ? hmi.beepHMI(BEEP_YES) : hmi.playSound(YES);
        tx_status ? hmi.setPage(txPage) : hmi.setPage(startPage);
        EEPROM.update(eeprom_current, glo_current_setting);
      }
    break; 

    case current_calc_test_control: { 
      glo_current_setting = hmi.readVP(current_calc_display);
      usebeep ? hmi.beepHMI(BEEP_YES) : hmi.playSound(YES);
      }
    break;

    case current_calc_cancel_control: { 
        glo_current_setting = saveCurrent;
        setting_current_calc = false;
        usebeep ? hmi.beepHMI(BEEP_CANCEL) : hmi.playSound(BEEPERROR);
        tx_status ? hmi.setPage(txPage) : hmi.setPage(startPage); 
      }
    break; 
// ======= TRIP settings page 6 ======== 
    case trip_set_touch: { 
        usebeep ? hmi.beepHMI(BEEP_YES) : hmi.playSound(YES);
        delay(20);  // delays are my test
        hmi.setVPWord(trip_temp_display, intSettingsArray[TEMPSETPOINT]);
        delay(20);
        hmi.setVPWord(trip_volt_display, intSettingsArray[VOLTSETPOINT]);
        delay(20);
        hmi.setVPWord(trip_drive_display, intSettingsArray[DRIVESETPOINT]);
        delay(20);
        hmi.setVPWord(trip_current_display, intSettingsArray[CURRENTSETPOINT]);
        delay(20);
        hmi.setPage(tripSetPage);  
      }
    break;

     case trip_save_button: { 
        usebeep ? hmi.beepHMI(BEEP_YES) : hmi.playSound(YES);
        tx_status ? hmi.setPage(txPage) : hmi.setPage(startPage);
        intSettingsArray[TEMPSETPOINT] = hmi.readVP(trip_temp_display);
        delay(20);
        intSettingsArray[VOLTSETPOINT] = hmi.readVP(trip_volt_display);
        delay(20);
        intSettingsArray[DRIVESETPOINT] = hmi.readVP(trip_drive_display);
        delay(20);
        intSettingsArray[CURRENTSETPOINT] = hmi.readVP(trip_current_display);
        delay(20);
        eeprom_write_intSetting_values();
        delay(60);
        eeprom_read_intSetting_values(); //test
      }
    break;

    case global_reset: { 
        EEPROM.update(eeprom_new_on_address, 0xff);
        delay(50);
        usebeep ? hmi.beepHMI(BEEP_CANCEL) : hmi.playSound(BEEPERROR);
        delay(200);
        hmi.restartHMI();
        delay(100);
        resetFunc();
      }
    break; 

   // ======== page 15 Keyboard Page ============= 
   case keyboard_keys: {
    
   switch (lastByte) {
    case escape_key:{
        usebeep ? hmi.beepHMI(BEEP_CANCEL) : hmi.playSound(BEEPERROR);
        hmi.setPage(startPage);    
    } 
    break;
    case backspace_key: {
     displayText.removeLast(1);  
    }
    break;
    case clear_key: {
     colorSave = COLOR_WHITE ; displayText.clear();
     hmi.setTextColor(tmpTextDisplaySP, 3, COLOR_WHITE);
    }
    break;
    case ok_key:{
       if (!colorSave) colorSave = COLOR_WHITE; // default to white
    if (whichText == 1){ // top text on the start page
        hmi.setText(main_text1,""); 
        hmi.setText(main_text1,displayText.c_str());
        hmi.setTextColor(main_text1_sp,3,colorSave);
        safeStringToEEPROM(eeprom_text1_address,displayText.length(), displayText);
        intSettingsArray[TEXT1COLOR] = colorSave;
    }else{
        hmi.setText(main_text2,""); 
        hmi.setText(main_text2,displayText.c_str());
        hmi.setTextColor(main_text2_sp,3,colorSave);
        safeStringToEEPROM(eeprom_text2_address,displayText.length(), displayText);
        intSettingsArray[TEXT2COLOR] = colorSave;
    }
    eeprom_write_intSetting_values();
    hmi.setPage(startPage);
    
    }   // end case ok_key
    break;
    case case_key:{
      which_case = !which_case;
      if (which_case){
        hmi.setVPByte(case_image,LOC_ICON);    
      }else{
        hmi.setVPByte(case_image,UPC_ICON);
      }
    }
    break;
    case red_key:{
      colorSave = COLOR_RED;displayText.clear();
      hmi.setTextColor(tmpTextDisplaySP, 3, COLOR_RED);
    }
    break;
    case green_key: {
      colorSave = COLOR_GREEN; displayText.clear();
      hmi.setTextColor(tmpTextDisplaySP, 3, COLOR_GREEN);
    } 
    break;
    case blue_key: {
      colorSave = COLOR_BLUE; displayText.clear();
      hmi.setTextColor(tmpTextDisplaySP, 3, COLOR_BLUE);
    } 
    break;
    case yellow_key: {
      colorSave = COLOR_YELLOW; displayText.clear();
      hmi.setTextColor(tmpTextDisplaySP, 3, COLOR_YELLOW); 
    }
    break;
    case liteblue_key: {
      colorSave = COLOR_LIGHT_BLUE; displayText.clear();
      hmi.setTextColor(tmpTextDisplaySP, 3, COLOR_LIGHT_BLUE);
    }
    break;
    case orange_key: {
      colorSave = COLOR_ORANGE; displayText.clear();
      hmi.setTextColor(tmpTextDisplaySP, 3, COLOR_ORANGE);
    }
    break;
    default:{
    }
   } // end  keyboard switch

  if (lastByte < 127) {  // asscii end
        if ((lastByte >= 65) && (lastByte <= 97) && which_case){
          displayText += char(lastByte+32);   // lower case
        }else{
          displayText += char(lastByte); 
        }
  }
  hmi.setText(tmpTextDisplay,displayText.c_str()); 
  hmi.beepHMI(BEEP_TINY);
  
 } // end keyboard case
  
      
} // end main switch 


} // end tag onHMIEvent

/* ======= Display String Display Functions ======== */
String swrStrings(bool whichSWR) {
    if (whichSWR) {
        return stringSWR2;
    } else {
        return stringSWR1;
    }
}

String bandStrings(uint16_t arrayOffset) {
    #ifdef SINGLE_BAND
    return stringSingleBand;
    #endif
    String retString = "ERROR No Index";
    switch (arrayOffset) {
#ifdef DISPLAY160M
    case offsetSWR160M:
        retString = string160M;
        break;
#endif
    case offsetSWR80M:
        retString = string80M;
        break;
    case offsetSWR60_40M:
        retString = string60_40M;
        break;
    case offsetSWR30_20M:
        retString = string30_20M;
        break;
    case offsetSWR17_15M:
        retString = string17_15M;
        break;
    case offsetSWR12_10M:
        retString = string12_10M;
        break;
    case offsetSWR6M:
        retString = string6M;
        break;
#ifndef DISPLAY160M
    case offsetSWR4M:
        retString = string4M;
        break;
#endif
    }
    return retString;
}

#ifdef MYDEBUG
//only used in debugging
// noWords should be your char length / 2
// eg. read 12 chars Serial.println(readVPText(0x2300,6)) ;
String readVPText(uint16_t vpAddress,byte noWords){
  int i = 0;
  bool nextByte = true; //true = read first char
  byte byteRead = 0;
  String textMessage;
  while(i < noWords){
    byteRead = hmi.readVPByte((vpAddress+i),nextByte);
    // 128 can be 254 for extended ascii
    if (byteRead == 0x00 || byteRead < 32 || byteRead > 128){
      break;
    }
    textMessage += char(byteRead);
    nextByte = !nextByte;
    if (nextByte){i++;}
  }
  return textMessage;
}
#endif

