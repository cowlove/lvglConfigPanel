#include <Arduino.h>
#include <lvgl.h>
#include "WiFiClient.h"

#define WAVESHARE // use to switch between WaveShare4.3 and LilyGO T-RGB 2.1

#ifdef WAVESHARE
#include <ESP_Panel_Library.h>
#include <ESP_IOExpander_Library.h>

/* LVGL porting configurations */
#define LVGL_TICK_PERIOD_MS     (2)
#define LVGL_TASK_MAX_DELAY_MS  (20)
#define LVGL_TASK_MIN_DELAY_MS  (1)
#define LVGL_TASK_STACK_SIZE    (4 * 1024)
#define LVGL_TASK_PRIORITY      (2)
#define LVGL_BUF_SIZE           (ESP_PANEL_LCD_H_RES * 20)

ESP_Panel *panel = NULL;

#if ESP_PANEL_LCD_BUS_TYPE == ESP_PANEL_BUS_TYPE_RGB
/* Display flushing */
void lvgl_port_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    panel->getLcd()->drawBitmap(area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_p);
    lv_disp_flush_ready(disp);
}
#else
/* Display flushing */
void lvgl_port_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    panel->getLcd()->drawBitmap(area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_p);
}

bool notify_lvgl_flush_ready(void *user_ctx)
{
    lv_disp_drv_t *disp_driver = (lv_disp_drv_t *)user_ctx;
    lv_disp_flush_ready(disp_driver);
    return false;
}
#endif /* ESP_PANEL_LCD_BUS_TYPE */

#if ESP_PANEL_USE_LCD_TOUCH
/* Read the touchpad */
void lvgl_port_tp_read(lv_indev_drv_t * indev, lv_indev_data_t * data)
{
    panel->getLcdTouch()->readData();

    bool touched = panel->getLcdTouch()->getTouchState();
    if(!touched) {
        data->state = LV_INDEV_STATE_REL;
    } else {
        TouchPoint point = panel->getLcdTouch()->getPoint();

        data->state = LV_INDEV_STATE_PR;
        /*Set the coordinates*/
        data->point.x = point.x;
        data->point.y = point.y;

        //Serial.printf("Touch point: x %d, y %d\n", point.x, point.y);
    }
}
#endif

void panel_setup() { 
    panel = new ESP_Panel();
    lv_init();
    static lv_disp_draw_buf_t draw_buf;
    uint8_t *buf = (uint8_t *)heap_caps_calloc(1, LVGL_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_INTERNAL);
    assert(buf);
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, LVGL_BUF_SIZE);

    /* Initialize the display device */
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    /* Change the following line to your display resolution */
    disp_drv.hor_res = ESP_PANEL_LCD_H_RES;
    disp_drv.ver_res = ESP_PANEL_LCD_V_RES;
    disp_drv.flush_cb = lvgl_port_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

#if ESP_PANEL_USE_LCD_TOUCH
    /* Initialize the input device */
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = lvgl_port_tp_read;
    lv_indev_drv_register(&indev_drv);
#endif

#if defined(ESP_PANEL_BOARD_ESP32_S3_LCD_EV_BOARD) || defined(ESP_PANEL_BOARD_ESP32_S3_KORVO_2)
    ESP_IOExpander *expander = new ESP_IOExpander_TCA95xx_8bit(ESP_PANEL_LCD_TOUCH_BUS_HOST_ID, ESP_IO_EXPANDER_I2C_TCA9554_ADDRESS_000, ESP_PANEL_LCD_TOUCH_I2C_IO_SCL, ESP_PANEL_LCD_TOUCH_I2C_IO_SDA);
    expander->init();
    expander->begin();
    panel->addIOExpander(expander);
#endif

    panel->init();
#if ESP_PANEL_LCD_BUS_TYPE != ESP_PANEL_BUS_TYPE_RGB
    panel->getLcd()->setCallback(notify_lvgl_flush_ready, &disp_drv);
#endif
    panel->begin();
}
#else // #ifdef WAVESHARE

#include <LilyGo_RGBPanel.h>
#include <LV_Helper.h>

LilyGo_RGBPanel panel;

void panel_setup() { 
   if (!panel.begin()) {
        while (1) {
            Serial.println("Error, failed to initialize T-RGB"); delay(1000);
        }
    }
    // Call lvgl initialization
    beginLvglHelper(panel);
    panel.setBrightness(16);
}
#endif

// enable font in ~/Arduino/libraries/lvgl/lv_conf_template.h
static const lv_font_t *default_font =  &lv_font_montserrat_42;

// callback for monitor value labels to toggle the adjoining checkbox 
void select_monitor_event(lv_event_t *e) {
  Serial.println("select_monitor_event()");
  lv_obj_t *obj = (lv_obj_t *)lv_event_get_user_data(e);
  if (lv_obj_has_state(obj, LV_STATE_CHECKED)) 
    lv_obj_clear_state(obj, LV_STATE_CHECKED); /*Make the checkbox unchecked*/
  else
    lv_obj_add_state(obj, LV_STATE_CHECKED);   /*Make the checkbox checked*/
}

void mon_menu_create(lv_obj_t *parent)
{
    static lv_coord_t col_dsc[] = {50, LV_GRID_FR(2), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[50];

    int nr_rows = 20;
    for (int r = 0; r < nr_rows; r++) { 
	     row_dsc[r] = 50;
    }
    row_dsc[nr_rows] = LV_GRID_TEMPLATE_LAST;

    /*Create a container with grid*/
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_style_grid_column_dsc_array(cont, col_dsc, 0);
    lv_obj_set_style_grid_row_dsc_array(cont, row_dsc, 0);
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    //lv_obj_center(cont);
    lv_obj_set_layout(cont, LV_LAYOUT_GRID);  

    lv_obj_t * label;
    lv_obj_t * obj;

    for(int row = 0; row < nr_rows; row++) {
        //uint8_t col = i % 3;
        //uint8_t row = i;

        obj = lv_checkbox_create(cont); // docs say style padding value can be used to make checkbox bigger
        lv_checkbox_set_text(obj, "");
        lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 0, 1,
                             LV_GRID_ALIGN_CENTER, row, 1);
        lv_obj_t *cb = obj;

        obj = lv_label_create(cont);
        lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 1, 1,
                             LV_GRID_ALIGN_END, row, 1);
        lv_label_set_text_fmt(obj, "DATAPOINT %d", row);
        lv_obj_set_style_text_font(obj, default_font, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(obj, select_monitor_event, LV_EVENT_CLICKED, (void *)cb);  

        obj = lv_label_create(cont);
        lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 2, 1,
                             LV_GRID_ALIGN_END, row, 1);
        lv_label_set_text_fmt(obj, "VAL%d", row);
        lv_obj_set_style_text_font(obj, default_font, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(obj, select_monitor_event, LV_EVENT_CLICKED, (void *)cb);  
    }
}


void set_btn_red(lv_obj_t *b) { 
  static lv_style_t style_btn_red2;
  lv_style_init(&style_btn_red2);
  lv_style_set_bg_color(&style_btn_red2, lv_color_make(200, 0, 0));
  lv_style_set_bg_opa(&style_btn_red2, LV_OPA_COVER);
  lv_obj_add_style(b, &style_btn_red2, 0);
}

void set_btn_blue(lv_obj_t *b) { 
  static lv_style_t style_btn_red2;
  lv_style_init(&style_btn_red2);
  lv_style_set_bg_color(&style_btn_red2, lv_color_make(0, 0, 200));
  lv_style_set_bg_opa(&style_btn_red2, LV_OPA_COVER);
  lv_obj_add_style(b, &style_btn_red2, 0);
}
void set_btn_blue2(lv_obj_t *b) { 
  static lv_style_t style_btn_red2;
  lv_style_init(&style_btn_red2);
  lv_style_set_bg_color(&style_btn_red2, lv_color_make(150, 150, 220));
  lv_style_set_bg_opa(&style_btn_red2, LV_OPA_COVER);
  lv_obj_add_style(b, &style_btn_red2, 0);
}

#include <vector>
#include <string>;
using namespace std;

#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>

// trim from start
inline std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end
inline std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

// trim from both ends
inline std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
}

vector<string> split(const char *line, const char *delim) {
  char *buf = strdup(line); 
  std::vector<string> rval;
  for(char *w = strtok(buf, delim); w != NULL; w = strtok(NULL, delim)) {
    string ws(w);
    trim(ws);
    rval.push_back(ws);
  }
  free(buf);
  return rval;
}


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
    char label[32] = {0};
    char fmt[32] = {0};
    float inc;
    float def;
    float min;
    float max;
    int wrap;
    char enumlabels[32] = {0};
    float current;
    bool selected = false;
    bool changed = false;
  };
  lv_obj_t *multBut = NULL;
  //int nr_rows = 13;
  int selected_btn = -1;
  vector<ConfPanelParam> rows;

  void addConfig(const char *buf) { 
    vector<string> words = split(buf, ",");
    if (words.size() == 8) { 
      ConfPanelParam n;
      strncpy(n.label, words[0].c_str(), sizeof(n.label));
      strncpy(n.fmt, words[1].c_str(), sizeof(n.fmt));
      sscanf(words[2].c_str(), "%f", &n.inc);
      sscanf(words[3].c_str(), "%f", &n.min);
      sscanf(words[4].c_str(), "%f", &n.max);
      sscanf(words[5].c_str(), "%f", &n.def);
      sscanf(words[6].c_str(), "%f", &n.wrap);
      strncpy(n.enumlabels, words[7].c_str(), sizeof(n.enumlabels));
      if (strchr(n.enumlabels,'/') != NULL) { 
        n.inc = 1;
        n.min = 0;
        n.max = split(n.enumlabels,"/").size() - 1;
      }
      n.current = n.def;
      n.parent = this;
      n.index = rows.size();
      rows.push_back(n);
    }
  }

  void parseConfig(const char *config) { 
    vector<string> lines = split(config, "\n");
    for(auto l = lines.begin(); l != lines.end(); l++) {
      addConfig((*l).c_str());
    }
  }

  void selectButton(int idx) { 
    if (idx >= 0 && idx < rows.size()) { 
      // configuration panel mode?  deselect prev selection if we're selecting a different param
      if (isConfigPanel && selected_btn >= 0 && selected_btn < rows.size() && selected_btn != idx) { 
        set_btn_blue(rows[selected_btn].sel);
        rows[selected_btn].selected = false;
      }
      // toggle selected param 
      if (rows[idx].selected == true) { 
        selected_btn = -1;
        set_btn_blue(rows[idx].sel);
        rows[idx].selected = false;
      } else { 
        selected_btn = idx;
        set_btn_red(rows[idx].sel);
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
    Serial.printf("pressed\n");
    const char *t = lv_label_get_text(l);
    int v;
    if (t != NULL && sscanf(t, "%dX", &v) == 1) { 
      v *= 10;
      if (v > 100) v = 1;
      lv_label_set_text_fmt(l, "%dX", v);
    }
  }

  bool toggle = false;
  void onRecv(const string &buf) {
    Serial.printf("processing line: %s\n", buf.c_str());
    vector<string> lines = split(buf.c_str(), "\n");
    for(string s : lines) {
      //Serial.printf("processing line: %s\n", s.c_str());
      int i1, i2;
      float v;
      if (sscanf(s.c_str(), "VALUE %d %d %f", &i1, &i2, &v) == 3 && i1 == index && i2 >= 0 && i2 < rows.size()) {
        ConfPanelParam *p = &rows[i2];
        p->current = v;
        paramIncrement(i2, 0);
      }
    }
    if (multBut != NULL) { 
      if (toggle = !toggle) {
        set_btn_blue(multBut);
      } else {
        set_btn_blue2(multBut);
      }
    }
  }
  string readData() { 
    string rval;
    for(int i = 0; i < rows.size(); i++) { 
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
      int mult = 1;
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
      for(w = strtok(buf, "/"); w != NULL && idx > 0; w = strtok(NULL, "/"), idx--) {
      }
      if (w != NULL) { 
        lv_label_set_text(p->val, w);
      }
    }
  }

  static const int max_rows = 399;
  lv_coord_t col_dsc[4] = {50, LV_GRID_FR(2), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  lv_coord_t row_dsc[max_rows];

  bool isConfigPanel = true;

  void create(lv_obj_t *parent)
  {
    for (int r = 0; r < rows.size(); r++) { 
      row_dsc[r] = 50;
    }
    row_dsc[rows.size()] = LV_GRID_TEMPLATE_LAST;

    lv_obj_t *cont, *obj, *label; 

    if (isConfigPanel) { 
      /*Create a container with grid*/
      cont = lv_obj_create(parent);
      lv_obj_set_style_grid_column_dsc_array(cont, col_dsc, 0);
      lv_obj_set_style_grid_row_dsc_array(cont, row_dsc, 0);
      lv_obj_set_size(cont, LV_PCT(100), LV_PCT(80));
      //lv_obj_center(cont);
      lv_obj_set_layout(cont, LV_LAYOUT_GRID);

      lv_obj_t * cont2 = lv_obj_create(parent);
      lv_obj_set_size(cont2, LV_PCT(100), LV_PCT(20));
      lv_obj_align_to(cont2, cont, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);

      obj = lv_btn_create(cont2);
      label = lv_label_create(obj);
      lv_label_set_text_fmt(label, "DECREASE");
      lv_obj_set_style_text_font(label, default_font, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_center(label);
      lv_obj_align_to(obj, cont2, LV_ALIGN_LEFT_MID, 0, 0);
      set_btn_blue(obj);
      lv_obj_add_event_cb(obj, btn_event_dec, LV_EVENT_CLICKED, this);

      obj = lv_btn_create(cont2);
      label = lv_label_create(obj);
      lv_label_set_text_fmt(label, "1X");
      lv_obj_set_style_text_font(label, default_font, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_center(label);
      lv_obj_align_to(obj, cont2, LV_ALIGN_CENTER, 0, 0);
      set_btn_blue(obj);
      lv_obj_add_event_cb(obj, btn_event_1x, LV_EVENT_CLICKED, this);
      multBut = obj;

      obj = lv_btn_create(cont2);
      label = lv_label_create(obj);
      lv_label_set_text_fmt(label, "INCREASE");
      lv_obj_set_style_text_font(label, default_font, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_center(label);
      lv_obj_align_to(obj, cont2, LV_ALIGN_RIGHT_MID, 0, 0);
      set_btn_blue(obj);
      lv_obj_add_event_cb(obj, btn_event_inc, LV_EVENT_CLICKED, this);

    } else { 
      cont = lv_obj_create(parent);
      lv_obj_set_style_grid_column_dsc_array(cont, col_dsc, 0);
      lv_obj_set_style_grid_row_dsc_array(cont, row_dsc, 0);
      lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
      //lv_obj_center(cont);
      lv_obj_set_layout(cont, LV_LAYOUT_GRID);  
    }

    for(int i = 0; i < rows.size(); i++) {
      ConfPanelParam *p = &rows[i];

      obj = lv_btn_create(cont);
      lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 0, 1,
                          LV_GRID_ALIGN_STRETCH, i, 1);
      lv_obj_add_event_cb(obj, btn_event_sel, LV_EVENT_CLICKED, &rows[i]);
      set_btn_blue(obj);
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
    DispPanel(int i, const string conf, lv_obj_t *tile) : ConfPanel(i, conf, tile) { 
      isConfigPanel = false; 
    }
};

vector <ConfPanel *> panels;

#define GIT_VERSION "gitversion"
#include "/home/jim/Arduino/libraries/jimlib/src/jimlib.h"

JStuff j;

WiFiClient tcpClient;
//WiFiUDP udp;

void setup() {
    Serial.begin(115200); /* prepare for possible serial debug */
    panel_setup();
    Serial.println("Setup done");
    delay(1000);
    j.run();
    //udp.begin(4444);
}

bool parsingSchema = false;
string schema;
int schema_idx = 0;
lv_obj_t *tileview = NULL;
LineBuffer lb;

void processData(const char *buf, int n) { 
  lb.add((char *)buf, n, [](const char *l) { 
    //Serial.printf("Got line: %s\n", l);
    if (parsingSchema) {
      Serial.printf("parsing schema %d: %s\n", schema_idx, l);
      schema += string(l) + "\n";
      if (strcmp(l, "END") == 0)
        parsingSchema = false;
      if (strcmp(l, "END") == 0 && panels.size() == schema_idx) { 
        //Serial.printf("creating schema %d:\n%s\n", schema_idx, schema.c_str());
        if (tileview == NULL) { 
          tileview = lv_tileview_create(lv_scr_act());
          lv_obj_set_size(tileview, LV_PCT(100), LV_PCT(100));
          lv_obj_set_scrollbar_mode(tileview, LV_SCROLLBAR_MODE_OFF);
        }
        panels.push_back(new ConfPanel(schema_idx, schema, lv_tileview_add_tile(tileview, schema_idx, 0, LV_DIR_HOR | LV_DIR_BOTTOM)));
        parsingSchema = false;
      }
    } else {
      if (sscanf(l, "SCHEMA %d", &schema_idx) == 1) {
        parsingSchema = true;
        schema = "";
      }
      for (auto p : panels) { 
        p->onRecv(l);
      }
    }
  }); 
}

int tcpTimeout = 0;
void loop() {
    j.run();
    if (j.hz(1)) {
      //printf("loop()\n");
    }
    //Serial.printf("%f %f\n", panelComm.to, panelComm.from);
    lv_timer_handler();
    delay(1);

    //if (panels.size() > 0)
    //    return; 

    string s;
    for (auto p : panels) 
      s += p->readData();

    if(tcpClient.connected() == false) { 
      //tcpClient.close();
      Serial.printf("tcpClient.connect() %lx\n", (long)&tcpClient);
      tcpClient.connect("192.168.68.111", 4444);
      tcpTimeout = 0;
    }
    if (s.length() > 0) {
      //udp.beginPacket("255.255.255.255", 4444);
      //udp.write((uint8_t *)s.c_str(), s.length());
      //udp.endPacket();
      if (tcpClient.connected()) {
        tcpClient.write(s.c_str(), s.length());
      }
    }

    if(panels.size() == 0 && j.hz(.5)) {
      s = "SCHEMA\n";
      //udp.beginPacket("255.255.255.255", 4444);
      //udp.write((uint8_t *)s.c_str(), s.length());
      //udp.endPacket();
      if (tcpClient.connected()) {
        tcpClient.write(s.c_str(), s.length());
      }
    }
    if (tcpClient.available() > 0) {
        char buf[1024];
        int n = tcpClient.read((uint8_t *)buf, sizeof(buf));
        Serial.printf("read %d %d\n", n, (int)millis());
        processData(buf, n);
        tcpTimeout = 0;
        s = "ACK\n";
        tcpClient.write(s.c_str(), s.length());
     }
    if(j.hz(1) && tcpTimeout++ > 3) { 
      Serial.printf("tcpTimeout\n");
      tcpClient.stop();
      tcpTimeout = 0;
    }
   
    //if (udp.parsePacket() > 0) {
    //  char buf[1024];
    //  int n = udp.read((uint8_t *)buf, sizeof(buf));
    //  processData(buf, n);
    //}
}
  
