#include <Arduino.h>
#ifndef INCLUDES_H
    #include <includes.h>
#endif

/* ======= Read EEPROM on boot and set bands etc. ======== */
void readEEPROM() {
    if (EEPROM.read(eeprom_new_on_address) != 0x5b) {
        for (int i = 1; i < EEPROM_SIZE; i++) {
            EEPROM.write(i, 0xff); // wipe eeprom
            delay(5);
        }
        EEPROM.update(eeprom_new_on_address, 0x5b);
        EEPROM.update(eeprom_band, band80Mtr_Selected);   // set to 80m when new
        band_select = band80Mtr_Selected;
        EEPROM.update(eeprom_auto, 0); EEPROM.update(eeprom_antenna, 0);
        band_auto = 0;
        which_antenna = 0;
        intSettingsArray[TEXT1COLOR] = COLOR_WHITE;
        intSettingsArray[TEXT2COLOR] = COLOR_WHITE;
        safeStringToEEPROM(eeprom_text1_address,MainText1.length(), MainText1);
        safeStringToEEPROM(eeprom_text2_address,MainText2.length(), MainText2);
        delay(500);
        eeprom_write_power_calc_values();
        eeprom_write_intSetting_values();
    } else {
        band_select = EEPROM.read(eeprom_band);
        #ifdef SINGLE_BAND
            band_select = band80Mtr_Selected;  // single band use 80mtr eeprom space
        #endif
        band_auto = EEPROM.read(eeprom_auto);
        which_antenna = EEPROM.read(eeprom_antenna);
        glo_volt_setting = EEPROM.read(eeprom_volt);
        glo_current_setting = EEPROM.read(eeprom_current);

        if (glo_current_setting > 100) glo_current_setting = 50;
        if (glo_volt_setting > 100) glo_volt_setting = 50;
        //eeprom_write_power_calc_values();  //TEST FORCE WRITE
        //eeprom_write_intSetting_values();    //TEST FORCE WRITE
        eeprom_read_power_calc_values();
        // check for out of range
        for (int j = 0; j < (sizeof(powerCalcArray) / sizeof(uint16_t)); j++) {
             //  Serial.print(powerCalcArray[j]);
             //  Serial.print(" ");
            if (powerCalcArray[j] > 2000) powerCalcArray[j] = 50;
        }

        /*
        for (int j = 0; j < (sizeof(intSettingsArray) / sizeof(uint16_t)); j++) {
             Serial.print(intSettingsArray[j]);
             Serial.print(" ");
        }

       for (int j = 210; j <= 240; j++){
         //Serial.println(char((EEPROM.read(j))));  
         Serial.print(j);
         Serial.print(" 0x");
         Serial.print(EEPROM.read(j),HEX);
          Serial.print(" ");
        }
        */  

        eeprom_read_intSetting_values();
    }
}

void eeprom_write_power_calc_values() {
    EEPROM_writeAnything(eeprom_power_calc_address, powerCalcArray);
}
void eeprom_read_power_calc_values() {
    EEPROM_readAnything(eeprom_power_calc_address, powerCalcArray);
}

void eeprom_write_intSetting_values() {
    EEPROM_writeAnything(eeprom_intSettings_address, intSettingsArray);
}

void eeprom_read_intSetting_values() {
    EEPROM_readAnything(eeprom_intSettings_address, intSettingsArray);
}


    void safeStringToEEPROM(uint16_t EEPROM_address, uint16_t safeStringSize, SafeString& strin) {
    uint16_t maxLen = strin.length();
    if (maxLen > safeStringSize) {
        maxLen = safeStringSize;
    }
    uint16_t j = EEPROM_address;
    for (uint16_t i = 0; i < maxLen; i++, j++) {
        delay(10);
        EEPROM.update(j, strin.charAt(i));
    }
    delay(10);
    EEPROM.update(j, '\0');
}
    


void safeStringFromEEPROM(uint16_t EEPROM_address,
                          uint16_t safeStringMaxSize,
                          SafeString& strin) {
    strin.clear();
    uint16_t end = EEPROM_address + safeStringMaxSize;
    for (uint16_t j = EEPROM_address; j < end; j++) {
        uint8_t retByte = EEPROM.read(j);

        if (retByte == 0) break;

        if (!strin.isFull()) {
            strin += char(retByte);
        } else {
            break;
        }
    }
}

template<class T> int EEPROM_writeAnything(int ee, const T &value) {
    const byte *p = (const byte *)(const void *)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++){
    delay(10);    
    EEPROM.write(ee++, *p++);
    }
    return i;
}

template<class T> int EEPROM_readAnything(int ee, T &value) {
    byte *p = (byte *)(void *)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++) *p++ = EEPROM.read(ee++);
    return i;
}
