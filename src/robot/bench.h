// ============================================================================
//  bench.h - che do KIEM TRA PHAN CUNG va HIEU CHUAN.
//
//  Muc dich: ngay lap xe xong, thu tung khoi RIENG LE thay vi nap ca thuat toan
//  roi doan xem cai gi sai. Va sinh san bang so lieu cho buoc hieu chuan
//  (docs/RUNBOOK.md muc 3 va 4.4).
//
//  Cac lenh (go vao Serial):
//     mot <L> <R>   cap PWM THO cho hai banh trong 1,5 s   (-255..255)
//                   -> kiem tra chieu day motor va tim PWM_MIN_MOVE
//     drive <cm>    di thang mot doan roi bao ket qua odometry
//                   -> do bang thuoc de hieu chuan WHEEL_DIAMETER_MM
//     turn <deg>    quay mot goc roi bao ket qua
//                   -> kiem tra gyro va TURN_KP
//     enc           in so xung encoder lien tuc 20 s (day banh bang tay)
//     bump          in trang thai hai cong tac va cham lien tuc 20 s
//     gas           in bang ADC / Rs / ppm moi 0,5 s trong 60 s
//                   -> dan thang vao bang tinh de ve duong dac tuyen
//     sniff         thuc hien DUNG MOT phep "dung ngui" roi in mot dong
//                   -> dung cho bang khoang cach <-> norm o muc 4.4
//     selftest      kiem tra LAN LUOT moi khoi phan cung roi in bang PASS/FAIL
//                   -> lenh dau tien nen go sau khi lap xong xe
//     ping          gui 5 goi LoRa thu de do RSSI ben tram
//     bench off     dung ngay che do kiem tra
// ============================================================================
#pragma once
#include <Arduino.h>

class RobotIO;

namespace bench {

void begin(RobotIO* io);
// Tra ve true neu chuoi la lenh cua bench va da duoc xu ly.
bool handleCommand(const char* cmd);
bool active();
void update();  // goi moi vong lap
void abort();
void printHelp();

}  // namespace bench
