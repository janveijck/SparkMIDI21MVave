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
        case 0:              change_hardware_preset(0);                 break; // M-Vave 01
        case 1:              change_hardware_preset(1);                 break; // M-Vave 02
        case 2:              change_hardware_preset(2);                 break; // M-Vave 03
        case 3:              change_hardware_preset(3);                 break; // M-Vave 04

        case 4:           change_comp_toggle();                         break; // M-Vave 05
        case 5:           change_drive_toggle();                        break; // M-Vave 06    
        case 6:           change_reverb_toggle();                       break; // M-Vave 07
        case 7:           change_delay_toggle();                        break; // M-Vave 08

        case 8:           change_mod_toggle();                          break; // M-Vave 09
        case 9:           change_noisegate_toggle();                    break; // M-Vave 11
// preset codes

case 10:  /*  Metallica - M-Vave position:  12  */  
						change_noisegate_onoff(true);
						change_noisegate_param(0,0.49386200308799744);
						change_noisegate_param(1,0.6338970065116882);
						change_comp_model("LA2AComp");change_comp_onoff(false);
						change_comp_param(0,0);
						change_comp_param(1,0.7126444578170776);
						change_comp_param(2,0.7248061895370483);
						change_drive_model("DistortionTS9");change_drive_onoff(true);
						change_drive_param(0,0.5554119944572449);
						change_drive_param(1,0.5);
						change_drive_param(2,0.5);
						change_amp_model("Rectifier");
						change_amp_param(0,0.9121090173721313);
						change_amp_param(1,0.771884024143219);
						change_amp_param(2,0.20000000298023224);
						change_amp_param(3,0.6000000238418579);
						change_amp_param(4,0.5331509709358215);
						change_mod_model("Tremolo");change_mod_onoff(false);
						change_mod_param(0,0.4541335105895996);
						change_mod_param(1,0.699933648109436);
						change_mod_param(2,0.5961538553237915);
						change_delay_model("DelayMono");change_delay_onoff(false);
						change_delay_param(0,0.38177722692489624);
						change_delay_param(1,0.39830508828163147);
						change_delay_param(2,0.7363313436508179);
						change_delay_param(3,0.6000000238418579);
						change_delay_param(4,0);
						change_reverb_onoff(true);
						change_reverb_param(0,0.2857142984867096);
						change_reverb_param(1,0.4083535671234131);
						change_reverb_param(2,0.28948888182640076);
						change_reverb_param(3,0.38831719756126404);
						change_reverb_param(4,0.5821428298950195);
						change_reverb_param(5,0.6500000357627869);
						change_reverb_param(6,0.20000000298023224);
						break;
case 11:  /*  JvE Metallica  */  
						change_noisegate_onoff(true);
						change_noisegate_param(0,0.49386200308799744);
						change_noisegate_param(1,0.6338970065116882);
						change_comp_model("Compressor");change_comp_onoff(true);
						change_comp_param(0,0.4350247278357997);
						change_comp_param(1,0.6475722060059056);
						change_drive_model("DistortionTS9");change_drive_onoff(true);
						change_drive_param(0,0.5554119944572449);
						change_drive_param(1,0.5);
						change_drive_param(2,0.5);
						change_amp_model("Rectifier");
						change_amp_param(0,0.9121090173721313);
						change_amp_param(1,0.771884024143219);
						change_amp_param(2,0.20000000298023224);
						change_amp_param(3,0.6000000238418579);
						change_amp_param(4,0.5331509709358215);
						change_mod_model("GuitarEQ6");change_mod_onoff(true);
						change_mod_param(0,0.8208955223880596);
						change_mod_param(1,0.19651748884969686);
						change_mod_param(2,0.36815916602291276);
						change_mod_param(3,0.5);
						change_mod_param(4,0.5);
						change_mod_param(5,0.5671641791044776);
						change_mod_param(6,0.6567164179104478);
						change_delay_model("DelayMono");change_delay_onoff(false);
						change_delay_param(0,0.38177722692489624);
						change_delay_param(1,0.39830508828163147);
						change_delay_param(2,0.7363313436508179);
						change_delay_param(3,0.6000000238418579);
						change_delay_param(4,0);
						change_reverb_onoff(true);
						change_reverb_param(0,0.2857142984867096);
						change_reverb_param(1,0.4083535671234131);
						change_reverb_param(2,0.28948888182640076);
						change_reverb_param(3,0.38831719756126404);
						change_reverb_param(4,0.5821428298950195);
						change_reverb_param(5,0.6500000357627869);
						change_reverb_param(6,0.20000000298023224);
						break;

case 12:  /*  Bend the Fend - M-Vave position:  14  */  
						change_noisegate_onoff(true);
						change_noisegate_param(0,0.0010000000474974513);
						change_noisegate_param(1,0);
						change_comp_model("Compressor");change_comp_onoff(true);
						change_comp_param(0,0.5);
						change_comp_param(1,0.5);
						change_drive_model("Booster");change_drive_onoff(false);
						change_drive_param(0,0.7670454382896423);
						change_amp_model("Twin");
						change_amp_param(0,0.510155975818634);
						change_amp_param(1,0.6299999952316284);
						change_amp_param(2,0.6000000238418579);
						change_amp_param(3,0.6000000238418579);
						change_amp_param(4,0.5799999833106995);
						change_mod_model("GuitarEQ6");
						change_mod_onoff(true);
						change_mod_param(0,0.8208955223880596);
						change_mod_param(1,0.19651748884969686);
						change_mod_param(2,0.36815916602291276);
						change_mod_param(3,0.5);
						change_mod_param(4,0.5);
						change_mod_param(5,0.5671641791044776);
						change_mod_param(6,0.6567164179104478);
						change_reverb_onoff(true);
						change_reverb_param(0,0.2857142984867096);
						change_reverb_param(1,0.4083535671234131);
						change_reverb_param(2,0.28948888182640076);
						change_reverb_param(3,0.38831719756126404);
						change_reverb_param(4,0.5821428298950195);
						change_reverb_param(5,0.6500000357627869);
						change_reverb_param(6,0.20000000298023224);
						break;

case 13:  /*  Classic Goldface - M-Vave position:  15  */  
						change_noisegate_onoff(true);
						change_noisegate_param(0,0.4912109971046448);
						change_noisegate_param(1,0.49849799275398254);
						change_comp_model("LA2AComp");change_comp_onoff(false);
						change_comp_param(0,0);
						change_comp_param(1,0.7126444578170776);
						change_comp_param(2,0.7248061895370483);
						change_drive_model("Overdrive");change_drive_onoff(true);
						change_drive_param(0,0.5);
						change_drive_param(1,0.6000000238418579);
						change_drive_param(2,0.6000000238418579);
						change_amp_model("OverDrivenJM45");
						change_amp_param(0,0.6148959994316101);
						change_amp_param(1,0.8734430074691772);
						change_amp_param(2,0.5333499908447266);
						change_amp_param(3,0.6000000238418579);
						change_amp_param(4,0.699999988079071);
						change_mod_model("GuitarEQ6");
						change_mod_onoff(true);
						change_mod_param(0,0.8208955223880596);
						change_mod_param(1,0.19651748884969686);
						change_mod_param(2,0.36815916602291276);
						change_mod_param(3,0.5);
						change_mod_param(4,0.5);
						change_mod_param(5,0.5671641791044776);
						change_mod_param(6,0.6567164179104478);
						change_delay_model("DelayMono");change_delay_onoff(true);
						change_delay_param(0,0.1737940013408661);
						change_delay_param(1,0.19404999911785126);
						change_delay_param(2,0);
						change_delay_param(3,0.30000001192092896);
						change_delay_param(4,0);
						change_reverb_onoff(true);
						change_reverb_param(0,0.2857142984867096);
						change_reverb_param(1,0.4083535671234131);
						change_reverb_param(2,0.28948888182640076);
						change_reverb_param(3,0.38831719756126404);
						change_reverb_param(4,0.5821428298950195);
						change_reverb_param(5,0.6500000357627869);
						change_reverb_param(6,0.20000000298023224);
						break;

case 14:  /*  Hendrix  - M-Vave position:  16  */  
						change_noisegate_onoff(true);
						change_noisegate_param(0,0.8193359971046448);
						change_noisegate_param(1,1);
						change_comp_model("LA2AComp");change_comp_onoff(false);
						change_comp_param(0,0);
						change_comp_param(1,0.7126444578170776);
						change_comp_param(2,0.7248061895370483);
						change_drive_model("Fuzz");change_drive_onoff(true);
						change_drive_param(0,0.6185389757156372);
						change_drive_param(1,0.6578500270843506);
						change_amp_model("OrangeAD30");
						change_amp_param(0,0.6000000238418579);
						change_amp_param(1,0.699999988079071);
						change_amp_param(2,0.5318359732627869);
						change_amp_param(3,0.9310669898986816);
						change_amp_param(4,0.7549480199813843);
						change_mod_model("GuitarEQ6");
						change_mod_onoff(true);
						change_mod_param(0,0.8208955223880596);
						change_mod_param(1,0.19651748884969686);
						change_mod_param(2,0.36815916602291276);
						change_mod_param(3,0.5);
						change_mod_param(4,0.5);
						change_mod_param(5,0.5671641791044776);
						change_mod_param(6,0.6567164179104478);
						change_delay_model("DelayMono");change_delay_onoff(false);
						change_delay_param(0,0.38177722692489624);
						change_delay_param(1,0.39830508828163147);
						change_delay_param(2,0.7363313436508179);
						change_delay_param(3,0.6000000238418579);
						change_delay_param(4,0);
						change_reverb_onoff(true);
						change_reverb_param(0,0.33571431040763855);
						change_reverb_param(1,0.6438256502151489);
						change_reverb_param(2,0.5208232402801514);
						change_reverb_param(3,0.28129544854164124);
						change_reverb_param(4,0.4695378243923187);
						change_reverb_param(5,0.45546218752861023);
						change_reverb_param(6,0.6000000238418579);
						break;

case 15:  /*  Engl Fireball - M-Vave position:  17  */  
						change_noisegate_onoff(true);
						change_noisegate_param(0,0.5455359816551208);
						change_noisegate_param(1,0.5694590210914612);
						change_comp_model("LA2AComp");change_comp_onoff(false);
						change_comp_param(0,0);
						change_comp_param(1,0.7126444578170776);
						change_comp_param(2,0.7248061895370483);
						change_drive_model("DistortionTS9");change_drive_onoff(true);
						change_drive_param(0,0.12645499408245087);
						change_drive_param(1,0.8161389827728271);
						change_drive_param(2,1);
						change_amp_model("SwitchAxeLead");
						change_amp_param(0,0.699999988079071);
						change_amp_param(1,0.5039759874343872);
						change_amp_param(2,0.0562950000166893);
						change_amp_param(3,1);
						change_amp_param(4,0.699999988079071);
						change_mod_model("GuitarEQ6");
						change_mod_onoff(true);
						change_mod_param(0,0.8208955223880596);
						change_mod_param(1,0.19651748884969686);
						change_mod_param(2,0.36815916602291276);
						change_mod_param(3,0.5);
						change_mod_param(4,0.5);
						change_mod_param(5,0.5671641791044776);
						change_mod_param(6,0.6567164179104478);
						change_delay_model("DelayMono");change_delay_onoff(false);
						change_delay_param(0,0.38177722692489624);
						change_delay_param(1,0.39830508828163147);
						change_delay_param(2,0.7363313436508179);
						change_delay_param(3,0.6000000238418579);
						change_delay_param(4,0);
						change_reverb_onoff(true);
						change_reverb_param(0,0.2857142984867096);
						change_reverb_param(1,0.4083535671234131);
						change_reverb_param(2,0.28948888182640076);
						change_reverb_param(3,0.38831719756126404);
						change_reverb_param(4,0.5821428298950195);
						change_reverb_param(5,0.6500000357627869);
						change_reverb_param(6,0.20000000298023224);
						break;

case 16:  /*  Eddie Van Halen - M-Vave position:  18  */  
						change_noisegate_onoff(true);
						change_noisegate_param(0,0.492561012506485);
						change_noisegate_param(1,0.5122069716453552);
						change_comp_model("BlueComp");change_comp_onoff(true);
						change_comp_param(0,0.6000000238418579);
						change_comp_param(1,0.5674999952316284);
						change_comp_param(2,0.574999988079071);
						change_comp_param(3,0.7678940296173096);
						change_drive_model("Booster");change_drive_onoff(false);
						change_drive_param(0,0.7670454382896423);
						change_amp_model("EVH");
						change_amp_param(0,0.6000000238418579);
						change_amp_param(1,0.6202049851417542);
						change_amp_param(2,0.637499988079071);
						change_amp_param(3,0.6000000238418579);
						change_amp_param(4,0.6208329796791077);
						change_mod_model("GuitarEQ6");
						change_mod_onoff(true);
						change_mod_param(0,0.8208955223880596);
						change_mod_param(1,0.19651748884969686);
						change_mod_param(2,0.36815916602291276);
						change_mod_param(3,0.5);
						change_mod_param(4,0.5);
						change_mod_param(5,0.5671641791044776);
						change_mod_param(6,0.6567164179104478);
						change_delay_model("DelayMono");change_delay_onoff(true);
						change_delay_param(0,0.41652798652648926);
						change_delay_param(1,0.16249999403953552);
						change_delay_param(2,0.49022701382637024);
						change_delay_param(3,0.4000000059604645);
						change_delay_param(4,0);
						change_reverb_onoff(true);
						change_reverb_param(0,0.2857142984867096);
						change_reverb_param(1,0.4083535671234131);
						change_reverb_param(2,0.28948888182640076);
						change_reverb_param(3,0.38831719756126404);
						change_reverb_param(4,0.5821428298950195);
						change_reverb_param(5,0.6500000357627869);
						change_reverb_param(6,0.20000000298023224);
						break;

case 17:  /*  SRV - M-Vave position:  19  */  
						change_noisegate_onoff(true);
						change_noisegate_param(0,0.07552083333333334);
						change_noisegate_param(1,0.08160717288653056);
						change_noisegate_param(2,1);
						change_comp_model("Compressor");change_comp_onoff(false);
						change_comp_param(0,0.4350247278357997);
						change_comp_param(1,0.6475722060059056);
						change_drive_model("Overdrive");change_drive_onoff(true);
						change_drive_param(0,0.6480110234977132);
						change_drive_param(1,0.7);
						change_drive_param(2,0.4581767712508776);
						change_amp_model("TwoStoneSP50");
						change_amp_param(0,0.5808736923312353);
						change_amp_param(1,0.5076922700955318);
						change_amp_param(2,0.6109799186060252);
						change_amp_param(3,0.3970588235294118);
						change_amp_param(4,0.6502951405757715);
						change_mod_model("GuitarEQ6");
						change_mod_onoff(true);
						change_mod_param(0,0.8208955223880596);
						change_mod_param(1,0.19651748884969686);
						change_mod_param(2,0.36815916602291276);
						change_mod_param(3,0.5);
						change_mod_param(4,0.5);
						change_mod_param(5,0.5671641791044776);
						change_mod_param(6,0.6567164179104478);
						change_delay_model("DelayRe201");change_delay_onoff(false);
						change_delay_param(0,0.26535865664482117);
						change_delay_param(1,0.37925663590431213);
						change_delay_param(2,0.29451701045036316);
						change_delay_param(3,0.5995410084724426);
						change_delay_param(4,1);
						change_reverb_onoff(true);
						change_reverb_param(0,0.6371087509652843);
						change_reverb_param(1,0.5066586085728237);
						change_reverb_param(2,0.4178571428571428);
						change_reverb_param(3,0.3008474576271186);
						change_reverb_param(4,0.60228724862848);
						change_reverb_param(5,0.5941176593303681);
						change_reverb_param(6,0);
						break;

case 18:  /*  Gilmour - M-Vave position:  20  */  
						change_noisegate_onoff(false);
						change_noisegate_param(0,0.2181818187236786);
						change_noisegate_param(1,0.06487986445426941);
						change_comp_model("BlueComp");change_comp_onoff(true);
						change_comp_param(0,0.6000000238418579);
						change_comp_param(1,0.47999998927116394);
						change_comp_param(2,0.4000000059604645);
						change_comp_param(3,0.6000000238418579);
						change_drive_model("Booster");change_drive_onoff(false);
						change_drive_param(0,0.7670454382896423);
						change_amp_model("YJM100");
						change_amp_param(0,0.6499999761581421);
						change_amp_param(1,0.699999988079071);
						change_amp_param(2,0.4000000059604645);
						change_amp_param(3,0.6000000238418579);
						change_amp_param(4,0.699999988079071);
						change_mod_model("GuitarEQ6");
						change_mod_onoff(true);
						change_mod_param(0,0.8208955223880596);
						change_mod_param(1,0.19651748884969686);
						change_mod_param(2,0.36815916602291276);
						change_mod_param(3,0.5);
						change_mod_param(4,0.5);
						change_mod_param(5,0.5671641791044776);
						change_mod_param(6,0.6567164179104478);
						change_delay_model("DelayRe201");change_delay_onoff(true);
						change_delay_param(0,0.6000000238418579);
						change_delay_param(1,0.39924201369285583);
						change_delay_param(2,0.5321210026741028);
						change_delay_param(3,0.75);
						change_delay_param(4,0);
						change_reverb_onoff(true);
						change_reverb_param(0,0.33571431040763855);
						change_reverb_param(1,0.6438256502151489);
						change_reverb_param(2,0.5208232402801514);
						change_reverb_param(3,0.28129544854164124);
						change_reverb_param(4,0.4695378243923187);
						change_reverb_param(5,0.45546218752861023);
						change_reverb_param(6,0.6000000238418579);
						break;

case 19:  /*  Nothing else matters - M-Vave position:  21  */  
						change_noisegate_onoff(true);
						change_noisegate_param(0,0.054980138937632234);
						change_noisegate_param(1,0.11526797215143839);
						change_noisegate_param(2,1);
						change_comp_model("BlueComp");change_comp_onoff(true);
						change_comp_param(0,0.49309322663715904);
						change_comp_param(1,0.7031683428155864);
						change_comp_param(2,0.18633624765039222);
						change_comp_param(3,0.7288892524582999);
						change_drive_model("Booster");change_drive_onoff(false);
						change_drive_param(0,0.7699530552810346);
						change_amp_model("ADClean");
						change_amp_param(0,0.6770833730697632);
						change_amp_param(1,0.5703526038389939);
						change_amp_param(2,0.5315881835949885);
						change_amp_param(3,0.34118591730411235);
						change_amp_param(4,0.68);
						change_mod_model("GuitarEQ6");
						change_mod_onoff(true);
						change_mod_param(0,0.8208955223880596);
						change_mod_param(1,0.19651748884969686);
						change_mod_param(2,0.36815916602291276);
						change_mod_param(3,0.5);
						change_mod_param(4,0.5);
						change_mod_param(5,0.5671641791044776);
						change_mod_param(6,0.6567164179104478);
						change_delay_model("DelayMono");change_delay_onoff(true);
						change_delay_param(0,0.05729743633005355);
						change_delay_param(1,0.3983050847457627);
						change_delay_param(2,0.7363313244235131);
						change_delay_param(3,0.6000000238418579);
						change_delay_param(4,1);
						change_reverb_onoff(true);
						change_reverb_param(0,0.8355405978534532);
						change_reverb_param(1,0.565181314945221);
						change_reverb_param(2,0.6466543078422546);
						change_reverb_param(3,0.6669116383013518);
						change_reverb_param(4,0.48823529992784775);
						change_reverb_param(5,0.2163865566253662);
						change_reverb_param(6,0.3);
						break;

case 20:  /*  Sweet Child Of Mine - M-Vave position:  22  */  
						change_noisegate_onoff(true);
						change_noisegate_param(0,0.3082520067691803);
						change_noisegate_param(1,0.4663360118865967);
						change_comp_model("LA2AComp");change_comp_onoff(false);
						change_comp_param(0,0);
						change_comp_param(1,0.7126444578170776);
						change_comp_param(2,0.7248061895370483);
						change_drive_model("DistortionTS9");change_drive_onoff(true);
						change_drive_param(0,0.4053240120410919);
						change_drive_param(1,0.32471901178359985);
						change_drive_param(2,0.477306991815567);
						change_amp_model("OverDrivenJM45");
						change_amp_param(0,0.7321159839630127);
						change_amp_param(1,0.8548920154571533);
						change_amp_param(2,0.6432960033416748);
						change_amp_param(3,0.3079409897327423);
						change_amp_param(4,0.820002019405365);
						change_mod_model("GuitarEQ6");
						change_mod_onoff(true);
						change_mod_param(0,0.8208955223880596);
						change_mod_param(1,0.19651748884969686);
						change_mod_param(2,0.36815916602291276);
						change_mod_param(3,0.5);
						change_mod_param(4,0.5);
						change_mod_param(5,0.5671641791044776);
						change_mod_param(6,0.6567164179104478);
						change_delay_model("DelayMono");change_delay_onoff(true);
						change_delay_param(0,0.2644500136375427);
						change_delay_param(1,0.27424201369285583);
						change_delay_param(2,0.29398399591445923);
						change_delay_param(3,0.6000000238418579);
						change_delay_param(4,0);
						change_reverb_onoff(true);
						change_reverb_param(0,0.2857142984867096);
						change_reverb_param(1,0.4083535671234131);
						change_reverb_param(2,0.28948888182640076);
						change_reverb_param(3,0.38831719756126404);
						change_reverb_param(4,0.5821428298950195);
						change_reverb_param(5,0.6500000357627869);
						change_reverb_param(6,0.20000000298023224);
						break;

case 21:  /*   Purple Rain - M-Vave position:  23  */  
						change_noisegate_onoff(true);
						change_noisegate_param(0,0.5665820240974426);
						change_noisegate_param(1,0.5455330014228821);
						change_comp_model("Compressor");change_comp_onoff(true);
						change_comp_param(0,0.2043690967415026);
						change_comp_param(1,0.20134924478859073);
						change_drive_model("Booster");change_drive_onoff(true);
						change_drive_param(0,0.6897847652435303);
						change_amp_model("Twin");
						change_amp_param(0,1);
						change_amp_param(1,0.7716535329818726);
						change_amp_param(2,0.6000000238418579);
						change_amp_param(3,0.7546391990995899);
						change_amp_param(4,0.5511810779571533);
						change_mod_model("GuitarEQ6");
						change_mod_onoff(true);
						change_mod_param(0,0.8208955223880596);
						change_mod_param(1,0.19651748884969686);
						change_mod_param(2,0.36815916602291276);
						change_mod_param(3,0.5);
						change_mod_param(4,0.5);
						change_mod_param(5,0.5671641791044776);
						change_mod_param(6,0.6567164179104478);
						change_delay_model("DelayRe201");change_delay_onoff(true);
						change_delay_param(0,0.06635988523987946);
						change_delay_param(1,0.3017686374725834);
						change_delay_param(2,0.665913999080658);
						change_delay_param(3,0.09891061867489426);
						change_delay_param(4,1);
						change_reverb_onoff(true);
						change_reverb_param(0,0.34040117689541405);
						change_reverb_param(1,0.4083535671234131);
						change_reverb_param(2,0.28948888182640076);
						change_reverb_param(3,0.4002219957964761);
						change_reverb_param(4,0.5821428298950195);
						change_reverb_param(5,0.6500000357627869);
						change_reverb_param(6,0.20000000298023224);
						break;

case 22:  /*  John mayer ish - M-Vave position:  24  */  
						change_noisegate_onoff(true);
						change_noisegate_param(0,0.0010000000474974513);
						change_noisegate_param(1,0);
						change_comp_model("Compressor");change_comp_onoff(true);
						change_comp_param(0,0.5);
						change_comp_param(1,0.5);
						change_drive_model("Booster");change_drive_onoff(false);
						change_drive_param(0,0.7670454382896423);
						change_amp_model("Twin");
						change_amp_param(0,0.6478509902954102);
						change_amp_param(1,0.3291989862918854);
						change_amp_param(2,0.5514709949493408);
						change_amp_param(3,0.6691399812698364);
						change_amp_param(4,0.5799999833106995);
						change_mod_model("GuitarEQ6");
						change_mod_onoff(true);
						change_mod_param(0,0.8208955223880596);
						change_mod_param(1,0.19651748884969686);
						change_mod_param(2,0.36815916602291276);
						change_mod_param(3,0.5);
						change_mod_param(4,0.5);
						change_mod_param(5,0.5671641791044776);
						change_mod_param(6,0.6567164179104478);
						change_delay_model("DelayMono");change_delay_onoff(true);
						change_delay_param(0,0.064021997153759);
						change_delay_param(1,0.21516799926757812);
						change_delay_param(2,0.6917970180511475);
						change_delay_param(3,0.699999988079071);
						change_delay_param(4,0);
						change_reverb_onoff(true);
						change_reverb_param(0,0.2857142984867096);
						change_reverb_param(1,0.4083535671234131);
						change_reverb_param(2,0.28948888182640076);
						change_reverb_param(3,0.38831719756126404);
						change_reverb_param(4,0.5821428298950195);
						change_reverb_param(5,0.6500000357627869);
						change_reverb_param(6,0.20000000298023224);
						break;

case 23:  /*  Shine On Crazy Diamond Clean - M-Vave position:  25  */  
						change_noisegate_onoff(false);
						change_noisegate_param(0,0.2181818187236786);
						change_noisegate_param(1,0.06487986445426941);
						change_comp_model("Compressor");change_comp_onoff(true);
						change_comp_param(0,0.5);
						change_comp_param(1,0.5);
						change_drive_model("Booster");change_drive_onoff(false);
						change_drive_param(0,0.7670454382896423);
						change_amp_model("Twin");
						change_amp_param(0,0.6000000238418579);
						change_amp_param(1,0.6299999952316284);
						change_amp_param(2,0.6000000238418579);
						change_amp_param(3,0.6000000238418579);
						change_amp_param(4,0.5799999833106995);
						change_mod_model("GuitarEQ6");
						change_mod_onoff(true);
						change_mod_param(0,0.8208955223880596);
						change_mod_param(1,0.19651748884969686);
						change_mod_param(2,0.36815916602291276);
						change_mod_param(3,0.5);
						change_mod_param(4,0.5);
						change_mod_param(5,0.5671641791044776);
						change_mod_param(6,0.6567164179104478);
						change_delay_model("DelayMono");change_delay_onoff(false);
						change_delay_param(0,0.38177722692489624);
						change_delay_param(1,0.39830508828163147);
						change_delay_param(2,0.7363313436508179);
						change_delay_param(3,0.6000000238418579);
						change_delay_param(4,0);
						change_reverb_onoff(true);
						change_reverb_param(0,0.2857142984867096);
						change_reverb_param(1,0.4083535671234131);
						change_reverb_param(2,0.28948888182640076);
						change_reverb_param(3,0.38831719756126404);
						change_reverb_param(4,0.5821428298950195);
						change_reverb_param(5,0.6500000357627869);
						change_reverb_param(6,0.20000000298023224);
						break;

case 24:  /*  BB king - M-Vave position:  26  */  
						change_noisegate_onoff(true);
						change_noisegate_param(0,0.3698920011520386);
						change_noisegate_param(1,0.16577300429344177);
						change_comp_model("LA2AComp");change_comp_onoff(false);
						change_comp_param(0,0);
						change_comp_param(1,0.7126444578170776);
						change_comp_param(2,0.7248061895370483);
						change_drive_model("Booster");change_drive_onoff(false);
						change_drive_param(0,0.7670454382896423);
						change_amp_model("Plexi");
						change_amp_param(0,0.5210819840431213);
						change_amp_param(1,0.31972700357437134);
						change_amp_param(2,0.5552769899368286);
						change_amp_param(3,0.8254600167274475);
						change_amp_param(4,0.7566409707069397);
						change_mod_model("GuitarEQ6");
						change_mod_onoff(true);
						change_mod_param(0,0.8208955223880596);
						change_mod_param(1,0.19651748884969686);
						change_mod_param(2,0.36815916602291276);
						change_mod_param(3,0.5);
						change_mod_param(4,0.5);
						change_mod_param(5,0.5671641791044776);
						change_mod_param(6,0.6567164179104478);
						change_delay_model("DelayMono");change_delay_onoff(false);
						change_delay_param(0,0.38177722692489624);
						change_delay_param(1,0.39830508828163147);
						change_delay_param(2,0.7363313436508179);
						change_delay_param(3,0.6000000238418579);
						change_delay_param(4,0);
						change_reverb_onoff(true);
						change_reverb_param(0,0.2857142984867096);
						change_reverb_param(1,0.4083535671234131);
						change_reverb_param(2,0.28948888182640076);
						change_reverb_param(3,0.38831719756126404);
						change_reverb_param(4,0.5821428298950195);
						change_reverb_param(5,0.6500000357627869);
						change_reverb_param(6,0.20000000298023224);
						break;

case 25:  /*  Clean Rhythm Delay - M-Vave position:  27  */  
						change_noisegate_onoff(true);
						change_noisegate_param(0,0.1435218056042989);
						change_noisegate_param(1,0.16784806549549103);
						change_noisegate_param(2,1);
						change_comp_model("BlueComp");change_comp_onoff(true);
						change_comp_param(0,0.5532860330172947);
						change_comp_param(1,0.5896714274868643);
						change_comp_param(2,0.4930847342014313);
						change_comp_param(3,0.6842463953154427);
						change_drive_model("Booster");change_drive_onoff(true);
						change_drive_param(0,0.7370892018779343);
						change_amp_model("Twin");
						change_amp_param(0,0.6134334206581116);
						change_amp_param(1,0.5923030376434326);
						change_amp_param(2,0.37878668308258057);
						change_amp_param(3,0.4707774817943573);
						change_amp_param(4,0.4387507736682892);
						change_mod_model("GuitarEQ6");
						change_mod_onoff(true);
						change_mod_param(0,0.8208955223880596);
						change_mod_param(1,0.19651748884969686);
						change_mod_param(2,0.36815916602291276);
						change_mod_param(3,0.5);
						change_mod_param(4,0.5);
						change_mod_param(5,0.5671641791044776);
						change_mod_param(6,0.6567164179104478);
						change_delay_model("DelayRe201");change_delay_onoff(true);
						change_delay_param(0,0.21367231753137378);
						change_delay_param(1,0.47184064984321594);
						change_delay_param(2,0.18150149285793304);
						change_delay_param(3,0.4837649166584015);
						change_delay_param(4,1);
						change_reverb_onoff(true);
						change_reverb_param(0,0.862500011920929);
						change_reverb_param(1,0.30273453826489655);
						change_reverb_param(2,0.5785714387893677);
						change_reverb_param(3,0.7791148916534756);
						change_reverb_param(4,0.6249062418937683);
						change_reverb_param(5,0.6767376661300659);
						change_reverb_param(6,0.5);
						break;

case 26:  /*  The mighty sabbath - M-Vave position:  28  */  
						change_noisegate_onoff(true);
						change_noisegate_param(0,0.6699540019035339);
						change_noisegate_param(1,0.8994550108909607);
						change_comp_model("LA2AComp");change_comp_onoff(false);
						change_comp_param(0,0);
						change_comp_param(1,0.7126444578170776);
						change_comp_param(2,0.7248061895370483);
						change_drive_model("DistortionTS9");change_drive_onoff(true);
						change_drive_param(0,0.5816050171852112);
						change_drive_param(1,0.7523720264434814);
						change_drive_param(2,0.5);
						change_amp_model("Rectifier");
						change_amp_param(0,0.9225469827651978);
						change_amp_param(1,0.5497509837150574);
						change_amp_param(2,0.048618000000715256);
						change_amp_param(3,0.6978769898414612);
						change_amp_param(4,0.7644150257110596);
						change_mod_model("GuitarEQ6");
						change_mod_onoff(true);
						change_mod_param(0,0.8208955223880596);
						change_mod_param(1,0.19651748884969686);
						change_mod_param(2,0.36815916602291276);
						change_mod_param(3,0.5);
						change_mod_param(4,0.5);
						change_mod_param(5,0.5671641791044776);
						change_mod_param(6,0.6567164179104478);
						change_delay_model("DelayMono");change_delay_onoff(false);
						change_delay_param(0,0.38177722692489624);
						change_delay_param(1,0.39830508828163147);
						change_delay_param(2,0.7363313436508179);
						change_delay_param(3,0.6000000238418579);
						change_delay_param(4,0);
						change_reverb_onoff(true);
						change_reverb_param(0,0.35932204127311707);
						change_reverb_param(1,0.5066586136817932);
						change_reverb_param(2,0.4178571403026581);
						change_reverb_param(3,0.30084747076034546);
						change_reverb_param(4,0.602287232875824);
						change_reverb_param(5,0.5941176414489746);
						change_reverb_param(6,0);
						break;

case 27:  /*  Modern Vintage Clean - M-Vave position:  29  */  
						change_noisegate_onoff(true);
						change_noisegate_param(0,0.13704976439476013);
						change_noisegate_param(1,0);
						change_noisegate_param(2,1);
						change_comp_model("BlueComp");change_comp_onoff(true);
						change_comp_param(0,0.6314110330172947);
						change_comp_param(1,0.8364433342693773);
						change_comp_param(2,0.13912359830791965);
						change_comp_param(3,0.4074606810297285);
						change_drive_model("DistortionTS9");change_drive_onoff(false);
						change_drive_param(0,0.0441989041864872);
						change_drive_param(1,1);
						change_drive_param(2,0.939226508140564);
						change_drive_param(3,0);
						change_amp_model("ADClean");
						change_amp_param(0,0.6770833730697632);
						change_amp_param(1,0.5703526038389939);
						change_amp_param(2,0.5315881835949885);
						change_amp_param(3,0.34118591730411235);
						change_amp_param(4,0.68);
						change_mod_model("GuitarEQ6");
						change_mod_onoff(true);
						change_mod_param(0,0.8208955223880596);
						change_mod_param(1,0.19651748884969686);
						change_mod_param(2,0.36815916602291276);
						change_mod_param(3,0.5);
						change_mod_param(4,0.5);
						change_mod_param(5,0.5671641791044776);
						change_mod_param(6,0.6567164179104478);
						change_delay_model("DelayRe201");change_delay_onoff(false);
						change_delay_param(0,0.26535865664482117);
						change_delay_param(1,0.37925663590431213);
						change_delay_param(2,0.29451701045036316);
						change_delay_param(3,0.5995410084724426);
						change_delay_param(4,1);
						change_reverb_onoff(true);
						change_reverb_param(0,0.6500000391687666);
						change_reverb_param(1,0.1499999863760812);
						change_reverb_param(2,0.5);
						change_reverb_param(3,0.6214285884584699);
						change_reverb_param(4,0.3699047565460205);
						change_reverb_param(5,0.35);
						change_reverb_param(6,0.1);
						break;

case 28:  /*  Pink Floyd - Comfortably Numb (solo 2) - M-Vave position:  30  */  
						change_noisegate_onoff(true);
						change_noisegate_param(0,0.299888014793396);
						change_noisegate_param(1,0.37761199474334717);
						change_comp_model("LA2AComp");change_comp_onoff(false);
						change_comp_param(0,0);
						change_comp_param(1,0.7126444578170776);
						change_comp_param(2,0.7248061895370483);
						change_drive_model("ProCoRat");change_drive_onoff(true);
						change_drive_param(0,0.5524680018424988);
						change_drive_param(1,0.6889780163764954);
						change_drive_param(2,0.6620680093765259);
						change_amp_model("OverDrivenLuxVerb");
						change_amp_param(0,0.8664169907569885);
						change_amp_param(1,0.862500011920929);
						change_amp_param(2,0.5631809830665588);
						change_amp_param(3,0.7429180145263672);
						change_amp_param(4,0.6303489804267883);
						change_mod_model("GuitarEQ6");
						change_mod_onoff(true);
						change_mod_param(0,0.8208955223880596);
						change_mod_param(1,0.19651748884969686);
						change_mod_param(2,0.36815916602291276);
						change_mod_param(3,0.5);
						change_mod_param(4,0.5);
						change_mod_param(5,0.5671641791044776);
						change_mod_param(6,0.6567164179104478);
						change_delay_model("DelayRe201");change_delay_onoff(true);
						change_delay_param(0,0.26744499802589417);
						change_delay_param(1,0.3033210039138794);
						change_delay_param(2,0);
						change_delay_param(3,0.6923609972000122);
						change_delay_param(4,0);
						change_reverb_onoff(true);
						change_reverb_param(0,0.2857142984867096);
						change_reverb_param(1,0.4083535671234131);
						change_reverb_param(2,0.28948888182640076);
						change_reverb_param(3,0.38831719756126404);
						change_reverb_param(4,0.5821428298950195);
						change_reverb_param(5,0.6500000357627869);
						change_reverb_param(6,0.20000000298023224);
						break;

case 29:  /*  SRV Clean - M-Vave position:  31  */  
						change_noisegate_onoff(true);
						change_noisegate_param(0,0.5420539975166321);
						change_noisegate_param(1,0.19391800463199615);
						change_comp_model("Compressor");change_comp_onoff(true);
						change_comp_param(0,0.6381239891052246);
						change_comp_param(1,0.8851770162582397);
						change_drive_model("Booster");change_drive_onoff(false);
						change_drive_param(0,0.7670454382896423);
						change_amp_model("Twin");
						change_amp_param(0,0.5571079850196838);
						change_amp_param(1,1);
						change_amp_param(2,0.9151210188865662);
						change_amp_param(3,0.276434987783432);
						change_amp_param(4,0.5799999833106995);
						change_mod_model("GuitarEQ6");
						change_mod_onoff(true);
						change_mod_param(0,0.8208955223880596);
						change_mod_param(1,0.19651748884969686);
						change_mod_param(2,0.36815916602291276);
						change_mod_param(3,0.5);
						change_mod_param(4,0.5);
						change_mod_param(5,0.5671641791044776);
						change_mod_param(6,0.6567164179104478);
						change_delay_model("DelayMono");change_delay_onoff(true);
						change_delay_param(0,0.05803599953651428);
						change_delay_param(1,0.20121900737285614);
						change_delay_param(2,0.13676099479198456);
						change_delay_param(3,0.699999988079071);
						change_delay_param(4,0);
						change_reverb_onoff(true);
						change_reverb_param(0,0.2857142984867096);
						change_reverb_param(1,0.4083535671234131);
						change_reverb_param(2,0.28948888182640076);
						change_reverb_param(3,0.38831719756126404);
						change_reverb_param(4,0.5821428298950195);
						change_reverb_param(5,0.6500000357627869);
						change_reverb_param(6,0.20000000298023224);
						break;

case 30:  /*  OLD VH - M-Vave position:  32  */  
						change_noisegate_onoff(true);
						change_noisegate_param(0,0.5455359816551208);
						change_noisegate_param(1,0.5694590210914612);
						change_comp_model("LA2AComp");change_comp_onoff(false);
						change_comp_param(0,0);
						change_comp_param(1,0.7126444578170776);
						change_comp_param(2,0.7248061895370483);
						change_drive_model("DistortionTS9");change_drive_onoff(true);
						change_drive_param(0,0.6000000238418579);
						change_drive_param(1,0.5);
						change_drive_param(2,0.5);
						change_amp_model("EVH");
						change_amp_param(0,0.768750011920929);
						change_amp_param(1,0.9104170203208923);
						change_amp_param(2,0.3853429853916168);
						change_amp_param(3,0.6000000238418579);
						change_amp_param(4,0.699999988079071);
						change_mod_model("GuitarEQ6");
						change_mod_onoff(true);
						change_mod_param(0,0.8208955223880596);
						change_mod_param(1,0.19651748884969686);
						change_mod_param(2,0.36815916602291276);
						change_mod_param(3,0.5);
						change_mod_param(4,0.5);
						change_mod_param(5,0.5671641791044776);
						change_mod_param(6,0.6567164179104478);
						change_delay_model("DelayMono");change_delay_onoff(false);
						change_delay_param(0,0.38177722692489624);
						change_delay_param(1,0.39830508828163147);
						change_delay_param(2,0.7363313436508179);
						change_delay_param(3,0.6000000238418579);
						change_delay_param(4,0);
						change_reverb_onoff(true);
						change_reverb_param(0,0.2857142984867096);
						change_reverb_param(1,0.4083535671234131);
						change_reverb_param(2,0.28948888182640076);
						change_reverb_param(3,0.38831719756126404);
						change_reverb_param(4,0.5821428298950195);
						change_reverb_param(5,0.6500000357627869);
						change_reverb_param(6,0.20000000298023224);
						break;

case 31:  /*  Come As You Are - Nirvana - M-Vave position:  33  */  
						change_noisegate_onoff(false);
						change_noisegate_param(0,0.1383134772380193);
						change_noisegate_param(1,0.224642942349116);
						change_noisegate_param(2,0);
						change_comp_model("Compressor");change_comp_onoff(true);
						change_comp_param(0,0.4350247278357997);
						change_comp_param(1,0.6475722060059056);
						change_drive_model("DistortionTS9");change_drive_onoff(false);
						change_drive_param(0,0);
						change_drive_param(1,0.5);
						change_drive_param(2,0.62);
						change_amp_model("ADClean");
						change_amp_param(0,0.8701298701298701);
						change_amp_param(1,1);
						change_amp_param(2,0.6419778071440659);
						change_amp_param(3,0.48733977354489844);
						change_amp_param(4,0.5631168902694405);
						change_mod_model("GuitarEQ6");
						change_mod_onoff(true);
						change_mod_param(0,0.8208955223880596);
						change_mod_param(1,0.19651748884969686);
						change_mod_param(2,0.36815916602291276);
						change_mod_param(3,0.5);
						change_mod_param(4,0.5);
						change_mod_param(5,0.5671641791044776);
						change_mod_param(6,0.6567164179104478);
						change_delay_model("VintageDelay");change_delay_onoff(false);
						change_delay_param(0,0.37873878828069785);
						change_delay_param(1,0.4257446501136485);
						change_delay_param(2,0.4198161845175636);
						change_delay_param(3,1);
						change_reverb_onoff(true);
						change_reverb_param(0,0.33571430529866897);
						change_reverb_param(1,0.6438256774629866);
						change_reverb_param(2,0.5208232607160296);
						change_reverb_param(3,0.2812954425811768);
						change_reverb_param(4,0.4695378363132477);
						change_reverb_param(5,0.4554621968950544);
						change_reverb_param(6,0.6);
						break;

case 32:  /*   Solo Santana  - M-Vave position:  34  */  
						change_noisegate_onoff(true);
						change_noisegate_param(0,0.5455359816551208);
						change_noisegate_param(1,0.5694590210914612);
						change_comp_model("LA2AComp");change_comp_onoff(false);
						change_comp_param(0,0);
						change_comp_param(1,0.7126444578170776);
						change_comp_param(2,0.7248061895370483);
						change_drive_model("Booster");change_drive_onoff(false);
						change_drive_param(0,0.7670454382896423);
						change_amp_model("Rectifier");
						change_amp_param(0,0.643750011920929);
						change_amp_param(1,0.699999988079071);
						change_amp_param(2,0.20000000298023224);
						change_amp_param(3,0.9206240177154541);
						change_amp_param(4,0.7479169964790344);
						change_mod_model("GuitarEQ6");
						change_mod_onoff(true);
						change_mod_param(0,0.8208955223880596);
						change_mod_param(1,0.19651748884969686);
						change_mod_param(2,0.36815916602291276);
						change_mod_param(3,0.5);
						change_mod_param(4,0.5);
						change_mod_param(5,0.5671641791044776);
						change_mod_param(6,0.6567164179104478);
						change_delay_model("DelayMono");change_delay_onoff(false);
						change_delay_param(0,0.38177722692489624);
						change_delay_param(1,0.39830508828163147);
						change_delay_param(2,0.7363313436508179);
						change_delay_param(3,0.6000000238418579);
						change_delay_param(4,0);
						change_reverb_onoff(true);
						change_reverb_param(0,0.2857142984867096);
						change_reverb_param(1,0.4083535671234131);
						change_reverb_param(2,0.28948888182640076);
						change_reverb_param(3,0.38831719756126404);
						change_reverb_param(4,0.5821428298950195);
						change_reverb_param(5,0.6500000357627869);
						change_reverb_param(6,0.20000000298023224);
						break;

case 33:  /*   Clean for Ballads - M-Vave position:  35  */  
						change_noisegate_onoff(false);
						change_noisegate_param(0,0.21818182);
						change_noisegate_param(1,0.064879864);
						change_noisegate_param(2,0);
						change_comp_model("LA2AComp");change_comp_onoff(true);
						change_comp_param(0,0);
						change_comp_param(1,0.71264446);
						change_comp_param(2,0.7248062);
						change_drive_model("Booster");change_drive_onoff(true);
						change_drive_param(0,0.4045);
						change_amp_model("ADClean");
						change_amp_param(0,0.74282265);
						change_amp_param(1,0.5986);
						change_amp_param(2,0.3978);
						change_amp_param(3,0.8976);
						change_amp_param(4,1);
						change_mod_model("GuitarEQ6");
						change_mod_onoff(true);
						change_mod_param(0,0.8208955223880596);
						change_mod_param(1,0.19651748884969686);
						change_mod_param(2,0.36815916602291276);
						change_mod_param(3,0.5);
						change_mod_param(4,0.5);
						change_mod_param(5,0.5671641791044776);
						change_mod_param(6,0.6567164179104478);
						change_delay_model("VintageDelay");change_delay_onoff(true);
						change_delay_param(0,0.663975);
						change_delay_param(1,0.188906);
						change_delay_param(2,0.344989);
						change_delay_param(3,1);
						change_reverb_onoff(true);
						change_reverb_param(0,0.2857143);
						change_reverb_param(1,0.40835357);
						change_reverb_param(2,0.2996);
						change_reverb_param(3,0.7524);
						change_reverb_param(4,0.3001);
						change_reverb_param(5,0.2985);
						change_reverb_param(6,0.2);
						change_reverb_param(7,1);
						break;

case 34:  /*  12 str acou strumming.CN1 - M-Vave position:  36  */  
						change_noisegate_onoff(true);
						change_noisegate_param(0,0.10436899960041046);
						change_noisegate_param(1,0.5);
						change_comp_model("Compressor");change_comp_onoff(true);
						change_comp_param(0,0.6510909795761108);
						change_comp_param(1,0.049600999802351);
						change_drive_model("Booster");change_drive_onoff(false);
						change_drive_param(0,0.7670454382896423);
						change_amp_model("AcousticAmpV2");
						change_amp_param(0,0.6783008575439453);
						change_amp_param(1,0.619717001914978);
						change_amp_param(2,0.5388020277023315);
						change_amp_param(3,0.4137910306453705);
						change_amp_param(4,0.8025019764900208);
						change_mod_model("GuitarEQ6");
						change_mod_onoff(true);
						change_mod_param(0,0.8208955223880596);
						change_mod_param(1,0.19651748884969686);
						change_mod_param(2,0.36815916602291276);
						change_mod_param(3,0.5);
						change_mod_param(4,0.5);
						change_mod_param(5,0.5671641791044776);
						change_mod_param(6,0.6567164179104478);
						change_delay_model("DelayRe201");change_delay_onoff(true);
						change_delay_param(0,0.09583300352096558);
						change_delay_param(1,0.1437229961156845);
						change_delay_param(2,0.6712870001792908);
						change_delay_param(3,0.5038350224494934);
						change_delay_param(4,0);
						change_reverb_onoff(false);
						change_reverb_param(0,0.35932204127311707);
						change_reverb_param(1,0.5066586136817932);
						change_reverb_param(2,0.4178571403026581);
						change_reverb_param(3,0.30084747076034546);
						change_reverb_param(4,0.602287232875824);
						change_reverb_param(5,0.5941176414489746);
						change_reverb_param(6,0);
						break;
case 35:  /*  Jazz Chorus - M-Vave position:  37  */  
						change_noisegate_onoff(false);
						change_noisegate_param(0,0.2181818187236786);
						change_noisegate_param(1,0.06487986445426941);
						change_comp_model("BlueComp");change_comp_onoff(true);
						change_comp_param(0,0.5327230095863342);
						change_comp_param(1,0.615327000617981);
						change_comp_param(2,0.4000000059604645);
						change_comp_param(3,0.6000000238418579);
						change_drive_model("Booster");change_drive_onoff(false);
						change_drive_param(0,0.7670454382896423);
						change_amp_model("RolandJC120");
						change_amp_param(0,0.28652599453926086);
						change_amp_param(1,0.9842990040779114);
						change_amp_param(2,0.3525499999523163);
						change_amp_param(3,0.8382710218429565);
						change_amp_param(4,0.6499999761581421);
						change_mod_model("GuitarEQ6");
						change_mod_onoff(true);
						change_mod_param(0,0.8208955223880596);
						change_mod_param(1,0.19651748884969686);
						change_mod_param(2,0.36815916602291276);
						change_mod_param(3,0.5);
						change_mod_param(4,0.5);
						change_mod_param(5,0.5671641791044776);
						change_mod_param(6,0.6567164179104478);
						change_delay_model("DelayMono");change_delay_onoff(true);
						change_delay_param(0,0.47319701313972473);
						change_delay_param(1,0);
						change_delay_param(2,0.658594012260437);
						change_delay_param(3,0.30000001192092896);
						change_delay_param(4,0);
						change_reverb_onoff(true);
						change_reverb_param(0,0.2857142984867096);
						change_reverb_param(1,0.4083535671234131);
						change_reverb_param(2,0.28948888182640076);
						change_reverb_param(3,0.38831719756126404);
						change_reverb_param(4,0.5821428298950195);
						change_reverb_param(5,0.6500000357627869);
						change_reverb_param(6,0.20000000298023224);
						break;

case 36:  /*  Smooth Jazz - M-Vave position:  38  */  
						change_noisegate_onoff(false);
						change_noisegate_param(0,0.2181818187236786);
						change_noisegate_param(1,0.06487986445426941);
						change_comp_model("BlueComp");change_comp_onoff(true);
						change_comp_param(0,0.7463269829750061);
						change_comp_param(1,0.47999998927116394);
						change_comp_param(2,0.4000000059604645);
						change_comp_param(3,0.6000000238418579);
						change_drive_model("Booster");change_drive_onoff(false);
						change_drive_param(0,0.7670454382896423);
						change_amp_model("Bassman");
						change_amp_param(0,0.3732079863548279);
						change_amp_param(1,0.6506069898605347);
						change_amp_param(2,0.5805780291557312);
						change_amp_param(3,0.6000000238418579);
						change_amp_param(4,0.4972172677516937);
						change_mod_model("GuitarEQ6");
						change_mod_onoff(true);
						change_mod_param(0,0.8208955223880596);
						change_mod_param(1,0.19651748884969686);
						change_mod_param(2,0.36815916602291276);
						change_mod_param(3,0.5);
						change_mod_param(4,0.5);
						change_mod_param(5,0.5671641791044776);
						change_mod_param(6,0.6567164179104478);
						change_delay_model("DelayMono");change_delay_onoff(false);
						change_delay_param(0,0.38177722692489624);
						change_delay_param(1,0.39830508828163147);
						change_delay_param(2,0.7363313436508179);
						change_delay_param(3,0.6000000238418579);
						change_delay_param(4,0);
						change_reverb_onoff(true);
						change_reverb_param(0,0.33571431040763855);
						change_reverb_param(1,0.6438256502151489);
						change_reverb_param(2,0.5208232402801514);
						change_reverb_param(3,0.28129544854164124);
						change_reverb_param(4,0.4695378243923187);
						change_reverb_param(5,0.45546218752861023);
						change_reverb_param(6,0.6000000238418579);
						break;

case 37:  /*  Clean jazz - M-Vave position:  39  */  
						change_noisegate_onoff(true);
						change_noisegate_param(0,0.14809900522232056);
						change_noisegate_param(1,0);
						change_comp_model("LA2AComp");change_comp_onoff(false);
						change_comp_param(0,0);
						change_comp_param(1,0.7126444578170776);
						change_comp_param(2,0.7248061895370483);
						change_drive_model("Booster");change_drive_onoff(false);
						change_drive_param(0,0.7670454382896423);
						change_amp_model("Bassman");
						change_amp_param(0,0.5391340255737305);
						change_amp_param(1,0.6000000238418579);
						change_amp_param(2,0.49923598766326904);
						change_amp_param(3,1);
						change_amp_param(4,0.4972172677516937);
						change_mod_model("GuitarEQ6");
						change_mod_onoff(true);
						change_mod_param(0,0.8208955223880596);
						change_mod_param(1,0.19651748884969686);
						change_mod_param(2,0.36815916602291276);
						change_mod_param(3,0.5);
						change_mod_param(4,0.5);
						change_mod_param(5,0.5671641791044776);
						change_mod_param(6,0.6567164179104478);
						change_delay_model("DelayMono");change_delay_onoff(false);
						change_delay_param(0,0.38177722692489624);
						change_delay_param(1,0.39830508828163147);
						change_delay_param(2,0.7363313436508179);
						change_delay_param(3,0.6000000238418579);
						change_delay_param(4,0);
						change_reverb_onoff(true);
						change_reverb_param(0,0.2857142984867096);
						change_reverb_param(1,0.4083535671234131);
						change_reverb_param(2,0.28948888182640076);
						change_reverb_param(3,0.38831719756126404);
						change_reverb_param(4,0.5821428298950195);
						change_reverb_param(5,0.6500000357627869);
						change_reverb_param(6,0.20000000298023224);
						break;

case 38:  /*  JC JAZZ - M-Vave position:  40  */  
						change_noisegate_onoff(true);
						change_noisegate_param(0,0.8193359971046448);
						change_noisegate_param(1,1);
						change_comp_model("LA2AComp");change_comp_onoff(true);
						change_comp_param(0,0);
						change_comp_param(1,0.75);
						change_comp_param(2,0.4000000059604645);
						change_drive_model("Booster");change_drive_onoff(false);
						change_drive_param(0,0.7670454382896423);
						change_amp_model("RolandJC120");
						change_amp_param(0,0.6000000238418579);
						change_amp_param(1,0.5);
						change_amp_param(2,0.800000011920929);
						change_amp_param(3,0.6000000238418579);
						change_amp_param(4,0.6499999761581421);
						change_mod_model("GuitarEQ6");
						change_mod_onoff(true);
						change_mod_param(0,0.8208955223880596);
						change_mod_param(1,0.19651748884969686);
						change_mod_param(2,0.36815916602291276);
						change_mod_param(3,0.5);
						change_mod_param(4,0.5);
						change_mod_param(5,0.5671641791044776);
						change_mod_param(6,0.6567164179104478);
						change_delay_model("DelayMono");change_delay_onoff(false);
						change_delay_param(0,0.38177722692489624);
						change_delay_param(1,0.39830508828163147);
						change_delay_param(2,0.7363313436508179);
						change_delay_param(3,0.6000000238418579);
						change_delay_param(4,0);
						change_reverb_onoff(true);
						change_reverb_param(0,0.2857142984867096);
						change_reverb_param(1,0.4083535671234131);
						change_reverb_param(2,0.28948888182640076);
						change_reverb_param(3,0.38831719756126404);
						change_reverb_param(4,0.5821428298950195);
						change_reverb_param(5,0.6500000357627869);
						change_reverb_param(6,0.20000000298023224);
						break;

case 39:  /*  Warm jazz clean - M-Vave position:  41  */  
						change_noisegate_onoff(false);
						change_noisegate_param(0,0.06250855326652527);
						change_noisegate_param(1,0);
						change_noisegate_param(2,0);
						change_comp_model("BBEOpticalComp");change_comp_onoff(true);
						change_comp_param(0,0.9520958065986633);
						change_comp_param(1,0.39101797342300415);
						change_comp_param(2,0);
						change_drive_model("DistortionTS9");change_drive_onoff(false);
						change_drive_param(0,0.1202365830540657);
						change_drive_param(1,0.3807947039604187);
						change_drive_param(2,0.5102882981300354);
						change_drive_param(3,0);
						change_amp_model("Twin");
						change_amp_param(0,0.9740259647369385);
						change_amp_param(1,0.5076923370361328);
						change_amp_param(2,0.5440759062767029);
						change_amp_param(3,0.3876703679561615);
						change_amp_param(4,1);
						change_mod_model("GuitarEQ6");
						change_mod_onoff(true);
						change_mod_param(0,0.8208955223880596);
						change_mod_param(1,0.19651748884969686);
						change_mod_param(2,0.36815916602291276);
						change_mod_param(3,0.5);
						change_mod_param(4,0.5);
						change_mod_param(5,0.5671641791044776);
						change_mod_param(6,0.6567164179104478);
						change_delay_model("DelayMono");change_delay_onoff(true);
						change_delay_param(0,0.06119631603360176);
						change_delay_param(1,0.2544043958187103);
						change_delay_param(2,0.383419394493103);
						change_delay_param(3,0.6000000238418579);
						change_delay_param(4,0);
						change_reverb_onoff(true);
						change_reverb_param(0,0.2857142984867096);
						change_reverb_param(1,0.4083535671234131);
						change_reverb_param(2,0.5642856955528259);
						change_reverb_param(3,0.3214285671710968);
						change_reverb_param(4,0.5821428298950195);
						change_reverb_param(5,0.8369394540786743);
						change_reverb_param(6,0.20000000298023224);
						break;

case 40:  /*  Warm Jazz  - M-Vave position:  42  */  
						change_noisegate_onoff(false);
						change_noisegate_param(0,0.2181818187236786);
						change_noisegate_param(1,0.06487986445426941);
						change_comp_model("LA2AComp");change_comp_onoff(false);
						change_comp_param(0,0);
						change_comp_param(1,0.7126444578170776);
						change_comp_param(2,0.7248061895370483);
						change_drive_model("Booster");change_drive_onoff(false);
						change_drive_param(0,0.7670454382896423);
						change_amp_model("Twin");
						change_amp_param(0,0.43007799983024597);
						change_amp_param(1,0.3616490066051483);
						change_amp_param(2,0.7145100235939026);
						change_amp_param(3,0.7628909945487976);
						change_amp_param(4,0.5799999833106995);
						change_mod_model("GuitarEQ6");
						change_mod_onoff(true);
						change_mod_param(0,0.8208955223880596);
						change_mod_param(1,0.19651748884969686);
						change_mod_param(2,0.36815916602291276);
						change_mod_param(3,0.5);
						change_mod_param(4,0.5);
						change_mod_param(5,0.5671641791044776);
						change_mod_param(6,0.6567164179104478);
						change_delay_model("DelayRe201");change_delay_onoff(true);
						change_delay_param(0,0.10351599752902985);
						change_delay_param(1,0.13631300628185272);
						change_delay_param(2,0.11625900119543076);
						change_delay_param(3,0.8659489750862122);
						change_delay_param(4,0);
						change_reverb_onoff(true);
						change_reverb_param(0,0.33571431040763855);
						change_reverb_param(1,0.6438256502151489);
						change_reverb_param(2,0.5208232402801514);
						change_reverb_param(3,0.28129544854164124);
						change_reverb_param(4,0.4695378243923187);
						change_reverb_param(5,0.45546218752861023);
						change_reverb_param(6,0.6000000238418579);
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
