#include <Arduino.h>
#ifndef INCLUDES_H
    #include <includes.h>
#endif

float driveWatts() {
    float Voltage = 0;
    float Power;
    if (tx_status == false){return 0;}
    for (int i = 0; i < ADC_SAMPLES; i++) {
        Voltage += analogRead(RFSENSE);
        delayMicroseconds(120);
     }
    Voltage = (Voltage / (float)ADC_SAMPLES); 
    Voltage = map(Voltage, 0, 1023, 1, 4300);
    Voltage = Voltage + diodeLossMVdrive;
    Voltage = driveFilter.filter(Voltage);

    if (Voltage <= diodeLossMVdrive + 1) {
        return 0;
    } else {
        Voltage = (Voltage / (float)(map(glo_drive_power, 1, 100, DRIVECALCMAJOR, 1)));
        if (isNegative(Voltage)) Voltage = 0;
        Power = (pow(Voltage, 2.00)); // should be * 50 but we would only have to div. again by 50

        if (Power > 9.90) {            // drive set point max is 10 watts
            return (9.90);
        } else {
            return Power;
        }
    }
}

/* ======= Get Voltage from SWR1/REF1 SWR2/REF2 ======== */
float fwd1Voltage() {
    float Voltage = 0;
    for (int i = 0; i < ADC_SAMPLES; i++) {
        Voltage += analogRead(SWR1);
        delayMicroseconds(120);
     }
    Voltage = (Voltage / (float)ADC_SAMPLES);
    Voltage = map(Voltage, 0, 1023, SWR1FwdMapLow, SWR1FwdMapHigh);
    Voltage += diodeLossMV;
    return Voltage;
}
float ref1Voltage() {
    float Voltage = 0;
       for (int i = 0; i < ADC_SAMPLES; i++) {
        Voltage += analogRead(REF1);
        delayMicroseconds(120);
    } 
    Voltage = (Voltage / (float)ADC_SAMPLES);
    Voltage = refFilter1.filter(Voltage);
    Voltage = map(Voltage, 0, 1023, SWR1RefMapLow, SWR1RefMapHigh);
    return Voltage;
}
float fwd2Voltage() {
  float Voltage = 0;
    for (int i = 0; i < ADC_SAMPLES; i++) {
        Voltage += analogRead(SWR2);
        delayMicroseconds(120);
     }
    Voltage = (Voltage / (float)ADC_SAMPLES);
    Voltage = map(Voltage, 0, 1023, SWR2FwdMapLow, SWR2FwdMapHigh);
    Voltage += diodeLossMV;
    return Voltage;
}
float ref2Voltage() {
    float Voltage = 0;
     for (int i = 0; i < ADC_SAMPLES; i++) {
        Voltage += analogRead(REF2);
        delayMicroseconds(120);
    }
    Voltage = (Voltage / (float)ADC_SAMPLES);
    Voltage = refFilter2.filter(Voltage);
    Voltage = map(Voltage, 0, 1023, SWR2RefMapLow, SWR2RefMapHigh);
    return Voltage;
}

/* ======= Calc and Display Output Power/REF/SWR etc. ======== */
void calcPowerandDisplay() {
    float rawFwdVoltage; float rawRefVoltage;
    float fwdPower; float refPower;
    float powerCalc;
    uint8_t swr_calc_major;
    static unsigned long peakTime;
    static float decayTime;


    if ((which_swr == false)) {  // false the LPF tandem match
        rawFwdVoltage = fwd2Voltage(); rawRefVoltage = ref2Voltage();
        swr_calc_major = SWR2CALCMAJOR;
    } else {
        rawFwdVoltage = fwd1Voltage(); rawRefVoltage = ref1Voltage();
        swr_calc_major = SWR1CALCMAJOR;
    }
   
    powerCalc = powerCalcArray[calc_array_swr_offset + swrOffset];
    powerCalc = map(powerCalc, 0, 250, 250, 0);  // reverse it

    if ((rawRefVoltage >= rawFwdVoltage) || (rawFwdVoltage <= (diodeLossMV))) {
        rawRefVoltage = 0.00; rawFwdVoltage = 0.01;
    }
    // swr/ref power adjustment
    rawRefVoltage = correctRefVoltage(rawRefVoltage + diodeLossMV, rawFwdVoltage, swr_calc_major);
 
    fwdPower = pow((rawFwdVoltage), 2.00); refPower = pow((rawRefVoltage), 2.00);
    fwdPower = fwdPower/powerCalc/MAXAMPPOWERCALC; refPower = refPower/powerCalc/MAXAMPPOWERCALC;


   // Peak Hold 
    if ((fwdPower > fwdPower_max)) {
        fwdPower_max = fwdPower;
        refPower_max = refPower;
        peakTime = millis();
        decayTime = 1;
    }
    else if ((millis() - peakTime) > PEAKREFRESH){
        fwdPower_max = fwdPower_max / float(decayTime + 0.05); 
        refPower_max = refPower_max  / float(decayTime + 0.05);
        peakTime = millis();
        decayTime += 0.1;
    }
    // end Peak Hold
   
    if (refPower_max < 0.1f) refPower_max = 0;  // clear float to zero
    //Test both should work out the same
    //const float SWR = (rawFwdVoltage + rawRefVoltage) / (rawFwdVoltage - rawRefVoltage);
    const float SWR = (1.00 + sqrtf(refPower_max/fwdPower_max)) / (1.00 - sqrtf(refPower_max/fwdPower_max));
    float swr_display = ((SWR * 10.00)); // Float x 10 for our display
    if ((swr_display < 10.00) || isNegative(swr_display)) {
        swr_display = 10;
    }
    // Display Power when setting power value
    if (setting_power_calc) {
        hmi.setVPWord(power_display_startPage, (int)fwdPower_max); // update page 1 display
    }
    // Used when setting the SWR correction
    if (setting_swr_calc) {
        glo_swr_display = swr_display / 10.00;   // used by calc swr
        hmi.setFloatValue(swr_display_glo_swr, glo_swr_display);
        // power_swr_reset = false;
        if (which_swr == false) {
            hmi.setVPWord(fwd_millivolts, map(analogRead(SWR2), 0, 1023, SWR2FwdMapLow, SWR2FwdMapHigh));
            hmi.setVPWord(ref_millivolts, map(analogRead(REF2), 0, 1023, SWR2RefMapLow, SWR2RefMapHigh));
        } else {
            hmi.setVPWord(fwd_millivolts, map(analogRead(SWR1), 0, 1023, SWR1FwdMapLow, SWR1FwdMapHigh));
            hmi.setVPWord(ref_millivolts, map(analogRead(REF1), 0, 1023, SWR1RefMapLow, SWR1RefMapHigh));
        }
    }  // end setting_swr_calc

    
    if (power_swr_reset) {  // Ticker reset update display - powerSwrTickerDelay
        // Ticker timeout to update display
        if (fwdPower_max < 5) fwdPower_max = 0;
        power_swr_reset = false; // ticker reset
        float driveWattsIn = driveWatts();
        // swr color change
        if (swr_display >= SWRColorChange){
           hmi.setTextColor(swr_graph_sp,0x06,SWRHighSWRColor);
        }else{
           hmi.setTextColor(swr_graph_sp,0x06,SWROriginalColor);
        }
    //    fwdPower_max = fwdFilter.filter(fwdPower_max);
        hmi.setVPWord(power_graph, (int)fwdPower_max);          //1-powerRangeMax
        hmi.setVPWord(swr_graph, ((int)swr_display * 10));      // 100-swrRangeMax
        hmi.setVPWord(power_display, (int)fwdPower_max);        // int 4 digits
        hmi.setFloatValue(rev_display, refPower_max);            // float
        hmi.setFloatValue(swr_digits, (float)swr_display / 10); // float int 1 decimal 2
        hmi.setFloatValue(drive_display, driveWattsIn);

#ifdef MYDEBUG
        Serial.print("SWR ");
        Serial.println(SWR);
        Serial.print("FWD ");
        Serial.println(fwdPower);
        Serial.print("REF ");
        Serial.println(refPower);
#endif
    }
}

float correctRefVoltage(float refVoltage, float fwdVoltage, uint8_t swr_calc_major) {
    if (refVoltage > 1) {
        float swrCalc;
        //swrCalc = (swrCalc / 1.5)  // test
        swrCalc = powerCalcArray[calc_array_swr_offset + swrOffset + (EEPROMROW * 2)];
        if (swrCalc <= 500) {                                // 500 will be about 0
            swrCalc = map(swrCalc, 500, 1, 1, 500);
            refVoltage = refVoltage - (float)(swrCalc * swr_calc_major);
            if (isNegative(refVoltage)) refVoltage = 0;
        } else {
            swrCalc = map(swrCalc, 505, 1000, 1, 500);
            refVoltage = refVoltage + (float)(swrCalc * swr_calc_major);
            if (refVoltage > fwdVoltage) refVoltage = 0;
        }
    }
    return refVoltage;
}

static int isNegative(float negTest) {
    return ((static_cast<int>(negTest + 1) > 0) ? 0 : 1);
}
