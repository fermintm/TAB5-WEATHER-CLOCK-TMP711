#include <Arduino.h>
#include <M5Unified.h>
#include <M5GFX.h>
#include <SPI.h>
#include <WiFi.h>
#include <TimeLib.h>
#include <lvgl.h>
#include <Preferences.h>
#include <Wire.h>
#include <WiFiUdp.h>
#include "mini100.c"
#include <SparkFun_TMP117.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <SPIFFS.h>
#include <SD.h>

LV_FONT_DECLARE(mini100);

TMP117 tempsensor;

// ============================= DEFINICIONES DE COLORES EXTENDIDAS =============================

// --- NEUTROS Y GRISES ---
#define LV_COLOR_BLANCO lv_color_make(255, 255, 255)
#define LV_COLOR_NEGRO lv_color_make(0, 0, 0)
#define LV_COLOR_GRIS_OSCURO lv_color_make(50, 50, 50)    // Fondo y Texto secundario
#define LV_COLOR_GRIS_MEDIO lv_color_make(128, 128, 128)  // Gris Estándar
#define LV_COLOR_PLATA lv_color_make(192, 192, 192)       // Gris claro metálico
#define LV_COLOR_GRIS_CLARO lv_color_make(220, 220, 220)  // Gris muy claro
#define LV_COLOR_MARRON lv_color_make(165, 42, 42)

// --- ROJOS Y ROSAS ---
#define LV_COLOR_ROJO_PURO lv_color_make(255, 0, 0)
#define LV_COLOR_CARMESI lv_color_make(220, 20, 60)  // Rojo intenso
#define LV_COLOR_ESCarlata lv_color_make(255, 36, 0)
#define LV_COLOR_LADRILLO lv_color_make(176, 48, 96)
#define LV_COLOR_BORGOÑA lv_color_make(128, 0, 32)
#define LV_COLOR_ROSA lv_color_make(255, 192, 203)
#define LV_COLOR_FUCSIA lv_color_make(255, 0, 255)  // Magenta Puro
#define LV_COLOR_PURPURA lv_color_make(128, 0, 128)

// --- NARANJAS Y AMARILLOS ---
#define LV_COLOR_NARANJA_PURO lv_color_make(255, 165, 0)  // Naranja Estándar (Advertencia)
#define LV_COLOR_NARANJA_OSCURO lv_color_make(255, 140, 0)
#define LV_COLOR_MELOCOTON lv_color_make(255, 218, 185)
#define LV_COLOR_MANDARINA lv_color_make(255, 160, 0)
#define LV_COLOR_DORADO lv_color_make(255, 215, 0)
#define LV_COLOR_AMARILLO_PURO lv_color_make(255, 255, 0)
#define LV_COLOR_AMARILLO_PALIDO lv_color_make(255, 255, 224)
#define LV_COLOR_AMBAR lv_color_make(255, 191, 0)

// --- VERDES ---
#define LV_COLOR_VERDE_PURO lv_color_make(0, 255, 0)
#define LV_COLOR_LIMA lv_color_make(0, 255, 0)  // Verde ácido
#define LV_COLOR_VERDE_BOSQUE lv_color_make(34, 139, 34)
#define LV_COLOR_VERDE_OLIVA lv_color_make(128, 128, 0)
#define LV_COLOR_MENTA lv_color_make(189, 252, 201)
#define LV_COLOR_VERDE_MAR lv_color_make(46, 139, 87)
#define LV_COLOR_ESMERALDA lv_color_make(80, 200, 120)

// --- CIANES Y AGUAS ---
#define LV_COLOR_CYAN_PURO lv_color_make(0, 255, 255)
#define LV_COLOR_ACUA lv_color_make(0, 255, 255)
#define LV_COLOR_TURQUESA lv_color_make(64, 224, 208)
#define LV_COLOR_AZUL_CLARO lv_color_make(0, 191, 255)
#define LV_COLOR_AZUL_AQUA lv_color_make(0, 206, 209)

// --- AZULES ---
#define LV_COLOR_AZUL_PURO lv_color_make(0, 0, 255)
#define LV_COLOR_AZUL_CIELO lv_color_make(135, 206, 235)
#define LV_COLOR_AZUL_ROYAL lv_color_make(65, 105, 225)
#define LV_COLOR_INDIGO lv_color_make(75, 0, 130)
#define LV_COLOR_AZUL_MARINO lv_color_make(0, 0, 128)
#define LV_COLOR_AZUL_MEDIO lv_color_make(0, 0, 205)

// --- MORADOS Y VIOLETAS ---
#define LV_COLOR_VIOLETA_PURO lv_color_make(148, 0, 211)
#define LV_COLOR_MORADO lv_color_make(128, 0, 128)
#define LV_COLOR_LILA lv_color_make(200, 162, 200)
#define LV_COLOR_LAVANDA lv_color_make(230, 230, 250)
#define LV_COLOR_CIRUELA lv_color_make(221, 160, 221)

// --- COLORES DE UTILIDAD (Estados) ---
#define LV_COLOR_EXITO lv_color_make(46, 204, 113)     // Verde para 'OK' o 'Normal'
#define LV_COLOR_ADVERTENCIA LV_COLOR_NARANJA_PURO     // Naranja para 'Advertencia'
#define LV_COLOR_ERROR LV_COLOR_ROJO_PURO              // Rojo para 'Crítico' o 'Alarma'
#define LV_COLOR_INFO LV_COLOR_AZUL_CIELO              // Azul para 'Información'
#define LV_COLOR_DESTELLO lv_color_make(255, 20, 147)  // Rosa Brillante (DeepPink)
#define LV_COLOR_ACENTO LV_COLOR_AZUL_ROYAL            // Un color de acento recomendado

// ============================= DEFINICIONES DE RESOLUCIÓN =============================
#define EXAMPLE_LCD_H_RES 1280
#define EXAMPLE_LCD_V_RES 720
#define LVGL_LCD_BUF_SIZE (EXAMPLE_LCD_H_RES * 20)

//Pines SDIO para ESP32-C6 en M5Stack Tab5
#define SDIO2_CLK GPIO_NUM_12
#define SDIO2_CMD GPIO_NUM_13
#define SDIO2_D0 GPIO_NUM_11
#define SDIO2_D1 GPIO_NUM_10
#define SDIO2_D2 GPIO_NUM_9
#define SDIO2_D3 GPIO_NUM_8
#define SDIO2_RST GPIO_NUM_15

//Pines SD M5Stack Tab5
#define SD_SPI_CS_PIN 42
#define SD_SPI_SCK_PIN 43
#define SD_SPI_MOSI_PIN 44
#define SD_SPI_MISO_PIN 39

// ============================= LVGL BUFFERS =============================
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *buf1;
static lv_color_t *buf2;

// ============================= OBJETOS LVGL =============================
lv_obj_t *ssid_input, *pass_input, *keyboard, *status_label;
lv_obj_t *main_screen, *wifi_screen, *config_screen;
lv_obj_t *btn_wifi, *btn_config, *btn_back_wifi, *btn_back_config;
lv_obj_t *bat_label, *slider_label;
lv_obj_t *rtc_date_label;
lv_obj_t *rtc_time_label;
lv_obj_t *scan_btn;
lv_obj_t *wifi_list = NULL;
lv_obj_t *list_container;
lv_obj_t *btn_carga, *label_carga;
lv_obj_t *month_dropdown, *year_dropdown;

// ICONOS GLOBALES DE BARRA DE ESTADO
lv_obj_t *wifi_icon;
lv_obj_t *bat_icon;
lv_obj_t *bat_percent_label;

bool wifi_icon_visible = true;
unsigned long lastBlink = 0;

// ============================= OBJETOS PANTALLA TEMPERATURA =============================
lv_obj_t *temp_screen;
lv_obj_t *btn_temp_main;
lv_obj_t *btn_back_temp;
lv_obj_t *temp_label;
lv_obj_t *temp_min_display_label;
lv_obj_t *temp_max_display_label;
lv_obj_t *temp_chart;
lv_chart_series_t *temp_series;

// ============================= OBJETOS CONFIG TEMP =============================
lv_obj_t *temp_disp_slider;
lv_obj_t *temp_disp_label;
lv_obj_t *graph_upd_slider;
lv_obj_t *graph_upd_label;

// ============================= NUEVO: OBJETOS CONTROL VOLUMEN Y ALARMAS =============================
lv_obj_t *volume_slider;
lv_obj_t *volume_label;
lv_obj_t *temp_max_slider;
lv_obj_t *temp_max_label;
lv_obj_t *temp_min_slider;
lv_obj_t *temp_min_label;
lv_obj_t *alarm_status_label;

// ============================= OBJETOS PANTALLA TIEMPO =============================
lv_obj_t *weather_screen;
lv_obj_t *btn_weather_main;
lv_obj_t *btn_back_weather;
lv_obj_t *weather_temp_label;
lv_obj_t *weather_cond_label;
lv_obj_t *weather_status_label;
lv_obj_t *weather_hum_label;
lv_obj_t *weather_press_label;

// ============================= ESTADO Y PREFERENCIAS =============================
String saved_ssid = "";
String saved_password = "";
int saved_brightness = 175;
int saved_temp_display_secs = 1;
int saved_graph_update_mins = 30;
int saved_volume = 50;
float saved_temp_max = 35.0;
float saved_temp_min = 15.0;
bool alarm_active = false;
unsigned long lastAlarmBeep = 0;

float current_temp_min = 999.0;
float current_temp_max = -999.0;

Preferences preferences;

// ============================= RTC Y WIFI =============================
enum WiFiState { DISCONNECTED,
                 CONNECTING,
                 CONNECTED };
WiFiState wifiState = DISCONNECTED;
static const char ntpServerName[] = "time.nist.gov";
WiFiUDP Udp;
bool udp_active = false;
unsigned int localPort = 8000;
const int NTP_PACKET_SIZE = 48;
byte packetBuffer[NTP_PACKET_SIZE];
bool ntpSyncNeeded = false;
unsigned long lastConnectionAttempt = 0;
const unsigned long connectionTimeout = 15000;

// ============================= CONFIG GRÁFICO / TEMP =============================
const int TEMP_POINTS = 144;
unsigned long lastTempUpdate = 0;
unsigned long lastChartUpdate = 0;
float tempAccum = 0.0f;
int tempAccumCount = 0;

// ============================= CONFIG TIEMPO =============================
const char *weatherCity = "Durango,ES";
const char *weatherApiKey = "API KEY";
unsigned long lastWeatherUpdate = 0;
const unsigned long weatherUpdateInterval = 900000;

// ============================= PROTOTIPOS =============================
void startWiFiConnection(const char *ssid, const char *password);
void syncNtpTime();
time_t getNtpTime();
void update_rtc_label();
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
void my_touchpad_read_cb(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);
void create_status_bar();
void update_weather();
void checkTemperatureAlarm(float temperatura);
void playAlarmBeep();
String traducirCondicion(String mainCond, String descCond);

// ============================= NUEVO: FUNCIÓN ALARMA SONORA =============================
void playAlarmBeep() {
  M5.Speaker.tone(1500, 200);  // 1000Hz durante 200ms
  delay(200);
  M5.Speaker.tone(2000, 200);  // 1500Hz durante 200ms
}

// ============================= VERIFICAR ALARMAS DE TEMPERATURA (CORREGIDO) =============================
void checkTemperatureAlarm(float temperatura) {
  bool alarm_triggered = false;

  char temp_buf[8];
  char buffer[64];

  dtostrf(temperatura, 4, 1, temp_buf);

  if (temperatura > saved_temp_max) {
    alarm_triggered = true;
    if (alarm_status_label && lv_scr_act() == temp_screen) {
      snprintf(buffer, sizeof(buffer), "ALARMA: TEMPERATURA ALTA (%s C)", temp_buf);
      lv_label_set_text(alarm_status_label, buffer);
      lv_obj_set_style_text_color(alarm_status_label, LV_COLOR_NARANJA_PURO, LV_PART_MAIN);
    }
  } else if (temperatura < saved_temp_min) {
    alarm_triggered = true;
    if (alarm_status_label && lv_scr_act() == temp_screen) {
      snprintf(buffer, sizeof(buffer), "ALARMA: TEMPERATURA BAJA (%s C)", temp_buf);
      lv_label_set_text(alarm_status_label, buffer);
      lv_obj_set_style_text_color(alarm_status_label, lv_color_make(0, 100, 255), LV_PART_MAIN);
    }
  } else {
    alarm_active = false;
    if (alarm_status_label && lv_scr_act() == temp_screen) {
      lv_label_set_text(alarm_status_label, "TEMPERATURA NORMAL");
      lv_obj_set_style_text_color(alarm_status_label, lv_color_make(0, 255, 0), LV_PART_MAIN);
    }
  }

  if (alarm_triggered) {
    if (!alarm_active || (millis() - lastAlarmBeep > 2000)) {
      playAlarmBeep();
      lastAlarmBeep = millis();
      alarm_active = true;
    }
  }
}

// ============================= GESTIÓN RTC =============================
void rtc_init() {
  if (!M5.Rtc.isEnabled()) {
    Serial.println("RTC BM8563 init failed!");
  } else {
    Serial.println("RTC BM8563 initialized successfully");
  }
}

void setRtcTime(int year, int month, int day, int hour, int minute, int second) {
  m5::rtc_datetime_t dt;
  dt.date.year = year;
  dt.date.month = month;
  dt.date.date = day;
  tmElements_t tm;
  tm.Year = year - 1970;
  tm.Month = month;
  tm.Day = day;
  time_t t = makeTime(tm);
  dt.date.weekDay = weekday(t) - 1;
  dt.time.hours = hour;
  dt.time.minutes = minute;
  dt.time.seconds = second;
  M5.Rtc.setDateTime(dt);
}

m5::rtc_datetime_t getRtcTime() {
  auto dt = M5.Rtc.getDateTime();
  if (dt.date.year < 2020 || dt.date.year > 2099) {
    dt.date.year = 2025;
    dt.date.month = 9;
    dt.date.date = 23;
    dt.time.hours = 0;
    dt.time.minutes = 0;
    dt.time.seconds = 0;
  }
  return dt;
}

// ============================= NTP =============================
long getMadridOffset(time_t unixTime) {
  tmElements_t tm;
  breakTime(unixTime, tm);

  auto lastSundayOfMonth = [](int year, int month) -> int {
    int monthLength;
    switch (month) {
      case 1:
      case 3:
      case 5:
      case 7:
      case 8:
      case 10:
      case 12: monthLength = 31; break;
      case 4:
      case 6:
      case 9:
      case 11: monthLength = 30; break;
      case 2:
        {
          bool leap = ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
          monthLength = leap ? 29 : 28;
          break;
        }
    }
    tmElements_t tml;
    tml.Year = year - 1970;
    tml.Month = month;
    tml.Day = monthLength;
    time_t lastDay = makeTime(tml);
    int wd = weekday(lastDay);
    int offsetDays = (wd == 1) ? 0 : (wd - 1);
    return monthLength - offsetDays;
  };

  long offset = 1 * SECS_PER_HOUR;

  if (tm.Month < 3 || tm.Month > 10) return offset;
  if (tm.Month > 3 && tm.Month < 10) return offset + SECS_PER_HOUR;

  int yearFull = tm.Year + 1970;
  if (tm.Month == 3) {
    int lastSunday = lastSundayOfMonth(yearFull, 3);
    if (tm.Day > lastSunday) return offset + SECS_PER_HOUR;
    if (tm.Day == lastSunday && tm.Hour >= 1) return offset + SECS_PER_HOUR;
    return offset;
  } else {
    int lastSunday = lastSundayOfMonth(yearFull, 10);
    if (tm.Day < lastSunday) return offset + SECS_PER_HOUR;
    if (tm.Day == lastSunday && tm.Hour < 1) return offset + SECS_PER_HOUR;
    return offset;
  }
}

void sendNTPpacket(IPAddress &address) {
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011;
  packetBuffer[1] = 0;
  packetBuffer[2] = 6;
  packetBuffer[3] = 0xEC;
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 56;
  Udp.beginPacket(address, 123);
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

time_t getNtpTime() {
  if (wifiState != CONNECTED) return 0;
  IPAddress ntpServerIP;
  while (Udp.parsePacket() > 0) Udp.flush();
  if (!WiFi.hostByName(ntpServerName, ntpServerIP)) return 0;
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 3000) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Udp.read(packetBuffer, NTP_PACKET_SIZE);
      unsigned long secsSince1900 =
        ((unsigned long)packetBuffer[40] << 24) | ((unsigned long)packetBuffer[41] << 16) | ((unsigned long)packetBuffer[42] << 8) | ((unsigned long)packetBuffer[43]);
      time_t ntpTime = secsSince1900 - 2208988800UL;
      time_t adjustedTime = ntpTime + getMadridOffset(ntpTime);
      return adjustedTime;
    }
    delay(10);
  }
  return 0;
}

void syncNtpTime() {
  if (wifiState != CONNECTED) {
    if (status_label) lv_label_set_text(status_label, "NO HAY Wi-Fi PARA NTP.");
    return;
  }
  if (!Udp.begin(localPort)) {
    if (status_label) lv_label_set_text(status_label, "UDP FALLO INICIALIZACION.");
    return;
  }
  udp_active = true;
  time_t ntpTime = getNtpTime();
  Udp.stop();
  udp_active = false;
  if (ntpTime != 0) {
    setTime(ntpTime);
    if (status_label) lv_label_set_text(status_label, "HORA SINCRONIZADA OK!");
    setRtcTime(year(), month(), day(), hour(), minute(), second());
  } else {
    if (status_label) lv_label_set_text(status_label, "FALLO SINCRONIZACION HORA.");
  }
}

// ============================= LVGL HANDLERS =============================
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  M5.Display.pushImage(area->x1, area->y1, w, h, (uint16_t *)color_p);
  lv_disp_flush_ready(disp);
}

void my_touchpad_read_cb(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  uint16_t touchX, touchY;
  bool pressed = M5.Display.getTouch(&touchX, &touchY);
  if (pressed) {
    data->state = LV_INDEV_STATE_PR;
    data->point.x = touchX;
    data->point.y = touchY;
  } else {
    data->state = LV_INDEV_STATE_REL;
  }
}

// ============================= UI: FECHA/HORA =============================
void update_rtc_label() {
  if (lv_scr_act() != main_screen) return;
  m5::rtc_datetime_t dt = getRtcTime();
  char date_buf[32], time_buf[32];
  sprintf(date_buf, "%02d/%02d/%04d", dt.date.date, dt.date.month, dt.date.year);
  sprintf(time_buf, "%02d:%02d:%02d", dt.time.hours, dt.time.minutes, dt.time.seconds);
  if (rtc_date_label) lv_label_set_text(rtc_date_label, date_buf);
  if (rtc_time_label) lv_label_set_text(rtc_time_label, time_buf);
}

// ============================= WIFI EVENTOS =============================
void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_START: wifiState = CONNECTING; break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      wifiState = CONNECTED;
      ntpSyncNeeded = true;
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      wifiState = DISCONNECTED;
      if (udp_active) {
        Udp.stop();
        udp_active = false;
      }
      WiFi.reconnect();
      break;
    default: break;
  }
}

void startWiFiConnection(const char *ssid, const char *password) {
  if (!ssid || strlen(ssid) == 0) return;
  if (WiFi.status() == WL_CONNECTED) return;
  if (wifiState == CONNECTING) return;
  WiFi.disconnect(false);
  delay(50);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
  WiFi.setHostname("Tab5-Fermin");
  if (password && strlen(password) > 0) {
    WiFi.begin(ssid, password);
  } else {
    WiFi.begin(ssid);
  }
  wifiState = CONNECTING;
  lastConnectionAttempt = millis();
}

// ============================= WIFI CALLBACKS =============================
void select_wifi_cb(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_target(e);
  lv_obj_t *label = lv_obj_get_child(obj, 1);
  const char *ssid = lv_label_get_text(label);
  lv_textarea_set_text(ssid_input, ssid);
  lv_label_set_text_fmt(status_label, "SSID SELECCIONADO: %s", ssid);
  if (wifi_list) {
    lv_obj_del(wifi_list);
    wifi_list = NULL;
  }
  if (list_container) lv_obj_add_flag(list_container, LV_OBJ_FLAG_HIDDEN);
}

void scan_wifi_cb(lv_event_t *e) {
  lv_label_set_text(status_label, "ESCANEANDO REDES...");
  if (wifi_list) {
    lv_obj_del(wifi_list);
    wifi_list = NULL;
  }
  int n = WiFi.scanNetworks(false, true);
  if (n <= 0) {
    lv_label_set_text(status_label, "NO SE ENCONTRARON REDES.");
    WiFi.scanDelete();
    return;
  }
  lv_label_set_text_fmt(status_label, "%d REDES ENCONTRADAS.", n);
  lv_obj_clear_flag(list_container, LV_OBJ_FLAG_HIDDEN);
  wifi_list = lv_list_create(list_container);
  lv_obj_set_size(wifi_list, LV_PCT(100), LV_PCT(100));
  lv_obj_align(wifi_list, LV_ALIGN_CENTER, 0, 0);
  for (int i = 0; i < n; ++i) {
    String ssid = WiFi.SSID(i);
    lv_obj_t *btn = lv_list_add_btn(wifi_list, LV_SYMBOL_WIFI, ssid.c_str());
    lv_obj_add_event_cb(btn, select_wifi_cb, LV_EVENT_CLICKED, NULL);
  }
  lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
  WiFi.scanDelete();
}

void connect_wifi_cb(lv_event_t *e) {
  saved_ssid = lv_textarea_get_text(ssid_input);
  saved_password = lv_textarea_get_text(pass_input);
  saved_ssid.trim();
  saved_password.trim();
  if (saved_ssid.length() == 0) {
    lv_label_set_text(status_label, "SSID vacío!");
    return;
  }
  preferences.begin("wifi", false);
  preferences.putString("ssid", saved_ssid);
  preferences.putString("pass", saved_password);
  preferences.end();
  lv_label_set_text_fmt(status_label, "CONECTANDO A %s...", saved_ssid.c_str());
  startWiFiConnection(saved_ssid.c_str(), saved_password.c_str());
}

// ============================= NAVEGACIÓN =============================
void goto_wifi_cb(lv_event_t *e) {
  lv_textarea_set_text(ssid_input, saved_ssid.c_str());
  lv_textarea_set_text(pass_input, saved_password.c_str());
  lv_scr_load(wifi_screen);
  if (WiFi.status() == WL_CONNECTED) {
    lv_label_set_text_fmt(status_label, "CONECTADO: %s", WiFi.localIP().toString().c_str());
  } else if (wifiState == CONNECTING) {
    lv_label_set_text(status_label, "ESTADO: CONECTANDO...");
  } else {
    lv_label_set_text(status_label, "ESTADO: NO CONECTADO");
  }
}

void goto_config_cb(lv_event_t *e) {
  lv_scr_load(config_screen);
}
void back_to_main_from_wifi(lv_event_t *e) {
  if (list_container) lv_obj_add_flag(list_container, LV_OBJ_FLAG_HIDDEN);
  lv_scr_load(main_screen);
}
void back_to_main_from_config(lv_event_t *e) {
  lv_scr_load(main_screen);
}

// ============================= CONFIG CALLBACKS =============================
void slider_event_cb(lv_event_t *e) {
  lv_obj_t *slider = lv_event_get_target(e);
  int value = lv_slider_get_value(slider);
  M5.Display.setBrightness(value);
  saved_brightness = value;
  lv_label_set_text_fmt(slider_label, "Brillo: %d", value);
  preferences.begin("wifi", false);
  preferences.putInt("brightness", saved_brightness);
  preferences.end();
}

void temp_disp_slider_event_cb(lv_event_t *e) {
  lv_obj_t *slider = lv_event_get_target(e);
  int value = lv_slider_get_value(slider);
  saved_temp_display_secs = value;
  if (temp_disp_label) lv_label_set_text_fmt(temp_disp_label, "ACTUALIZACION TEMP: %d SEG", value);
  preferences.begin("wifi", false);
  preferences.putInt("temp_disp_secs", value);
  preferences.end();
}

void graph_upd_slider_event_cb(lv_event_t *e) {
  lv_obj_t *slider = lv_event_get_target(e);
  int value = lv_slider_get_value(slider);
  saved_graph_update_mins = value;
  if (graph_upd_label) lv_label_set_text_fmt(graph_upd_label, "ACTUALIZACION GRAFICA: %d MIN", value);
  preferences.begin("wifi", false);
  preferences.putInt("graph_upd_mins", value);
  preferences.end();
}

// ============================= NUEVO: CALLBACKS VOLUMEN Y ALARMAS =============================
void volume_slider_event_cb(lv_event_t *e) {
  lv_obj_t *slider = lv_event_get_target(e);
  int value = lv_slider_get_value(slider);
  saved_volume = value;
  uint8_t speaker_vol = map(value, 0, 100, 0, 255);
  M5.Speaker.setVolume(speaker_vol);
  if (volume_label) lv_label_set_text_fmt(volume_label, "VOLUMEN: %d%%", value);
  preferences.begin("wifi", false);
  preferences.putInt("volume", saved_volume);
  preferences.end();
}

void temp_max_slider_event_cb(lv_event_t *e) {
  lv_obj_t *slider = lv_event_get_target(e);
  int valuemax = lv_slider_get_value(slider);
  saved_temp_max = (float)valuemax;
  if (temp_max_label) {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "TEMP MAX ALARMA: %.1f C", saved_temp_max);
    lv_label_set_text(temp_max_label, buffer);
  }
  preferences.begin("wifi", false);
  preferences.putFloat("temp_max", saved_temp_max);
  preferences.end();
}

void temp_min_slider_event_cb(lv_event_t *e) {
  lv_obj_t *slider = lv_event_get_target(e);
  int valuemin = lv_slider_get_value(slider);
  saved_temp_min = (float)valuemin;
  if (temp_min_label) {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "TEMP MIN ALARMA: %.1f C", saved_temp_min);
    lv_label_set_text(temp_min_label, buffer);
  }
  preferences.begin("wifi", false);
  preferences.putFloat("temp_min", saved_temp_min);
  preferences.end();
}

void create_status_bar() {
  lv_obj_t *top_layer = lv_layer_top();
  const int BUTTON_WIDTH = 200;
  const int BUTTON_MARGIN_RIGHT = 20;
  const int ICON_GAP_FROM_BUTTON = 40;
  const int ICON_MARGIN_X = BUTTON_MARGIN_RIGHT + ICON_GAP_FROM_BUTTON + BUTTON_WIDTH;

  wifi_icon = lv_label_create(top_layer);
  lv_label_set_text(wifi_icon, LV_SYMBOL_WIFI);
  lv_obj_set_style_text_font(wifi_icon, &lv_font_montserrat_32, LV_PART_MAIN);
  lv_obj_set_style_text_color(wifi_icon, lv_color_hex(0x888888), LV_PART_MAIN);
  lv_obj_align(wifi_icon, LV_ALIGN_TOP_RIGHT, -(ICON_MARGIN_X), 50);

  bat_icon = lv_label_create(top_layer);
  lv_label_set_text(bat_icon, LV_SYMBOL_BATTERY_FULL);
  lv_obj_set_style_text_font(bat_icon, &lv_font_montserrat_40, LV_PART_MAIN);
  lv_obj_set_style_text_color(bat_icon, lv_color_make(0, 255, 0), LV_PART_MAIN);
  lv_obj_align_to(bat_icon, wifi_icon, LV_ALIGN_OUT_RIGHT_MID, +50, 0);

  bat_percent_label = lv_label_create(top_layer);
  lv_label_set_text(bat_percent_label, "0%");
  lv_obj_set_style_text_font(bat_percent_label, &lv_font_montserrat_34, LV_PART_MAIN);
  lv_obj_set_style_text_color(bat_percent_label, lv_color_white(), LV_PART_MAIN);
  lv_obj_align_to(bat_percent_label, bat_icon, LV_ALIGN_OUT_RIGHT_MID, +25, 0);
}

// ============================= CREAR PANTALLAS =============================
void create_main_screen() {
  main_screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(main_screen, lv_color_hex(0x111111), LV_PART_MAIN);

  btn_wifi = lv_btn_create(main_screen);
  lv_obj_set_size(btn_wifi, 200, 80);
  lv_obj_align(btn_wifi, LV_ALIGN_TOP_LEFT, 20, 20);
  lv_obj_set_style_bg_color(btn_wifi, lv_color_hex(0x444444), LV_PART_MAIN);
  lv_obj_t *label_wifi = lv_label_create(btn_wifi);
  lv_label_set_text(label_wifi, "WIFI");
  lv_obj_set_style_text_font(label_wifi, &lv_font_montserrat_32, LV_PART_MAIN);
  lv_obj_center(label_wifi);
  lv_obj_add_event_cb(btn_wifi, goto_wifi_cb, LV_EVENT_CLICKED, NULL);

  btn_config = lv_btn_create(main_screen);
  lv_obj_set_size(btn_config, 200, 80);
  lv_obj_align(btn_config, LV_ALIGN_TOP_LEFT, 20, 220);
  lv_obj_set_style_bg_color(btn_config, lv_color_hex(0x444444), LV_PART_MAIN);
  lv_obj_t *label_config = lv_label_create(btn_config);
  lv_label_set_text(label_config, "CONFIG");
  lv_obj_set_style_text_font(label_config, &lv_font_montserrat_32, LV_PART_MAIN);
  lv_obj_center(label_config);
  lv_obj_add_event_cb(btn_config, goto_config_cb, LV_EVENT_CLICKED, NULL);

  btn_temp_main = lv_btn_create(main_screen);
  lv_obj_set_size(btn_temp_main, 200, 80);
  lv_obj_align(btn_temp_main, LV_ALIGN_TOP_LEFT, 20, 420);
  lv_obj_set_style_bg_color(btn_temp_main, lv_color_hex(0x444444), LV_PART_MAIN);
  lv_obj_t *label_temp_btn = lv_label_create(btn_temp_main);
  lv_label_set_text(label_temp_btn, "TEMP");
  lv_obj_set_style_text_font(label_temp_btn, &lv_font_montserrat_32, LV_PART_MAIN);
  lv_obj_center(label_temp_btn);

  btn_weather_main = lv_btn_create(main_screen);
  lv_obj_set_size(btn_weather_main, 200, 80);
  lv_obj_align(btn_weather_main, LV_ALIGN_TOP_LEFT, 20, 620);
  lv_obj_set_style_bg_color(btn_weather_main, lv_color_hex(0x444444), LV_PART_MAIN);
  lv_obj_t *label_weather_btn = lv_label_create(btn_weather_main);
  lv_label_set_text(label_weather_btn, "METEO");
  lv_obj_set_style_text_font(label_weather_btn, &lv_font_montserrat_32, LV_PART_MAIN);
  lv_obj_center(label_weather_btn);

  rtc_date_label = lv_label_create(main_screen);
  lv_label_set_text(rtc_date_label, "00/00/0000");
  lv_obj_set_style_text_font(rtc_date_label, &mini100, LV_PART_MAIN);
  lv_obj_set_style_text_color(rtc_date_label, lv_color_white(), LV_PART_MAIN);
  lv_obj_align(rtc_date_label, LV_ALIGN_CENTER, 0, -80);

  rtc_time_label = lv_label_create(main_screen);
  lv_label_set_text(rtc_time_label, "00:00:00");
  lv_obj_set_style_text_font(rtc_time_label, &mini100, LV_PART_MAIN);
  lv_obj_set_style_text_color(rtc_time_label, lv_color_white(), LV_PART_MAIN);
  lv_obj_align(rtc_time_label, LV_ALIGN_CENTER, 0, 120);
}

void create_wifi_screen() {
  wifi_screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(wifi_screen, lv_color_black(), LV_PART_MAIN);

  status_label = lv_label_create(wifi_screen);
  lv_label_set_text(status_label, "ESTADO: NO CONECTADO");
  lv_obj_set_style_text_color(status_label, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_text_font(status_label, &lv_font_montserrat_32, LV_PART_MAIN);
  lv_obj_set_style_text_align(status_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_align(status_label, LV_ALIGN_TOP_MID, 0, 150);

  btn_back_wifi = lv_btn_create(wifi_screen);
  lv_obj_set_size(btn_back_wifi, 200, 80);
  lv_obj_align(btn_back_wifi, LV_ALIGN_TOP_LEFT, 20, 20);
  lv_obj_set_style_bg_color(btn_back_wifi, lv_color_hex(0x555555), LV_PART_MAIN);
  lv_obj_t *label_back_wifi = lv_label_create(btn_back_wifi);
  lv_label_set_text(label_back_wifi, "VOLVER");
  lv_obj_set_style_text_font(label_back_wifi, &lv_font_montserrat_28, LV_PART_MAIN);
  lv_obj_set_style_text_color(label_back_wifi, lv_color_white(), LV_PART_MAIN);
  lv_obj_center(label_back_wifi);
  lv_obj_add_event_cb(btn_back_wifi, back_to_main_from_wifi, LV_EVENT_CLICKED, NULL);

  scan_btn = lv_btn_create(wifi_screen);
  lv_obj_set_size(scan_btn, 200, 80);
  lv_obj_align(scan_btn, LV_ALIGN_TOP_MID, 0, 20);
  lv_obj_set_style_bg_color(scan_btn, lv_color_hex(0x555555), LV_PART_MAIN);
  lv_obj_t *label_scan_btn = lv_label_create(scan_btn);
  lv_label_set_text(label_scan_btn, "ESCANEAR");
  lv_obj_set_style_text_font(label_scan_btn, &lv_font_montserrat_28, LV_PART_MAIN);
  lv_obj_set_style_text_color(label_scan_btn, lv_color_white(), LV_PART_MAIN);
  lv_obj_center(label_scan_btn);
  lv_obj_add_event_cb(scan_btn, scan_wifi_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *container = lv_obj_create(wifi_screen);
  lv_obj_set_size(container, EXAMPLE_LCD_H_RES - 40, 200);
  lv_obj_align(container, LV_ALIGN_TOP_MID, 0, 210);
  lv_obj_set_style_bg_color(container, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_border_width(container, 0, LV_PART_MAIN);

  ssid_input = lv_textarea_create(container);
  lv_obj_set_width(ssid_input, LV_PCT(95));
  lv_obj_set_style_text_font(ssid_input, &lv_font_montserrat_30, LV_PART_MAIN);
  lv_textarea_set_placeholder_text(ssid_input, "SSID");
  lv_obj_align(ssid_input, LV_ALIGN_TOP_MID, 0, -10);
  lv_textarea_set_one_line(ssid_input, true);
  lv_obj_set_style_bg_color(ssid_input, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_text_color(ssid_input, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_border_color(ssid_input, lv_color_white(), LV_PART_MAIN);

  pass_input = lv_textarea_create(container);
  lv_obj_set_width(pass_input, LV_PCT(95));
  lv_obj_set_style_text_font(pass_input, &lv_font_montserrat_30, LV_PART_MAIN);
  lv_textarea_set_placeholder_text(pass_input, "PASSWORD");
  lv_obj_align(pass_input, LV_ALIGN_TOP_MID, 0, 50);
  lv_textarea_set_one_line(pass_input, true);
  lv_textarea_set_password_mode(pass_input, true);
  lv_obj_set_style_bg_color(pass_input, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_text_color(pass_input, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_border_color(pass_input, lv_color_white(), LV_PART_MAIN);

  lv_obj_t *btn_connect = lv_btn_create(container);
  lv_obj_set_size(btn_connect, 300, 60);
  lv_obj_align(btn_connect, LV_ALIGN_TOP_MID, 0, 120);
  lv_obj_set_style_bg_color(btn_connect, lv_color_hex(0x444444), LV_PART_MAIN);
  lv_obj_t *label_btn = lv_label_create(btn_connect);
  lv_label_set_text(label_btn, "CONECTAR WiFi");
  lv_obj_set_style_text_font(label_btn, &lv_font_montserrat_30, LV_PART_MAIN);
  lv_obj_set_style_text_color(label_btn, lv_color_white(), LV_PART_MAIN);
  lv_obj_center(label_btn);
  lv_obj_add_event_cb(btn_connect, connect_wifi_cb, LV_EVENT_CLICKED, NULL);

  keyboard = lv_keyboard_create(wifi_screen);
  lv_obj_set_size(keyboard, EXAMPLE_LCD_H_RES, 300);
  lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_keyboard_set_textarea(keyboard, ssid_input);
  lv_obj_set_style_bg_color(keyboard, lv_color_hex(0x222222), LV_PART_MAIN);
  lv_obj_set_style_border_width(keyboard, 0, LV_PART_MAIN);
  lv_obj_set_style_bg_color(keyboard, lv_color_hex(0x444444), LV_PART_ITEMS);
  lv_obj_set_style_text_color(keyboard, lv_color_white(), LV_PART_ITEMS);
  lv_obj_set_style_text_font(keyboard, &lv_font_montserrat_32, LV_PART_ITEMS);

  lv_obj_add_event_cb(
    ssid_input, [](lv_event_t *e) {
      lv_keyboard_set_textarea(keyboard, ssid_input);
      lv_obj_clear_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
      if (list_container) lv_obj_add_flag(list_container, LV_OBJ_FLAG_HIDDEN);
    },
    LV_EVENT_CLICKED, NULL);

  lv_obj_add_event_cb(
    pass_input, [](lv_event_t *e) {
      lv_keyboard_set_textarea(keyboard, pass_input);
      lv_obj_clear_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
      if (list_container) lv_obj_add_flag(list_container, LV_OBJ_FLAG_HIDDEN);
    },
    LV_EVENT_CLICKED, NULL);

  list_container = lv_list_create(wifi_screen);
  lv_obj_set_size(list_container, 700, 450);
  lv_obj_align(list_container, LV_ALIGN_CENTER, 0, 150);
  lv_obj_set_style_bg_color(list_container, lv_color_hex(0x222222), LV_PART_MAIN);
  lv_obj_set_style_border_width(list_container, 2, LV_PART_MAIN);
  lv_obj_set_style_border_color(list_container, lv_color_hex(0xAAAAAA), LV_PART_MAIN);
  lv_obj_add_flag(list_container, LV_OBJ_FLAG_HIDDEN);
}

void create_config_screen() {
  config_screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(config_screen, lv_color_hex(0x111111), LV_PART_MAIN);

  btn_back_config = lv_btn_create(config_screen);
  lv_obj_set_size(btn_back_config, 200, 80);
  lv_obj_align(btn_back_config, LV_ALIGN_TOP_LEFT, 20, 20);
  lv_obj_set_style_bg_color(btn_back_config, lv_color_hex(0x555555), LV_PART_MAIN);
  lv_obj_t *label_back_config = lv_label_create(btn_back_config);
  lv_label_set_text(label_back_config, "VOLVER");
  lv_obj_set_style_text_font(label_back_config, &lv_font_montserrat_28, LV_PART_MAIN);
  lv_obj_center(label_back_config);
  lv_obj_add_event_cb(btn_back_config, back_to_main_from_config, LV_EVENT_CLICKED, NULL);

  // Slider Brillo
  lv_obj_t *slider = lv_slider_create(config_screen);
  lv_obj_set_size(slider, 400, 30);
  lv_obj_align(slider, LV_ALIGN_TOP_MID, 0, 60);
  lv_slider_set_range(slider, 0, 255);
  lv_slider_set_value(slider, saved_brightness, LV_ANIM_OFF);
  lv_obj_set_style_bg_color(slider, lv_color_hex(0x444444), LV_PART_MAIN);
  lv_obj_set_style_bg_color(slider, LV_COLOR_VERDE_MAR, LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(slider, LV_COLOR_VERDE_MAR, LV_PART_KNOB);
  slider_label = lv_label_create(config_screen);
  lv_label_set_text_fmt(slider_label, "BRILLO: %d", saved_brightness);
  lv_obj_set_style_text_color(slider_label, LV_COLOR_VERDE_MAR, LV_PART_MAIN);
  lv_obj_align(slider_label, LV_ALIGN_TOP_MID, 0, 110);
  lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

  // NUEVO: Slider Volumen
  volume_slider = lv_slider_create(config_screen);
  lv_obj_set_size(volume_slider, 400, 30);
  lv_obj_align(volume_slider, LV_ALIGN_TOP_MID, 0, 160);
  lv_slider_set_range(volume_slider, 0, 100);
  lv_slider_set_value(volume_slider, saved_volume, LV_ANIM_OFF);
  lv_obj_set_style_bg_color(volume_slider, lv_color_hex(0x444444), LV_PART_MAIN);
  lv_obj_set_style_bg_color(volume_slider, LV_COLOR_MANDARINA, LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(volume_slider, LV_COLOR_MANDARINA, LV_PART_KNOB);
  volume_label = lv_label_create(config_screen);
  lv_label_set_text_fmt(volume_label, "VOLUMEN: %d%%", saved_volume);
  lv_obj_set_style_text_color(volume_label, LV_COLOR_MANDARINA, LV_PART_MAIN);
  lv_obj_align(volume_label, LV_ALIGN_TOP_MID, 0, 210);
  lv_obj_add_event_cb(volume_slider, volume_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

  // Slider Temp Display
  temp_disp_slider = lv_slider_create(config_screen);
  lv_obj_set_size(temp_disp_slider, 400, 30);
  lv_obj_align(temp_disp_slider, LV_ALIGN_TOP_MID, 0, 260);
  lv_slider_set_range(temp_disp_slider, 1, 30);
  lv_slider_set_value(temp_disp_slider, saved_temp_display_secs, LV_ANIM_OFF);
  lv_obj_set_style_bg_color(temp_disp_slider, lv_color_hex(0x444444), LV_PART_MAIN);
  lv_obj_set_style_bg_color(temp_disp_slider, LV_COLOR_PLATA, LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(temp_disp_slider, LV_COLOR_PLATA, LV_PART_KNOB);
  temp_disp_label = lv_label_create(config_screen);
  lv_obj_set_style_text_color(temp_disp_label, LV_COLOR_PLATA, LV_PART_MAIN);
  lv_label_set_text_fmt(temp_disp_label, "ACTUALIZACION TEMP: %d SEG", saved_temp_display_secs);
  lv_obj_align(temp_disp_label, LV_ALIGN_TOP_MID, 0, 310);
  lv_obj_add_event_cb(temp_disp_slider, temp_disp_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

  // Slider Graph Update
  graph_upd_slider = lv_slider_create(config_screen);
  lv_obj_set_size(graph_upd_slider, 400, 30);
  lv_obj_align(graph_upd_slider, LV_ALIGN_TOP_MID, 0, 360);
  lv_slider_set_range(graph_upd_slider, 1, 60);
  lv_slider_set_value(graph_upd_slider, saved_graph_update_mins, LV_ANIM_OFF);
  lv_obj_set_style_bg_color(graph_upd_slider, lv_color_hex(0x444444), LV_PART_MAIN);
  lv_obj_set_style_bg_color(graph_upd_slider, LV_COLOR_AZUL_CIELO, LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(graph_upd_slider, LV_COLOR_AZUL_CIELO, LV_PART_KNOB);
  graph_upd_label = lv_label_create(config_screen);
  lv_obj_set_style_text_color(graph_upd_label, LV_COLOR_AZUL_CIELO, LV_PART_MAIN);
  lv_label_set_text_fmt(graph_upd_label, "ACTUALIZACION GRAFICA: %d MIN", saved_graph_update_mins);
  lv_obj_align(graph_upd_label, LV_ALIGN_TOP_MID, 0, 410);
  lv_obj_add_event_cb(graph_upd_slider, graph_upd_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

  // NUEVO: Slider Temp Max Alarm
  temp_max_slider = lv_slider_create(config_screen);
  lv_obj_set_size(temp_max_slider, 400, 30);
  lv_obj_align(temp_max_slider, LV_ALIGN_TOP_MID, 0, 460);
  lv_slider_set_range(temp_max_slider, -20, 60);
  lv_slider_set_value(temp_max_slider, (int)saved_temp_max, LV_ANIM_OFF);
  lv_obj_set_style_bg_color(temp_max_slider, lv_color_hex(0x444444), LV_PART_MAIN);
  lv_obj_set_style_bg_color(temp_max_slider, lv_color_hex(0xFF0000), LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(temp_max_slider, lv_color_hex(0xFF0000), LV_PART_KNOB);
  temp_max_label = lv_label_create(config_screen);
  lv_obj_set_style_text_color(temp_max_label, lv_color_hex(0xFF0000), LV_PART_MAIN);
  char max_buf[64];
  snprintf(max_buf, sizeof(max_buf), "TEMP MAX ALARMA: %d C", (int)saved_temp_max);
  lv_label_set_text(temp_max_label, max_buf);
  lv_obj_align(temp_max_label, LV_ALIGN_TOP_MID, 0, 510);
  lv_obj_add_event_cb(temp_max_slider, temp_max_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

  // NUEVO: Slider Temp Min Alarm
  temp_min_slider = lv_slider_create(config_screen);
  lv_obj_set_size(temp_min_slider, 400, 30);
  lv_obj_align(temp_min_slider, LV_ALIGN_TOP_MID, 0, 560);
  lv_slider_set_range(temp_min_slider, -20, 60);
  lv_slider_set_value(temp_min_slider, (int)saved_temp_min, LV_ANIM_OFF);
  lv_obj_set_style_bg_color(temp_min_slider, lv_color_hex(0x444444), LV_PART_MAIN);
  lv_obj_set_style_bg_color(temp_min_slider, lv_color_hex(0x0066FF), LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(temp_min_slider, lv_color_hex(0x0066FF), LV_PART_KNOB);
  temp_min_label = lv_label_create(config_screen);
  lv_obj_set_style_text_color(temp_min_label, lv_color_hex(0x0066FF), LV_PART_MAIN);
  char min_buf[64];
  snprintf(min_buf, sizeof(min_buf), "TEMP MIN ALARMA: %d C", (int)saved_temp_min);
  lv_label_set_text(temp_min_label, min_buf);
  lv_obj_align(temp_min_label, LV_ALIGN_TOP_MID, 0, 610);
  lv_obj_add_event_cb(temp_min_slider, temp_min_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

  bat_label = lv_label_create(config_screen);
  lv_obj_align(bat_label, LV_ALIGN_BOTTOM_MID, 0, -20);
  lv_obj_set_style_text_color(bat_label, lv_color_white(), LV_PART_MAIN);
  lv_label_set_text(bat_label, "CARGANDO INFO...");
  lv_obj_set_style_text_font(bat_label, &lv_font_montserrat_28, LV_PART_MAIN);
}

void create_temp_screen() {
  temp_screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(temp_screen, lv_color_hex(0x222222), LV_PART_MAIN);

  btn_back_temp = lv_btn_create(temp_screen);
  lv_obj_set_size(btn_back_temp, 200, 80);
  lv_obj_align(btn_back_temp, LV_ALIGN_TOP_LEFT, 20, 20);
  lv_obj_set_style_bg_color(btn_back_temp, lv_color_hex(0x555555), LV_PART_MAIN);
  lv_obj_t *label_back_temp = lv_label_create(btn_back_temp);
  lv_label_set_text(label_back_temp, "VOLVER");
  lv_obj_set_style_text_font(label_back_temp, &lv_font_montserrat_28, LV_PART_MAIN);
  lv_obj_center(label_back_temp);
  lv_obj_add_event_cb(
    btn_back_temp, [](lv_event_t *e) {
      lv_scr_load(main_screen);
    },
    LV_EVENT_CLICKED, NULL);

  temp_label = lv_label_create(temp_screen);
  lv_label_set_text(temp_label, "TEMPERATURA: --.- C");
  lv_obj_set_style_text_font(temp_label, &lv_font_montserrat_48, LV_PART_MAIN);
  lv_obj_set_style_text_color(temp_label, lv_color_white(), LV_PART_MAIN);
  lv_obj_align(temp_label, LV_ALIGN_TOP_MID, 0, 120);

  // NUEVO: Labels para temperatura mínima y máxima
  temp_min_display_label = lv_label_create(temp_screen);
  lv_label_set_text(temp_min_display_label, "TEMP MIN\n--.- C");
  lv_obj_set_style_text_font(temp_min_display_label, &lv_font_montserrat_32, LV_PART_MAIN);
  lv_obj_set_style_text_color(temp_min_display_label, lv_color_hex(0x0066FF), LV_PART_MAIN);
  lv_obj_set_style_text_align(temp_min_display_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_align(temp_min_display_label, LV_ALIGN_TOP_LEFT, 30, 120);

  temp_max_display_label = lv_label_create(temp_screen);
  lv_label_set_text(temp_max_display_label, "TEMP MAX\n--.- C");
  lv_obj_set_style_text_font(temp_max_display_label, &lv_font_montserrat_32, LV_PART_MAIN);
  lv_obj_set_style_text_color(temp_max_display_label, LV_COLOR_NARANJA_PURO, LV_PART_MAIN);
  lv_obj_set_style_text_align(temp_max_display_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_align(temp_max_display_label, LV_ALIGN_TOP_RIGHT, -30, 120);

  // NUEVO: Label estado alarma
  alarm_status_label = lv_label_create(temp_screen);
  lv_label_set_text(alarm_status_label, "TEMPERATURA NORMAL");
  lv_obj_set_style_text_font(alarm_status_label, &lv_font_montserrat_32, LV_PART_MAIN);
  lv_obj_set_style_text_color(alarm_status_label, lv_color_make(0, 255, 0), LV_PART_MAIN);
  lv_obj_align(alarm_status_label, LV_ALIGN_TOP_MID, 0, 180);

  const int BAR_WIDTH_PIXELS = 50;
  const int BAR_SPACING_PIXELS = 50;
  const int AXIS_LABEL_SPACE = 80;
  const int BASE_X_OFFSET = 10;
  const int CHART_HEIGHT = 480;
  int CHART_WIDTH = EXAMPLE_LCD_H_RES - (AXIS_LABEL_SPACE + BASE_X_OFFSET + 10);
  if (CHART_WIDTH < 200) CHART_WIDTH = EXAMPLE_LCD_H_RES - 10;
  int VISIBLE_POINTS = CHART_WIDTH / (BAR_WIDTH_PIXELS + BAR_SPACING_PIXELS);
  if (VISIBLE_POINTS < 10) VISIBLE_POINTS = 10;

  temp_chart = lv_chart_create(temp_screen);
  lv_obj_set_size(temp_chart, CHART_WIDTH, CHART_HEIGHT);
  lv_obj_align(temp_chart, LV_ALIGN_TOP_LEFT, BASE_X_OFFSET + AXIS_LABEL_SPACE, 240);
  lv_chart_set_type(temp_chart, LV_CHART_TYPE_LINE);
  lv_chart_set_update_mode(temp_chart, LV_CHART_UPDATE_MODE_SHIFT);
  lv_chart_set_point_count(temp_chart, VISIBLE_POINTS);
  lv_chart_set_range(temp_chart, LV_CHART_AXIS_PRIMARY_Y, -20, 60);
  lv_chart_set_axis_tick(temp_chart, LV_CHART_AXIS_PRIMARY_Y, 5, 2, 9, 10, true, 50);
  lv_obj_set_style_pad_left(temp_chart, AXIS_LABEL_SPACE, LV_PART_MAIN);
  lv_obj_set_style_width(temp_chart, BAR_WIDTH_PIXELS, LV_PART_ITEMS);
  lv_obj_set_style_pad_column(temp_chart, BAR_SPACING_PIXELS, LV_PART_MAIN);
  lv_obj_set_style_line_width(temp_chart, 0, LV_PART_MAIN);
  lv_obj_set_style_border_width(temp_chart, 0, LV_PART_MAIN);
  lv_obj_set_style_text_font(temp_chart, &lv_font_montserrat_28, LV_PART_MAIN);
  lv_obj_set_style_bg_color(temp_chart, lv_color_hex(0x333333), LV_PART_MAIN);
  lv_obj_set_style_bg_color(temp_chart, LV_COLOR_TURQUESA, LV_PART_ITEMS);
  temp_series = lv_chart_add_series(temp_chart, LV_COLOR_TURQUESA, LV_CHART_AXIS_PRIMARY_Y);
  for (int i = 0; i < VISIBLE_POINTS; ++i) {
    lv_chart_set_next_value(temp_chart, temp_series, LV_CHART_POINT_NONE);
  }
}

void create_weather_screen() {
  weather_screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(weather_screen, lv_color_hex(0x111111), LV_PART_MAIN);

  btn_back_weather = lv_btn_create(weather_screen);
  lv_obj_set_size(btn_back_weather, 200, 80);
  lv_obj_align(btn_back_weather, LV_ALIGN_TOP_LEFT, 20, 20);
  lv_obj_set_style_bg_color(btn_back_weather, lv_color_hex(0x555555), LV_PART_MAIN);
  lv_obj_t *label_back_weather = lv_label_create(btn_back_weather);
  lv_label_set_text(label_back_weather, "VOLVER");
  lv_obj_set_style_text_font(label_back_weather, &lv_font_montserrat_28, LV_PART_MAIN);
  lv_obj_center(label_back_weather);
  lv_obj_add_event_cb(
    btn_back_weather, [](lv_event_t *e) {
      lv_scr_load(main_screen);
    },
    LV_EVENT_CLICKED, NULL);

  weather_temp_label = lv_label_create(weather_screen);
  lv_label_set_text(weather_temp_label, "TEMPERATURA: --");
  lv_obj_set_style_text_font(weather_temp_label, &lv_font_montserrat_48, LV_PART_MAIN);
  lv_obj_set_style_text_color(weather_temp_label, lv_color_white(), LV_PART_MAIN);
  lv_obj_align(weather_temp_label, LV_ALIGN_CENTER, 0, -150);

  weather_cond_label = lv_label_create(weather_screen);
  lv_label_set_text(weather_cond_label, "ESTADO: --");
  lv_obj_set_style_text_font(weather_cond_label, &lv_font_montserrat_48, LV_PART_MAIN);
  lv_obj_set_style_text_color(weather_cond_label, lv_color_white(), LV_PART_MAIN);
  lv_obj_align(weather_cond_label, LV_ALIGN_CENTER, 0, -50);

  weather_hum_label = lv_label_create(weather_screen);
  lv_label_set_text(weather_hum_label, "HUMEDAD: --");
  lv_obj_set_style_text_font(weather_hum_label, &lv_font_montserrat_32, LV_PART_MAIN);
  lv_obj_set_style_text_color(weather_hum_label, lv_color_white(), LV_PART_MAIN);
  lv_obj_align(weather_hum_label, LV_ALIGN_CENTER, 0, 50);

  weather_press_label = lv_label_create(weather_screen);
  lv_label_set_text(weather_press_label, "PRESION: --");
  lv_obj_set_style_text_font(weather_press_label, &lv_font_montserrat_32, LV_PART_MAIN);
  lv_obj_set_style_text_color(weather_press_label, lv_color_white(), LV_PART_MAIN);
  lv_obj_align(weather_press_label, LV_ALIGN_CENTER, 0, 100);

  weather_status_label = lv_label_create(weather_screen);
  lv_label_set_text(weather_status_label, "ACTUALIZANDO...");
  lv_obj_set_style_text_font(weather_status_label, &lv_font_montserrat_32, LV_PART_MAIN);
  lv_obj_set_style_text_color(weather_status_label, lv_color_white(), LV_PART_MAIN);
  lv_obj_align(weather_status_label, LV_ALIGN_CENTER, 0, 200);
}

String traducirCondicion(String mainCond, String descCond) {
  mainCond.toLowerCase();
  descCond.toLowerCase();
  if (mainCond == "clear") return "CIELOS DESPEJADOS";
  if (mainCond == "clouds") {
    if (descCond.indexOf("few") >= 0) return "POCO NUBOSO";
    if (descCond.indexOf("scattered") >= 0) return "INTERVALOS NUBOSOS";
    if (descCond.indexOf("broken") >= 0) return "MUY NUBOSO";
    if (descCond.indexOf("overcast") >= 0) return "CUBIERTO";
    return "NUBOSIDAD VARIABLE";
  }
  if (mainCond == "rain") {
    if (descCond.indexOf("light") >= 0) return "LLUVIA DEBIL";
    if (descCond.indexOf("moderate") >= 0) return "LLUVIA MODERADA";
    if (descCond.indexOf("heavy") >= 0) return "LLUVIA INTENSA";
    return "LLUVIA";
  }
  if (mainCond == "drizzle") return "LLOVIZNA";
  if (mainCond == "thunderstorm") return "TORMENTA ELECTRICA";
  if (mainCond == "snow") {
    if (descCond.indexOf("light") >= 0) return "NIEVE DEBIL";
    if (descCond.indexOf("heavy") >= 0) return "NIEVE INTENSA";
    return "NEVADAS";
  }
  if (mainCond == "mist") return "NEBLINA";
  if (mainCond == "fog") return "NIEBLA";
  if (mainCond == "haze") return "CALIMA";
  if (mainCond == "smoke") return "HUMO";
  if (mainCond == "dust") return "POLVO EN SUSPENSION";
  if (mainCond == "sand") return "ARENA EN SUSPENSION";
  if (mainCond == "tornado") return "TORNADO";
  return "CONDICION DESCONOCIDA";
}

void update_weather() {
  if (wifiState != CONNECTED) {
    if (weather_status_label) lv_label_set_text(weather_status_label, "NO HAY Wi-Fi");
    return;
  }

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  String url = String("https://api.openweathermap.org/data/2.5/weather?q=") + weatherCity + "&appid=" + weatherApiKey + "&units=metric&lang=es";
  http.begin(client, url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
      if (weather_status_label) lv_label_set_text(weather_status_label, "Error en JSON");
      return;
    }

    float temperaturew = doc["main"]["temp"];
    String mainCond = doc["weather"][0]["main"];
    String descCond = doc["weather"][0]["description"];
    String cond = traducirCondicion(mainCond, descCond);
    int hum = doc["main"]["humidity"];
    int press = doc["main"]["pressure"];

    char tempw_buf[16];
    dtostrf(temperaturew, 4, 1, tempw_buf);
    if (weather_temp_label) {
      String tempStr = String("TEMPERATURA: ") + tempw_buf + " C";
      lv_label_set_text(weather_temp_label, tempStr.c_str());
      lv_obj_invalidate(weather_temp_label);
    }
    if (weather_cond_label) {
      lv_label_set_text_fmt(weather_cond_label, "ESTADO: %s", cond.c_str());
      lv_obj_invalidate(weather_cond_label);
    }
    if (weather_hum_label) {
      lv_label_set_text_fmt(weather_hum_label, "HUMEDAD: %d %%", hum);
      lv_obj_invalidate(weather_hum_label);
    }
    if (weather_press_label) {
      lv_label_set_text_fmt(weather_press_label, "PRESION: %d Mb", press);
      lv_obj_invalidate(weather_press_label);
    }
    if (weather_status_label) {
      lv_label_set_text(weather_status_label, "ACTUALIZADO OK");
      lv_obj_invalidate(weather_status_label);
    }
  } else {
    if (weather_status_label) lv_label_set_text_fmt(weather_status_label, "ERROR: %d", httpCode);
  }
  http.end();
}

void ui_init() {
  create_main_screen();
  create_wifi_screen();
  create_config_screen();
  create_temp_screen();
  create_weather_screen();
  create_status_bar();

  if (btn_temp_main && temp_screen) {
    lv_obj_add_event_cb(
      btn_temp_main, [](lv_event_t *e) {
        lv_scr_load(temp_screen);
      },
      LV_EVENT_CLICKED, NULL);
  }

  if (btn_weather_main && weather_screen) {
    lv_obj_add_event_cb(
      btn_weather_main, [](lv_event_t *e) {
        lv_scr_load(weather_screen);
        update_weather();
        lastWeatherUpdate = millis();
      },
      LV_EVENT_CLICKED, NULL);
  }

  lv_scr_load(main_screen);
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  Serial.begin(115200);
  M5.Display.setRotation(3);

  lv_init();
  buf1 = (lv_color_t *)heap_caps_malloc(sizeof(lv_color_t) * LVGL_LCD_BUF_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  buf2 = (lv_color_t *)heap_caps_malloc(sizeof(lv_color_t) * LVGL_LCD_BUF_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if (!buf1 || !buf2)
    while (true)
      ;

  lv_disp_draw_buf_init(&draw_buf, buf1, buf2, LVGL_LCD_BUF_SIZE);

  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = EXAMPLE_LCD_H_RES;
  disp_drv.ver_res = EXAMPLE_LCD_V_RES;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  disp_drv.sw_rotate = 0;
  disp_drv.rotated = LV_DISP_ROT_NONE;
  lv_disp_drv_register(&disp_drv);

  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read_cb;
  lv_indev_drv_register(&indev_drv);

  rtc_init();

  preferences.begin("wifi", false);
  saved_ssid = preferences.getString("ssid", "");
  saved_password = preferences.getString("pass", "");
  saved_brightness = preferences.getInt("brightness", 200);
  saved_temp_display_secs = preferences.getInt("temp_disp_secs", 1);
  saved_graph_update_mins = preferences.getInt("graph_upd_mins", 10);
  saved_volume = preferences.getInt("volume", 50);
  saved_temp_max = preferences.getFloat("temp_max", 30.0);
  saved_temp_min = preferences.getFloat("temp_min", 10.0);
  preferences.end();

  WiFi.setPins(SDIO2_CLK, SDIO2_CMD, SDIO2_D0, SDIO2_D1, SDIO2_D2, SDIO2_D3, SDIO2_RST);
  WiFi.setAutoReconnect(true);
  startWiFiConnection(saved_ssid.c_str(), saved_password.c_str());
  WiFi.onEvent(WiFiEvent);

  M5.Display.setBrightness(saved_brightness);

  // Inicializar speaker con volumen guardado
  M5.Speaker.begin();
  uint8_t speaker_vol = map(saved_volume, 0, 100, 0, 255);
  M5.Speaker.setVolume(speaker_vol);

  m5::rtc_datetime_t dt = getRtcTime();
  if (dt.date.year < 2020 || dt.date.year > 2099) {
    if (saved_ssid.length() > 0) {
      startWiFiConnection(saved_ssid.c_str(), saved_password.c_str());
    }
  } else {
    setTime(dt.time.hours, dt.time.minutes, dt.time.seconds, dt.date.date, dt.date.month, dt.date.year);
  }

  M5.Ex_I2C.begin();
  Wire.begin(M5.Ex_I2C.getSDA(), M5.Ex_I2C.getSCL());
  tempsensor.begin(0x48, Wire);

  SPIFFS.begin();
  SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);
  if (!SD.begin(SD_SPI_CS_PIN, SPI, 25000000)) {
    Serial.println("SD card not detected");
  }

  ui_init();
  update_rtc_label();
  lastTempUpdate = millis();
  lastChartUpdate = millis();
  tempAccum = 0.0f;
  tempAccumCount = 0;
}

void update_wifi_icon() {
  if (!wifi_icon) return;
  switch (wifiState) {
    case CONNECTED:
      lv_obj_clear_flag(wifi_icon, LV_OBJ_FLAG_HIDDEN);
      lv_obj_set_style_text_color(wifi_icon, lv_color_hex(0x00FF00), LV_PART_MAIN);
      break;
    case CONNECTING:
      if (millis() - lastBlink > 250) {
        lastBlink = millis();
        wifi_icon_visible = !wifi_icon_visible;
        if (wifi_icon_visible) {
          lv_obj_clear_flag(wifi_icon, LV_OBJ_FLAG_HIDDEN);
          lv_obj_set_style_text_color(wifi_icon, lv_color_hex(0xFFFF00), LV_PART_MAIN);
        } else {
          lv_obj_add_flag(wifi_icon, LV_OBJ_FLAG_HIDDEN);
        }
      }
      break;
    case DISCONNECTED:
    default:
      lv_obj_clear_flag(wifi_icon, LV_OBJ_FLAG_HIDDEN);
      lv_obj_set_style_text_color(wifi_icon, lv_color_hex(0xFF0000), LV_PART_MAIN);
      break;
  }
}

void loop() {
  M5.update();
  lv_timer_handler();
  update_wifi_icon();

  if (ntpSyncNeeded && wifiState == CONNECTED) {
    ntpSyncNeeded = false;
    syncNtpTime();
  }

  if (wifiState == CONNECTING && (millis() - lastConnectionAttempt) > connectionTimeout) {
    WiFi.disconnect(false);
    wifiState = DISCONNECTED;
    if (status_label) lv_label_set_text(status_label, "FALLO DE CONEXION.");
  }

  static unsigned long lastRtcUpdate = 0;
  if (millis() - lastRtcUpdate >= 1000) {
    lastRtcUpdate = millis();
    update_rtc_label();
  }

  static unsigned long lastStatusUpdate = 0;
  if (millis() - lastStatusUpdate >= 1000) {
    lastStatusUpdate = millis();
    bool bat_ischarging = M5.Power.isCharging();
    int bat_mv = M5.Power.getBatteryVoltage();
    int bat_vol_entera = bat_mv / 1000;
    int bat_vol_decimal = (bat_mv % 1000) / 100;
    int bat_level = M5.Power.getBatteryLevel();

    if (bat_icon) {
      const char *bat_symbol;
      lv_color_t bat_color;
      if (bat_ischarging) {
        bat_symbol = LV_SYMBOL_CHARGE;
        bat_color = lv_color_make(0, 150, 255);
      } else if (bat_level > 75) {
        bat_symbol = LV_SYMBOL_BATTERY_FULL;
        bat_color = lv_color_make(0, 255, 0);
      } else if (bat_level > 40) {
        bat_symbol = LV_SYMBOL_BATTERY_3;
        bat_color = lv_color_make(255, 255, 0);
      } else if (bat_level > 10) {
        bat_symbol = LV_SYMBOL_BATTERY_1;
        bat_color = lv_color_make(255, 128, 0);
      } else {
        bat_symbol = LV_SYMBOL_BATTERY_EMPTY;
        bat_color = lv_color_make(255, 0, 0);
      }
      lv_label_set_text(bat_icon, bat_symbol);
      lv_obj_set_style_text_color(bat_icon, bat_color, LV_PART_MAIN);
    }

    if (bat_percent_label) {
      lv_label_set_text_fmt(bat_percent_label, "%d%%", bat_level);
      lv_obj_set_style_text_color(bat_percent_label, bat_level < 10 ? lv_color_make(255, 0, 0) : lv_color_white(), LV_PART_MAIN);
    }

    if (bat_label) {
      lv_label_set_text_fmt(bat_label, "BATERIA: %d.%dV (%d%%) - %s", bat_vol_entera, bat_vol_decimal, bat_level, bat_ischarging ? "CARGANDO" : "DESCARGANDO");
    }
  }

  // Actualización de temperatura
  static bool firstTempSet = false;
  if (millis() - lastTempUpdate >= (unsigned long)saved_temp_display_secs * 1000UL) {
    lastTempUpdate = millis();
    float temperatura = tempsensor.readTempC();

    // Actualizar min/max
    if (temperatura < current_temp_min) current_temp_min = temperatura;
    if (temperatura > current_temp_max) current_temp_max = temperatura;

    char temp_buf[16];
    dtostrf(temperatura, 4, 1, temp_buf);
    if (temp_label) {
      String tempStr = String("TEMPERATURA: ") + temp_buf + " C";
      lv_label_set_text(temp_label, tempStr.c_str());
      lv_obj_invalidate(temp_label);
    }

    // Actualizar labels min/max
    if (temp_min_display_label && current_temp_min < 999.0) {
      char min_buf[32];
      char min_temp_str[16];
      dtostrf(current_temp_min, 4, 1, min_temp_str);
      snprintf(min_buf, sizeof(min_buf), "TEMP MIN\n%s C", min_temp_str);
      lv_label_set_text(temp_min_display_label, min_buf);
      lv_obj_invalidate(temp_min_display_label);
    }

    if (temp_max_display_label && current_temp_max > -999.0) {
      char max_buf[32];
      char max_temp_str[16];
      dtostrf(current_temp_max, 4, 1, max_temp_str);
      snprintf(max_buf, sizeof(max_buf), "TEMP MAX\n%s C", max_temp_str);
      lv_label_set_text(temp_max_display_label, max_buf);
      lv_obj_invalidate(temp_max_display_label);
    }

    // NUEVO: Verificar alarmas
    checkTemperatureAlarm(temperatura);

    if (!firstTempSet && temp_chart && temp_series) {
      lv_chart_set_next_value(temp_chart, temp_series, (int)round(temperatura));
      lv_chart_refresh(temp_chart);
      firstTempSet = true;
    }
    tempAccum += temperatura;
    tempAccumCount++;
  }

  if (millis() - lastChartUpdate >= (unsigned long)saved_graph_update_mins * 60UL * 1000UL) {
    lastChartUpdate = millis();
    if (tempAccumCount > 0) {
      float avg = tempAccum / tempAccumCount;
      int avgInt = (int)round(avg);
      if (temp_chart && temp_series) {
        lv_chart_set_next_value(temp_chart, temp_series, avgInt);
        lv_chart_refresh(temp_chart);
      }
    }
    tempAccum = 0.0f;
    tempAccumCount = 0;
  }

  if (lv_scr_act() == weather_screen && millis() - lastWeatherUpdate >= weatherUpdateInterval) {
    lastWeatherUpdate = millis();
    update_weather();
  }
}