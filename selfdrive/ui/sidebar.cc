#include <stdio.h>
#include <string.h>
#include <math.h>
#include <map>
#include "ui.hpp"

static void ui_draw_sidebar_background(UIState *s) {
  int sbr_x = !s->scene.uilayout_sidebarcollapsed ? 0 : -(sbr_w) + bdr_s * 2;
  ui_draw_rect(s->vg, sbr_x, 0, sbr_w, vwp_h, COLOR_BLACK_ALPHA(85));
}

static void ui_draw_sidebar_settings_button(UIState *s) {
  const float alpha = s->active_app == cereal::UiLayoutState::App::SETTINGS ? 1.0f : 0.65f;
  ui_draw_image(s->vg, settings_btn_x, settings_btn_y, settings_btn_w, settings_btn_h, s->img_button_settings, alpha);
}

static void ui_draw_sidebar_home_button(UIState *s) {
  const float alpha = s->active_app == cereal::UiLayoutState::App::HOME ? 1.0f : 0.65f;;
  ui_draw_image(s->vg, home_btn_x, home_btn_y, home_btn_w, home_btn_h, s->img_button_home, alpha);
}

static void ui_draw_sidebar_ipaddress(UIState *s) {
  const int ipaddress_x = 50;
  const int ipaddress_y = 210;
  const int ipaddress_w = 250;
  nvgFillColor(s->vg, COLOR_GREEN);
  nvgFontSize(s->vg, 35);
  nvgFontFaceId(s->vg, s->font_sans_bold);
  nvgTextBox(s->vg, ipaddress_x, ipaddress_y, ipaddress_w, s->scene.thermal.getWifiIpAddress().cStr(), NULL);
}

static void ui_draw_sidebar_battery_icon(UIState *s) {
  const int battery_img_x = 65;
  const int battery_img_y = 245;
  const int battery_img_w = 160;
  const int battery_img_h = 65;

  int battery_img = s->scene.thermal.getBatteryStatus() == "Charging" ? s->img_battery_charging : s->img_battery;

  ui_draw_rect(s->vg, battery_img_x + 6, battery_img_y + 5,
               ((battery_img_w - 19) * (s->scene.thermal.getBatteryPercent() * 0.01)), battery_img_h - 11, COLOR_YELLOW);

  ui_draw_image(s->vg, battery_img_x, battery_img_y, battery_img_w, battery_img_h, battery_img, 1.0f);
}

static void ui_draw_sidebar_metric(UIState *s, const char* label_str, const char* value_str, const int severity, const int y_offset, const char* message_str) {
  const int metric_x = 30;
  const int metric_y = 338 + y_offset;
  const int metric_w = 240;
  const int metric_h = message_str ? strchr(message_str, '\n') ? 124 : 100 : 148;

  NVGcolor status_color;

  if (severity == 0) {
    status_color = COLOR_WHITE;
  } else if (severity == 1) {
    status_color = COLOR_YELLOW;
  } else if (severity > 1) {
    status_color = COLOR_RED;
  }

  ui_draw_rect(s->vg, metric_x, metric_y, metric_w, metric_h,
               severity > 0 ? COLOR_WHITE : COLOR_WHITE_ALPHA(85), 20, 2);

  nvgBeginPath(s->vg);
  nvgRoundedRectVarying(s->vg, metric_x + 6, metric_y + 6, 18, metric_h - 12, 25, 0, 0, 25);
  nvgFillColor(s->vg, status_color);
  nvgFill(s->vg);

  if (!message_str) {
    nvgFillColor(s->vg, COLOR_WHITE);
    nvgFontSize(s->vg, 60);
    nvgFontFaceId(s->vg, s->font_sans_bold);
    nvgTextAlign(s->vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    nvgTextBox(s->vg, metric_x + 50, metric_y + 50, metric_w - 60, value_str, NULL);

    nvgFillColor(s->vg, COLOR_WHITE);
    nvgFontSize(s->vg, 38);
    nvgFontFaceId(s->vg, s->font_sans_bold);
    nvgTextAlign(s->vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    nvgTextBox(s->vg, metric_x + 50, metric_y + 50 + 66, metric_w - 60, label_str, NULL);
  } else {
    nvgFillColor(s->vg, COLOR_WHITE);
    nvgFontSize(s->vg, 38);
    nvgFontFaceId(s->vg, s->font_sans_bold);
    nvgTextAlign(s->vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    nvgTextBox(s->vg, metric_x + 35, metric_y + (strchr(message_str, '\n') ? 40 : 50), metric_w - 50, message_str, NULL);
  }
}

static void ui_draw_sidebar_temp_metric(UIState *s) {
  static std::map<cereal::ThermalData::ThermalStatus, const int> temp_severity_map = {
      {cereal::ThermalData::ThermalStatus::GREEN, 0},
      {cereal::ThermalData::ThermalStatus::YELLOW, 1},
      {cereal::ThermalData::ThermalStatus::RED, 2},
      {cereal::ThermalData::ThermalStatus::DANGER, 3}};
  char temp_label_str[32];
  char temp_value_str[32];
  char temp_value_unit[32];
  const int temp_y_offset = 0;
  snprintf(temp_value_str, sizeof(temp_value_str), "%d", s->scene.thermal.getPa0());
  snprintf(temp_value_unit, sizeof(temp_value_unit), "%s", "°C");
  snprintf(temp_label_str, sizeof(temp_label_str), "%s", "시스템 온도");
  strcat(temp_value_str, temp_value_unit);

  ui_draw_sidebar_metric(s, temp_label_str, temp_value_str, temp_severity_map[s->scene.thermal.getThermalStatus()], temp_y_offset, NULL);
}

static void ui_draw_sidebar_panda_metric(UIState *s) {
  int panda_severity = 2;
  char panda_message_str[32];
  const int panda_y_offset = 32 + 148;

  if (s->scene.hwType == cereal::HealthData::HwType::UNKNOWN) {
    panda_severity = 2;
    snprintf(panda_message_str, sizeof(panda_message_str), "%s", "판다\n오프라인");
  } else {
    if (s->started){
      if (s->scene.satelliteCount < 6) {
        panda_severity = 1;
        snprintf(panda_message_str, sizeof(panda_message_str), "%s", "판다\nNO GPS");
      } else if (s->scene.satelliteCount >= 6) {
        panda_severity = 0;
        snprintf(panda_message_str, sizeof(panda_message_str), "%s", "판다\nGOOD GPS");
      }
    } else {
      panda_severity = 0;
      snprintf(panda_message_str, sizeof(panda_message_str), "%s", "판다\n온라인");
    }
  }

  ui_draw_sidebar_metric(s, NULL, NULL, panda_severity, panda_y_offset, panda_message_str);
}

static void ui_draw_sidebar_connectivity(UIState *s) {
  if (s->scene.athenaStatus == NET_DISCONNECTED) {
    ui_draw_sidebar_metric(s, NULL, NULL, 1, 180+158, "네트워크\n오프라인");
  } else if (s->scene.athenaStatus == NET_CONNECTED) {
    ui_draw_sidebar_metric(s, NULL, NULL, 0, 180+158, "네트워크\n온라인");
  } else {
    ui_draw_sidebar_metric(s, NULL, NULL, 2, 180+158, "네트워크\n에러");
  }
}

void ui_draw_sidebar(UIState *s) {
  ui_draw_sidebar_background(s);
  if (s->scene.uilayout_sidebarcollapsed){
    return;
  }
  ui_draw_sidebar_settings_button(s);
  ui_draw_sidebar_home_button(s);
  ui_draw_sidebar_ipaddress(s);
  ui_draw_sidebar_battery_icon(s);
  ui_draw_sidebar_temp_metric(s);
  ui_draw_sidebar_panda_metric(s);
  ui_draw_sidebar_connectivity(s);
}
