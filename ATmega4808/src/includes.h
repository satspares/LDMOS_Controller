#ifndef INCLUDES_H
#define INCLUDES_H

/* ======= User Settings ======== */
// Also see main.c at the top

/* ======= First page text when new each 30 chars max======== */
createSafeString(MainText1,30,"-- Touch to Change --");
createSafeString(MainText2,30," --- YOUR CALL --- ")

/* ======= SWR Progress Bar Max Display eg. 200=2.00 swr 300=3.00 swr or other swr value * 100 ======== */
const uint16_t swrRangeMax = 250;  // 250 swr graph max is swr 2.50
const uint16_t powerRangeMax = 800; // Max Power graph range eg. 600=600 watts

/* ======= TX delay before tx needed for slow relays ======== */
const uint16_t TX_DELAY = 10;  // maybe 35 for slow relays

// swr display adjustment control is 1-1000 this multiples the steps
// to add/minus the reflected power - probably no need to touch depends on your tandem match
const uint8_t SWR2CALCMAJOR = 1;
const uint8_t SWR1CALCMAJOR = 1;

const uint16_t ICALCMAJOR = 80;        // I step change in I set
const uint16_t DRIVECALCMAJOR = 280;   // used in the drive calc (lower = higher)
                                       // depends on how you obtain drive power     
const uint16_t MAXAMPPOWERCALC = 300;   // used in the auto power calculations keep at 600 or less?
const uint16_t PEAKREFRESH = 1400;        // peak hold refresk in ms

//Try to correct the power ratio between fwd/ref power
// as tandem matches dont appear linear across power levels
// my tandem high power devices dont like to work well at power levels under about 200w
// used in ref1Voltage and ref2Voltage
// correct would be Low = 0 - High = 4300
// more than 4300 (adc 4.3 volts) we are adding voltage to the reflected/fwd power

//test values


const uint16_t SWR1RefMapLow = 0;
const uint16_t SWR1RefMapHigh = 4300;
const uint16_t SWR1FwdMapLow = 0;
const uint16_t SWR1FwdMapHigh = 4300;

#ifdef MY_ALL_BAND_AMP
const uint16_t SWR2RefMapLow = 50;      // my test values
const uint16_t SWR2RefMapHigh = 2800;
const uint16_t SWR2FwdMapLow = 400;
const uint16_t SWR2FwdMapHigh = 3700;
#else
const uint16_t SWR2FwdMapLow = 0;
const uint16_t SWR2FwdMapHigh = 4300;
const uint16_t SWR2RefMapLow = 0;
const uint16_t SWR2RefMapHigh = 4300;
#endif


const uint16_t SWRColorChange = 16;         //change swr bar color on high swr (swr * 10 eg 16 = 1.6swr )
const uint16_t SWROriginalColor = 0xFC00;   // orange
const uint16_t SWRHighSWRColor = 0xF800;    //red

const uint16_t Soft_Start_time = 1000;       // for my current trip board in ms 
// The diode loss is after the resistor split on the adc inputs
// if the split is eg. 1:4  the diode loss needs to be (real diode loss / 4)
// some experimentation is required a incorrect value will make power and swr display not linear
const float diodeLossMV = 140;  // diode loss in volts
// if drive detect device is using a diode eg 400 milivolts loss / input resistor split
const uint16_t diodeLossMVdrive = 130;

// user temperature settings
#define SENSOR_COUNT (1)                //ds18b20 one sensor or multiple upto 4 not used yet!
// sensor our names
#if SENSOR_COUNT > 1
String sensor0 = "Sensor 0";
String sensor1 = "Sensor 1";
String sensor2 = "Sensor 2";
String sensor3 = "Sensor 3";
#endif
const uint8_t AUXFANON = 34;   // test
const uint8_t AUXFANOFF = 32;
// user beep settings in milli seconds
#define BEEP_TINY 20
#define BEEP_YES 50
#define BEEP_CANCEL 300
#define BEEP_ERROR 700
// use beep or sound false for sound true for beep
// need cfg file setting to change sound to beep.
// sound will play on displays with beeper but not loud!
const bool usebeep = true;
// spurious swr interrupts relay hot change etc. greater than this fires
const uint8_t interrupt_Count_trigger = 3;

/* ======= User SettingsEND ======== */

/* ======= Colors or any 16bit color for below text used by keyboard ======== */
const uint16_t COLOR_WHITE = 0xFFFF;
const uint16_t COLOR_RED = 0xF800;
const uint16_t COLOR_GREEN = 0x7E0;
const uint16_t COLOR_BLUE = 0x001F;
const uint16_t COLOR_YELLOW = 0xFFE0;
const uint16_t COLOR_ORANGE = 0xFC08;
const uint16_t COLOR_LIGHT_BLUE = 0x87FF;

/* ======= EEPROM Settings ======== */
#define EEPROM_SIZE 230   // less than new on address
#define eeprom_band 0
#define eeprom_antenna 1
#define eeprom_auto 2
#define eeprom_drive 3
#define eeprom_volt 4
#define eeprom_current 5
#define eeprom_power_calc_address 10
#define eeprom_text1_address 135
#define eeprom_text2_address 170
#define eeprom_intSettings_address 210
#define eeprom_new_on_address 240
#define EEPROMROW 8   // we can have 8 bands max index eeprom 0-7
#define DRIVEOFFSET 32 // drive place in calc array



/* ======= Ticker Delays ======== */
const uint16_t peakHoldResetDelay = 500;  //should be about 500ms ?
const uint16_t tempIDTickerDelay = 300; //update display temp etc.
const uint16_t keepingHouseTickerDelay = 2000; // general stuff
const uint16_t powerSwrTickerDelay = 100; // should be 50ms ?

/* ======= Temperature Volt Globals ======== */

unsigned long OW_PIN = A9;
float DS18B20Temperature(const DSTherm::Scratchpad &scrpd);
static bool returnSensorId(const OneWireNg::Id &id, uint8_t sensorCount);
typedef struct {
    uint8_t count;
    String sensor_our_name;
    String sensor_real_name;
    String mac_address;
    float sensor_value;
}sensor_type;
// array of found 18b20 sensors
sensor_type sensor_array[4];


/* ======= Resistor Calcs on power/rf input 1 an 2 ======== */
const float Res50v_1 = 100000.00; // Set R31 of voltage devider
const float Res50v_2 = 5100.00; // Set R14 of voltage devider

const float ResRF_1 = 1500; // not used at the moment
const float ResRF_2 = 680;

/* ======= Global Vars ======== */
uint16_t debugTest = 0;          // looking for problems
uint32_t debugTestLong = 0;
createSafeString(displayTextSave,30); // used in dwin_callback
createSafeString(displayText,30);
bool tx_status = false;             // true when tx on
uint8_t band_select;            // which band is active
bool band_auto = false;             // band man auto
bool which_antenna = false;         // Antenna one
bool which_swr = false;             // which swr result to display
bool band_auto_touched = false; // detect band auto to manual change - used = bcd band select
uint8_t swrOffset = 0;          // offset into eeprom for swr1/swr2
bool temp_id_reset = false;     // ticker reset for temp and ID
bool peak_hold_reset = false;   // ticker reset for power peak hold etc
bool power_swr_reset = false;   // power swr ticker reset
//bool peak_hold_clear = false;
// error globaal vars
bool error_i_status = false;
bool error_swr_status = false;
bool error_po_status = false;
bool error_temp_status = false;
bool error_vo_status = false;
bool error_50v_status = false;
bool error_od_status = false;
bool error_od_status_stop = false; // overdrive stop tx and need reset
bool swr_soft_trip = false;
bool setting_power_calc = false;   // we are in the power calc page
bool setting_swr_calc = false;      // we are in the swr calc page
bool setting_volt_calc = false;    // we are in the volt calc page
bool setting_current_calc = false; // we are in the I calc page
bool bias_current_test = false;    // not used
volatile bool intBChange = false;  // mvp
volatile bool pttIntActive = false;
volatile bool swr1IntActiveHigh = false;
volatile bool swr2IntActiveHigh = false;
volatile uint16_t swr2IntCount = 0;
volatile uint16_t swr1IntCount = 0;
uint16_t glo_power_fwd = 0;        // power_fwd copy
uint16_t glo_drive_power = 0;      // drive power from eeprom
uint16_t glo_volt_setting = 0;      // volt calc read from eeprom
uint16_t glo_current_setting = 0;   // current calc read from eeprom
float glo_swr_display = 0;
float glo_volt_display = 0;
float fwdPower_max; 
float refPower_max;


#ifdef DISPLAY160M
const uint8_t band_display_offset = 7;   //display second band icons 160m-6m 7 icons per. display
#else
const uint8_t band_display_offset = 0;   // display first band icons 80m-4m
#endif
const float adcRef = (4.3 / 1023);
const uint8_t ADC_SAMPLES = 4;
// Power Calc initial values
uint16_t powerCalcArray[] = { 50, 50, 50, 50, 50, 50, 50, 0xff,              // power antenna
    50, 50, 50, 50, 50, 50, 50, 0xff,               // power LPF
    500, 500, 500, 500, 500, 500, 500, 0xff,        // swr calc antenna
    500, 500, 500, 500, 500, 500, 500, 0xff,        // swr calc LPF
    50, 50, 50, 50, 50, 50, 50, 0xff, 0xff, 0xff }; // drive offset
   

// intSettingsArray[0] is the setting to set power to set to
//  200 watts when new
#define POWERSETPOINT 0     // 200w locations for power set value etc.
#define TEMPSETPOINT 1      // 45c
#define VOLTSETPOINT 2      // 55v        
#define DRIVESETPOINT 3     // 8v
#define CURRENTSETPOINT 4   // 20a
#define TEXT1COLOR 5
#define TEXT2COLOR 6
uint16_t intSettingsArray[] =  { 200, 45, 55, 8, 20, COLOR_WHITE, COLOR_WHITE };

/* ======= Display Settings ======== */
#define DGUS_BAUD 115200
const uint32_t LCDBackLightNormal = ((unsigned long)0x643A03E8); //Normal Brigtness
const uint32_t LCDBackLightLow = ((unsigned long)0x641503E8);    //0x641503E8)
const uint16_t LCDBackLightAddress = 0x0082;
// ICON NUMBERS
#define GREEN_ICON 3
#define RED_ICON 4
#define BAND_MANUAL 23
#define BAND_AUTO 24
#define ANTENNA_ONE 21
#define ANTENNA_TWO 22
#define SWR_DISPLAY 25
#define LPF_DISPLAY 26
#define UPC_ICON 60 // upper lower case
#define LOC_ICON 61 //

// Page numbers
const uint8_t startPage = 0;
const uint8_t powerSetPage = 1;
const uint8_t driveSetPage = 2;
const uint8_t voltSetPage = 3;
const uint8_t swrSetPage = 4;
const uint8_t currentSetPage = 5;
const uint8_t tripSetPage = 6;
const uint8_t txPage = 10;
const uint8_t keyboardPage = 15;

// DWIN Display memory locations
// error icons
const uint16_t swr_error = 0x1000;
const uint16_t swr2_error = 0x1001;
const uint16_t current_error = 0x1002;
const uint16_t volt_error = 0x1003;
const uint16_t temp_error = 0x1004;
const uint16_t overdrive_error = 0x1005;
const uint16_t external_trip_error = 0x1006;

// Page 0 page switch touch
const uint16_t calc_power_touch = 0x1501;       // turn to our page one
const uint16_t calc_drive_touch = 0x1502;       //select drive calc page2
const uint16_t calc_volt_touch = 0x1503;        //Volt Calc page3
const uint16_t calc_swr_touch = 0x1504;        //Temp Calc page4
const uint16_t calc_current_touch = 0x1505;     //Current_Calc page5
const uint16_t touch_reset = 0x1010;            // dxworld board reset
const uint16_t band_switch = 0x1012;           // returns 31-37 dec
const uint16_t band_manual_switch = 0x1021;     // returns 0x01 band auto
const uint16_t antenna_switch = 0x1020;         // returns 0x01
const uint16_t swr_meter_change = 0x1022;       // returns 0x01
const uint16_t trip_set_touch = 0x1023;         // returns 0x01
const uint16_t top_bottom_text_touch = 0x102A;


// Page 0 Var Icons
const uint16_t band_manual_display = 0x1031;    // 23-24 to switch image
const uint16_t antenna_switch_display = 0x1032; // 21-22 to switch image
const uint16_t swr_meter_switch_display = 0x1033; //25-26 to switch image
const uint16_t band_display = 0x1030;           // 31-44 DECIMAL to switch image

// Page 0 start page display controls
const uint16_t power_graph = 0x3000; // 0-powerRangeMax
const uint16_t power_graph_sp = 0x6800;
const uint16_t swr_graph = 0x3010;
const uint16_t swr_graph_sp = 0x6900;
const uint16_t swr_digits = 0x2100;
const uint16_t volt_display = 0x2500;
const uint16_t power_display = 0x2400;
const uint16_t current_display = 0x2800;
const uint16_t rev_display = 0x2700;
const uint16_t temp_display = 0x2600;
const uint16_t drive_display = 0x2810;
const uint16_t main_text1 = 0x5100;
const uint16_t main_text1_sp = 0x5200;
const uint16_t main_text2 = 0x5150;
const uint16_t main_text2_sp = 0x5250;



// ************** Page 1 power calc page ***********
// Display controls
const uint16_t startPage_band_text = 0x5030;        //String 21 dec
const uint16_t startPage_swr_text = 0x5060;         //String 21 dec also used on page 4 swr calc
const uint16_t power_display_startPage = 0x2402;    // power in watt display
const uint16_t display_power_set_point = 0x2404; // power set point

// Page 1 touch controls
const uint16_t save_power_calc = 0x1024;        // save array to EEPROM page 1
const uint16_t test_power_calc = 0x1026;        // test power display page 1
const uint16_t startPage_cancel = 0x1027;
// Page 2 drive calc page
// Display Controls
const uint16_t power_eeprom_display2 = 0x2420;  // page 2 drive
const uint16_t power_display_page2 = 0x2406;  // float 4 bytes
// Page 2 touch controls
const uint16_t save_drive_calc = 0x1029;        // save drive calc page 2
const uint16_t test_drive_calc = 0x101A;         // test drive button page2
const uint16_t page2_cancel = 0x101B;

//********* Page 4 swr set page ***********
// Display Controls
const uint16_t swr_display_glo_swr = 0x4500; // float 4 bytes
const uint16_t swr_calc_display = 0x4510;
const uint16_t fwd_millivolts = 0x4511;
const uint16_t ref_millivolts = 0x4512;
// Touch controls
const uint16_t swr_calc_save_control = 0x4000;
const uint16_t swr_calc_test_control = 0x4001;
const uint16_t swr_calc_cancel_control = 0x4002;

// ******** Page 3 Voltage adjust
// Touch controls
const uint16_t volt_calc_save_control = 0x4010;
const uint16_t volt_calc_test_control = 0x4011;
const uint16_t volt_calc_cancel_control = 0x4012;
// Display Controls
const uint16_t volt_calc_display = 0x4530;
const uint16_t volt_calc_volt_display = 0x4540;  //float 4 bytes

// ******** Page 5 current adjust
//Touch Controls
const uint16_t current_calc_save_control = 0x4020;
const uint16_t current_calc_test_control = 0x4021;
const uint16_t current_calc_cancel_control = 0x4022;
// Display Controls
const uint16_t current_calc_display = 0x4550;
const uint16_t current_calc_current_display = 0x4560;  //float 4 bytes
// ******** Page 6 Trip set page
//Touch Controls
const uint16_t trip_save_button = 0x5500;
const uint16_t global_reset = 0x5509;

// Display Controls
const uint16_t trip_temp_display = 0x5501;
const uint16_t trip_volt_display = 0x5502;
const uint16_t trip_drive_display = 0x5503;
const uint16_t trip_current_display = 0x5504;

// ******* Page 15 Keyboard 
// Touch Controls
const uint16_t keyboard_keys = 0x5520;
//Control keys
const uint8_t escape_key = 0xF0;
const uint8_t backspace_key = 0xF1;
const uint8_t ok_key = 0xF2;
const uint8_t clear_key = 0xF3;
const uint16_t orange_key = 0xE0;
const uint16_t red_key = 0xE1;
const uint16_t green_key = 0xE2;
const uint16_t blue_key = 0xE3;
const uint16_t yellow_key = 0xE4;
const uint16_t liteblue_key = 0xE5;
const uint16_t case_key = 0xE6;
// display controls
const uint16_t tmpTextDisplay = 0x5600;
const uint16_t tmpTextDisplaySP = 0x5700;
const uint16_t case_image = 0x5521;



/* ======= Band Settings ======== */
//
#define FIRST_ICON 31     // first icon 31.jpg
#ifndef DISPLAY160M
    #define band80Mtr_Selected 31
    #define band60_40Mtr_Selected 32
    #define band30_20Mtr_Selected 33
    #define band17_15Mtr_Selected 34
    #define band12_10Mtr_Selected 35
    #define band6Mtr_Selected 36
    #define band4Mtr_Selected 37
    #define band80Mtr_Selected_Icon 31
    #define band60_40Mtr_Selected_Icon 32
    #define band30_20Mtr_Selected_Icon 33
    #define band17_15Mtr_Selected_Icon 34
    #define band12_10Mtr_Selected_Icon 35
    #define band6Mtr_Selected_Icon 36
//#define band160Mtr_Selected_Icon 44
#else
    #define band160Mtr_Selected 0x1f       //160 alt. display icons
    #define band80Mtr_Selected 32
    #define band60_40Mtr_Selected 33
    #define band30_20Mtr_Selected 34
    #define band17_15Mtr_Selected 35
    #define band12_10Mtr_Selected 36
    #define band6Mtr_Selected 37

    #define band80Mtr_Selected_Icon 38
    #define band60_40Mtr_Selected_Icon 39
    #define band30_20Mtr_Selected_Icon 40
    #define band17_15Mtr_Selected_Icon 41
    #define band12_10Mtr_Selected_Icon 42
    #define band6Mtr_Selected_Icon 43
    #define band160Mtr_Selected_Icon 44
#endif


//offset in band array for   bands 0=80mtr
uint8_t calc_array_swr_offset;
uint8_t calc_array_lpf_offset;
uint8_t calc_array_drive_offset;

// Power Calc Array Offsets
#ifndef DISPLAY160M
const uint8_t offsetSWR80M = 0; const uint8_t offsetLPF80M = 7;
const uint8_t offsetSWR60_40M = 1; const uint8_t offsetLPF60_40M = 8;
const uint8_t offsetSWR30_20M = 2; const uint8_t offsetLPF30_20M = 9;
const uint8_t offsetSWR17_15M = 3; const uint8_t offsetLPF17_15M = 10;
const uint8_t offsetSWR12_10M = 4; const uint8_t offsetLPF12_10M = 11;
const uint8_t offsetSWR6M = 5; const uint8_t offsetLPF6M = 12;
const uint8_t offsetSWR4M = 6; const uint8_t offsetLPF4M = 13;
//const uint8_t offsetSWR160M = 6; const uint8_t offsetLPF4M = 13;

#else
const uint8_t offsetSWR160M = 0; const uint8_t offsetLPF4M = 7;
const uint8_t offsetSWR80M = 1; const uint8_t offsetLPF80M = 8;
const uint8_t offsetSWR60_40M = 2; const uint8_t offsetLPF60_40M = 9;
const uint8_t offsetSWR30_20M = 3; const uint8_t offsetLPF30_20M = 10;
const uint8_t offsetSWR17_15M = 4; const uint8_t offsetLPF17_15M = 11;
const uint8_t offsetSWR12_10M = 5; const uint8_t offsetLPF12_10M = 12;
const uint8_t offsetSWR6M = 6; const uint8_t offsetLPF6M = 13;

#endif

/* ======= Strings ======== */

String string160M = "160M";
String string80M = "80M";
String string60_40M = "60-40M";
String string30_20M = "30-20M";
String string17_15M = "17-15M";
String string12_10M = "12-10M";
String string6M = "6M";
String string4M = "4M";
String stringSWR2 = "SWR2"; // or LPF etc.
String stringSWR1 = "SWR1";
String stringSingleBand = "  2 MTR";


/* ======= MCP23017 Settings ======== */
//MCP23017 port A outputs
const uint8_t band160Mtr_switch =   0b10000000;
const uint8_t band80Mtr_switch =    0b00000010;
const uint8_t band60_40Mtr_switch = 0b00000100;
const uint8_t band30_20Mtr_switch = 0b00001000;
const uint8_t band17_15Mtr_switch = 0b00010000;
const uint8_t band12_10Mtr_switch = 0b00100000;
const uint8_t band6Mtr_switch =     0b01000000;
const uint8_t band4Mtr_switch =     0b00000001;

// MCP23017 port B pins
const uint8_t A600BIAS = 8;         //< pin GPB0 (1) of the MCP23017. via. resistor split
const uint8_t AUX_FAN_POWER = 9;    //< pin GPB1 (2) of the MCP23017. Transistor Base
const uint8_t FAN_POWER = 10;       //< pin GPB2 (3) of the MCP23017.
const uint8_t EN_I_BOARD = 11;      //< pin GPB3 (4) of the MCP23017.
const uint8_t BIAS12V = 12;         //< pin GPB4 (5) of the MCP23017.
const uint8_t SWR_TRIP = 13;        //< pin GPB5 (6) of the MCP23017.
const uint8_t ANTSWITCH = 14;       //< pin GPB6 (7) of the MCP23017.
const uint8_t GPB7 = 15;            //< pin GPB7 (8) of the MCP23017.



/* ======= Other defines pins etc ======== */
#define FANPWM      A8
#define TEMPSENSOR  A9
#define OPTOUT1     13  // also onboard led use as output
#define VIN50V      A11
#define SWR1        A10
#define REF1        A3
#define SWR2        A7
#define REF2        A6
#define ID_IN       A5
#define RFSENSE     A4
#define PTT         12 // new
#define LED_I       9  // DX-World errors
#define LED_PO      10 
#define LED_SWR     15 
#define SWR1_INT    A2  // new
#define SWR2_INT    A0 //new
#define AUX1_IN_OUT 7  // ltv-827
#define AUX2_IN_OUT 8  // ltv-827
#define SPARE1      11
#define TRIP50v     26  // also cpu reset see platformio.ini
#define INTB 6  //MCP Int
//#define A600_BIAS_ON 13


/* ======= Sound file id's ======== */
#define DING 11
#define BEEPERROR 5
#define BEEP 3
#define ALARM 10
#define STARTING4 9
#define STARTING3 8
#define STARTING2 7
#define STARTING1 6
#define RETURN 4
#define NO 1
#define YES 2

/* ======= function forward references ======== */
//

void select_band(uint8_t lastByteRX);
//void setGlobalVars();
void mcp23017_setup();
String checkHex(byte currentNo);
static int isNegative(float negTest);
void keepingHouse();
void tempIDTickerReset();
void powerSWRTickerReset();
void peakHoldReset();
void readEEPROM();
void setupDisplay();
float readTemp(bool starting);
void setupPins();
float read_volt();
float readI();
void fanspeed(float temperature);
void onHMIEvent(String address, int lastuint8_t, String message, String response);
void all_band_relays_off(); void band160_4Mtr_Active(); void band80Mtr_Active();
void band60_40Mtr_Active(); void band30_20Mtr_Active(); void band17_15Mtr_Active();
void band12_10Mtr_Active(); void band6Mtr_Active();
float fwd1Voltage(); float ref1Voltage(); float fwd2Voltage(); float ref2Voltage();
float driveWatts();
void calcPowerandDisplay();
void error_temp(float temperature);
void error_swr();
void error_i(); // dxworld
void errorI(float I);
void error_vo(float V);
void error_odrive();
void eeprom_write_power_calc_values();
void eeprom_read_power_calc_values();
void eeprom_write_intSetting_values();
void eeprom_read_intSetting_values();
void enable50v();
void fanpower(bool enable);
void auxFanpower(bool enable);
template<class T> int EEPROM_writeAnything(int ee, const T &value);
template<class T> int EEPROM_readAnything(int ee, T &value);

void configureInterrupts();   // mcp23017
void PTTservice();
void mcp23017ChangeOnPortB();
String readVPText(uint16_t vpAddress,byte noWords);
void bias_on();  // 12v bias voltage dxworld etc.
void bias_off();
void auto_band();
void setup_dallas_sensors();
void setScreenRotate(byte angle);
float correctRefVoltage(float refVoltage, float fwdVoltage, uint8_t swr_calc_major);
String bandStrings(uint16_t arrayOffset);
String swrStrings(bool whichSWR);
void swr1IntRising();
void swr2IntRising();
void clearSWRInterruptFlags();
void safeStringToEEPROM(uint16_t EEPROM_address, uint16_t  safeStringSize, SafeString& strin);
void safeStringFromEEPROM(uint16_t EEPROM_address,uint16_t  safeStringMaxSize, SafeString& strin);
bool inRange(float val, float minimum, float maximum);

void (*resetFunc)(void) = 0; //declare reset function at address 0



template<class X, class M, class N, class O, class Q>
X map_Generic(X x, M in_min, N in_max, O out_min, Q out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


/* ======= Global Instances ======== */
//
MCP23017 mcp23017 = MCP23017(0x20);
Ewma refFilter1(0.5);
Ewma refFilter2(0.5);
//Ewma fwdFilter(0.4);
Ewma driveFilter(0.1); // for oDrive
Ewma voltFilter(0.5);
//Ewma swrFilter(0.4);
DWIN hmi(Serial2, DGUS_BAUD, false);     // the false do not init the port yet
#ifndef DXWORLD_I
    ACS712  ACS(ID_IN, 5.0, 1023, 100);
#endif


Ticker peakHoldTicker(peakHoldReset, peakHoldResetDelay);  
Ticker houseKeeping(keepingHouse, keepingHouseTickerDelay);
//Update temperature and ID every 1500ms
Ticker temperatureIDTicker(tempIDTickerReset, tempIDTickerDelay);
// Update display with Power/SWR/Ref
Ticker sendPowerSwrRefTicker(powerSWRTickerReset, powerSwrTickerDelay);




#ifdef AUTO_BAND
ADS1015_WE adc = ADS1015_WE(0x48);
const float adc_level = 1.50f;   // voltage on band switch to count as 1 when greater
float readChannel(ADS1015_MUX channel);
#endif // AUTO_BAND

//#endif

#endif
// endif include guard
