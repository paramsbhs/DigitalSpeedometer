#include <lvgl.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "fonts/montserrat_bold_120.h"
// Touchscreen pins
#define XPT2046_IRQ 36   // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK 25   // T_CLK
#define XPT2046_CS 33    // T_CS

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

// LVGL display buffer
#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

// Global components
lv_obj_t *scr; // Screen background
lv_obj_t *speed_arc; // Speed arc
lv_obj_t *speed_needle; // Speed needle
lv_obj_t *fuel_meter; // Fuel meter
lv_obj_t *fuel_icon; // fuel icon
lv_obj_t *rpm_label; // Digital MPH
lv_obj_t *speed_border;

const int mwidth = 240;
const int mheight = 240;
const int rpmwidth = 16;
const int rpmlinewidth = 4;
const int dimension = 240;
const int arcend = 344;
const int arcstart = 120;
const int rpmmax = 8000;
const int rpm_arc_size = dimension - (rpmlinewidth * 12);
const int rpm_freq = 200;
const float marker_gap = ((float)arcend - arcstart) / (rpmmax / 1000);
const int rpmredline = 6800;
// Global styles
static lv_style_t style_unit_text;
static lv_style_t style_icon;
static lv_style_t style_speed_text;

// Color palette
lv_color_t palette_amber = lv_color_hex(0xFA8C00);
lv_color_t palette_cyan = lv_color_hex(0x00FFFF);
lv_color_t palette_black = lv_color_hex(0x000000);
lv_color_t palette_red = lv_color_hex(0xFF0000);
lv_color_t palette_white = lv_color_hex(0xFFFFFF);
lv_color_t palette_grey = lv_color_hex(0x5A5A5A);
lv_color_t palette_dark_grey = lv_color_hex(0x3C3C3C);
lv_color_t palette_purple = lv_color_hex(0x9D00FF);
lv_color_t palette_yellow = lv_color_hex(0xFFFF00);

#define FUEL_SYMBOL "\xEF\x94\xAF"

void make_styles(void) {
    lv_style_init(&style_unit_text);
    lv_style_set_text_font(&style_unit_text, &lv_font_montserrat_28);
    lv_style_set_text_color(&style_unit_text, palette_red);
}

void make_speed_meter(void) {
    speed_arc = lv_arc_create(scr);
    lv_obj_set_size(speed_arc, mwidth, mheight);
    lv_obj_set_pos(speed_arc, 1, 1);
    lv_arc_set_bg_angles(speed_arc, arcstart, arcend); // 270 degree arc
    lv_arc_set_mode(speed_arc, LV_ARC_MODE_NORMAL);

    lv_obj_set_style_arc_color(speed_arc, palette_grey, LV_PART_MAIN);
    lv_obj_set_style_arc_color(speed_arc, palette_yellow, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(speed_arc, rpmwidth, LV_PART_MAIN);
    lv_obj_set_style_arc_rounded(speed_arc, false, LV_PART_MAIN);
    lv_obj_set_style_arc_width(speed_arc, rpmwidth, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(speed_arc, false, LV_PART_INDICATOR);
    lv_obj_remove_style(speed_arc, NULL, LV_PART_KNOB);

    lv_arc_set_range(speed_arc, 0, rpmmax);
    lv_arc_set_value(speed_arc, 7000);
}

void position_markers(lv_obj_t *marker, int position) {
    lv_obj_set_size(marker, mwidth, mheight);
    lv_obj_set_pos(marker, 1, 1);
    
    lv_obj_set_style_arc_width(marker, rpmwidth + (rpmlinewidth * 2), LV_PART_MAIN);
    lv_obj_set_style_arc_rounded(marker, false, LV_PART_MAIN);
    lv_obj_set_style_arc_color(marker, palette_black, LV_PART_MAIN);
    float marker_angle = arcstart + (position * marker_gap);
    lv_arc_set_bg_angles(marker, (int)marker_angle, (int)marker_angle + 2);
    lv_obj_remove_style(marker, NULL, LV_PART_KNOB); 
    lv_obj_remove_style(marker, NULL, LV_PART_INDICATOR);    
}


void make_speed_border(void){
    speed_border = lv_arc_create(scr);
    lv_obj_set_size(speed_border, mwidth-27, mheight-27);
    lv_obj_set_pos(speed_border, 15, 15);
    lv_arc_set_bg_angles(speed_border, arcstart, arcend-56);

    lv_obj_set_style_arc_width(speed_border, rpmlinewidth, LV_PART_MAIN);
    lv_obj_set_style_arc_rounded(speed_border, false, LV_PART_MAIN);
    lv_obj_set_style_arc_color(speed_border, palette_black, LV_PART_MAIN);

    lv_obj_remove_style(speed_border, NULL, LV_PART_KNOB);
    lv_obj_remove_style(speed_border, NULL, LV_PART_INDICATOR);
    int rpmmarker = (rpmmax/1000) + 1;
    lv_obj_t *rpmmarkers[rpmmarker];
    for(int i = 0; i < rpmmarker; i++){
      rpmmarkers[i] = lv_arc_create(scr);
      position_markers(rpmmarkers[i], i);
    }
}

void make_redline(void){
    lv_obj_t *redlinemarker = lv_arc_create(scr);
    lv_obj_set_size(redlinemarker, mwidth-27, mheight-27);
    lv_obj_set_pos(redlinemarker, 15, 15);
    lv_arc_set_bg_angles(redlinemarker, arcend-56, arcend);

    lv_obj_set_style_arc_width(redlinemarker, rpmlinewidth, LV_PART_MAIN);
    lv_obj_set_style_arc_rounded(redlinemarker, false, LV_PART_MAIN);
    lv_obj_set_style_arc_color(redlinemarker, palette_cyan, LV_PART_MAIN);

    lv_obj_remove_style(redlinemarker, NULL, LV_PART_KNOB);
    lv_obj_remove_style(redlinemarker, NULL, LV_PART_INDICATOR);
}
// void make_fuel_meter(void) {
//   // Adjust size and position for smaller screen
//   fuel_meter = lv_arc_create(scr);
//   lv_obj_set_size(fuel_meter, 180, 180);
//   lv_arc_set_rotation(fuel_meter, 65);
//   lv_arc_set_bg_angles(fuel_meter, 0, 50);
//   lv_arc_set_range(fuel_meter, 0, 100);
//   lv_obj_set_pos(fuel_meter, 30, 30);
//   lv_arc_set_mode(fuel_meter, LV_ARC_MODE_REVERSE);

//   lv_obj_set_style_arc_color(fuel_meter, palette_dark_grey, LV_PART_MAIN);
//   lv_obj_set_style_arc_color(fuel_meter, palette_white, LV_PART_INDICATOR);
//   lv_obj_set_style_arc_rounded(fuel_meter, false, LV_PART_MAIN);
//   lv_obj_set_style_arc_rounded(fuel_meter, false, LV_PART_INDICATOR);
//   lv_obj_set_style_arc_width(fuel_meter, 3, LV_PART_MAIN);
//   lv_obj_set_style_arc_width(fuel_meter, 3, LV_PART_INDICATOR);
//   lv_obj_remove_style(fuel_meter, NULL, LV_PART_KNOB); 
  
//   lv_obj_set_style_arc_color(fuel_meter, palette_amber, LV_PART_INDICATOR);
//   lv_arc_set_value(fuel_meter, 50);

//   fuel_icon = lv_label_create(scr);
//   lv_label_set_text(fuel_icon, "F");  // Simple F instead of icon
//   lv_obj_add_style(fuel_icon, &style_icon, 0);
//   lv_obj_set_pos(fuel_icon, 115, 170);
// }

void make_rpm_digital(void) {
    static lv_style_t style_rpm_text;
    lv_style_init(&style_rpm_text);
    lv_style_set_text_font(&style_rpm_text, &lv_font_montserrat_28);
    lv_style_set_text_color(&style_rpm_text, palette_black);

    rpm_label = lv_label_create(scr);
    lv_label_set_text(rpm_label, "7000");
    lv_obj_add_style(rpm_label, &style_rpm_text, 0);
    lv_obj_align(rpm_label, LV_ALIGN_LEFT_MID, 220, 0);

    lv_obj_t *rpm_unit_label = lv_label_create(scr);
    lv_label_set_text(rpm_unit_label, "rpm");
    lv_obj_add_style(rpm_unit_label, &style_unit_text, 0);
    lv_obj_align(rpm_unit_label, LV_ALIGN_LEFT_MID, 260, 20);
}


void make_speed_digital(void) {
    static lv_style_t style_speed_text;
    lv_style_init(&style_speed_text);
    lv_style_set_text_font(&style_speed_text, &montserrat_bold_120);
    lv_style_set_text_color(&style_speed_text, palette_white);

    kph_label = lv_label_create(scr);
    lv_label_set_text(kph_label, "85");
    lv_obj_add_style(kph_label, &style_speed_text, 0);
    lv_obj_align(kph_label, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *mph_unit_label = lv_label_create(scr);
    lv_label_set_text(mph_unit_label, "kph");
    lv_obj_add_style(mph_unit_label, &style_unit_text, 0);
    lv_obj_align(mph_unit_label, LV_ALIGN_CENTER, 40, 60);
}


void setup() {
  Serial.begin(115200);
  
  String LVGL_Arduino = String("LVGL Library Version: ") + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.println(LVGL_Arduino);
  
  lv_init();

  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(2);

  lv_display_t * disp;
  disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf, sizeof(draw_buf));
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_270);
    
  scr = lv_screen_active();
  lv_obj_set_style_bg_color(scr, palette_black, 1);

  make_styles();

  make_speed_meter();
  make_speed_border();
  make_redline();
  make_rpm_digital();
  make_speed_digital();

  Serial.println("Speedometer initialized!");
}

void loop() {
  lv_task_handler();
  lv_tick_inc(5);
  delay(5);
  
  // Simulate data updates
 // if (millis() - last_update > 2000) {
    // Example: simulate changing speed and fuel
    // Replace this with your actual data reading code
    //last_update = millis();
  //}
}