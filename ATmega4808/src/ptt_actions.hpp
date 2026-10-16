#include <Arduino.h>
#ifndef INCLUDES_H
    #include <includes.h>
#endif

/* === read ptt and set tx delay  === */

void tx_actions() {
    static bool tx_running;
    static bool display_dim;

    if ((digitalRead(PTT) == HIGH) || (error_vo_status)|| 
        (error_temp_status) || 
        #ifndef DRIVE_NO_STOP 
        (error_od_status_stop) || 
        #endif
        #ifdef EXT_CURRENT_TRIP
        (error_50v_status) ||
        #endif
        (error_i_status) || 
        (swr2IntActiveHigh) || 
        (swr1IntActiveHigh)) {
            
#ifndef BIAS_ON
        bias_off();
#endif
        tx_status = false;
        tx_running = false;
        fwdPower_max = 0;
        refPower_max = 0;
        if (display_dim == true) {
            display_dim = false;
            hmi.setPage(startPage);
            hmi.writeLongValue(LCDBackLightAddress, LCDBackLightLow);
        }
    } else if ((digitalRead(PTT) == LOW) && (!tx_running) && (error_od_status_stop == false)) {
        //   Serial.println("PTT_LOW");   // tx on
        tx_status = true;
        tx_running = true;
        // may not be needed if using solid state switching
        delay(TX_DELAY);
        // call for meters now
        temp_id_reset = true;
        peak_hold_reset = true;
        power_swr_reset = true;
        //Serial.println("Bios On");
        bias_on();
        usebeep ? hmi.beepHMI(BEEP_YES) : hmi.playSound(BEEP);
        if (display_dim == false) {
            display_dim = true;
            hmi.writeLongValue(LCDBackLightAddress, LCDBackLightNormal);
            hmi.setPage(txPage);
        }

    }

}


void bias_on() {
    clearSWRInterruptFlags();
    swr2IntCount = 0; swr1IntCount = 0;
    mcp23017.digitalWrite(BIAS12V, true);
}

void bias_off() {
    swr2IntCount = 0; swr1IntCount = 0;
    mcp23017.digitalWrite(BIAS12V, false);
}

void PTTservice() {
    pttIntActive = true;
}

void enable50v() {
   mcp23017.digitalWrite(EN_I_BOARD, true); 
   delay(Soft_Start_time);
   mcp23017.digitalWrite(EN_I_BOARD, false);
}