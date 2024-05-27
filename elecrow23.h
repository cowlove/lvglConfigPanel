#ifndef ELECROW23_H
#define ELECROW23_H

// https://www.elecrow.com/wiki/2.4-inch-esp32-dispaly-arduino-tutorial.html
//ESP32-WROOM-DA
// needs pre-packaged libraries and lvgl-3
// seems to work with just cp -av ~/Downloads/Arduino_5inch/Hardware-Version-2.0/libraries/{LovyanGFX,gt911-arduino-main} ~/Arduino/libraries
// and cd ~/Arduino/libraries/lvgl/src && ln -s ../examples ../demos . 
// 2.4 needs TFT_eSPI/User_Config.h copied for electrow zip file

#define ESP_PANEL_LCD_H_RES 320
#define ESP_PANEL_LCD_V_RES 240
//#define TOUCH_CS

#include <lvgl.h>
#include <TFT_eSPI.h>
#include <demos/lv_demos.h>
#include <examples/lv_examples.h>

#include <U8g2lib.h>
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#include <SPI.h>
#endif 

char buf[128] = {};
int bufindex = 0;
int wifi_close_flag = 0;
char *info[128] = {};
int wifi_flag = 0;
int i = 0;
int touch_flag = 0;
U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C  u8g2(U8G2_R0, /* clock=*/ 21, /* data=*/ 22, /* reset=*/ U8X8_PIN_NONE);   // Adafruit Feather ESP8266/32u4 Boards + FeatherWing OLED

//2.4
#define SD_MOSI 23
#define SD_MISO 19
#define SD_SCK 18
#define SD_CS 5
 
static const uint16_t screenWidth  = 320;
static const uint16_t screenHeight = 240;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[ screenWidth * screenHeight / 8 ];

TFT_eSPI lcd = TFT_eSPI(); 

void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p )
{
  uint32_t w = ( area->x2 - area->x1 + 1 );
  uint32_t h = ( area->y2 - area->y1 + 1 );

  lcd.startWrite();
  lcd.setAddrWindow( area->x1, area->y1, w, h );
  lcd.pushColors( ( uint16_t * )&color_p->full, w * h, true );
  lcd.endWrite();

  lv_disp_flush_ready( disp );
}


uint16_t touchX, touchY;

void my_touchpad_read( lv_indev_drv_t * indev_driver, lv_indev_data_t * data )
{
  bool touched = lcd.getTouch( &touchX, &touchY, 600);
  if ( !touched )
  {
    data->state = LV_INDEV_STATE_REL;
  }
  else
  {
    data->state = LV_INDEV_STATE_PR;

    /*设置坐标*/
    data->point.x = touchX;
    data->point.y = touchY;

    Serial.print( "Data x " );
    Serial.println( touchX );

    Serial.print( "Data y " );
    Serial.println( touchY );
  }
}

uint16_t calData[5] = { 557, 3263, 369, 3493, 3  };

void panel_setup() { 
    u8g2.begin();
    u8g2.enableUTF8Print();        // enable UTF8 support for the Arduino print() function
    u8g2.setFont(u8g2_font_ncenB14_tr);
    u8g2.setFontDirection(0);
    lv_init();

    lcd.begin();          
    delay(1000);
    lcd.fillScreen(TFT_GREEN);
    pinMode(27, OUTPUT);
    digitalWrite(27, HIGH);
    lcd.setRotation(1); /* 旋转 */
    lcd.setTouch( calData );
    lv_disp_draw_buf_init( &draw_buf, buf1, NULL, screenWidth * screenHeight / 8 );

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init( &disp_drv );
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register( &disp_drv );

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init( &indev_drv );
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register( &indev_drv );



/////////////////
/*
  lv_init();
  touch_init();

  screenWidth = lcd.width();
  screenHeight = lcd.height();

  lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, screenWidth * screenHeight / 10); //4

  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, 1);
*/
}

#endif // ELECROW5_H
