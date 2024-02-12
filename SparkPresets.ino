void ChangeTCPreset() {
  change_noisegate_onoff(false);
  change_comp_model("LA2AComp");
  change_comp_onoff(true);
  change_drive_model("DistortionTS9");
  change_drive_onoff(false);
  change_amp_model("ADClean");
  change_mod_model("Flanger");
  change_mod_onoff(true);
  change_delay_model("VintageDelay");
  change_delay_onoff(true);
  change_reverb_onoff(true);
}