//#define MYDEBUG
//#define DISPLAYDEBUG
#define MY_ALL_BAND_AMP     // my amp some different test configs
//#define DISPLAY160M       // alternate icons 160m-6m or 80m-4m my own
//#define DXWORLD_I     //Id from dxworld protection board else ACS712    

//#define RESETDUBUG
//#define SCREENROTATE      // see setupdisplay function
//#define BIAS_ON           // keep bias on test etc
#define AUX_FAN             // aux fan 
//#define AUTO_BAND
//#define DRIVE_NO_STOP     // overdrive error only dont stop tx
#define YAESU               // auto band yaesu or otherwise icom not fully tested
#define BLINKLED            // if we are not using pin13 OPTOUT1 (a600_bias) flash onboard led
#define SENSOR_DEBUG        // debug dallas sensors
#define EXT_CURRENT_TRIP    // my current trip board
#define SINGLE_BAND         // single band amp

#include <Arduino.h>
#include <DWIN_Arduino.h>
#include <Wire.h>

#ifndef DXWORLD_I
    #include <ACS712.h>
#endif
#ifdef AUTO_BAND
    #include<ADS1015_WE.h> 
#endif
#include <SafeString.h>    
#include <MCP23017.h>
#include <avr/wdt.h>
#include <Ticker.h>
#include <Ewma.h>
//#include <EwmaT.h>
#include "OneWireNg_CurrentPlatform.h"
#include "drivers/DSTherm.h"
#include "utils/Placeholder.h"
static Placeholder<OneWireNg_CurrentPlatform> ow;
DSTherm drv(ow);
#include <EEPROM.h>
#include <includes.h>
#ifdef RESETDUBUG
    #include <resets.h>
#endif
#include <setup_functions.hpp>
#include <errors_and_board_reading.hpp>
#include <calc_power_and_display.hpp>
#include <dwin_callback.hpp>
#include <eeprom_functions.hpp>
#include <ptt_actions.hpp>
#include <band_select_functions.hpp>




void setup() {
    setupPins();
    analogReference(INTERNAL4V3);
    analogWriteFrequency(16); // 16k
    analogWrite(FANPWM, 0);
    /*
      if we are using jtag2updi which is on tx0/rx0 to program
      the serial print functions also use the same port
      dont serial print early or we can't program easily
      this is why we need a big delay so we can jump in
      we can always use the updi port!  
    */
    delay(3000);
    Serial.begin(115200);
#ifdef RESETDUBUG
    printResetFlags();
    clearResetFlags();
#endif
    Wire.begin();
    //may need changing to slower eg. 100000UL
    Wire.setClock(400000UL); // probably needs 3k3 pullups
    #ifndef DXWORLD_I
        ACS.autoMidPoint();
    #endif
    mcp23017_setup();
    readEEPROM();
    setupDisplay();
    setup_dallas_sensors();

    analogWrite(FANPWM, 56);  //start fan
    houseKeeping.start(); temperatureIDTicker.start();
    peakHoldTicker.start(); sendPowerSwrRefTicker.start();
//    attachInterrupt(PTT, PTTservice, FALLING); // Falling edge at PTT
#ifndef SINGLE_BAND
    attachInterrupt(SWR2_INT, swr2IntRising, RISING);
#endif 
    attachInterrupt(SWR1_INT, swr1IntRising, RISING);
    clearSWRInterruptFlags();
    wdt_enable(WDT_PERIOD_4KCLK_gc);      // set watchdog to 4 secs
    keepingHouse(); // if band auto pull band relays now
    delay(500);     // dont start just yet
    Serial.println("Starting...");
#ifdef BIAS_ON
    bias_on();
#endif

//fanpower(true);
#ifdef AUX_FAN
    auxFanpower(false);
#endif
#ifdef EXT_CURRENT_TRIP
    digitalWrite(TRIP50v,LOW);
    enable50v();
#endif

#ifdef AUTO_BAND
    if (!adc.init()) {
        Serial.println("ADS1015 not connected!");
    }else{
        adc.setVoltageRange_mV(ADS1015_RANGE_4096);         
    }
#endif //AUTOBAND    

#ifdef MYDEBUG
    displayText.setOutput(Serial);
    safeStringFromEEPROM(eeprom_text1_address,30, displayText);
    displayText.debug();
    safeStringFromEEPROM(eeprom_text2_address,30, displayText);
    displayText.debug();
#endif

}

void loop() {
    wdt_reset();
    hmi.listen();
    // keep our tickers alive
    houseKeeping.update();temperatureIDTicker.update();
    sendPowerSwrRefTicker.update();peakHoldTicker.update();

    tx_actions();
    if (tx_status) {
       calcPowerandDisplay();  
    }

    if (temp_id_reset) {
        //this ticker is temperatureIDTicker
        hmi.setFloatValue(temp_display, readTemp(false));
        hmi.setFloatValue(volt_display, read_volt());
        hmi.setFloatValue(current_display, readI());
        error_swr(); 
        error_odrive();
        temp_id_reset = false;
        }
    }  // end loop

void keepingHouse() {
    // runs continuous every 2000ms
#ifdef BLINKLED
    static bool toggle;
    digitalWrite(OPTOUT1, toggle);
    toggle = !toggle;
#endif // blinkLED

#ifdef AUTO_BAND    
    if ((!tx_status) && (band_auto)){
     auto_band();
    }else if (!band_auto){
     band_auto_touched = true;
    }
#endif // AUTO_BAND    
    

}  // end keepingHouse



