#include "display.h"

#if defined(ARDUINO)
#define ENABLE_ST7XX_FRAMEBUFFER (1)

#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <stdarg.h>

#include "icons.h"
#include "scheduler.h"
#include "trace.h"

// Display:
// [ BATT %  ]      [NFC ]
// [ SIG     ]      [Wifi]
// [        Speed        ]
// [        Speed        ]
// [        Status       ]

#define DISPLAY_INTERVAL_MS (75)
#define DISPLAY_W (240)
#define DISPLAY_H (135)

#define GRID_HEIGHT (27)
#define GRID_WIDTH (27)

#define BLACK (0x0000)
#define BLUE (0x001F)
#define RED (0xF800)
#define GREEN (0x07E0)
#define CYAN (0x07FF)
#define MAGENTA (0xF81F)
#define YELLOW (0xFFE0)
#define WHITE (0xFFFF)

#define LOW_SPEED_COLOR (0x645F)
#define MID_SPEED_COLOR (0x3DE8)
#define HIGH_SPEED_COLOR (0xB6C5)
#define MAX_SPEED_COLOR (0xED65)

#define ICON_PAD (1)
#define BIG_ICON_PAD (2)

#define DISPLAY_UPDATE_MS (250)

#define CPU_NEOPIXEL_PIN (PIN_NEOPIXEL)
#define KEY_NEOPIXEL_PIN (A3)
#define N_CPU_PIXELS (1)
#define N_KEY_PIXELS (2)

// Display flow [ LOCK ] <-> [ READY ]
namespace {
void update_display_cb(void* ctx);
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
uint8_t task_handle;
GFXcanvas16 framebuffer(DISPLAY_W, DISPLAY_H);
DisplayData display_data;
ColorData cpu_color[N_CPU_PIXELS];
ColorData key_colors[N_KEY_PIXELS];
Adafruit_NeoPixel cpu_pixel(N_CPU_PIXELS, CPU_NEOPIXEL_PIN,
                            NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel key_pixels(N_KEY_PIXELS, KEY_NEOPIXEL_PIN,
                             NEO_GRB + NEO_KHZ800);

void draw_battery_status(uint8_t percent) {
    framebuffer.drawRGBBitmap(ICON_PAD, ICON_PAD,
                              icon_battery_level_100_charged, ICON_WIDTH,
                              ICON_HEIGHT);
}

void draw_radio_signal(uint8_t sig) {
    framebuffer.drawRGBBitmap((DISPLAY_W) - (ICON_WIDTH + (ICON_PAD * 2)),
                              ICON_PAD, icon_satellite, ICON_WIDTH,
                              ICON_HEIGHT);
}

void draw_nfc_icon(uint8_t nfc) {
    framebuffer.drawRGBBitmap(ICON_PAD, (ICON_HEIGHT + ICON_PAD), icon_rfid,
                              ICON_WIDTH, ICON_HEIGHT);
}
void draw_wifi_icon(uint8_t sig) {
    framebuffer.drawRGBBitmap(
        (DISPLAY_W) - (ICON_WIDTH + (ICON_PAD * 2)), (ICON_HEIGHT + ICON_PAD),
        icon_network_wireless_no_route, ICON_WIDTH, ICON_HEIGHT);
}

void draw_speed_bar(uint16_t kph, uint16_t limit) {
    const size_t n_display = 5;
    const size_t buffer_len = n_display + 1;
    char buf[buffer_len] = {'\0'};
    uint8_t high = kph / KPH_RESOLUTION;
    uint8_t low = kph % KPH_RESOLUTION;
    const int16_t x = BIG_ICON_PAD, y = DISPLAY_H - (GRID_HEIGHT * 3);
    uint16_t bin = limit / 3;
    uint16_t color = WHITE;
    int16_t w = (int16_t)((DISPLAY_W * kph) / limit);
    if (kph < bin) {
        color = LOW_SPEED_COLOR;
    } else if (kph < (bin * 2)) {
        color = MID_SPEED_COLOR;
    } else if (kph < (bin * 3)) {
        color = HIGH_SPEED_COLOR;
    } else {
        // We are at our limit
        color = MAX_SPEED_COLOR;
    }

    snprintf(buf, buffer_len, "%02u.%02u", high, low);
    framebuffer.fillRect(x, y, w, GRID_HEIGHT * 2, color);
    const size_t TEXT_SIZE = 4;
    framebuffer.setTextColor(WHITE);
    framebuffer.setTextSize(TEXT_SIZE);
    // 3 and 4 here are the base text size / 2
    int16_t text_x = (DISPLAY_W / 2) - ((3 * TEXT_SIZE) * n_display),
            text_y = y + (GRID_HEIGHT - (4 * TEXT_SIZE));
    framebuffer.setCursor(text_x, text_y);
    framebuffer.print(buf);
}
void draw_status_text(const char* line) {
    const int16_t x = ICON_PAD,
                  y = DISPLAY_H - (GRID_HEIGHT / 2) - (ICON_HEIGHT / 2);
    framebuffer.setTextSize(2);
    framebuffer.setTextColor(WHITE);
    framebuffer.setCursor(0, 0);
    framebuffer.setCursor(x, y);
    framebuffer.print(line);
}

void update_display_cb(void* ctx) {
    framebuffer.fillScreen(ST77XX_BLACK);
    // Handles writing
    if (display_data.locked) {
        // Render lockscreen
        framebuffer.drawRGBBitmap(
            (DISPLAY_W / 2) - (BIG_ICON_WIDTH / 2) + BIG_ICON_PAD,
            (DISPLAY_H / 2) - (BIG_ICON_HEIGHT / 2) + BIG_ICON_PAD,
            bigicon_lock, BIG_ICON_WIDTH, BIG_ICON_HEIGHT);
    } else {
        draw_battery_status(display_data.charge);
        draw_radio_signal(display_data.radio_sig);
        draw_nfc_icon(display_data.nfc_status);
        draw_wifi_icon(display_data.wifi_sig);
        draw_speed_bar(display_data.kph, display_data.limit);
        draw_status_text(display_data.status_line);
    }
    tft.drawRGBBitmap(0, 0, framebuffer.getBuffer(), framebuffer.width(),
                      framebuffer.height());
    cpu_pixel.clear();
    key_pixels.clear();

    for (uint8_t i = 0; i < N_CPU_PIXELS; ++i) {
        cpu_pixel.setPixelColor(
            i, cpu_pixel.Color(cpu_color[i].r, cpu_color[i].g, cpu_color[i].b));
    }

    for (uint8_t i = 0; i < N_KEY_PIXELS; ++i) {
        key_pixels.setPixelColor(
            i, key_pixels.Color(key_colors[i].r, key_colors[i].g,
                                key_colors[i].b));
    }
    cpu_pixel.show();
    key_pixels.show();
}
}  // namespace

// Public Functions

void display_init(Scheduler* sched) {
    ENTER;
    task_handle = TASK_INVALID;
    pinMode(TFT_BACKLITE, OUTPUT);
    digitalWrite(TFT_BACKLITE, HIGH);
    framebuffer.setTextWrap(false);
    tft.init(DISPLAY_H, DISPLAY_W);
    tft.setRotation(3);
    tft.fillScreen(ST77XX_BLACK);
    display_data = {.kph = 0 * KPH_RESOLUTION,
                    .limit = 30 * KPH_RESOLUTION,
                    .charge = 50,
                    .radio_sig = 0,
                    .wifi_sig = 0,
                    .nfc_status = 0,
                    .locked = true};
    memset(display_data.status_line, '\0', STATUS_LINE_MAX);

    cpu_color[0] = {.r = 128, .g = 0, .b = 0};
    key_colors[0] = {.r = 0, .g = 128, .b = 0};
    key_colors[1] = {.r = 0, .g = 0, .b = 128};
    cpu_pixel.begin();
    key_pixels.begin();
    cpu_pixel.show();
    key_pixels.show();

    if (NULL != sched) {
        task_handle =
            sched->register_task(SCHED_MILLISECONDS(DISPLAY_UPDATE_MS),
                                 update_display_cb, TASK_FLAG_ENABLED);
    }
    EXIT;
}

void display_set_kph(uint16_t kph) { display_data.kph = kph; }
void display_set_kph_limit(uint16_t limit) { display_data.limit = limit; }
void display_set_battery_charge(uint8_t percent_charge) {
    display_data.charge = percent_charge;
}
void display_set_radio_signal(uint8_t signal) {
    display_data.radio_sig = signal;
}
void display_set_wifi_signal(uint8_t signal) { display_data.wifi_sig = signal; }
void display_set_nfc_status(uint8_t status) {
    display_data.nfc_status = status;
}
void display_set_locked(bool locked) { display_data.locked = locked; }
void display_set_status_line(const char* message) {
    size_t len = strlen(message);
    if (len > STATUS_LINE_MAX) {
        // Account for null term
        len = STATUS_LINE_MAX - 1;
    }
    strncpy(display_data.status_line, message, len);
}
void display_printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(display_data.status_line, STATUS_LINE_MAX, fmt, args);
    va_end(args);
}
#else

void display_init(Scheduler* sched) {}
void set_backlight(bool on) {}
void update_display_cb(void) {}

// Fixed point kph
void display_set_kph(uint16_t kph) {}
void display_set_kph_limit(uint16_t limit) {}
void display_set_battery_charge(uint8_t percent_charge) {}
void display_set_radio_signal(uint8_t signal) {}
void display_set_wifi_signal(uint8_t signal) {}
void display_set_nfc_status(uint8_t status) {}
void display_set_locked(bool locked) {}
void display_set_status_line(const char* status) {}
void display_printf(const char* fmt, ...) {}

#endif
