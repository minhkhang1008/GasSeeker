// ============================================================================
//  lora_link.h - lop boc quanh SX1262 (module Ra-01SH / MKE-M24), dung chung
//  cho ca firmware tren xe va firmware tram thu.
//
//  QUAN TRONG: truyen KHONG CHAN. radio.transmit() cua RadioLib chan toi ~200 ms
//  o SF9 - du de robot chay lo mat mot doan khi dang di. Vi vay o day dung
//  startTransmit() + co bao ngat, va vong lap chinh goi poll() moi luot.
//
//  Robot PHAI chay duoc khi khong co LoRa (de cuong muc 11.4). Moi ham deu
//  that bai "em" chu khong treo may.
// ============================================================================
#pragma once
#include <Arduino.h>

namespace radiolink {

bool begin(bool verbose = true);
bool ok();
void poll();  // goi moi vong lap de hoan tat TX / khoi dong lai RX

// Gui mot dong. Tra ve false neu dang ban gui goi truoc (goi tin bi bo qua).
bool send(const char* line);
bool busy();

// Nhan mot dong neu co. Tra ve true khi buf co du lieu moi.
bool receive(char* buf, size_t n, float* rssi = nullptr, float* snr = nullptr);

const char* lastError();

}  // namespace radiolink
