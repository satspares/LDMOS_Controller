//From youtube Robert Smorgasbord
// Thankyou Robert
#include <Arduino.h>
#include <avr/io.h>

bool resetFlagUPDI() {
    return (RSTCTRL.RSTFR & RSTCTRL_UPDIRF_bm ? true : false);
}

bool resetFlagSoftware() {
    return (RSTCTRL.RSTFR & RSTCTRL_SWRF_bm ? true : false);
}

bool resetFlagWatchdog() {
    return (RSTCTRL.RSTFR & RSTCTRL_WDRF_bm ? true : false);
}

bool resetFlagExternal() {
    return (RSTCTRL.RSTFR & RSTCTRL_EXTRF_bm ? true : false);
}

bool resetFlagBrownout() {
    return (RSTCTRL.RSTFR & RSTCTRL_BORF_bm ? true : false);
}

bool resetFlagPowerOn() {
    return (RSTCTRL.RSTFR & RSTCTRL_PORF_bm ? true : false);
}

void clearResetFlags() {
    RSTCTRL.RSTFR =   RSTCTRL_UPDIRF_bm
        | RSTCTRL_SWRF_bm
        | RSTCTRL_WDRF_bm
        | RSTCTRL_EXTRF_bm
        | RSTCTRL_BORF_bm
        | RSTCTRL_PORF_bm;
}


void printResetFlags() {
    // Reset Controller Reset Flag Register of megaAVR 0-series (e.g. ATmega4809)
    // --------------------------------------------------------------------------
    // Defined in e.g. iom4809.h for ATmega4809 via io.h
    // Before io.h:      #define __AVR_ATmega4809__
    // Within iom4809.h: #define _AVR_IOXXX_H_ "iom4809.h"
    //                   #define _AVR_ATMEGA4809_H_INCLUDED
    // Windows path:     C:\Program Files (x86)\Arduino\hardware\tools\avr\avr\include\avr
    //
    // RSTCTRL           - Reset Controller (struct)
    // RSTCTRL.RSTFR     - Reset Flag Register (byte), also RSTCTRL_RSTFR
    // RSTCTRL_PORF_bm   - Power on Reset flag bit mask
    // RSTCTRL_PORF_bp   - Power on Reset flag bit position
    // RSTCTRL_BORF_bm   - Brown out detector Reset flag bit mask
    // RSTCTRL_BORF_bp   - Brown out detector Reset flag bit position
    // RSTCTRL_EXTRF_bm  - External Reset flag bit mask
    // RSTCTRL_EXTRF_bp  - External Reset flag bit position
    // RSTCTRL_WDRF_bm   - Watch dog Reset flag bit mask
    // RSTCTRL_WDRF_bp   - Watch dog Reset flag bit position
    // RSTCTRL_SWRF_bm   - Software Reset flag bit mask
    // RSTCTRL_SWRF_bp   - Software Reset flag bit position
    // RSTCTRL_UPDIRF_bm - UPDI (Unified Program and Debug Interface) Reset flag bit mask
    // RSTCTRL_UPDIRF_bp - UPDI (Unified Program and Debug Interface) Reset flag bit position

    Serial.println("RSTCTRL.RSTFR\tUPDIRF\tSWRF\tWDRF\tEXTRF\tBORF\tPORF\t");
    Serial.print(RSTCTRL.RSTFR, BIN);
    Serial.print("\t\t");
    Serial.print(resetFlagUPDI());
    Serial.print("\t");
    Serial.print(resetFlagSoftware());
    Serial.print("\t");
    Serial.print(resetFlagWatchdog());
    Serial.print("\t");
    Serial.print(resetFlagExternal());
    Serial.print("\t");
    Serial.print(resetFlagBrownout());
    Serial.print("\t");
    Serial.print(resetFlagPowerOn());
    Serial.println("");
}
