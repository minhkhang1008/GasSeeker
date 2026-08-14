// ============================================================================
//  config.h - TOAN BO hang so cua he thong nam o day.
//
//  Quy uoc chu thich:
//    [CHOT]  = da duoc chot trong de cuong de tai, khong tu y doi.
//    [CHON]  = gia tri toi tu chon lam mac dinh -> xem docs/DECISIONS.md.
//    [DO]    = BAT BUOC do lai / hieu chuan lai tren phan cung that.
//
//  File nay KHONG duoc include Arduino.h (con dung chung cho simulator native).
// ============================================================================
#pragma once
#include <cstdint>

namespace cfg {

// ---------------------------------------------------------------------------
// 0. Phien ban firmware (in ra luc khoi dong + gui kem telemetry)
// ---------------------------------------------------------------------------
constexpr char FW_VERSION[] = "GasSeeker-0.1";

// ---------------------------------------------------------------------------
// 1. SAN THI NGHIEM
//    He toa do: goc (0,0) o goc san, truc X sang phai, truc Y len tren
//    (nhin tu tren xuong). Goc heading tinh bang do, nguoc chieu kim dong ho,
//    0 deg = huong +X.
// ---------------------------------------------------------------------------
// Ba hang so nay co the ghi de luc build de quet thu nhieu co san, vi du:
//   PLATFORMIO_BUILD_FLAGS="-DGS_ARENA_W_CM=300.0f -DGS_ARENA_H_CM=300.0f" pio run -e sim
// Doi han thi cu sua thang so mac dinh o day.
#ifndef GS_ARENA_W_CM
#define GS_ARENA_W_CM 200.0f
#endif
#ifndef GS_ARENA_H_CM
#define GS_ARENA_H_CM 200.0f
#endif
#ifndef GS_CELL_CM
#define GS_CELL_CM 25.0f
#endif

constexpr float ARENA_W_CM = GS_ARENA_W_CM;  // [CHON] be rong san (truc X)
constexpr float ARENA_H_CM = GS_ARENA_H_CM;  // [CHON] chieu dai san (truc Y)
constexpr float CELL_CM    = GS_CELL_CM;     // [CHON] canh mot o luoi

constexpr int GRID_NX = (int)(ARENA_W_CM / CELL_CM);  // 8
constexpr int GRID_NY = (int)(ARENA_H_CM / CELL_CM);  // 8

// Le an toan: robot khong di ra sat mep san (tranh dam tuong / ra ngoai san).
constexpr float ARENA_MARGIN_CM = 10.0f;  // [CHON]

// Vi tri xuat phat cua robot (tam o luoi (0,0)) va huong ban dau.
constexpr float START_X_CM   = CELL_CM * 0.5f;
constexpr float START_Y_CM   = CELL_CM * 0.5f;
constexpr float START_HEADING_DEG = 0.0f;  // huong +X

// Ban kinh coi la "da tim thay nguon" khi cham diem cuoi voi nguon that.
// Chi dung de CHAM DIEM sau khi chay (trong sim va trong file phan tich),
// robot khong biet gia tri nay.
constexpr float SUCCESS_RADIUS_CM = 30.0f;  // [CHON]

// Thoi gian toi da cho mot lan chay truoc khi tu dong dung (an toan pin).
#ifndef GS_MISSION_TIMEOUT_S
#define GS_MISSION_TIMEOUT_S 480
#endif
constexpr uint32_t MISSION_TIMEOUT_MS = GS_MISSION_TIMEOUT_S * 1000UL;  // [CHON] 8 phut

// ---------------------------------------------------------------------------
// 2. HINH HOC XE  -  PHAI KHOP VOI PHAN CUNG THAT (xem docs/wiring/)
//
// Cau hinh thuc te dang lap:
//   - 4 motor vang V1, hai TB6612 dung CHUNG day dieu khien
//     (ben trai: GPIO5/6/7 cho ca FL va RL; ben phai: GPIO15/16/17 cho FR va RR)
//     -> ve mat dieu khien van la xe VI SAI hai ben, firmware khong doi.
//   - CHI MOT encoder HC-020K, lap o banh sau trai, tren GPIO1.
//   - CHI MOT cong tac va cham V156 o phia truoc, tren GPIO38.
// ---------------------------------------------------------------------------
constexpr float WHEEL_DIAMETER_MM = 65.0f;   // [DO] banh V1
constexpr float WHEEL_BASE_MM     = 130.0f;  // [DO] khoang cach hai BEN banh
constexpr int   ENCODER_SLOTS     = 20;      // [DO] DEM so khe tren dia that

// So encoder va so cong tac va cham THUC TE co tren xe.
// Dat = 1 la cau hinh hien tai. Neu sau nay lap them thi doi thanh 2.
constexpr int ENCODER_COUNT = 1;  // [CHOT theo phan cung] encoder o BEN TRAI
constexpr int BUMPER_COUNT  = 1;  // [CHOT theo phan cung] cong tac o GPIO38

// Voi ENCODER_COUNT == 1, robot KHONG suy duoc goc quay tu encoder ->
// MPU6050 tro thanh BAT BUOC. Firmware se canh bao that to neu thieu.
constexpr bool IMU_REQUIRED = (ENCODER_COUNT < 2);

// Quang duong mot xung encoder (mot kenh, dem canh len).
constexpr float MM_PER_TICK = 3.14159265f * WHEEL_DIAMETER_MM / ENCODER_SLOTS;
constexpr float CM_PER_TICK = MM_PER_TICK / 10.0f;

// Khoang cach tu tam truc hai banh den dau do MQ-3 (gan o dau xe).
// QUAN TRONG: chinh nho khoang lech nay ma viec QUAY TAI CHO cung lam dau do
// doi cho -> thuat toan gradient "quet 3 huong" moi lay duoc thong tin khong
// gian. Neu gan cam bien ngay tam xe, quet 3 huong se vo nghia.
constexpr float SENSOR_OFFSET_CM = 12.0f;  // [DO] do lai sau khi lap xong

// ---------------------------------------------------------------------------
// 3. CHUYEN DONG
// ---------------------------------------------------------------------------
constexpr int PWM_MAX        = 255;
constexpr int PWM_DRIVE      = 150;  // [CHON] toc do di thang
constexpr int PWM_TURN       = 130;  // [CHON] toc do quay tai cho
constexpr int PWM_MIN_MOVE   = 70;   // [DO]   duoi muc nay motor khong quay noi

// Bo dieu khien giu huong khi di thang (P tren sai so goc, dung gyro).
constexpr float HEADING_KP        = 3.0f;   // [DO] PWM tren moi do lech
constexpr int   HEADING_MAX_CORR  = 60;     // [CHON] gioi han bu PWM

// Dung khi quay: sai so goc chap nhan duoc.
constexpr float TURN_TOLERANCE_DEG = 3.0f;  // [CHON]
// Bo dieu khien quay (P tren sai so goc con lai).
constexpr float TURN_KP           = 2.2f;   // [DO]

// Thoi gian phanh / cho xe dung han truoc khi coi la "het lenh".
constexpr uint32_t BRAKE_MS = 250;  // [CHON]

// Timeout cho mot lenh chuyen dong (chong ket khi banh bi ke).
constexpr uint32_t MOTION_TIMEOUT_MS = 12000;  // [CHON]

// ---------------------------------------------------------------------------
// 4. CAM BIEN KHI - LOP 1: TIN HIEU DIEU KHIEN (ADC tho da loc)
//    Thuat toan CHI duoc dung nhung gia tri trong muc nay.
// ---------------------------------------------------------------------------
constexpr uint32_t GAS_SAMPLE_PERIOD_MS = 50;   // [CHON] 20 Hz
constexpr int      GAS_MA_WINDOW        = 16;   // [CHON] cua so trung binh truot (~0.8 s)

// Do dai giai doan do baseline (khong khi sach) khi khoi dong.
constexpr uint32_t BASELINE_MS = 5000;  // [CHON]

// Nguong tren gia tri da chuan hoa (gas_normalized = gas_raw - baseline), don vi
// dem ADC. PHAI do lai tren san that o Ngay 4.
constexpr int16_t DETECT_DELTA    = 40;   // [DO] coi la "co phat hien khi"
constexpr int16_t STOP_HIGH_DELTA = 800;  // [DO] du cao de xet dieu kien dung
constexpr int16_t PLATEAU_EPS     = 30;   // [DO] muc tang duoi nguong nay = "khong tang"

// Dieu kien dung phai thoa DONG THOI ca 3 (xem muc 13.2 de cuong):
//   (a) normalized > STOP_HIGH_DELTA
//   (b) khong tang qua PLATEAU_EPS so voi tot nhat truoc do
//   (c) duy tri lien tuc STOP_HOLD_MS
constexpr uint32_t STOP_HOLD_MS = 6000;  // [CHON] ~2 chu ky do lien tiep khong cai thien

// "Dung ngui" (stop-and-sniff): MQ-3 co tre vai giay, doc trong luc dang chay
// se lech vi tri. Vi vay moi phep do deu dung han xe roi moi lay mau.
// LUU Y QUAN TRONG: cam bien MQ-3 hoi phuc rat cham (datasheet: recovery <= 30 s).
// Neu ngui qua ngan, gia tri doc duoc con mang "ky uc" cua vi tri truoc do ->
// robot vuot qua nguon roi moi biet. Day la ly do phai ngui du lau VA phai
// quay lai diem do cao nhat khi ket thuc (xem RETURN_TO_BEST).
constexpr uint32_t SNIFF_SETTLE_MS = 1500;  // [CHON] bo qua doan qua do
constexpr uint32_t SNIFF_AVG_MS    = 800;   // [CHON] lay trung binh trong doan nay
constexpr uint32_t SNIFF_TOTAL_MS  = SNIFF_SETTLE_MS + SNIFF_AVG_MS;

// ---------------------------------------------------------------------------
// 5. CAM BIEN KHI - LOP 2: NONG DO UOC LUONG (ppm) - CHI de hien thi
//    TUYET DOI khong dua ppm vao logic dieu khien.
// ---------------------------------------------------------------------------
constexpr float ADC_MAX_COUNT = 4095.0f;  // ESP32 12 bit
constexpr float ADC_REF_MV    = 3100.0f;  // [DO] tam do ADC voi atten 12 dB

// Mach chia ap tren duong AO cua MQ-3 (theo de cuong muc 19.3):
//   MQ3_AO --[ R_TOP ]--+--> GPIO
//                       |
//                    [ R_BOT ]
//                       |
//                      GND
constexpr float DIV_R_TOP_OHM = 10000.0f;  // [CHOT]
constexpr float DIV_R_BOT_OHM = 20000.0f;  // [CHOT]
constexpr float DIV_GAIN = DIV_R_BOT_OHM / (DIV_R_TOP_OHM + DIV_R_BOT_OHM);  // 0.667

constexpr float MQ3_VC_V   = 5.0f;      // dien ap cap cho cam bien
constexpr float MQ3_RL_OHM = 10000.0f;  // [DO] dien tro tai TREN MODULE - phai do bang dong ho!

// R0 = Rs trong khong khi sach / RATIO_CLEAN_AIR.
constexpr float MQ3_RATIO_CLEAN_AIR = 60.0f;  // [CHOT] datasheet MQ-3
// Neu MQ3_R0_OHM <= 0 -> firmware tu tinh R0 tu giai doan do baseline luc khoi dong.
constexpr float MQ3_R0_OHM = -1.0f;  // [CHON] mac dinh: tu hieu chuan moi lan bat may

// Duong dac tuyen log-log:  ppm = A * ratio^B  voi ratio = Rs/R0.
// Gia tri duoi day duoc fit tu HAI diem doc THO tren do thi ethanol cua
// datasheet MQ-3:  (0.1 mg/L, ratio 2.6) va (10 mg/L, ratio 0.22),
// quy doi 1 mg/L ethanol ~ 531 ppm o 25 C.
// -> BAT BUOC fit lai bang tools/mq3_fit.py voi datasheet cua module ban mua.
constexpr float MQ3_CURVE_A = 315.2f;   // [CHON/DO]
constexpr float MQ3_CURVE_B = -1.865f;  // [CHON/DO]

// Nguong canh bao cho NGUOI GIAM SAT (khong dung cho thuat toan).
constexpr float PPM_T1 = 100.0f;   // [DO] SAFE      -> DETECTED
constexpr float PPM_T2 = 500.0f;   // [DO] DETECTED  -> HIGH
constexpr float PPM_T3 = 1500.0f;  // [DO] HIGH      -> CRITICAL

// ---------------------------------------------------------------------------
// 6. THAM SO 3 THUAT TOAN
// ---------------------------------------------------------------------------

// --- 6.1 Quet toan bo (baseline) ---
// Buoc di doc theo mot hang, va khoang cach giua hai hang.
constexpr float EXH_STEP_CM     = CELL_CM;  // [CHON] = mot o luoi
constexpr float EXH_ROW_GAP_CM  = CELL_CM;  // [CHON]

// Khi ket thuc, robot quay lai diem do duoc gia tri cao nhat roi moi bao
// "da tim thay nguon". Ap dung cho CA BA thuat toan.
// Ly do: do tre cua MQ-3, robot thuong di qua dinh roi moi nhan ra. Diem ket
// luan dung phai la diem do CAO NHAT chu khong phai cho no tinh co dung lai.
constexpr bool RETURN_TO_BEST = true;  // [CHON]
// Chi quay lai neu diem tot nhat cach cho dang dung hon nguong nay.
constexpr float RETURN_MIN_DIST_CM = 12.0f;  // [CHON]

// --- 6.2 Bam gradient (quet 3 huong) ---
constexpr float    GRAD_SWEEP_DEG   = 55.0f;  // [CHON] goc lech trai/phai khi thu huong
constexpr float    GRAD_STEP_CM     = 30.0f;  // [CHON] do dai mot buoc tien
// Neu sau ngan nay buoc ma gia tri tot nhat khong cai thien -> coi la ket o cuc
// tri dia phuong, thuc hien hanh vi thoat (quay mot goc lon roi di tiep).
constexpr int      GRAD_STUCK_STEPS = 5;      // [CHON]
constexpr float    GRAD_ESCAPE_DEG  = 120.0f; // [CHON]

// Chong ket (ap dung cho gradient va surge-casting): neu sau ngan nay phep do
// LIEN TIEP ma khong lap duoc ky luc moi thi robot ket luan bang diem cao nhat
// da do duoc thay vi chay lang thang cho het gio. Day cung la hanh vi dung cua
// mot robot that: tim kiem phai co diem dung.
constexpr int STALL_LIMIT_SNIFFS = 12;  // [CHON]

// Giai doan "do tim ban dau" (SEEK): khi chua he phat hien khi thi bam gradient
// vo nghia (chi bam nhieu). Ca gradient va surge-casting deu quet tho theo
// duong zig-zag cach nhau SEEK_STRIDE o luoi cho toi khi bat duoc tin hieu
// dau tien. Dung chung mot co che de hai thuat toan duoc so sanh cong bang:
// diem khac biet giua chung nam o hanh vi SAU khi bat duoc luong khi.
constexpr int SEEK_STRIDE = 2;  // [CHON] quet thua: 2 o luoi mot lan do

// --- 6.3 Surge-casting ---
// Huong gio KHAI BAO TRUOC (de cuong muc 11.2: robot khong tu do huong gio).
// WIND_FROM_DEG = huong ma gio THOI TOI TU do, trong he toa do san.
// Do do "di nguoc gio" = di theo heading = WIND_FROM_DEG.
constexpr float WIND_FROM_DEG = 0.0f;  // [CHON] quat dat o phia +X thoi ve goc

constexpr float    SC_SURGE_STEP_CM   = 25.0f;  // [CHON] moi buoc tien nguoc gio
constexpr uint32_t SC_LOST_MS         = 7000;   // [CHON] mat tin hieu bao lau thi chuyen sang cast
// (~2 chu ky do lien tiep khong thay khi. Cho phep robot giu "quan tinh" di
//  xuyen qua mot khoang trong giua hai cum khi, dung nhu hanh vi con trung.)
constexpr float    SC_CAST_STEP_CM    = 15.0f;  // [CHON] bien do cast lan dau
constexpr float    SC_CAST_GROWTH     = 1.6f;   // [CHON] he so tang bien do moi lan doi ben
constexpr float    SC_CAST_MAX_CM     = 90.0f;  // [CHON] vuot qua -> quay lai SEARCHING
constexpr float    SC_SEEK_STEP_CM    = 30.0f;  // [CHON] buoc di khi chua he thay khi

// ---------------------------------------------------------------------------
// 7. TELEMETRY / LoRa
// ---------------------------------------------------------------------------
constexpr uint32_t TELEMETRY_PERIOD_MS = 1000;  // [CHON] 1 Hz

// Bang tan SRD Viet Nam 920-925 MHz. Cong suat gioi han ~25 mW EIRP = 14 dBm.
constexpr float LORA_FREQ_MHZ    = 923.0f;  // [CHON]
constexpr float LORA_BW_KHZ      = 125.0f;  // [CHON]
constexpr int   LORA_SF          = 9;       // [CHON] doi 7 neu can nhanh hon
constexpr int   LORA_CR          = 5;       // [CHON] 4/5
constexpr int   LORA_TX_POWER_DBM = 14;     // [CHON] khong vuot qua muc cho phep
constexpr uint8_t LORA_SYNC_WORD = 0x34;    // [CHON] mang rieng
constexpr int   LORA_PREAMBLE    = 8;

// Cho phep tram gui lenh nguoc len xe (START / STOP / ALGO). Robot van chay
// doc lap hoan toan neu mat song - lenh chi la tien ich khi demo.
constexpr bool ENABLE_UPLINK = true;  // [CHON]

// ---------------------------------------------------------------------------
// 8. SO DO CHAN ESP32-S3 DevKitC-1
//    Tranh: GPIO0/3/45/46 (strapping), 19/20 (USB), 26-32 (flash),
//           33-37 (PSRAM octal tren ban N16R8), 43/44 (UART0).
//    Chi tiet + ly do: docs/WIRING.md
// ---------------------------------------------------------------------------
namespace pin {

// --- Cam bien khi ---
constexpr int MQ3_AO = 4;  // ADC1_CH3

// --- I2C (MPU6050) ---
constexpr int I2C_SDA = 8;
constexpr int I2C_SCL = 9;

// --- LoRa SX1262 (Ra-01SH) qua SPI ---
constexpr int LORA_MOSI = 11;
constexpr int LORA_SCK  = 12;
constexpr int LORA_MISO = 13;
constexpr int LORA_NSS  = 10;
constexpr int LORA_RST  = 14;
constexpr int LORA_BUSY = 21;
constexpr int LORA_DIO1 = 47;

// --- TB6612FNG ---
constexpr int MOT_STBY = 18;
constexpr int MOT_L_PWM = 5;   // PWMA
constexpr int MOT_L_IN1 = 6;   // AIN1
constexpr int MOT_L_IN2 = 7;   // AIN2
constexpr int MOT_R_PWM = 15;  // PWMB
constexpr int MOT_R_IN1 = 16;  // BIN1
constexpr int MOT_R_IN2 = 17;  // BIN2

// --- Encoder quang HC-020K (1 kenh). Chi ENC_L duoc dung khi ENCODER_COUNT = 1.
constexpr int ENC_L = 1;
constexpr int ENC_R = 2;  // KHONG NOI trong cau hinh hien tai

// --- Cong tac va cham (INPUT_PULLUP, cham = LOW) ---
constexpr int BUMP_L = 38;
constexpr int BUMP_R = 39;  // KHONG NOI trong cau hinh hien tai

// --- Giao dien nguoi dung ---
// Nut BOOT co san tren board: nhan ngan = START/STOP, nhan giu = doi thuat toan.
constexpr int BTN = 0;
// LED RGB dia chi (WS2812) co san tren DevKitC-1.
constexpr int RGB_LED = 48;
constexpr int BUZZER  = 42;
// Hai LED roi tuy chon (de mac dinh tat, xem UI_USE_DISCRETE_LEDS).
constexpr int LED_A = 40;
constexpr int LED_B = 41;

}  // namespace pin

constexpr bool UI_USE_RGB_LED       = true;   // [CHON] dung LED RGB onboard
constexpr bool UI_USE_DISCRETE_LEDS = false;  // [CHON] bat neu ban han them LED roi
constexpr uint32_t BTN_LONG_PRESS_MS = 800;   // [CHON]

// ---------------------------------------------------------------------------
// 9. VONG LAP CHINH
// ---------------------------------------------------------------------------
constexpr uint32_t CONTROL_PERIOD_MS = 20;  // [CHON] 50 Hz
constexpr uint32_t IMU_PERIOD_MS     = 10;  // [CHON] 100 Hz (tich phan gyro)

// Thoi gian say nong MQ-3 truoc khi so lieu dang tin (de cuong: 5-10 phut).
// Firmware chi CANH BAO chu khong chan, de con test nhanh.
constexpr uint32_t MQ3_PREHEAT_MS = 5UL * 60UL * 1000UL;  // [CHOT]

}  // namespace cfg
