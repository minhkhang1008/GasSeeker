#include "lora_link.h"

#include <RadioLib.h>
#include <SPI.h>

#include "../core/config.h"

namespace radiolink {

static SPIClass loraSpi(FSPI);
static SX1262 radio =
    new Module(cfg::pin::LORA_NSS, cfg::pin::LORA_DIO1, cfg::pin::LORA_RST,
               cfg::pin::LORA_BUSY, loraSpi);

enum class St : uint8_t { DOWN, RX, TX };
static St st_ = St::DOWN;
static volatile bool op_done_ = false;
static char err_[48] = "chua khoi tao";

static void IRAM_ATTR onDio1() { op_done_ = true; }

static bool tryBegin(float tcxo_v) {
  const int s = radio.begin(cfg::LORA_FREQ_MHZ, cfg::LORA_BW_KHZ, cfg::LORA_SF,
                            cfg::LORA_CR, cfg::LORA_SYNC_WORD,
                            cfg::LORA_TX_POWER_DBM, cfg::LORA_PREAMBLE, tcxo_v);
  return s == RADIOLIB_ERR_NONE;
}

bool begin(bool verbose) {
  loraSpi.begin(cfg::pin::LORA_SCK, cfg::pin::LORA_MISO, cfg::pin::LORA_MOSI,
                cfg::pin::LORA_NSS);

  // Ra-01SH dung TCXO 1.6 V. Mot so ban sao lai dung thach anh thuong, luc do
  // phai truyen 0. Thu ca hai de khong phai doan.
  bool okv = tryBegin(1.6f);
  if (!okv) {
    if (verbose) Serial.println("[LoRa] thu lai voi thach anh thuong (TCXO = 0)");
    okv = tryBegin(0.0f);
  }
  if (!okv) {
    snprintf(err_, sizeof(err_), "radio.begin() that bai");
    st_ = St::DOWN;
    return false;
  }

  // Module Ai-Thinker dieu khien cong tac RF bang DIO2.
  radio.setDio2AsRfSwitch(true);
  radio.setCurrentLimit(140.0f);
  radio.setDio1Action(onDio1);

  op_done_ = false;
  if (radio.startReceive() != RADIOLIB_ERR_NONE) {
    snprintf(err_, sizeof(err_), "startReceive() that bai");
    st_ = St::DOWN;
    return false;
  }
  st_ = St::RX;
  snprintf(err_, sizeof(err_), "OK");
  return true;
}

bool ok() { return st_ != St::DOWN; }
bool busy() { return st_ == St::TX; }
const char* lastError() { return err_; }

void poll() {
  if (st_ != St::TX) return;
  if (!op_done_) return;
  op_done_ = false;
  radio.finishTransmit();
  radio.startReceive();
  st_ = St::RX;
}

bool send(const char* line) {
  if (st_ == St::DOWN) return false;
  if (st_ == St::TX) return false;  // goi truoc chua gui xong -> bo qua goi nay
  op_done_ = false;
  if (radio.startTransmit(line) != RADIOLIB_ERR_NONE) {
    radio.startReceive();
    st_ = St::RX;
    return false;
  }
  st_ = St::TX;
  return true;
}

bool receive(char* buf, size_t n, float* rssi, float* snr) {
  if (st_ != St::RX) return false;
  if (!op_done_) return false;
  op_done_ = false;

  String s;
  const int state = radio.readData(s);
  if (state == RADIOLIB_ERR_NONE) {
    if (rssi) *rssi = radio.getRSSI();
    if (snr) *snr = radio.getSNR();
    strncpy(buf, s.c_str(), n - 1);
    buf[n - 1] = '\0';
    radio.startReceive();
    return true;
  }
  radio.startReceive();
  return false;
}

}  // namespace radiolink
