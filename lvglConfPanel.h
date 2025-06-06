#pragma once

// TODO move this to library, name it more appropriately 
 
#ifndef CSIM
// Uncomment one of the following display boards:
//#include "elecrow5.h"     // use ESP32S3-DEV board, OPI-PSRAM, huge app partition
//#include "elecrow23.h"      // use ESP32-WROOM-DA board type, huge app partition, needs manual boot button 
#include "elecrow7.h"    // use ESP32S3-DEV board, OPI-PSRAM, huge app partition
//#include "waveshare43.h"
//#include "lilgoRGB.h"
#include <Arduino.h>
#include "WiFiClient.h"
#endif

#ifdef CSIM
#include "lvgl.h"
#include "demos/lv_demos.h"

#include "driver_backends.h"
#include "simulator_util.h"
#include "simulator_settings.h"

#define ESP_PANEL_LCD_H_RES 800
#define ESP_PANEL_LCD_V_RES 480
extern simulator_settings_t settings;

static inline void panel_setup() {
  lv_init();
  driver_backends_register();
  settings.window_height = ESP_PANEL_LCD_H_RES;
  settings.window_width = ESP_PANEL_LCD_V_RES; 
  driver_backends_init_backend((char *)"X11");
  driver_backends_init_backend((char *)"EVDEV");
}
#endif

#ifndef GIT_VERSION
#define GIT_VERSION "gitversion"
#endif
#include "jimlib.h"
#include "espNowMux.h"
#include "reliableStream.h"
#include "confPanel.h"
#include "serialLog.h"

// enable font in ~/Arduino/libraries/lv_conf.h ie: #define LV_FONT_MONTSERRAT_42 1
static const lv_font_t *default_font = &lv_font_montserrat_32;
static const int row_height = 35;

#include <vector>
#include <string>
using namespace std;

#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

class ConfPanel {
public:
  int index;
  ConfPanel(int idx, const string cfg, lv_obj_t *tile) {
    index = idx;
    parseConfig(cfg.c_str());
    create(tile);
  }
  struct ConfPanelParam {
    ConfPanel *parent;
    int index;
    lv_obj_t *sel;
    lv_obj_t *val;
    char label[32] = { 0 };
    char fmt[32] = { 0 };
    float inc;
    float def;
    float min;
    float max;
    int wrap;
    int flags;
    char enumlabels[32] = { 0 };
    float current;
    bool selected = false;
    bool changed = false;
  };
  int selected_btn = -1;
  vector<ConfPanelParam> rows;

  lv_obj_t *multBut = NULL;
  void heartbeat(bool reset) {
    if (multBut == NULL)
      return;
    if (reset) {
      lv_obj_set_style_bg_color(multBut, lv_palette_main(LV_PALETTE_BLUE), LV_PART_MAIN);
    } else {
      lv_color_t c = lv_obj_get_style_bg_color(multBut, LV_PART_MAIN);
      c = lv_color_lighten(c, 60);
      lv_obj_set_style_bg_color(multBut, c, LV_PART_MAIN);
    }
  }

  void run() {
    heartbeat(false);
  }
  void addConfig(const char *buf) {
    vector<string> words = split(buf, ",");
    if (words.size() == 9) {
      ConfPanelParam n;
      strncpy(n.label, words[0].c_str(), sizeof(n.label));
      strncpy(n.fmt, words[1].c_str(), sizeof(n.fmt));
      sscanf(words[2].c_str(), "%f", &n.inc);
      sscanf(words[3].c_str(), "%f", &n.min);
      sscanf(words[4].c_str(), "%f", &n.max);
      sscanf(words[5].c_str(), "%f", &n.def);
      sscanf(words[6].c_str(), "%d", &n.wrap);
      strncpy(n.enumlabels, words[7].c_str(), sizeof(n.enumlabels));
      sscanf(words[8].c_str(), "%d", &n.flags);
      if (strchr(n.enumlabels, '/') != NULL) {
        n.inc = 1;
        n.min = 0;
        n.max = split(n.enumlabels, "/").size() - 1;
      }
      n.current = n.def;
      n.parent = this;
      n.index = rows.size();
      rows.push_back(n);
    }
  }

  void parseConfig(const char *config) {
    vector<string> lines = split(config, "\n");
    for (auto l = lines.begin(); l != lines.end(); l++) {
      addConfig((*l).c_str());
    }
  }

  void selectButton(int idx) {
    if (idx >= 0 && idx < rows.size()) {
      // configuration panel mode?  deselect prev selection if we're selecting a different param
      if (isConfigPanel && selected_btn >= 0 && selected_btn < rows.size() && selected_btn != idx) {
        lv_obj_set_style_bg_color(rows[selected_btn].sel, lv_palette_main(LV_PALETTE_BLUE), LV_PART_MAIN);
        rows[selected_btn].selected = false;
      }
      // toggle selected param
      if (rows[idx].selected == true) {
        selected_btn = -1;
        lv_obj_set_style_bg_color(rows[idx].sel, lv_palette_main(LV_PALETTE_BLUE), LV_PART_MAIN);
        rows[idx].selected = false;
      } else {
        selected_btn = idx;
        lv_obj_set_style_bg_color(rows[idx].sel, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);
        rows[idx].selected = true;
      }
    }
    // reset multiplier button to 1X
    if (multBut != NULL) {

      lv_obj_t *l = lv_obj_get_child(multBut, 0);
      if (l != NULL)
        lv_label_set_text(l, "1X");
    }
  }

  void multiplierButton() {
    lv_obj_t *l = lv_obj_get_child(multBut, 0);
    if (l == NULL)
      return;
    //string s("HEY!");
    //espNowMux.send("g5", (uint8_t *)s.c_str(), s.length());
    const char *t = lv_label_get_text(l);
    int v;
    if (t != NULL && sscanf(t, "%dX", &v) == 1) {
      v *= 10;
      if (v > 100) v = 1;
      lv_label_set_text_fmt(l, "%dX", v);
    }
  }

  void onRecv(const string &buf) {
    //Serial.printf("processing line: %s\n", buf.c_str());
    vector<string> lines = split(buf.c_str(), "\n");
    for (string s : lines) {
      //Serial.printf("processing line: %s\n", s.c_str());
      int i1, i2;
      float v;
      if (sscanf(s.c_str(), "VALUE %d %d %f", &i1, &i2, &v) == 3 && i1 == index && i2 >= 0 && i2 < rows.size()) {
        ConfPanelParam *p = &rows[i2];
        p->current = v;
        paramIncrement(i2, 0);
      }
    }
    if (buf.length() > 0)
      heartbeat(true);
  }

  string readData() {
    string rval;
    for (int i = 0; i < rows.size(); i++) {
      if (rows[i].changed) {
        char buf[128];
        snprintf(buf, sizeof(buf), "SET %d %d %f\n", index, i, rows[i].current);
        rval += buf;
        rows[i].changed = false;
      }
    }
    return rval;
  }

  void paramIncrement(int idx, float dir) {
    if (idx < 0 || idx >= rows.size())
      return;

    ConfPanelParam *p = &rows[idx];

    int mult = 1;
    if (multBut != NULL) {
      lv_obj_t *l = lv_obj_get_child(multBut, 0);
      if (l == NULL)
        return;
      const char *t = lv_label_get_text(l);
      if (t != NULL)
        sscanf(t, "%dX", &mult);
    }

    float val = p->current + p->inc * dir * mult;
    if (p->max > p->min) {
      if (p->wrap) {
        if (val > p->max) val = p->min;
        if (val < p->min) val = p->max;
      } else {
        if (val > p->max) val = p->max;
        if (val < p->min) val = p->min;
      }
    }
    p->changed = (p->current != val);
    p->current = val;

    if (strchr(p->enumlabels, '/') == NULL) {
      char buf[32];
      snprintf(buf, sizeof(buf), p->fmt, val);
      lv_label_set_text(p->val, buf);
    } else {
      char buf[128];
      strncpy(buf, p->enumlabels, sizeof(buf));
      int idx = val;
      char *w;
      for (w = strtok(buf, "/"); w != NULL && idx > 0; w = strtok(NULL, "/"), idx--) {
      }
      if (w != NULL) {
        lv_label_set_text(p->val, w);
      }
    }
  }

  static const int max_rows = 399;
  lv_coord_t col_dsc[4] = { row_height, LV_GRID_FR(2), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
  lv_coord_t row_dsc[max_rows];

  bool isConfigPanel = true;

  void create(lv_obj_t *parent) {
    for (int r = 0; r < rows.size(); r++) {
      row_dsc[r] = row_height;
    }
    row_dsc[rows.size()] = LV_GRID_TEMPLATE_LAST;

    lv_obj_t *cont, *obj, *label;

    if (isConfigPanel) {
      /*Create a container with grid*/
      cont = lv_obj_create(parent);
      lv_obj_set_style_grid_column_dsc_array(cont, col_dsc, 0);
      lv_obj_set_style_grid_row_dsc_array(cont, row_dsc, 0);
      int buttonRowHeightPercent = row_height * 100 / ESP_PANEL_LCD_V_RES + 6;
      Serial.printf("Row height percent %d\n", buttonRowHeightPercent);
      lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100 - buttonRowHeightPercent));
      //lv_obj_center(cont);
      lv_obj_set_layout(cont, LV_LAYOUT_GRID);

      lv_obj_t *cont2 = lv_obj_create(parent);
      lv_obj_set_size(cont2, LV_PCT(100), LV_PCT(buttonRowHeightPercent));
      lv_obj_align_to(cont2, cont, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
      lv_obj_set_scrollbar_mode(cont2, LV_SCROLLBAR_MODE_OFF);
      lv_obj_clear_flag(cont2, LV_OBJ_FLAG_SCROLLABLE);

      obj = lv_btn_create(cont2);
      label = lv_label_create(obj);
      lv_label_set_text_fmt(label, "DECREASE");
      lv_obj_set_style_text_font(label, default_font, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_center(label);
      lv_obj_align_to(obj, cont2, LV_ALIGN_LEFT_MID, 0, 0);
      //set_btn_blue(obj);
      lv_obj_add_event_cb(obj, btn_event_dec, LV_EVENT_CLICKED, this);

      obj = lv_btn_create(cont2);
      label = lv_label_create(obj);
      lv_label_set_text_fmt(label, "1X");
      lv_obj_set_style_text_font(label, default_font, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_center(label);
      lv_obj_align_to(obj, cont2, LV_ALIGN_CENTER, 0, 0);
      //set_btn_blue(obj);
      lv_obj_add_event_cb(obj, btn_event_1x, LV_EVENT_CLICKED, this);
      multBut = obj;

      obj = lv_btn_create(cont2);
      label = lv_label_create(obj);
      lv_label_set_text_fmt(label, "INCREASE");
      lv_obj_set_style_text_font(label, default_font, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_center(label);
      lv_obj_align_to(obj, cont2, LV_ALIGN_RIGHT_MID, 0, 0);
      //set_btn_blue(obj);
      lv_obj_add_event_cb(obj, btn_event_inc, LV_EVENT_CLICKED, this);

    } else {
      cont = lv_obj_create(parent);
      lv_obj_set_style_grid_column_dsc_array(cont, col_dsc, 0);
      lv_obj_set_style_grid_row_dsc_array(cont, row_dsc, 0);
      lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
      //lv_obj_center(cont);
      lv_obj_set_layout(cont, LV_LAYOUT_GRID);
    }

    for (int i = 0; i < rows.size(); i++) {
      ConfPanelParam *p = &rows[i];

      obj = lv_btn_create(cont);
      lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 0, 1,
                           LV_GRID_ALIGN_STRETCH, i, 1);
      lv_obj_add_event_cb(obj, btn_event_sel, LV_EVENT_CLICKED, &rows[i]);
      //set_btn_blue(obj);
      rows[i].sel = obj;

      obj = lv_label_create(cont);
      lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 1, 1,
                           LV_GRID_ALIGN_END, i, 1);
      lv_label_set_text(obj, p->label);
      lv_obj_set_style_text_font(obj, default_font, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
      lv_obj_add_event_cb(obj, btn_event_sel, LV_EVENT_CLICKED, &rows[i]);

      obj = lv_label_create(cont);
      lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 2, 1,
                           LV_GRID_ALIGN_END, i, 1);
      lv_obj_set_style_text_font(obj, default_font, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
      lv_obj_add_event_cb(obj, btn_event_sel, LV_EVENT_CLICKED, &rows[i]);
      rows[i].val = obj;
      rows[i].current = rows[i].def;
      paramIncrement(i, 0);
    }
  }
  static void btn_event_1x(lv_event_t *e) {
    ConfPanel *cp = (ConfPanel *)lv_event_get_user_data(e);
    cp->multiplierButton();
  }

  static void btn_event_sel(lv_event_t *e) {
    ConfPanel::ConfPanelParam *p = (ConfPanel::ConfPanelParam *)lv_event_get_user_data(e);
    p->parent->selectButton(p->index);
  }

  static void btn_event_inc(lv_event_t *e) {
    ConfPanel *cp = (ConfPanel *)lv_event_get_user_data(e);
    cp->paramIncrement(cp->selected_btn, +1);
  }

  static void btn_event_dec(lv_event_t *e) {
    ConfPanel *cp = (ConfPanel *)lv_event_get_user_data(e);
    cp->paramIncrement(cp->selected_btn, -1);
  }
};

class DispPanel : public ConfPanel {
public:
  DispPanel(int i, const string conf, lv_obj_t *tile)
    : ConfPanel(i, conf, tile) {
    isConfigPanel = false;
  }
};

class ConfPanelTransportScreen {
  WiFiUDP udp;
  ReliableStreamInterface *stream;
  bool parsingSchema = false;
  string schema;
  int schema_idx = 0;
  lv_obj_t *tileview = NULL;
  LineBuffer lb;
  vector<ConfPanel *> panels;
  bool initialized = false;
  uint32_t lastSchemaRequestTime = -1000;
  HzTimer schemaRequestTimer, runTimer;
public:
  ConfPanelTransportScreen(ReliableStreamInterface *s)
    : stream(s), schemaRequestTimer(3), runTimer(5) {}
  void run() {
    string s;
    for (auto p : panels) {
      s = p->readData();
      stream->write(s);
    }
    while (stream->read(s)) {
      onRecv(s.c_str(), s.length());
    }
    if (runTimer.tick()) {
      for (auto p : panels) {
        p->run();
      }
    }
    if (panels.size() == 0 && schemaRequestTimer.tick()) {
      printf("Requesting schema...\n"); 
      stream->write("SCHEMA\n");
      lastSchemaRequestTime = millis();
    }
    if (!initialized) {
      if (WiFi.isConnected())
        udp.begin(4444);
      initialized = true;
    }
    if (udp.parsePacket()) {
      unsigned char buf[1024];
      int n = udp.read(buf, sizeof(buf));
      if (n > 0) {
        string line;
        line.assign((char *)buf, n);
        stream->begin(line.c_str(), 4444);
        Serial.printf("Discovered client %s\n", line.c_str());
      }
    }
  }

  int expectedSchemaLength = 0;
  int tileCount = 0;
  int schemaFlags = 0;
  lv_obj_t *createTile()  {
      if (tileview == NULL) {
        tileview = lv_tileview_create(lv_scr_act());
        lv_obj_set_size(tileview, LV_PCT(100), LV_PCT(100));
        lv_obj_set_scrollbar_mode(tileview, LV_SCROLLBAR_MODE_OFF);
      }
      return lv_tileview_add_tile(tileview, tileCount++, 0, (lv_dir_t)(LV_DIR_HOR | LV_DIR_BOTTOM));
  }
  lv_obj_t *welcomeLabel = NULL;
  static void btn_reset(lv_event_t *e) {
    ESP.restart();
  }
  void createWelcomeTile()  { 
      lv_obj_t *tile = createTile();
      lv_obj_t *label = lv_label_create(tile);
      lv_label_set_text_fmt(label, "WELCOME JIMMY! xx");
      lv_obj_set_style_text_font(label, default_font, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_text_color(label, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);
      lv_obj_center(label);
      welcomeLabel = label;

      lv_obj_t *but = lv_btn_create(tile);
      label = lv_label_create(but);
      lv_label_set_text_fmt(label, "RESET");
      lv_obj_set_style_text_font(label, default_font, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_center(label);
 
      lv_obj_add_event_cb(but, btn_reset, LV_EVENT_CLICKED, 0);

      //lv_obj_align_to(obj, cont2, LV_ALIGN_RIGHT_MID, 0, 0);
  }
  void onRecv(const char *buf, int n) {
    string x;
    x.assign(buf, n);
    //printf("processData() %d bytes:\n%s\n", n, x.c_str());
    lb.add((char *)buf, n, [this](const char *l) {
      //printf("Got line: %s\n", l);
      if (parsingSchema) {
        printf("parsing schema %d: %s\n", schema_idx, l);
        if (strcmp(l, "END") == 0)
          parsingSchema = false;
        if (strcmp(l, "END") == 0 && panels.size() == schema_idx) {
          Serial.printf("creating schema %d:\n%s\n", schema_idx, schema.c_str());
          vector<string> slines = split(schema.c_str(), "\n");
          if (slines.size() == expectedSchemaLength) {
            panels.push_back(new ConfPanel(schema_idx, schema, createTile()));
            if (schemaFlags & 0x1) {
              createTile();
            }
            Serial.printf("Received valid schema length %d\n", slines.size());
            if (welcomeLabel != NULL)
              lv_label_set_text_fmt(welcomeLabel, "CONNECTED (%.3f sec)", millis() / 1000.0);
          } else {
            Serial.printf("Received schema length %d != expected length %d, discarding\n", slines.size(), expectedSchemaLength);
            stream->write("SCHEMA\n");
            lastSchemaRequestTime = millis();
          }
          parsingSchema = false;
        }
        schema += string(l) + "\n";
      } else {
        if (sscanf(l, "SCHEMA %d %d %d", &schema_idx, &expectedSchemaLength, &schemaFlags) == 3) {
          parsingSchema = true;
          schema = "";
        }
        for (auto p : panels) {
          p->onRecv(l);
        }
      }
    });
  }
};
