// ============================================================================
//  sim_config.h - tham so CHI DUNG CHO MO PHONG.
//
//  CANH BAO: nhung con so o day KHONG phai so lieu thuc nghiem. Chung duoc
//  chon de tao ra mot truong nong do co dang hinh hoc va dong hoc HOP LY,
//  du de so sanh HANH VI cua ba thuat toan. Ket qua chay sim phai duoc bao
//  cao rieng, khong duoc tron voi so lieu do tren robot that.
// ============================================================================
#pragma once
#include <cstdint>

namespace sim {

// ---------------------------------------------------------------------------
// Nguon phat
// ---------------------------------------------------------------------------
// Vung dat nguon (ti le so voi kich thuoc san). Robot xuat phat o goc (0,0).
constexpr float SRC_X_MIN_FRAC = 0.50f;
constexpr float SRC_X_MAX_FRAC = 0.90f;
constexpr float SRC_Y_MIN_FRAC = 0.20f;
constexpr float SRC_Y_MAX_FRAC = 0.80f;

// ---------------------------------------------------------------------------
// Moi truong 1 - KHUECH TAN THUAN (khong quat)
//   C(r) = r0^2 / (r0^2 + r^2)   -> tron, giam don dieu theo khoang cach,
//   khong ky di tai r = 0.
// ---------------------------------------------------------------------------
constexpr float DIFF_R0_CM = 25.0f;      // ban kinh dac trung (nguon nho: dia con)
constexpr float DIFF_NOISE = 0.010f;     // nhieu tuong doi cua truong

// ---------------------------------------------------------------------------
// Moi truong 2 - PHAT TAN DUT QUANG (co quat)
//   Mo hinh puff: nguon nha tung bui khi, bui bi gio cuon di va khuech tan
//   rong dan; nong do tai mot diem = tong dong gop cua cac bui.
//   Dac trung: tin hieu xuat hien thanh tung cum ngat quang.
// ---------------------------------------------------------------------------
constexpr float WIND_SPEED_CMS   = 25.0f;  // toc do gio
constexpr float PUFF_PERIOD_S    = 0.35f;  // moi ... giay nha mot bui
constexpr float PUFF_R0_CM       = 5.0f;   // ban kinh bui luc moi nha
constexpr float PUFF_GROWTH_CM2S = 70.0f;  // toc do no ra: R^2 += k*t
constexpr float PUFF_MASS        = 250.0f; // "khoi luong" moi bui (chuan hoa)
constexpr float PUFF_JITTER_CMS  = 14.0f;  // roi loan ngang (random walk)
constexpr float PUFF_MAX_AGE_S   = 22.0f;
constexpr int   PUFF_MAX         = 512;

// ---------------------------------------------------------------------------
// Mo hinh cam bien MQ-3
//   - Tre bac nhat, BAT DOI XUNG: len nhanh, xuong cham (dung dac tinh cua
//     cam bien ban dan oxit kim loai). Day la ly do phai "dung ngui".
//   - Dac tuyen ADC duoi tuyen tinh theo nong do.
// ---------------------------------------------------------------------------
constexpr float MQ3_TAU_RISE_S = 2.5f;
constexpr float MQ3_TAU_FALL_S = 8.0f;
constexpr float MQ3_BASE_ADC   = 450.0f;   // gia tri trong khong khi sach
constexpr float MQ3_GAIN_ADC   = 2600.0f;  // he so khuech dai
constexpr float MQ3_EXP        = 0.75f;    // do cong duoi tuyen tinh
constexpr float MQ3_NOISE_ADC  = 4.0f;     // do lech chuan nhieu ADC

// ---------------------------------------------------------------------------
// Dong hoc xe
// ---------------------------------------------------------------------------
constexpr float DRIVE_SPEED_CMS = 18.0f;
constexpr float TURN_RATE_DPS   = 90.0f;
constexpr float DT_S            = 0.020f;  // buoc mo phong 20 ms

// ---------------------------------------------------------------------------
// Sai so dead-reckoning (odometry): robot TIN vao mot vi tri, con vi tri THAT
// lech dan. Chi dung de danh gia trung thuc, thuat toan khong biet gi ve no.
// ---------------------------------------------------------------------------
constexpr float ODO_DIST_SCALE_ERR = 0.015f;  // sai so ti le quang duong
constexpr float ODO_HEADING_DRIFT_DPS = 0.06f;  // troi gyro (do/giay)
constexpr float ODO_TURN_ERR_DEG = 1.2f;        // sai so moi lan quay

}  // namespace sim
