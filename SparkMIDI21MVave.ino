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
        case 0:              change_hardware_preset(0);                 break; // MIDI Commander BIFX A
        case 1:              change_hardware_preset(1);                 break; // MIDI Commander BIFX B
        case 2:              change_hardware_preset(2);                 break; // MIDI Commander BIFX C
        case 3:              change_hardware_preset(3);                 break; // MIDI Commander BIFX D

        case 4:           change_comp_toggle();                         break; // M Vave Bank 8
        case 5:           change_drive_toggle();                        break; // M Vave Bank 8    
        case 6:           change_reverb_toggle();                       break; // M Vave Bank 8
        case 7:           change_delay_toggle();                        break; // M Vave Bank 8
        case 8:           change_mod_toggle();                          break; // M Vave Bank 8
        case 9:           change_noisegate_toggle();                    break; // M Vave Bank 8
        case 10:          change_tc_preset(true,"Compressor",false,"DistortionTS9",true,"Rectifier","Flanger",false,"DelayMono",false,true);                            break;               //Metallica        
        case 11:          change_tc_preset(false,"LA2AComp",true,"Booster",false,"ADClean","Flanger",true,"VintageDelay",true,true);                            break;               // Dire Straits
        case 12:          change_tc_preset(true,"LA2AComp",true,"Booster",false,"EVH","ChorusAnalog",true,"DelayMono",true,true);                            break;               //Eddie Van Halen
        case 13:          change_tc_preset(true,"Compressor",false,"Overdrive",true,"TwoStoneSP50","Flanger",false,"DelayMono",false,true);                            break;               //SRV
        case 14:          change_tc_preset(true,"BlueComp",true,"Booster",false,"ADClean","Flanger",true,"DelayMono",true,true);                            break;               //Nothing else matters
        case 15:          change_tc_preset(true,"Compressor",true,"Booster",true,"Twin","ChorusAnalog",true,"DelayRe201",true,true);                            break;               // Purple Rain
        case 16:          change_tc_preset(true,"BlueComp",true,"Booster",true,"Twin","ChorusAnalog",true,"DelayRe201",true,true);                            break;               //Clean Rhythm Delay
        case 17:          change_tc_preset(true,"BlueComp",true,"Booster",false,"ADClean","ChorusAnalog",true,"DelayMono",false,true);                            break;               //Modern Vintage Clean
        case 18:          change_tc_preset(false,"Compressor",true,"Booster",false,"ADClean","Cloner",true,"DelayMono",false,true);                            break;               //Come As You Are - Nirvana
        case 19:          change_tc_preset(true,"LA2AComp",true,"Booster",false,"FatAcousticV2","Flanger",true,"DelayMono",false,true);                            break;               //Grunge Acoustic
        case 20:          change_tc_preset(true,"Compressor",false,"Booster",true,"Bassman","Flanger",false,"DelayMono",false,true);                            break;               //Blues Legend
        case 21:          change_tc_preset(false,"LA2AComp",true,"Booster",true,"ADClean","Flanger",true,"VintageDelay",true,true);                            break;               // Dire Straits nice mid rock tone
        case 22:          change_tc_preset(true,"Compressor",false,"DistortionTS9",true,"Plexi","Flanger",false,"VintageDelay",true,true);                            break;               //GnR Lead
        case 23:          change_tc_preset(true,"Compressor",false,"Booster",true,"TwoStoneSP50","Flanger",false,"DelayMono",false,true);                            break;               //SRV II
        case 24:          change_tc_preset(true,"BlueComp",true,"DistortionTS9",true,"SwitchAxeLead","ChorusAnalog",true,"DelayMono",true,true);                            break;               //Motley Crue 80’s tone
        case 25:          change_tc_preset(true,"Compressor",false,"DistortionTS9",true,"AC Boost","Flanger",false,"DelayMono",false,true);                            break;               //Blues Strat
        case 26:          change_tc_preset(true,"BlueComp",true,"Booster",false,"94MatchDCV2","ChorusAnalog",true,"DelayRe201",true,true);                            break;               //Prog Wide Clean
        case 27:          change_tc_preset(true,"Compressor",true,"DistortionTS9",true,"YJM100","Flanger",false,"DelayRe201",true,true);                            break;               //1 Rocks 🔊🔊🔊 Pink Fl., Comfortably, ACDC
        case 28:          change_tc_preset(true,"BlueComp",true,"DistortionTS9",true,"SwitchAxeLead","ChorusAnalog",true,"DelayMono",true,true);                            break;               // 80’s-90’s Hair Metal Shred
        case 29:          change_tc_preset(true,"BlueComp",true,"Booster",false,"AC Boost","Flanger",false,"DelayMono",false,true);                            break;               //Oasis
        case 30:          change_tc_preset(true,"BassComp",true,"Booster",false,"Acoustic","Flanger",false,"DelayMono",false,true);                            break;               //Classic Words Acoustic
        case 31:          change_tc_preset(true,"LA2AComp",true,"Booster",true,"ADClean","Phaser",true,"DelayRe201",true,true);                            break;               //Funk Wah
        case 32:          change_tc_preset(true,"BassComp",true,"Booster",true,"FlatAcoustic","Flanger",false,"DelayEchoFilt",true,true);                            break;               //1-Cleanjazzy
        case 33:          change_tc_preset(true,"LA2AComp",true,"DistortionTS9",true,"ADClean","Flanger",false,"VintageDelay",true,true);                            break;               //Whole Lotta Love Jimmy Page Led Zeppelin



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
