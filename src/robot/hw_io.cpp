#include "hw_io.h"

#include "../core/config.h"

namespace hw {

// --- nut bam ---
static bool btn_down_ = false;
static uint32_t btn_t0_ = 0;
static BtnEvent pending_ = BtnEvent::NONE;

// --- coi ---
static uint32_t buzz_off_ms_ = 0;
static uint8_t beeps_left_ = 0;
static uint32_t beep_next_ms_ = 0;

void ioBegin() {
  pinMode(cfg::pin::BUMP_L, INPUT_PULLUP);
  pinMode(cfg::pin::BUMP_R, INPUT_PULLUP);
  pinMode(cfg::pin::BTN, INPUT_PULLUP);
  pinMode(cfg::pin::BUZZER, OUTPUT);
  digitalWrite(cfg::pin::BUZZER, LOW);

  if (cfg::UI_USE_DISCRETE_LEDS) {
    pinMode(cfg::pin::LED_A, OUTPUT);
    pinMode(cfg::pin::LED_B, OUTPUT);
  }
  statusOff();
}

// Cong tac thuong mo, noi GND -> cham = LOW.
bool bumperLeft() { return digitalRead(cfg::pin::BUMP_L) == LOW; }
bool bumperRight() { return digitalRead(cfg::pin::BUMP_R) == LOW; }
bool bumperAny() { return bumperLeft() || bumperRight(); }

void ioUpdate() {
  const uint32_t now = millis();

  // --- nut BOOT: LOW khi dang nhan ---
  const bool down = (digitalRead(cfg::pin::BTN) == LOW);
  if (down && !btn_down_) {
    btn_down_ = true;
    btn_t0_ = now;
  } else if (!down && btn_down_) {
    btn_down_ = false;
    const uint32_t held = now - btn_t0_;
    if (held > 40) {  // chong doi
      pending_ = (held >= cfg::BTN_LONG_PRESS_MS) ? BtnEvent::LONG_PRESS
                                                  : BtnEvent::SHORT_PRESS;
    }
  }

  // --- coi ---
  if (buzz_off_ms_ != 0 && now >= buzz_off_ms_) {
    digitalWrite(cfg::pin::BUZZER, LOW);
    buzz_off_ms_ = 0;
  }
  if (beeps_left_ > 0 && buzz_off_ms_ == 0 && now >= beep_next_ms_) {
    --beeps_left_;
    digitalWrite(cfg::pin::BUZZER, HIGH);
    buzz_off_ms_ = now + 90;
    beep_next_ms_ = now + 200;
  }
}

BtnEvent buttonPoll() {
  const BtnEvent e = pending_;
  pending_ = BtnEvent::NONE;
  return e;
}

void statusColor(uint8_t r, uint8_t g, uint8_t b) {
  if (cfg::UI_USE_RGB_LED) {
    // Ham co san trong arduino-esp32 >= 2.0.6 cho LED WS2812 tren board.
    neopixelWrite(cfg::pin::RGB_LED, r, g, b);
  }
  if (cfg::UI_USE_DISCRETE_LEDS) {
    digitalWrite(cfg::pin::LED_A, (g > 60) ? HIGH : LOW);
    digitalWrite(cfg::pin::LED_B, (r > 60) ? HIGH : LOW);
  }
}

void statusOff() { statusColor(0, 0, 0); }

// Bang mau lay dung theo de cuong muc 11.1c.
void statusByLevel(gs::AlarmLevel lv) {
  switch (lv) {
    case gs::AlarmLevel::Safe:     statusColor(0, 30, 0); break;    // xanh la
    case gs::AlarmLevel::Detected: statusColor(40, 35, 0); break;   // vang
    case gs::AlarmLevel::High:     statusColor(60, 20, 0); break;   // cam
    case gs::AlarmLevel::Critical: statusColor(80, 0, 0); break;    // do
  }
}

void beep(uint16_t ms) {
  digitalWrite(cfg::pin::BUZZER, HIGH);
  buzz_off_ms_ = millis() + ms;
}

void beepPattern(uint8_t times) {
  beeps_left_ = times;
  beep_next_ms_ = millis();
}

}  // namespace hw
