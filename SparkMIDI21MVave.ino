//       .----------------.  .----------------.  .----------------.  .----------------.  .----------------.      .----------------.  .----------------.  .----------------.  .----------------. 
//     | .--------------. || .--------------. || .--------------. || .--------------. || .--------------. |    | .--------------. || .--------------. || .--------------. || .--------------. |
//     | |    _______   | || |   ______     | || |      __      | || |  _______     | || |  ___  ____   | |    | | ____    ____ | || |     _____    | || |  ________    | || |     _____    | |
//     | |   /  ___  |  | || |  |_   __ \   | || |     /  \     | || | |_   __ \    | || | |_  ||_  _|  | |    | ||_   \  /   _|| || |    |_   _|   | || | |_   ___ `.  | || |    |_   _|   | |
//     | |  |  (__ \_|  | || |    | |__) |  | || |    / /\ \    | || |   | |__) |   | || |   | |_/ /    | |    | |  |   \/   |  | || |      | |     | || |   | |   `. \ | || |      | |     | |
//     | |   '.___`-.   | || |    |  ___/   | || |   / ____ \   | || |   |  __ /    | || |   |  __'.    | |    | |  | |\  /| |  | || |      | |     | || |   | |    | | | || |      | |     | |
//     | |  |`\____) |  | || |   _| |_      | || | _/ /    \ \_ | || |  _| |  \ \_  | || |  _| |  \ \_  | |    | | _| |_\/_| |_ | || |     _| |_    | || |  _| |___.' / | || |     _| |_    | |
//     | |  |_______.'  | || |  |_____|     | || ||____|  |____|| || | |____| |___| | || | |____||____| | |    | ||_____||_____|| || |    |_____|   | || | |________.'  | || |    |_____|   | |
//     | |              | || |              | || |              | || |              | || |              | |    | |              | || |              | || |              | || |              | |
//     | '--------------' || '--------------' || '--------------' || '--------------' || '--------------' |    | '--------------' || '--------------' || '--------------' || '--------------' |
//      '----------------'  '----------------'  '----------------'  '----------------'  '----------------'      '----------------'  '----------------'  '----------------'  '----------------' 

//
// Spark MIDI
//
// Open source MIDI control for the Spark 40 amp from Positive Grid
// See    https://github.com/paulhamsh/SparkMIDI for more details
// Also   https://github.com/paulhamsh/Spark     for documentation on the API and the message format to the Spark amp   
//

//Dependencies:
// NimBLE                          >= 1.2.0 (last tested with 1.3.6)
// SSD1306 by Adafruit             >= 2.3.0  
// ESP32 Dev Module board           = 2.0.4
// Heltec ESP32 Series Dev-boards   = 0.0.5
// Adafruit GFX

// For ESP32 Dev Kit update the preferences for board managers to point to: https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
// And load the latest board library - 2.0.2 when built on 23 Jan 2022
// This has the latest BLE code which has setMTU 
// The Heltec WIFI does not have a library supporting setMTU so that is removed by the preprocessor conditional (in SparkComms)


// Select the BLE type 
// -------------------
// if CLASSIC is defined, will compile with BLE stack which allows serial bluetooth
// - if not defined will use NimBLE which has no serial bluetooth
// it matters because Android Spark apps use serial bluetooth
// but BLE doesn't handle Spark disconnection well, whereas NimBLE does

//#define CLASSIC

// Select the BLE devices you need
// -------------------------------
// if BLE_APP_MIDI is defined it will allow a BLE MIDI device to connect (much like the app)
// if BLE_CONTROLLER is defined it will search for a bluetooth controller (like Akai LPD8 Wireless or IK Multimedia iRig Blueboard)

#define BLE_APP_MIDI
#define BLE_CONTROLLER

// Select whether to use external OLED
// -----------------------------------
//#define OLED_ON

// Select the USB host device type
// -------------------------------
// USB_HOST is the generic MAX 2341 host
// USB_TRINKET is a Trinket M0 device programmed as a USB Host

#define USB_HOST
//#define USB_TRINKET

// Select the ESP32 board type
// ---------------------------
// M5STICK, M5CORE2, ESP_DEVKIT, HELTEC_WIFI
// Don't forget to set your board type in Tools -> Board as well!!
//
// Allowed combinations are:
// M5STICK && USB_TRINKET
// M5CORE2 && USB_TRINKET
// M5CORE && USB_HOST  (the USB Host base from M5)
// ESP_DEVKIT && (USB_TRINKET || USB_HOST)  can be either - the only one with a circuit for both so far
// HELTEC_WIFI && USB_HOST
//
// OLED_ON is acceptable with all boards except HELTEC_WIFI

#define ESP_DEVKIT
//#define HELTEC_WIFI
//#define M5CORE
//#define M5CORE2
//#define M5STICK

// Some checks in the pre-processing to ensure no invalid combinations (probably not exhaustive)
// The combinations are only excluded because there has been no sample circuit made with them yet, 
// so GPIO pin assignments are not determined

#if defined HELTEC_WIFI && defined OLED_ON
#error ** Cannot use a Heltec WIFI and have OLED_ON set **
#endif

#if defined USB_TRINKET && defined HELTEC_WIFI
#error ** Cannot use a Heltec WIFI with a Tinket M0 USB Host **
#endif

#if defined USB_HOST && (defined M5STICK || defined M5CORE2)
#error ** Cannot use a M5 board with USB Host **
#endif 

// Board specific #includes

#if defined HELTEC_WIFI
  #include "heltec.h"
#elif defined M5STICK
  #include <M5StickC.h>
#elif defined M5CORE2
  #include <M5Core2.h>
#elif defined M5CORE
  #include <M5Stack.h>
#endif

#include "Spark.h"
#include "Screen.h"
#include "MIDI.h"

//Define some variables
int iCompressor = 1;
int iDrive = 1;
int iModulation = 1;
int iDelay = 1;
int iAmp = 1;
int EQLevel = 0; //range from 0 to 1
int EQ200 = 0; 
int EQ400 = 0;
int EQ800 = 0;
int EQ1600 = 0;
int  EQ3200 = 0;
int EQ32 = 0;

void setup() {
#if defined HELTEC_WIFI
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Enable*/, true /*Serial Enable*/);
#elif defined M5STICK || defined M5CORE2 || defined M5CORE
  M5.begin();
#endif
#if defined M5CORE
  M5.Power.begin();
#endif

  Serial.begin(115200);
  splash_screen();
  setup_midi();
  
  while (!spark_state_tracker_start()) {  // set up data to track Spark and app state, if it fails to find the Spark it will return false
    DEBUG("No Spark found - perhaps sleep?");// think about a deep sleep here if needed
  }
  DEBUG("Spark found and connected - starting");

  setup_screen();
}


void loop() {
  byte mi[3];
  int midi_chan, midi_cmd;
  bool onoff;

#if defined M5STICK || defined M5CORE2 || defined M5CORE
  M5.update();
#endif
  
  show_status();

  if (update_midi(mi)) {
    midi_chan = (mi[0] & 0x0f) + 1;
    midi_cmd = mi[0] & 0xf0;
    onoff = (mi[2] == 127 ? true : false);
    DEBUG("MID[2]: ");DEBUG(mi[2]);
    if (onoff=1){
      DEBUG("On");
      onoff=0;
    }
    else {
      DEBUG("Off");
      onoff=1;
    }
    if (midi_cmd == 0xc0) { 
      switch (mi[1]) {
        case 0:              change_hardware_preset(0);                 break; // M-Vave 00
        case 1:              change_hardware_preset(1);                 break; // M-Vave 01
        case 2:              change_hardware_preset(2);                 break; // M-Vave 02
        case 3:              change_hardware_preset(3);                 break; // M-Vave 03
        case 4:           change_noisegate_toggle();                    break; // M-Vave 04
        case 5:           change_comp_toggle();                         break; // M-Vave 05
        case 6:           change_drive_toggle();                        break; // M-Vave 06    
        case 7:           change_mod_toggle();                          break; // M-Vave 07
        case 8:           change_delay_toggle();                        break; // M-Vave 08
        case 9:           change_reverb_toggle();                       break; // M-Vave 09

/*
00 - hardware preset 1
01 - hardware preset 2
02-  hardware preset 3
03 - hardware preset 4
04 - noisegate toggle
05 - compression toggle
06 - drive toggle
07 - modulation toggle
08 - delay toggle
09 - reverb toggle
0A - Compressor models
0B - Drive models
0C - Amp models
0D - Modulaton models
0E - Delay models
0F - EQ Level
10 - EQ Level 200
11 - EQ Level 400
12 - EQ Level 800
13 - EQ Level 1600
14 - EQ Level 3200
*/



// preset codes


        case 10:    // walk through different compressor models
            change_comp_onoff(true);
            switch (iCompressor) {
              case 1:     DEBUG("iCompressor"); DEBUG(iCompressor);iCompressor = 2;    change_comp_model("LA2AComp");  update_ui();break;
              case 2:     DEBUG("iCompressor"); DEBUG(iCompressor);iCompressor = 3;    change_comp_model("BlueComp");  update_ui();break;
              case 3:     DEBUG("iCompressor"); DEBUG(iCompressor);iCompressor = 4;    change_comp_model("Compressor");  update_ui();break;
              case 4:     DEBUG("iCompressor"); DEBUG(iCompressor);iCompressor = 5;    change_comp_model("BassComp");  update_ui();break;
              case 5:     DEBUG("iCompressor"); DEBUG(iCompressor);iCompressor = 6;    change_comp_model("BBEOpticalComp");  update_ui();break;
              case 6:     DEBUG("iCompressor"); DEBUG(iCompressor);iCompressor = 1;    change_comp_model("JH.Vox846");  update_ui();break;
            }
            break;
        case 11:    // walk through different Drive models
            change_drive_onoff(true);
            switch (iDrive) {
              case 1:     iDrive = 2;    change_drive_model("Booster"); update_ui(); break;
              case 2:     iDrive = 3;    change_drive_model("DistortionTS9");  update_ui();break;
              case 3:     iDrive = 4;    change_drive_model("Overdrive");  update_ui();break;
              case 4:     iDrive = 5;    change_drive_model("Fuzz");  update_ui();break;
              case 5:     iDrive = 6;    change_drive_model("ProCoRat");  update_ui();break;
              case 6:     iDrive = 7;    change_drive_model("BassBigMuff");  update_ui();break;
              case 7:     iDrive = 8;    change_drive_model("GuitarMuff");  update_ui();break;
              case 8:     iDrive = 9;    change_drive_model("MaestroBassmaster");  update_ui();break;
              case 9:     iDrive = 10;    change_drive_model("SABdriver");  update_ui();break;
              case 10:     iDrive = 11;    change_drive_model("SABdriver");  update_ui();break;
              case 11:     iDrive = 12;    change_drive_model("JH.AxisFuzz");  update_ui();break;
              case 12:     iDrive = 13;    change_drive_model("JH.SupaFuzz");  update_ui();break;
              case 13:     iDrive = 14;    change_drive_model("JH.Octavia");  update_ui();break;
              case 14:     iDrive = 1;    change_drive_model("JH.FuzzTone");  update_ui();break;
            }
            break;
        case 12:    // walk through different Amp models
            switch (iAmp) {
              case 1:     iAmp = 2;           change_amp_model("RolandJC120"); break;
              case 2:     iAmp = 3;           change_amp_model("Twin"); break;
              case 3:     iAmp = 4;           change_amp_model("ADClean"); break;
              case 4:     iAmp = 5;           change_amp_model("94MatchDCV2"); break;
              case 5:     iAmp = 6;           change_amp_model("Bassman"); break;
              case 6:     iAmp = 7;           change_amp_model("AC Boost"); break;
              case 7:     iAmp = 8;           change_amp_model("Checkmate"); break;
              case 8:     iAmp = 9;           change_amp_model("TwoStoneSP50"); break;
              case 9:     iAmp = 10;           change_amp_model("Deluxe65"); break;
              case 10:     iAmp = 11;           change_amp_model("Plexi"); break;
              case 11:     iAmp = 12;           change_amp_model("OverDrivenJM45"); break;
              case 12:     iAmp = 13;           change_amp_model("OverDrivenLuxVerb"); break;
              case 13:     iAmp = 14;           change_amp_model("Bogner"); break;
              case 14:     iAmp = 15;           change_amp_model("OrangeAD30"); break;
              case 15:     iAmp = 16;           change_amp_model("AmericanHighGain"); break;
              case 16:     iAmp = 17;           change_amp_model("SLO100"); break;
              case 17:     iAmp = 18;           change_amp_model("YJM100"); break;
              case 18:     iAmp = 19;           change_amp_model("Rectifier"); break;
              case 19:     iAmp = 20;           change_amp_model("EVH"); break;
              case 20:     iAmp = 21;           change_amp_model("SwitchAxeLead"); break;
              case 21:     iAmp = 22;           change_amp_model("Invader"); break;
              case 22:     iAmp = 23;           change_amp_model("BE101"); break;
              case 23:     iAmp = 24;           change_amp_model("Acoustic"); break;
              case 24:     iAmp = 25;           change_amp_model("AcousticAmpV2"); break;
              case 25:     iAmp = 26;           change_amp_model("FatAcousticV2"); break;
              case 26:     iAmp = 27;           change_amp_model("FlatAcoustic"); break;
              case 27:     iAmp = 28;           change_amp_model("GK800"); break;
              case 28:     iAmp = 29;           change_amp_model("Sunny3000"); break;
              case 29:     iAmp = 30;           change_amp_model("W600"); break;
              case 30:     iAmp = 31;           change_amp_model("Hammer500"); break;
              case 31:     iAmp = 32;           change_amp_model("ODS50CN"); break;
              case 32:     iAmp = 33;           change_amp_model("JH.DualShowman"); break;
              case 33:     iAmp = 34;           change_amp_model("JH.Sunn100"); break;
              case 34:     iAmp = 35;           change_amp_model("BluesJrTweed"); break;
              case 35:     iAmp = 36;           change_amp_model("JH.JTM45"); break;
              case 36:     iAmp = 37;           change_amp_model("JH.Bassman50Silver"); break;
              case 37:     iAmp = 38;           change_amp_model("JH.SuperLead100"); break;
              case 38:     iAmp = 39;           change_amp_model("JH.SoundCity100"); break;
              case 39:     iAmp = 1;           change_amp_model("6505Plus"); break;
            }
            break;
        case 13:    // walk through different Modulation models
            change_mod_onoff(true);
            switch (iModulation) {
              case 1:     iModulation = 2;    change_mod_model("Tremolo");  update_ui();break;
              case 2:     iModulation = 3;    change_mod_model("ChorusAnalog");  update_ui();break;
              case 3:     iModulation = 4;    change_mod_model("Flanger");  update_ui();break;
              case 4:     iModulation = 5;    change_mod_model("Phaser");  update_ui();break;
              case 5:     iModulation = 6;    change_mod_model("Vibrato01");  update_ui();break;
              case 6:     iModulation = 7;    change_mod_model("UniVibe");  update_ui();break;
              case 7:     iModulation = 8;    change_mod_model("Cloner");  update_ui();break;
              case 8:     iModulation = 9;    change_mod_model("MiniVibe");  update_ui();break;
              case 9:     iModulation = 10;    change_mod_model("Tremolator");  update_ui();break;
              case 10:     iModulation = 11;    change_mod_model("TremoloSquare");  update_ui();break;
              case 11:     iModulation = 12;    change_mod_model("JH.VoodooVibeJr");  update_ui();break;
              case 12:     iModulation = 13;    change_mod_model("GuitarEQ6");  update_ui();break;
              case 13:     iModulation = 1;    change_mod_model("BassEQ6");  update_ui();break;
            }
            break;
        case 14:    // walk through different Delay models
            change_delay_onoff(true);
            switch (iDelay) {
              case 1:     iDelay = 2;    change_delay_model("DelayMono");  update_ui();break;
              case 2:     iDelay = 3;    change_delay_model("DelayEchoFilt");  update_ui();break;
              case 3:     iDelay = 4;    change_delay_model("VintageDelay");  update_ui();break;
              case 4:     iDelay = 5;    change_delay_model("DelayReverse");  update_ui();break;
              case 5:     iDelay = 6;    change_delay_model("DelayMultiHead");  update_ui();break;
              case 6:     iDelay = 1;    change_delay_model("DelayRe201");  update_ui();break;
            }
            break;

        case 15:  // EQ Level
            change_mod_onoff(true);
            switch (EQLevel) {
              case 0:   EQLevel = 1; change_mod_model("GuitarEQ6");	change_mod_param(0,0.0); break;
              case 1:   EQLevel = 2; change_mod_model("GuitarEQ6");	change_mod_param(0,0.2); break;
              case 2:   EQLevel = 3; change_mod_model("GuitarEQ6");	change_mod_param(0,0.4); break;
              case 3:   EQLevel = 4; change_mod_model("GuitarEQ6");	change_mod_param(0,0.6); break;
              case 4:   EQLevel = 5; change_mod_model("GuitarEQ6");	change_mod_param(0,0.8); break;
              case 5:   EQLevel = 0; change_mod_model("GuitarEQ6");	change_mod_param(0,1.0); break;
            }
            break;
        case 16:  // EQ 200
            change_mod_onoff(true);
            switch (EQ200) {
              case 0:   EQ200 = 1; change_mod_model("GuitarEQ6");	change_mod_param(1,0.0); break;
              case 1:   EQ200 = 2; change_mod_model("GuitarEQ6");	change_mod_param(1,0.2); break;
              case 2:   EQ200 = 3; change_mod_model("GuitarEQ6");	change_mod_param(1,0.4); break;
              case 3:   EQ200 = 4; change_mod_model("GuitarEQ6");	change_mod_param(1,0.6); break;
              case 4:   EQ200 = 5; change_mod_model("GuitarEQ6");	change_mod_param(1,0.8); break;
              case 5:   EQ200 = 0; change_mod_model("GuitarEQ6");	change_mod_param(1,1.0); break;
            }
            break;
        case 17:  // EQ 400
            change_mod_onoff(true);
            switch (EQ400) {
              case 0:   EQ400 = 1; change_mod_model("GuitarEQ6");	change_mod_param(2,0.0); break;
              case 1:   EQ400 = 2; change_mod_model("GuitarEQ6");	change_mod_param(2,0.2); break;
              case 2:   EQ400 = 3; change_mod_model("GuitarEQ6");	change_mod_param(2,0.4); break;
              case 3:   EQ400 = 4; change_mod_model("GuitarEQ6");	change_mod_param(2,0.6); break;
              case 4:   EQ400 = 5; change_mod_model("GuitarEQ6");	change_mod_param(2,0.8); break;
              case 5:   EQ400 = 0; change_mod_model("GuitarEQ6");	change_mod_param(2,1.0); break;
            }
            break;
        case 18:  // EQ 800
            change_mod_onoff(true);
            switch (EQ800) {
              case 0:   EQ800 = 1; change_mod_model("GuitarEQ6");	change_mod_param(3,0.0); break;
              case 1:   EQ800 = 2; change_mod_model("GuitarEQ6");	change_mod_param(3,0.2); break;
              case 2:   EQ800 = 3; change_mod_model("GuitarEQ6");	change_mod_param(3,0.4); break;
              case 3:   EQ800 = 4; change_mod_model("GuitarEQ6");	change_mod_param(3,0.6); break;
              case 4:   EQ800 = 5; change_mod_model("GuitarEQ6");	change_mod_param(3,0.8); break;
              case 5:   EQ800 = 0; change_mod_model("GuitarEQ6");	change_mod_param(0,1.0); break;
            }
            break;
        case 19:  // EQ 1600
            change_mod_onoff(true);
            switch (EQ1600) {
              case 0:   EQ1600 = 1; change_mod_model("GuitarEQ6");	change_mod_param(4,0.0); break;
              case 1:   EQ1600 = 2; change_mod_model("GuitarEQ6");	change_mod_param(4,0.2); break;
              case 2:   EQ1600 = 3; change_mod_model("GuitarEQ6");	change_mod_param(4,0.4); break;
              case 3:   EQ1600 = 4; change_mod_model("GuitarEQ6");	change_mod_param(4,0.6); break;
              case 4:   EQ1600 = 5; change_mod_model("GuitarEQ6");	change_mod_param(4,0.8); break;
              case 5:   EQ1600 = 0; change_mod_model("GuitarEQ6");	change_mod_param(4,1.0); break;
            }
            break;
        case 20:  // EQ 3200
            change_mod_onoff(true);
            switch (EQ3200) {
              case 0:   EQ3200 = 1; change_mod_model("GuitarEQ6");	change_mod_param(5,0.0); break;
              case 1:   EQ3200 = 2; change_mod_model("GuitarEQ6");	change_mod_param(5,0.2); break;
              case 2:   EQ3200 = 3; change_mod_model("GuitarEQ6");	change_mod_param(5,0.4); break;
              case 3:   EQ3200 = 4; change_mod_model("GuitarEQ6");	change_mod_param(5,0.6); break;
              case 4:   EQ3200 = 5; change_mod_model("GuitarEQ6");	change_mod_param(5,0.8); break;
              case 5:   EQ3200 = 0; change_mod_model("GuitarEQ6");	change_mod_param(5,1.0); break;
            }
            break;
        case 21:  // EQ 3200
            change_mod_onoff(true);
            switch (EQ32) {
              case 0:   EQ32 = 1; change_mod_model("GuitarEQ6");	change_mod_param(6,0.0); break;
              case 1:   EQ32 = 2; change_mod_model("GuitarEQ6");	change_mod_param(6,0.2); break;
              case 2:   EQ32 = 3; change_mod_model("GuitarEQ6");	change_mod_param(6,0.4); break;
              case 3:   EQ32 = 4; change_mod_model("GuitarEQ6");	change_mod_param(6,0.6); break;
              case 4:   EQ32 = 5; change_mod_model("GuitarEQ6");	change_mod_param(6,0.8); break;
              case 5:   EQ32 = 0; change_mod_model("GuitarEQ6");	change_mod_param(6,1.0); break;
            }
            break;
        case 124:         tuner_on_off(1);                              break; // M Vave Bank 8      
        case 125:         tuner_on_off(0);                              break; // M Vave Bank 8
      }
    }

    if (midi_cmd == 0xb0 && midi_chan == 4) {     
      switch (mi[1]) {
        case 1:              change_amp_param(AMP_GAIN, mi[2]/127.0);   break; // MIDI Commander CUS2 EXP l          
        case 2:              change_amp_param(AMP_MASTER, mi[2]/127.0); break; // MIDI Commander CUS2 EXP 2  
        case 3:              change_drive_toggle();                     break; // MIDI Commander CUS2 1  
        case 9:              change_mod_toggle();                       break; // MIDI Commander CUS2 2       
        case 14:             change_delay_toggle();                     break; // MIDI Commander CUS2 3
        case 29:             change_reverb_toggle();                    break; // MIDI Commander CUS2 4
        case 22:             change_hardware_preset(0);                 break; // MIDI Commander CUS2 A
        case 23:             change_hardware_preset(1);                 break; // MIDI Commander CUS2 B
        case 24:             change_hardware_preset(2);                 break; // MIDI Commander CUS2 C
        case 105:            change_hardware_preset(3);                 break; // MIDI Commander CUS2 D
        }
    }
    
    if (midi_cmd == 0xb0) {     
      switch (mi[1]) {
        case 4:              change_amp_param(AMP_GAIN, mi[2]/127.0);   break; // MIDI Commander BIFX EXP 1
        case 7:              change_amp_param(AMP_MASTER, mi[2]/127.0); break; // MIDI Commander BIFX EXP 2  
        }
    }

    if (midi_cmd == 0xb0) 
      switch (midi_chan) {
        case 1:
          switch (mi[1]) {
            case 80:             change_comp_onoff(onoff);               break; // MIDI Commander BIFX 1
            case 81:             change_drive_onoff(onoff);              break; // MIDI Commander BIFX 2      
            case 82:             change_mod_onoff(onoff);                break; // MIDI Commander BIFX 3     
            case 83:             change_delay_onoff(onoff);              break; // MIDI Commander BIFX 4
          };
          break;
        case 2:
          switch (mi[1]) {
            case 80:             change_comp_onoff(onoff);               break; // MIDI Commander BIFX 1
            case 81:             change_drive_onoff(onoff);              break; // MIDI Commander BIFX 2      
            case 82:             change_mod_onoff(onoff);                break; // MIDI Commander BIFX 3     
            case 83:             change_reverb_onoff(onoff);             break; // MIDI Commander BIFX 4
          };
          break;
        case 3:
          switch (mi[1]) {
            case 80:             change_comp_onoff(onoff);               break; // MIDI Commander BIFX 1
            case 81:             change_drive_onoff(onoff);              break; // MIDI Commander BIFX 2      
            case 82:             change_delay_onoff(onoff);              break; // MIDI Commander BIFX 3     
            case 83:             change_reverb_onoff(onoff);             break; // MIDI Commander BIFX 4
          };
          break;
        case 4:
          switch (mi[1]) {
            case 80:             change_comp_onoff(onoff);               break; // MIDI Commander BIFX 1
            case 81:             change_mod_onoff(onoff);                break; // MIDI Commander BIFX 2      
            case 82:             change_delay_onoff(onoff);              break; // MIDI Commander BIFX 3     
            case 83:             change_reverb_onoff(onoff);             break; // MIDI Commander BIFX 4
          };
          break;
        case 5:
          switch (mi[1]) {
            case 80:             change_drive_onoff(onoff);              break; // MIDI Commander BIFX 1      
            case 81:             change_mod_onoff(onoff);                break; // MIDI Commander BIFX 2     
            case 82:             change_delay_onoff(onoff);              break; // MIDI Commander BIFX 3
            case 83:             change_reverb_onoff(onoff);             break; // MIDI Commander BIFX 4
          };
          break;
        default:
          switch (mi[1]) {
            case 80:             change_drive_onoff(onoff);              break; // MIDI Commander BIFX 1      
            case 81:             change_mod_onoff(onoff);                break; // MIDI Commander BIFX 2     
            case 82:             change_delay_onoff(onoff);              break; // MIDI Commander BIFX 3
            case 83:             change_reverb_onoff(onoff);             break; // MIDI Commander BIFX 4
          };
      }
  }

  if (update_spark_state()) {
  }
}
