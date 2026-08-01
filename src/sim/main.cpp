// ============================================================================
//  main.cpp (simulator) - chay hang loat thi nghiem mo phong.
//
//  Chay DUNG file thuat toan cua firmware (src/core/) tren mot mo hinh plume
//  + dong hoc xe + mo hinh cam bien MQ-3. Muc dich:
//    1. Kiem tra logic ba thuat toan truoc khi co phan cung.
//    2. Do tham so (nguong, buoc di, bien do cast) mot cach re tien.
//    3. Co san bang so lieu + bieu do so sanh de dua vao bao cao, ghi ro la
//       KET QUA MO PHONG.
//
//  Vi du:
//      pio run -e sim
//      .pio/build/sim/program --trials 10 --traj
//      python3 tools/analyze.py --sim
// ============================================================================
#include <sys/stat.h>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "../core/config.h"
#include "../core/mission.h"
#include "plume.h"
#include "sim_robot.h"

using namespace gs;
using namespace sim;

struct TrialResult {
  Algo algo;
  Env env;
  int trial;
  uint32_t seed;
  float src_x, src_y;
  float time_s;
  float path_cm;
  float err_cm;       // sai so: dau do luc dung <-> nguon that
  float odo_drift_cm; // do lech dead-reckoning luc ket thuc
  int16_t best_norm;
  bool success;
  const char* outcome;
};

static const char* outcomeName(MissionResult m) {
  switch (m) {
    case MissionResult::FOUND: return "FOUND";
    case MissionResult::TIMEOUT: return "TIMEOUT";
    case MissionResult::ABORTED: return "ABORTED";
    default: return "RUNNING";
  }
}

// ---------------------------------------------------------------------------
static TrialResult runTrial(Algo algo, Env env, int trial, uint32_t seed,
                            const std::string& traj_dir, bool verbose) {
  // Vi tri nguon ngau nhien nhung lap lai duoc theo seed.
  std::mt19937 rng(seed * 7919u + 104729u);
  std::uniform_real_distribution<float> ux(cfg::ARENA_W_CM * SRC_X_MIN_FRAC,
                                           cfg::ARENA_W_CM * SRC_X_MAX_FRAC);
  std::uniform_real_distribution<float> uy(cfg::ARENA_H_CM * SRC_Y_MIN_FRAC,
                                           cfg::ARENA_H_CM * SRC_Y_MAX_FRAC);
  const float sx = ux(rng), sy = uy(rng);

  SimRobot robot;
  robot.setVerbose(verbose);
  robot.begin(env, sx, sy, seed);
  // Say nong + do baseline trong khong khi (gan) sach.
  robot.warmup(cfg::BASELINE_MS / 1000.0f + 1.5f);

  Mission mission;
  mission.begin(robot, algo);

  FILE* traj = nullptr;
  if (!traj_dir.empty()) {
    char path[256];
    snprintf(path, sizeof(path), "%s/traj_%s_%s_%02d.csv", traj_dir.c_str(),
             algoShortName(algo), envShortName(env), trial);
    traj = fopen(path, "w");
    if (traj) {
      fprintf(traj, "t_s,x_true,y_true,x_odom,y_odom,head_true,adc,norm,ppm,state\n");
      fprintf(traj, "# source_x=%.1f source_y=%.1f env=%s algo=%s\n", sx, sy,
              envShortName(env), algoShortName(algo));
    }
  }

  uint32_t next_log_ms = 0;
  const long max_steps = (long)(cfg::MISSION_TIMEOUT_MS / 1000.0f / DT_S) + 5000;

  for (long i = 0; i < max_steps && mission.running(); ++i) {
    robot.step(DT_S);
    mission.update(robot);

    if (traj && robot.nowMs() >= next_log_ms) {
      next_log_ms = robot.nowMs() + 250;
      const Pose t = robot.truePose();
      const Pose o = robot.pose();
      const GasReading g = robot.gas();
      fprintf(traj, "%.2f,%.1f,%.1f,%.1f,%.1f,%.1f,%u,%d,%.0f,%s\n",
              mission.elapsedMs(robot) / 1000.0f, t.x_cm, t.y_cm, o.x_cm, o.y_cm,
              t.heading_deg, g.raw, g.normalized, g.ppm, mission.stateName());
    }
  }
  if (traj) fclose(traj);

  // --- cham diem ---
  const Pose tp = robot.truePose();
  float ssx, ssy;
  project(tp.x_cm, tp.y_cm, tp.heading_deg, cfg::SENSOR_OFFSET_CM, ssx, ssy);

  TrialResult res;
  res.algo = algo;
  res.env = env;
  res.trial = trial;
  res.seed = seed;
  res.src_x = sx;
  res.src_y = sy;
  res.time_s = mission.elapsedMs(robot) / 1000.0f;
  res.path_cm = mission.pathCm(robot);
  res.err_cm = dist(ssx, ssy, sx, sy);
  res.odo_drift_cm = dist(tp.x_cm, tp.y_cm, robot.pose().x_cm, robot.pose().y_cm);
  res.best_norm = mission.best().normalized;
  res.outcome = outcomeName(mission.result());
  res.success = (mission.result() == MissionResult::FOUND) &&
                (res.err_cm <= cfg::SUCCESS_RADIUS_CM);
  return res;
}

// ---------------------------------------------------------------------------
static void printSummary(const std::vector<TrialResult>& rs) {
  printf("\n");
  printf("=========================================================================================\n");
  printf(" KET QUA MO PHONG  (trung binh +/- do lech chuan)\n");
  printf("=========================================================================================\n");
  printf(" %-11s %-6s %3s  %14s  %14s  %13s  %8s\n", "Thuat toan", "MT", "N",
         "Thoi gian (s)", "Quang duong(m)", "Sai so (cm)", "Th.cong");
  printf("-----------------------------------------------------------------------------------------\n");

  for (int a = 0; a < (int)Algo::COUNT; ++a) {
    for (int e = 0; e < 2; ++e) {
      std::vector<float> t, p, err;
      int ok = 0, n = 0;
      for (const auto& r : rs) {
        if ((int)r.algo != a || (int)r.env != e) continue;
        ++n;
        t.push_back(r.time_s);
        p.push_back(r.path_cm / 100.0f);
        err.push_back(r.err_cm);
        if (r.success) ++ok;
      }
      if (n == 0) continue;

      auto ms = [](const std::vector<float>& v, float& m, float& s) {
        m = 0;
        for (float x : v) m += x;
        m /= (float)v.size();
        s = 0;
        for (float x : v) s += (x - m) * (x - m);
        s = v.size() > 1 ? std::sqrt(s / (float)(v.size() - 1)) : 0.0f;
      };
      float tm, ts, pm, ps, em, es;
      ms(t, tm, ts);
      ms(p, pm, ps);
      ms(err, em, es);

      printf(" %-11s %-6s %3d  %6.1f +/-%5.1f  %6.2f +/-%5.2f  %5.1f +/-%5.1f  %5d/%-3d\n",
             algoName((Algo)a), envShortName((Env)e), n, tm, ts, pm, ps, em, es, ok, n);
    }
  }
  printf("=========================================================================================\n");
  printf(" Nguong thanh cong: dung cach nguon <= %.0f cm.  Sai so = khoang cach dau do <-> nguon that.\n",
         cfg::SUCCESS_RADIUS_CM);
  printf(" LUU Y: day la so lieu MO PHONG, khong phai do tren robot that.\n\n");
}

// ---------------------------------------------------------------------------
int main(int argc, char** argv) {
  int trials = 5;
  uint32_t seed0 = 1;
  std::string algo_sel = "all", env_sel = "all", out_dir = "data/sim";
  bool want_traj = false, verbose = false;

  for (int i = 1; i < argc; ++i) {
    const std::string a = argv[i];
    auto nextv = [&]() -> std::string { return (i + 1 < argc) ? argv[++i] : ""; };
    if (a == "--trials") trials = atoi(nextv().c_str());
    else if (a == "--seed") seed0 = (uint32_t)atoi(nextv().c_str());
    else if (a == "--algo") algo_sel = nextv();
    else if (a == "--env") env_sel = nextv();
    else if (a == "--out") out_dir = nextv();
    else if (a == "--traj") want_traj = true;
    else if (a == "--verbose") verbose = true;
    else if (a == "--help" || a == "-h") {
      printf("Cach dung: program [--algo all|exh|gra|sur] [--env all|diff|inter]\n"
             "                  [--trials N] [--seed S] [--out DIR] [--traj] [--verbose]\n");
      return 0;
    }
  }

  mkdir("data", 0755);
  mkdir(out_dir.c_str(), 0755);

  std::vector<Algo> algos;
  if (algo_sel == "all") algos = {Algo::EXHAUSTIVE, Algo::GRADIENT, Algo::SURGE_CAST};
  else if (algo_sel == "exh") algos = {Algo::EXHAUSTIVE};
  else if (algo_sel == "gra") algos = {Algo::GRADIENT};
  else algos = {Algo::SURGE_CAST};

  std::vector<Env> envs;
  if (env_sel == "all") envs = {Env::DIFFUSION, Env::INTERMITTENT};
  else if (env_sel == "diff") envs = {Env::DIFFUSION};
  else envs = {Env::INTERMITTENT};

  printf("GasSeeker simulator  |  %s  |  san %.0fx%.0f cm, o luoi %.0f cm\n",
         cfg::FW_VERSION, cfg::ARENA_W_CM, cfg::ARENA_H_CM, cfg::CELL_CM);
  printf("Cau hinh: %d thuat toan x %d moi truong x %d lan thu\n\n",
         (int)algos.size(), (int)envs.size(), trials);

  std::vector<TrialResult> results;
  for (Env e : envs) {
    for (Algo a : algos) {
      for (int k = 0; k < trials; ++k) {
        // Cung mot seed cho moi thuat toan -> cung vi tri nguon -> so sanh cong bang.
        const uint32_t seed = seed0 + (uint32_t)k * 101u + (uint32_t)e * 7u;
        printf("  [%-10s | %-5s] lan %2d/%2d ... ", algoShortName(a), envShortName(e),
               k + 1, trials);
        fflush(stdout);
        TrialResult r = runTrial(a, e, k + 1, seed, want_traj ? out_dir : "", verbose);
        printf("%7.1fs  %5.2fm  sai so %5.1fcm  %s\n", r.time_s, r.path_cm / 100.0f,
               r.err_cm, r.success ? "OK" : "TRUOT");
        results.push_back(r);
      }
    }
  }

  // --- ghi CSV ---
  const std::string csv = out_dir + "/summary.csv";
  FILE* f = fopen(csv.c_str(), "w");
  if (!f) {
    printf("Khong mo duoc %s\n", csv.c_str());
    return 1;
  }
  fprintf(f, "algo,env,trial,seed,src_x,src_y,time_s,path_cm,err_cm,odo_drift_cm,best_norm,outcome,success\n");
  for (const auto& r : results) {
    fprintf(f, "%s,%s,%d,%u,%.1f,%.1f,%.2f,%.1f,%.1f,%.1f,%d,%s,%d\n", algoName(r.algo),
            envShortName(r.env), r.trial, r.seed, r.src_x, r.src_y, r.time_s, r.path_cm,
            r.err_cm, r.odo_drift_cm, r.best_norm, r.outcome, r.success ? 1 : 0);
  }
  fclose(f);

  printSummary(results);
  printf("Da ghi: %s\n", csv.c_str());
  if (want_traj) printf("Da ghi quy dao: %s/traj_*.csv\n", out_dir.c_str());
  return 0;
}
