  #pragma once
#include <Arduino.h>
#ifndef INCLUDES_H
#include <includes.h>
#endif

/* === Over temp protection and show temp error=== */
void error_temp(float temperature)
{

    static bool highTemp;
    if (temperature >= ((intSettingsArray[TEMPSETPOINT]) - 1))
    {
        highTemp = true;
        analogWrite(FANPWM, 256); // Full Fan Speed;
    }

    if ((temperature <= ((intSettingsArray[TEMPSETPOINT]) - 5)))
    {
        highTemp = false;
        fanspeed(temperature); // fanspeed to normal
    }

    if ((error_temp_status == false) && (highTemp))
    {
        hmi.setVPByte(temp_error, RED_ICON);
        error_temp_status = true;
        usebeep ? hmi.beepHMI(BEEP_ERROR) : hmi.playSound(BEEPERROR);
    }

    if ((error_temp_status == true) && (!highTemp))
    {
        hmi.setVPByte(temp_error, GREEN_ICON);
        error_temp_status = false;
    }
    
    #ifdef AUX_FAN
     static bool auxFanStatus;
     if ((temperature > AUXFANON) && (!auxFanStatus))
     {
        auxFanStatus = true;
        auxFanpower(true);
     }
     if ((temperature < AUXFANOFF) && (auxFanStatus))
     {
       auxFanStatus = false;
       auxFanpower(false);
     }
      
    #endif
    

}


/* === Monitor and show over SWR error on display === */
void error_swr()  
{   
    bool error_swr_en;
    if ((swr1IntActiveHigh) || (swr2IntActiveHigh)){
      error_swr_en = false;
    }else{
      error_swr_en = true;
    }

   
    // if (swrSoftTrip == true) error_swr_en = false;

    if (!error_swr_en && !error_swr_status)
    {   
        if (swr1IntActiveHigh){
            hmi.setVPByte(swr_error, RED_ICON);
        }else{
            hmi.setVPByte(swr2_error, RED_ICON);
        }
        mcp23017.digitalWrite(SWR_TRIP, true);
        error_swr_status = true;
        usebeep ? hmi.beepHMI(BEEP_ERROR) : hmi.playSound(BEEPERROR);
    }
    else if (error_swr_en && error_swr_status)
    {
        hmi.setVPByte(swr_error, GREEN_ICON);
        hmi.setVPByte(swr2_error, GREEN_ICON);
        error_swr_status = false;
        mcp23017.digitalWrite(SWR_TRIP, false);
    }
}


void errorI(float I){  // not DXWorld
   if((I >= intSettingsArray[CURRENTSETPOINT]) && (error_i_status == false))
    {
        hmi.setVPByte(current_error, RED_ICON);
        error_i_status = true;
        usebeep ? hmi.beepHMI(BEEP_ERROR) : hmi.playSound(BEEPERROR);
    }
    else if (error_i_status == false) {
        hmi.setVPByte(current_error, GREEN_ICON);
    }
}


void error_vo(float V)
{
    // V = 55; // test
    if ((V >= intSettingsArray[VOLTSETPOINT]) && (!error_vo_status))
    {
        hmi.setVPByte(volt_error, RED_ICON);
        error_vo_status = true;
        usebeep ? hmi.beepHMI(BEEP_ERROR) : hmi.playSound(BEEPERROR);
    }
    else if (!error_vo_status)
    {
        hmi.setVPByte(volt_error, GREEN_ICON);
    }
#ifdef EXT_CURRENT_TRIP
    if ((V <= 45 ) && (tx_status) && (!error_50v_status)) // low 50 volt check on tx 
    {
        error_50v_status = true;
        usebeep ? hmi.beepHMI(BEEP_ERROR) : hmi.playSound(BEEPERROR);
        hmi.setVPByte(external_trip_error, RED_ICON);

    }
    else if (!error_50v_status)
    {
        hmi.setVPByte(external_trip_error, GREEN_ICON);
    }
#endif

}


/* ======= Read 48/65v Voltage ======== */
float read_volt()
{
    float vcc;
    float Vin = 0.00;
    float Vout = 0.00;
    int v1_val = 0;
    float Vresult = 0;
    float mapResult;

    v1_val = analogRead(VIN50V);

    Vout = (float)((v1_val * 4.30) / 1024.00);
    Vin = Vout / (Res50v_2 / (Res50v_1 + Res50v_2));
    Vresult = ((Vin * 10) + 0.5);
    Vresult = voltFilter.filter(Vresult);

    if (glo_volt_setting <= 50)
    {
        mapResult = map(glo_volt_setting, 1, 50, 25, 1);
        Vresult = Vresult - (float)(mapResult);
    }
    else
    {
        mapResult = map(glo_volt_setting, 51, 100, 1, 25);
        Vresult = Vresult + (float)(mapResult);
    }
    if (setting_volt_calc)
    {
        hmi.setFloatValue(volt_calc_volt_display, (float)Vresult / 10);
    }
    float Vreturn = (float)Vresult / 10;
    error_vo(Vreturn);
    return (Vreturn);
}

/* ======= Adjust Fanspeed to Temperature ======== */
void fanspeed(float temperature)
{

 //    analogWrite(FANPWM,0);  //TEST
 //    return;
    // temperature = 32.00;

    if (temperature > 31.0 && temperature < 42.0)
    {
        analogWrite(FANPWM, 64); // 25% duty cycle;
    }
    else if (temperature >= 42.0 && temperature < 44.0)
    {
        analogWrite(FANPWM, 128); // 128 50% duty cycle) test 256 14-09-24
    }
    else if (temperature >= 44.0 && temperature < 47.0)
    {
        analogWrite(FANPWM, 192); // 75% 
    }
    else if (temperature >= 47.0 && temperature < intSettingsArray[TEMPSETPOINT])
    {
        analogWrite(FANPWM, 255); // Full Fan Speed;
    }
    else if (temperature <= 5.0){   // sensor error!
        analogWrite(FANPWM, 64);    // 25% duty cycle;
    }
    else if (temperature <= 28.5)   // Hysteresis
    {
        analogWrite(FANPWM, 0); // no Fan Speed;
    }
}

/* ======= Read the LDMOS Drain Current ======== */
float readI()
{
    float currentI = 0;
    float mapResult;
#ifdef DXWORLD_I
    currentI = ((analogRead(ID_IN)) * 2); // *2 resistor split on ID_IN
    currentI *= adcRef;
    currentI = (currentI / 4800); // need working out TO DO
    if (glo_current_setting <= 50)
    {
        mapResult = map(glo_current_setting, 1, 50, 50, 1);
        currentI *= (13000 - (mapResult * ICALCMAJOR));
    }
    else
    {
        mapResult = map(glo_current_setting, 51, 100, 1, 50);
        currentI *= (13000 + (mapResult * ICALCMAJOR));
    }

       if (setting_current_calc)
    {
        hmi.setFloatValue(current_calc_current_display, currentI);
    } 
    errorI(currentI);
    return currentI;
        
#else  // do acs712
    mapResult = map(glo_current_setting, 1, 50, 50, 1);
    ACS.setmVperAmp(66); // 30 amp acs712 66mv per amp
    int mA = ACS.mA_DC();
    mA = mA + (mapResult*10);
    if (isNegative(mA) || mA < 500) mA = 0;
    currentI = (float)mA/1000;
    if (setting_current_calc)
    {
        hmi.setFloatValue(current_calc_current_display, currentI);
    }
    errorI(currentI);
    return currentI;
      
#endif
}   // end float readI()


/* ======= Read Temperature ======== */
float readTemp(bool starting)
{

    float temperature = 0;
    /* from OneWireNg example */
    DSTherm drv(ow);
    uint8_t sensorCount = 0;
    if (starting)
    {
        drv.convertTempAll(DSTherm::MAX_CONV_TIME_START, false);
    }
    else
    {
        // we probably need more MAX_CONV_TIME for multiple sensors
        drv.convertTempAll(DSTherm::MAX_CONV_TIME, false);
    }
#if SENSOR_COUNT == 1
    static PlaceholderInit<DSTherm::Scratchpad> scrpd;
    OneWireNg::ErrorCode ec = drv.readScratchpadSingle(scrpd);
    if (ec == OneWireNg::EC_SUCCESS)
    {
        temperature = (DS18B20Temperature(scrpd));
    }
    else if (ec == OneWireNg::EC_CRC_ERROR)
    {
#ifdef SENSOR_DEBUG
        Serial.println("  DS18B20 CRC error.");
#endif
    }
#else // #if SENSOR_COUNT > 1
    /* read sensors one-by-one from OneWireNg example */
    Placeholder<DSTherm::Scratchpad> scrpd;
    for (const auto &id : *ow)
    {
        Serial.print("sensor count ");
        Serial.println(sensorCount);
        if (returnSensorId(id, sensorCount))
        {
            if (drv.readScratchpad(id, scrpd) == OneWireNg::EC_SUCCESS)
            {
                temperature = DS18B20Temperature(scrpd);
                error_temp(temperature); // check error on all sensors
                sensor_array[sensorCount].sensor_value = temperature;
                sensor_array[sensorCount].count = sensorCount + 1; // we can use 0 as a end stop if needed
                sensorCount += 1;
            }
            else
            {
#ifdef SENSOR_DEBUG
                Serial.println("  Read scratchpad error.");
#endif
            }
        }
    }
#ifdef SENSOR_DEBUG
    Serial.println("Sensor Array");
    for (int i = 0; i < SENSOR_COUNT; i++)
    {
        Serial.print(sensor_array[i].sensor_our_name);
        Serial.print(" ");
        Serial.print(sensor_array[i].sensor_value);
        Serial.println(" ");
        Serial.println(sensor_array[i].mac_address);
        Serial.print(sensor_array[i].sensor_real_name);
        Serial.println(" ");
    }
#endif // #ifdef SENSOR_DEBUG
#endif // #if SENSOR_COUNT

    // Serial.println(temperature);
    //  temperature = 55.00;  // test
    error_temp(temperature);
    return temperature; // 18b20 return last sensor
}

float DS18B20Temperature(const DSTherm::Scratchpad &scrpd)
{
    const uint8_t *scrpd_raw = scrpd.getRaw();
    long temp = scrpd.getTemp2();
    return ((float)temp / 16);
}
static bool returnSensorId(const OneWireNg::Id &id, uint8_t sensorCount)
{
    const char *name = DSTherm::getFamilyName(id);
    String macAdress = "";
    for (size_t i = 1; i < sizeof(OneWireNg::Id); i++)
    {
        macAdress = checkHex(id[i]) + ":" + macAdress;
    }
    sensor_array[sensorCount].mac_address = macAdress;
    sensor_array[sensorCount].sensor_real_name = String(name);
    return (name != NULL);
}

void error_odrive()
{
    float driveWattsIn = driveWatts();
    
    if ((driveWattsIn >= ((float)intSettingsArray[DRIVESETPOINT])) && (error_od_status == false))
    {
        error_od_status = true;
        error_od_status_stop = true;
        hmi.setVPByte(overdrive_error, RED_ICON);

#ifndef DRIVE_NO_STOP        
        bias_off();
#endif
        usebeep ? hmi.beepHMI(BEEP_CANCEL) : hmi.playSound(BEEPERROR);
    }
    else if ((!error_od_status_stop) && (error_od_status))
    {
        error_od_status = false;
        hmi.setVPByte(overdrive_error, GREEN_ICON);
    }
}

// format hex string for single digits
String checkHex(byte currentNo)
{
    if (currentNo < 0x10)
    {
        return "0" + String(currentNo, HEX);
    }
    return String(currentNo, HEX);
}


void fanpower(bool enable){
    mcp23017.digitalWrite(FAN_POWER, enable); 
}

void auxFanpower(bool enable){
    mcp23017.digitalWrite(AUX_FAN_POWER, !enable); // note inverted for my use
}