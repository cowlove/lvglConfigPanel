#include <Arduino.h>
#include <lvgl.h>

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

        Serial.printf("Touch point: x %d, y %d\n", point.x, point.y);
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
    /**
     * These development boards require the use of an IO expander to configure the screen,
     * so it needs to be initialized in advance and registered with the panel for use.
     *
     */
    Serial.println("Initialize IO expander");
    /* Initialize IO expander */
    ESP_IOExpander *expander = new ESP_IOExpander_TCA95xx_8bit(ESP_PANEL_LCD_TOUCH_BUS_HOST_ID, ESP_IO_EXPANDER_I2C_TCA9554_ADDRESS_000, ESP_PANEL_LCD_TOUCH_I2C_IO_SCL, ESP_PANEL_LCD_TOUCH_I2C_IO_SDA);
    expander->init();
    expander->begin();
    /* Add into panel */
    panel->addIOExpander(expander);
#endif

    panel->init();
#if ESP_PANEL_LCD_BUS_TYPE != ESP_PANEL_BUS_TYPE_RGB
    /* Register a function to notify LVGL when the panel is ready to flush */
    /* This is useful for refreshing the screen using DMA transfers */
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

// create first tile for monitoring selections 
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

    for(int i = 0; i < nr_rows; i++) {
        uint8_t col = i % 3;
        uint8_t row = i;

        obj = lv_checkbox_create(cont); // docs say style padding value can be used to make checkbox bigger
        lv_checkbox_set_text(obj, "");
        lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 0, 1,
                             LV_GRID_ALIGN_CENTER, row, 1);
        //label = lv_label_create(obj);
        //lv_label_set_text_fmt(label, "c%d, r%d", 0, row);
        //lv_obj_center(label);

        obj = lv_label_create(cont);
        lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 1, 1,
                             LV_GRID_ALIGN_END, row, 1);
        lv_label_set_text_fmt(obj, "DATAPOINT %d", row);
        lv_obj_set_style_text_font(obj, default_font, LV_PART_MAIN | LV_STATE_DEFAULT);

        obj = lv_label_create(cont);
        lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 2, 1,
                             LV_GRID_ALIGN_END, row, 1);
        lv_label_set_text_fmt(obj, "VAL%d", row);
        lv_obj_set_style_text_font(obj, default_font, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
}

const static int nr_rows = 20;
struct ParamEntry {
  lv_obj_t *sel;
  lv_obj_t *val;
  const char *label;
  const char *fmt;
  float inc;
  float max;
  float min;
  bool wrap;
  const char **enumlabels;
} rows[nr_rows];

lv_obj_t *multBut;


void btn_event_1x(lv_event_t *e) {
  lv_obj_t *o = lv_event_get_target(e);
  lv_obj_t *l = lv_obj_get_child(o, 0);
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

int selected_btn = -1;
void btn_event_sel(lv_event_t *e) {
  int idx = (int)lv_event_get_user_data(e);
  if (idx >= 0 && idx < nr_rows) { 
    if (selected_btn >= 0 && selected_btn < nr_rows) { 
      set_btn_blue(rows[selected_btn].sel);
    }
    if (selected_btn == idx) { 
      selected_btn = -1;
    } else { 
      selected_btn = idx;
      set_btn_red(rows[selected_btn].sel);
    }
  }   
}

void btn_event_inc(lv_event_t *e) {
  int dir = (int)lv_event_get_user_data(e);
  if (selected_btn < 0 || selected_btn >= nr_rows) 
    return;

  lv_obj_t *l = lv_obj_get_child(multBut, 0);
  if (l == NULL)
    return;
  const char *t = lv_label_get_text(l);
  int mult = 1;
  if (t != NULL)
    sscanf(t, "%dX", &mult);

  t = lv_label_get_text(rows[selected_btn].val);
  float val = 0;
  if (t != 0) 
    sscanf(t, "%f", &val);
  val = val + dir * mult;
  char buf[32];
  snprintf(buf, sizeof(buf), "%.2f", val);
  lv_label_set_text(rows[selected_btn].val, buf);
}

// create tunable parameter configuration tile 
void conf_menu_create(lv_obj_t *parent)
{
    static lv_coord_t col_dsc[] = {50, LV_GRID_FR(2), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[50];
    for (int r = 0; r < nr_rows; r++) { 
	     row_dsc[r] = 50;
    }
    row_dsc[nr_rows] = LV_GRID_TEMPLATE_LAST;

    /*Create a container with grid*/
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_style_grid_column_dsc_array(cont, col_dsc, 0);
    lv_obj_set_style_grid_row_dsc_array(cont, row_dsc, 0);
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(80));
    //lv_obj_center(cont);
    lv_obj_set_layout(cont, LV_LAYOUT_GRID);

    lv_obj_t * cont2 = lv_obj_create(parent);
    lv_obj_set_size(cont2, LV_PCT(100), LV_PCT(20));
    lv_obj_align_to(cont2, cont, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);

    lv_obj_t * label;
    lv_obj_t * obj;

    obj = lv_btn_create(cont2);
    label = lv_label_create(obj);
    lv_label_set_text_fmt(label, "DECREASE");
    lv_obj_set_style_text_font(label, default_font, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(label);
    lv_obj_align_to(obj, cont2, LV_ALIGN_LEFT_MID, 0, 0);
    set_btn_blue(obj);
    lv_obj_add_event_cb(obj, btn_event_inc, LV_EVENT_CLICKED, (void *)-1);


    obj = lv_btn_create(cont2);
    label = lv_label_create(obj);
    lv_label_set_text_fmt(label, "1X");
    lv_obj_set_style_text_font(label, default_font, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(label);
    lv_obj_align_to(obj, cont2, LV_ALIGN_CENTER, 0, 0);
    set_btn_blue(obj);
    lv_obj_add_event_cb(obj, btn_event_1x, LV_EVENT_CLICKED, NULL);
    multBut = obj;

    obj = lv_btn_create(cont2);
    label = lv_label_create(obj);
    lv_label_set_text_fmt(label, "INCREASE");
    lv_obj_set_style_text_font(label, default_font, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(label);
    lv_obj_align_to(obj, cont2, LV_ALIGN_RIGHT_MID, 0, 0);
    set_btn_blue(obj);
    lv_obj_add_event_cb(obj, btn_event_inc, LV_EVENT_CLICKED, (void *)1);
  
    uint32_t i;
    for(i = 0; i < nr_rows; i++) {
        uint8_t col = i % 3;
        uint8_t row = i;

        obj = lv_btn_create(cont);
        lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 0, 1,
                             LV_GRID_ALIGN_STRETCH, row, 1);
        lv_obj_add_event_cb(obj, btn_event_sel, LV_EVENT_CLICKED, (void *)i);
        set_btn_blue(obj);
        rows[i].sel = obj;

        obj = lv_label_create(cont);
        lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 1, 1,
                             LV_GRID_ALIGN_END, row, 1);
        lv_label_set_text_fmt(obj, "LABEL %d", row);
        lv_obj_set_style_text_font(obj, default_font, LV_PART_MAIN | LV_STATE_DEFAULT);
        //lv_obj_add_event_cb(obj, btn_event_sel, LV_EVENT_CLICKED, (void *)i);  // doesn't seem clickable 

        obj = lv_label_create(cont);
        lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 2, 1,
                             LV_GRID_ALIGN_END, row, 1);
        lv_label_set_text_fmt(obj, "%d", row);
        lv_obj_set_style_text_font(obj, default_font, LV_PART_MAIN | LV_STATE_DEFAULT);
        rows[i].val = obj;
    }
}

void tiles_create() { 
    lv_obj_t *tileview = lv_tileview_create(lv_scr_act());
   // lv_obj_add_style(tileview, &bgStyle, LV_PART_MAIN);
    lv_obj_set_size(tileview, LV_PCT(100), LV_PCT(100));
  //  lv_obj_add_event_cb(tileview, tileview_change_cb, LV_EVENT_VALUE_CHANGED, NULL);
  //  lv_obj_add_event_cb(tileview, tileview_change_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_set_scrollbar_mode(tileview, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *t1 = lv_tileview_add_tile(tileview, 0, 0, LV_DIR_HOR | LV_DIR_BOTTOM);
    lv_obj_t *t2 = lv_tileview_add_tile(tileview, 1, 0, LV_DIR_HOR | LV_DIR_BOTTOM);
    lv_obj_t *t3 = lv_tileview_add_tile(tileview, 2, 0, LV_DIR_HOR | LV_DIR_BOTTOM);

    //mon_menu_create(t1);
    conf_menu_create(t1);
    //lv_btn_create(t3);
}

void setup() {
    Serial.begin(115200); /* prepare for possible serial debug */
    panel_setup();
    tiles_create();
    Serial.println("Setup done");
}

void loop(){
    //Serial.println("Loop");
    lv_timer_handler();
    delay(2);
}
