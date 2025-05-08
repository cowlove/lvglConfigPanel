#include "lvglConfPanel.h"

JStuff j; // Jstuff seems to disable to makeEspArduino build dunno 

//NO ReliableTcpClient client("0.0.0.0", 4444);
ReliableStreamESPNow client("CP");
ConfPanelTransportScreen cpt(&client);

void setup() {
  espNowMux.defaultChannel = 4;
  espNowMux.firstInit = true;
  Serial.begin(115200/*, SERIAL_8N1*/); 
  //j.mqtt.active = j.jw.enabled = false;
  panel_setup();
  cpt.createWelcomeTile();
  //lv_demo_widgets();
}

void loop() {
  //j.run();
  cpt.run();
  lv_timer_handler();
  lv_tick_inc(10);
  delay(10);
}
