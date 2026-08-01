// ============================================================================
//  Test tu dong cho src/core/  -  chay tren may tinh, khong can phan cung.
//
//      pio run -e test && ./.pio/build/test/program
//
//  Vi sao can: Ngay 3-4 ban se sua rat nhieu hang so trong config.h theo so do
//  duoc tren phan cung that. Bo test nay bat cac loi kieu "sua mot con so lam
//  thuat toan khong bao gio dung" NGAY LAP TUC, thay vi phat hien luc dang do
//  so lieu chinh thuc.
//
//  Nhom test quan trong nhat la NHOM 7: chay ca ba thuat toan tren mo hinh
//  plume va bat buoc chung phai KET THUC, khong duoc chay het gio. Chinh loi
//  nay da xay ra that (surge-casting ket vong lap vo han o goc san).
// ============================================================================
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>

#include "../core/config.h"
#include "../core/mission.h"
#include "../core/telemetry_fmt.h"
#include "../sim/sim_robot.h"

using namespace gs;

// --------------------------------------------------------------------------- //
static int g_pass = 0, g_fail = 0;
static std::string g_group;

static void group(const char* name) {
  g_group = name;
  printf("\n%s\n", name);
}

static void check(bool ok, const char* what) {
  if (ok) {
    ++g_pass;
    printf("   [ok]    %s\n", what);
  } else {
    ++g_fail;
    printf("   [TRUOT] %s\n", what);
  }
}

static void nearly(float got, float want, float tol, const char* what) {
  const bool ok = std::fabs(got - want) <= tol;
  if (!ok) printf("           (nhan %.3f, mong %.3f +/- %.3f)\n", got, want, tol);
  check(ok, what);
}

// --------------------------------------------------------------------------- //
static void test_geometry() {
  group("1. geometry - chuan hoa goc va kiem tra bien san");
  nearly(wrapDeg(370.0f), 10.0f, 0.01f, "wrapDeg(370) = 10");
  nearly(wrapDeg(-190.0f), 170.0f, 0.01f, "wrapDeg(-190) = 170");
  nearly(wrapDeg(180.0f), 180.0f, 0.01f, "wrapDeg(180) giu nguyen 180");
  nearly(wrapDeg(-180.0f), 180.0f, 0.01f, "wrapDeg(-180) = 180");
  check(insideArena(cfg::ARENA_W_CM * 0.5f, cfg::ARENA_H_CM * 0.5f),
        "tam san nam trong san");
  check(!insideArena(1.0f, 1.0f), "goc sat mep bi loai (le an toan)");
  check(!insideArena(cfg::ARENA_W_CM + 10.0f, 50.0f), "diem ngoai san bi loai");

  float ox, oy;
  project(0.0f, 0.0f, 90.0f, 10.0f, ox, oy);
  nearly(ox, 0.0f, 0.01f, "project 90 do: x khong doi");
  nearly(oy, 10.0f, 0.01f, "project 90 do: y tang 10");
}

// --------------------------------------------------------------------------- //
static void test_stop_detector() {
  group("2. StopDetector - dieu kien dung phai thoa DONG THOI ca ba");
  StopDetector sd;
  sd.reset();

  // (a) chua du cao thi khong bao gio dung, du giu bao lau
  check(!sd.feed(cfg::STOP_HIGH_DELTA - 50, 0), "duoi nguong cao: khong dung");
  check(!sd.feed(cfg::STOP_HIGH_DELTA - 50, 100000),
        "duoi nguong cao va giu rat lau: van khong dung");

  // (b) du cao nhung CON TANG thi khong duoc dung
  sd.reset();
  const int16_t v0 = cfg::STOP_HIGH_DELTA + 100;
  check(!sd.feed(v0, 0), "lan do dau tien: chua dung");
  check(!sd.feed(v0 + 5 * cfg::PLATEAU_EPS, 10000), "van con tang manh: khong dung");
  check(!sd.feed(v0 + 10 * cfg::PLATEAU_EPS, 20000), "van con tang manh: khong dung");

  // (c) du cao + het tang -> phai giu du STOP_HOLD_MS moi dung.
  //     Luu y: phai CO HAI phep do moi ket luan duoc la "het tang", nen dong ho
  //     giu chi bat dau tu phep do thu hai. Tong thoi gian toi thieu de dung la
  //     mot chu ky do + STOP_HOLD_MS.
  sd.reset();
  const uint32_t t0 = 50000;
  check(!sd.feed(v0, t0), "phep do dau o muc cao: chua the dung (chua biet con tang khong)");
  const uint32_t t_hold = t0 + 3000;
  check(!sd.feed(v0, t_hold), "phep do thu hai xac nhan het tang -> bat dau tinh gio giu");
  check(!sd.feed(v0, t_hold + cfg::STOP_HOLD_MS - 1), "chua giu du thoi gian: chua dung");
  check(sd.feed(v0, t_hold + cfg::STOP_HOLD_MS + 1), "da giu du thoi gian: DUNG");

  // Tang tro lai giua luc dang giu -> phai huy bo dem
  sd.reset();
  sd.feed(v0, 0);
  sd.feed(v0, 1000);
  check(!sd.feed(v0 + 3 * cfg::PLATEAU_EPS, 2000),
        "tang tro lai giua luc dang giu: huy bo dem");
  check(!sd.feed(v0 + 3 * cfg::PLATEAU_EPS, 2000 + cfg::STOP_HOLD_MS / 2),
        "bo dem da bi reset nen chua du");
}

// --------------------------------------------------------------------------- //
static void test_gas() {
  group("3. GasProcessor - baseline, chuan hoa, quy doi ppm");
  GasProcessor gp;
  gp.begin(0);

  // Nuoi 8 giay mau khong khi sach o muc 500.
  uint32_t t = 0;
  for (; t <= cfg::BASELINE_MS + 2000; t += cfg::GAS_SAMPLE_PERIOD_MS)
    gp.addSample(500, 0.0f, t);

  check(gp.baselineReady(), "do xong baseline sau BASELINE_MS");
  nearly((float)gp.baseline(), 500.0f, 3.0f, "baseline ~ 500");
  nearly((float)gp.reading().normalized, 0.0f, 3.0f, "normalized ~ 0 trong khong khi sach");
  check(gp.r0() > 0.0f, "R0 duoc tu hieu chuan thanh gia tri duong");

  // Do khi: ADC tang -> normalized phai tang theo
  for (int i = 0; i < 4 * cfg::GAS_MA_WINDOW; ++i, t += cfg::GAS_SAMPLE_PERIOD_MS)
    gp.addSample(1500, 0.0f, t);
  check(gp.reading().normalized > 900, "ADC 1500 tren baseline 500 -> normalized > 900");
  check(gp.baseline() == 500, "baseline KHONG tu troi theo khi do duoc khi");

  // Duong dac tuyen: ADC cao hon (Rs nho hon) phai cho ppm cao hon
  const float r0 = 10000.0f;
  const float ppm_thap = GasProcessor::rsToPpm(20000.0f, r0);
  const float ppm_cao = GasProcessor::rsToPpm(2000.0f, r0);
  check(ppm_cao > ppm_thap, "Rs nho hon -> ppm lon hon (dung chieu don dieu)");
  check(GasProcessor::rsToPpm(1000.0f, -1.0f) == 0.0f, "R0 khong hop le -> ppm = 0");
  check(std::isfinite(GasProcessor::rsToPpm(0.0f, r0)), "Rs = 0 khong sinh ra NaN/inf");

  check(GasProcessor::ppmToLevel(cfg::PPM_T1 - 1.0f) == AlarmLevel::Safe, "duoi T1 -> SAFE");
  check(GasProcessor::ppmToLevel(cfg::PPM_T1) == AlarmLevel::Detected, "tai T1 -> DETECTED");
  check(GasProcessor::ppmToLevel(cfg::PPM_T2) == AlarmLevel::High, "tai T2 -> HIGH");
  check(GasProcessor::ppmToLevel(cfg::PPM_T3 * 10.0f) == AlarmLevel::Critical,
        "rat cao -> CRITICAL");

  // Nguong phai xep tang dan, neu khong bang mau canh bao vo nghia
  check(cfg::PPM_T1 < cfg::PPM_T2 && cfg::PPM_T2 < cfg::PPM_T3,
        "config: T1 < T2 < T3");
  check(cfg::DETECT_DELTA < cfg::STOP_HIGH_DELTA,
        "config: DETECT_DELTA < STOP_HIGH_DELTA");
}

// --------------------------------------------------------------------------- //
static void test_lawnmower() {
  group("4. LawnmowerPath - duong quet zig-zag");
  LawnmowerPath p;
  p.begin(1);
  check(p.total() == cfg::GRID_NX * cfg::GRID_NY, "stride 1 phu het so o luoi");

  int n = 0;
  float x, y, px = -1, py = -1;
  bool all_inside = true, serpentine_ok = true;
  while (p.next(x, y)) {
    ++n;
    if (!insideArena(x, y)) all_inside = false;
    // Hai diem lien tiep phai ke nhau: khong duoc nhay cheo qua ca san.
    if (px >= 0 && dist(px, py, x, y) > cfg::CELL_CM * 1.6f) serpentine_ok = false;
    px = x;
    py = y;
  }
  check(n == p.total(), "sinh dung so diem roi bao het");
  check(all_inside, "moi diem quet deu nam trong le an toan cua san");
  check(serpentine_ok, "khong co buoc nhay dai -> dung la zig-zag lien tuc");
  check(p.done(), "bao done() sau khi het diem");

  LawnmowerPath q;
  q.begin(cfg::SEEK_STRIDE);
  check(q.total() < p.total(), "quet tho (SEEK_STRIDE) it diem hon quet day");
}

// --------------------------------------------------------------------------- //
static void test_telemetry() {
  group("5. telemetry_fmt - dong goi, checksum, do lai");
  TelemetrySample s;
  s.t_ms = 124500;
  s.algo = "GRA";
  s.state = "SURGE";
  s.adc = 612;
  s.norm = 318;
  s.ppm = 180.4f;
  s.level = AlarmLevel::High;
  s.pose = Pose{132.5f, 87.2f, -45.0f};
  s.dist_cm = 342.0f;
  s.best_norm = 700;

  char line[192];
  const size_t n = buildCsv(line, sizeof(line), s);
  check(n > 0, "dong goi thanh cong");
  check(line[0] == '$', "bat dau bang '$'");
  check(strchr(line, '*') != nullptr, "co dau '*' truoc checksum");
  check(verifyChecksum(line), "checksum cua goi vua tao la dung");
  check(n < 90, "goi du ngan cho LoRa (< 90 ky tu)");

  char pretty[192];
  check(prettyFromCsv(line, pretty, sizeof(pretty)), "doc lai duoc dang nguoi doc");
  check(strstr(pretty, "ALGO=GRA") != nullptr, "dang nguoi doc co truong ALGO");
  check(strstr(pretty, "LEVEL=HIGH") != nullptr, "dang nguoi doc co truong LEVEL");
  check(strstr(pretty, "STATE=SURGE") != nullptr, "dang nguoi doc co truong STATE");

  char broken[192];
  strcpy(broken, line);
  broken[7] = (broken[7] == 'X') ? 'Y' : 'X';
  check(!verifyChecksum(broken), "sua mot ky tu -> checksum bao sai");
  check(!verifyChecksum("khong phai goi tin"), "chuoi rac bi loai");
  check(!verifyChecksum("$GS,1,2,3"), "thieu checksum bi loai");

  // Bo dem nho hon goi tin thi phai tu choi, khong duoc ghi tran
  char tiny[16];
  check(buildCsv(tiny, sizeof(tiny), s) == 0, "bo dem qua nho -> tra ve 0, khong tran");
}

// --------------------------------------------------------------------------- //
static void test_sniffer() {
  group("6. Sniffer - chi lay mau SAU khi cam bien on dinh");
  // Robot gia don gian: gia tri khi doi giua hai muc theo thoi gian.
  struct FakeRobot : IRobot {
    uint32_t t = 0;
    int16_t v = 0;
    uint32_t nowMs() const override { return t; }
    GasReading gas() const override {
      GasReading g;
      g.normalized = v;
      g.raw = (uint16_t)(500 + v);
      g.valid = true;
      return g;
    }
    Pose pose() const override { return Pose{}; }
    float travelledCm() const override { return 0; }
    void cmdForward(float) override {}
    void cmdTurn(float) override {}
    void cmdStop() override {}
    bool motionBusy() const override { return false; }
    bool bumped() const override { return false; }
    void clearBump() override {}
    void log(const char*) override {}
  } r;

  Sniffer sn;
  sn.start(0);
  // Trong giai doan on dinh, gia tri rac phai bi BO QUA.
  r.v = 9999;
  for (uint32_t t = 0; t < cfg::SNIFF_SETTLE_MS; t += 20) {
    r.t = t;
    if (sn.update(r)) break;
  }
  check(sn.active(), "chua xong trong giai doan on dinh");

  // Sau do gia tri that la 100.
  r.v = 100;
  bool done = false;
  for (uint32_t t = cfg::SNIFF_SETTLE_MS; t <= cfg::SNIFF_TOTAL_MS + 40; t += 20) {
    r.t = t;
    if (sn.update(r)) {
      done = true;
      break;
    }
  }
  check(done, "ket thuc sau dung SNIFF_TOTAL_MS");
  check(!sn.active(), "khong con active sau khi xong");
  nearly((float)sn.value(), 100.0f, 1.0f,
         "chi lay trung binh gia tri SAU on dinh (bo qua so rac 9999)");
}

// --------------------------------------------------------------------------- //
// NHOM QUAN TRONG NHAT: ca ba thuat toan phai KET THUC, khong chay het gio.
static void test_missions_terminate() {
  group("7. Ba thuat toan phai KET THUC (chong ket vong lap vo han)");

  const Algo algos[3] = {Algo::EXHAUSTIVE, Algo::GRADIENT, Algo::SURGE_CAST};
  const sim::Env envs[2] = {sim::Env::DIFFUSION, sim::Env::INTERMITTENT};

  int n_run = 0, n_timeout = 0;
  float worst_time = 0.0f;

  for (Algo a : algos) {
    for (sim::Env e : envs) {
      for (uint32_t seed = 1; seed <= 4; ++seed) {
        // Vi tri nguon rai deu, ke ca sat goc san - noi da tung gay ket.
        const float fx = 0.55f + 0.10f * (float)(seed % 3);
        const float fy = 0.15f + 0.25f * (float)(seed % 3);
        sim::SimRobot robot;
        robot.begin(e, cfg::ARENA_W_CM * fx, cfg::ARENA_H_CM * fy, seed);
        robot.warmup(cfg::BASELINE_MS / 1000.0f + 1.5f);

        Mission m;
        m.begin(robot, a);
        const long max_steps = (long)(cfg::MISSION_TIMEOUT_MS / 1000.0f / sim::DT_S) + 2000;
        for (long i = 0; i < max_steps && m.running(); ++i) {
          robot.step(sim::DT_S);
          m.update(robot);
        }
        ++n_run;
        const float secs = m.elapsedMs(robot) / 1000.0f;
        if (secs > worst_time) worst_time = secs;
        if (m.result() == MissionResult::TIMEOUT) {
          ++n_timeout;
          printf("           het gio: %s / %s / seed %u\n", algoShortName(a),
                 sim::envShortName(e), seed);
        }
        if (m.result() == MissionResult::RUNNING) {
          printf("           KHONG KET THUC: %s / %s / seed %u\n", algoShortName(a),
                 sim::envShortName(e), seed);
        }
        check(m.result() != MissionResult::RUNNING,
              (std::string("ket thuc: ") + algoShortName(a) + " / " +
               sim::envShortName(e) + " / seed " + std::to_string(seed)).c_str());
      }
    }
  }

  printf("           %d lan chay, %d lan het gio, lau nhat %.0f s\n", n_run, n_timeout,
         worst_time);
  check(n_timeout == 0, "khong lan nao phai dung vi het gio");
  check(worst_time < cfg::MISSION_TIMEOUT_MS / 1000.0f,
        "lan lau nhat van duoi han thoi gian cho phep");
}

// --------------------------------------------------------------------------- //
// Quet toan bo la baseline: phai LUON quet het luoi, khong duoc dung som.
static void test_exhaustive_is_baseline() {
  group("8. Quet toan bo phai quet het san (dung la baseline cho H3)");
  sim::SimRobot robot;
  // Dat nguon NGAY canh diem xuat phat: mot thuat toan dung som se dung ngay.
  robot.begin(sim::Env::DIFFUSION, cfg::CELL_CM, cfg::CELL_CM, 99);
  robot.warmup(cfg::BASELINE_MS / 1000.0f + 1.5f);

  Mission m;
  m.begin(robot, Algo::EXHAUSTIVE);
  const long max_steps = (long)(cfg::MISSION_TIMEOUT_MS / 1000.0f / sim::DT_S) + 2000;
  for (long i = 0; i < max_steps && m.running(); ++i) {
    robot.step(sim::DT_S);
    m.update(robot);
  }

  const float path_m = m.pathCm(robot) / 100.0f;
  // Duong quet het luoi 8x8 o 25 cm dai it nhat khoang 14 m.
  printf("           quang duong = %.2f m, ket qua = %s\n", path_m, m.stateName());
  check(m.result() == MissionResult::FOUND, "ket thuc binh thuong");
  check(path_m > 0.6f * (cfg::GRID_NX * cfg::GRID_NY * cfg::CELL_CM / 100.0f),
        "quet het luoi du nguon ngay canh diem xuat phat");
}

// --------------------------------------------------------------------------- //
int main() {
  printf("=====================================================\n");
  printf(" Test src/core/  -  %s\n", cfg::FW_VERSION);
  printf(" San %.0fx%.0f cm, o luoi %.0f cm\n", cfg::ARENA_W_CM, cfg::ARENA_H_CM,
         cfg::CELL_CM);
  printf("=====================================================\n");

  test_geometry();
  test_stop_detector();
  test_gas();
  test_lawnmower();
  test_telemetry();
  test_sniffer();
  test_missions_terminate();
  test_exhaustive_is_baseline();

  printf("\n=====================================================\n");
  printf(" DAT %d  /  TRUOT %d\n", g_pass, g_fail);
  printf("=====================================================\n");
  return g_fail == 0 ? 0 : 1;
}
