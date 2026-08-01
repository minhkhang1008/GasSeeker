// ============================================================================
//  GasSeeker - firmware TRAM THU (ESP32-S3 thu hai, dat canh may tinh)
//
//  Nhiem vu rat don gian, co y:
//    1. Nhan goi LoRa tu xe, kiem tra checksum.
//    2. In ra USB Serial hai dang:
//         RX,<rssi>,<snr>,<goi CSV nguyen ban>   -> cho tools/receiver.py
//         t=..s | ALGO=.. | ADC=.. | ...          -> cho nguoi ngoi truc doc
//    3. Cho phep go lenh tu ban phim va phat len xe (start / stop / algo N).
//
//  Tram KHONG tham gia vao logic tim nguon. Mat tram, xe van chay binh thuong.
// ============================================================================
#include <Arduino.h>

#include "../core/config.h"
#include "../core/telemetry_fmt.h"
#include "../lora/lora_link.h"

static uint32_t rx_count_ = 0;
static uint32_t bad_count_ = 0;
static uint32_t last_rx_ms_ = 0;

static void banner() {
  Serial.println();
  Serial.println("=====================================================");
  Serial.print("  GasSeeker - TRAM THU LoRa  ");
  Serial.println(cfg::FW_VERSION);
  Serial.println("=====================================================");
  Serial.printf("  Tan so   : %.1f MHz, SF%d, BW %.0f kHz\n", cfg::LORA_FREQ_MHZ,
                cfg::LORA_SF, cfg::LORA_BW_KHZ);
  Serial.printf("  Radio    : %s (%s)\n", radiolink::ok() ? "OK" : "LOI",
                radiolink::lastError());
  Serial.println("  Go lenh roi Enter de gui len xe: start | stop | algo 0|1|2 | cal");
  Serial.println("-----------------------------------------------------");
}

static void pollKeyboard() {
  static char buf[48];
  static uint8_t n = 0;
  while (Serial.available()) {
    const char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {
      if (n == 0) continue;
      buf[n] = '\0';
      n = 0;
      if (!cfg::ENABLE_UPLINK) {
        Serial.println("[tram] duong len dang tat trong config.h");
        continue;
      }
      char out[64];
      snprintf(out, sizeof(out), "CMD,%s", buf);
      Serial.printf("[tram] gui: %s\n", out);
      if (!radiolink::send(out)) Serial.println("[tram] radio dang ban, thu lai");
    } else if (n < sizeof(buf) - 1) {
      buf[n++] = c;
    }
  }
}

void setup() {
  Serial.begin(115200);
  const uint32_t t0 = millis();
  while (!Serial && millis() - t0 < 2000) {
  }
  radiolink::begin();
  banner();
}

void loop() {
  radiolink::poll();
  pollKeyboard();

  char line[192];
  float rssi = 0, snr = 0;
  if (!radiolink::receive(line, sizeof(line), &rssi, &snr)) {
    // Bao mat lien lac neu qua lau khong nhan duoc gi.
    if (rx_count_ > 0 && millis() - last_rx_ms_ > 10000) {
      static uint32_t warn_ms = 0;
      if (millis() - warn_ms > 5000) {
        warn_ms = millis();
        Serial.printf("[tram] mat song %lu s (xe van tu chay binh thuong)\n",
                      (unsigned long)((millis() - last_rx_ms_) / 1000));
      }
    }
    return;
  }

  last_rx_ms_ = millis();
  if (!gs::verifyChecksum(line)) {
    ++bad_count_;
    Serial.printf("[tram] goi loi checksum (#%lu): %s\n", (unsigned long)bad_count_, line);
    return;
  }
  ++rx_count_;

  // Goi thu cu ly tu che do bench cua xe.
  if (strncmp(line, "$PING", 5) == 0) {
    Serial.printf("[tram] PING %s | RSSI=%.0f dBm | SNR=%.1f dB\n", line, rssi, snr);
    return;
  }

  // Dang may doc - tools/receiver.py bat dong nay.
  Serial.printf("RX,%.1f,%.1f,%s\n", rssi, snr, line);

  // Dang nguoi doc - dung mau de cuong muc 11.4.
  char pretty[192];
  if (gs::prettyFromCsv(line, pretty, sizeof(pretty))) {
    Serial.printf("   %s | RSSI=%.0f dBm\n", pretty, rssi);
  }
}
