#!/usr/bin/env python3
"""
analyze.py - dung bang so lieu va bieu do so sanh cho phan Ket qua cua bao cao.

Hai che do:

  --sim    Doc data/sim/summary.csv (+ traj_*.csv neu co) do simulator sinh ra.
             python3 tools/analyze.py --sim

  mac dinh Doc cac file do THUC TE trong data/runs/ (do tools/receiver.py ghi).
             python3 tools/analyze.py
           Lan chay dau se tao san data/runs/meta.csv de ban dien vi tri nguon
           that cua tung lan thu; co no moi tinh duoc sai so va ti le thanh cong.

Ket qua ghi vao data/figures/:
    bang_ketqua.csv     bang tong hop (trung binh +/- do lech chuan)
    fig_thoigian.png    thoi gian dinh vi theo thuat toan x moi truong
    fig_quangduong.png  chieu dai quang duong
    fig_saiso.png       sai so dinh vi
    fig_thanhcong.png   ti le thanh cong
    fig_quydao_*.png    quy dao tung lan chay
    fig_bandonhiet_*.png ban do nhiet nong do tho (san pham phu, muc 11.4)
"""
import argparse
import csv
import glob
import math
import os
import sys
from collections import defaultdict

FIG_DIR = "data/figures"
SUCCESS_RADIUS_CM = 30.0   # phai khop cfg::SUCCESS_RADIUS_CM
ARENA_W = ARENA_H = 200.0  # phai khop cfg::ARENA_W_CM / ARENA_H_CM

ALGO_ORDER = ["EXHAUSTIVE", "GRADIENT", "SURGE_CAST"]
ALGO_VN = {"EXHAUSTIVE": "Quet toan bo", "GRADIENT": "Bam gradient",
           "SURGE_CAST": "Surge-casting"}
ALGO_SHORT = {"EXH": "EXHAUSTIVE", "GRA": "GRADIENT", "SUR": "SURGE_CAST"}
ENV_VN = {"diff": "Khuech tan (khong quat)", "inter": "Dut quang (co quat)"}

try:
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    HAVE_PLT = True
except ImportError:
    HAVE_PLT = False


# --------------------------------------------------------------------------- #
def mean_sd(v):
    if not v:
        return 0.0, 0.0
    m = sum(v) / len(v)
    if len(v) < 2:
        return m, 0.0
    return m, math.sqrt(sum((x - m) ** 2 for x in v) / (len(v) - 1))


def fnum(s, default=0.0):
    try:
        return float(s)
    except (TypeError, ValueError):
        return default


# --------------------------------------------------------------------------- #
def load_sim(d="data/sim"):
    path = os.path.join(d, "summary.csv")
    if not os.path.exists(path):
        sys.exit(f"Khong thay {path}. Chay simulator truoc:\n"
                 f"   pio run -e sim && ./.pio/build/sim/program --trials 10 --traj")
    rows = []
    with open(path, newline="") as fh:
        for r in csv.DictReader(fh):
            rows.append({
                "algo": r["algo"], "env": r["env"], "trial": r["trial"],
                "time_s": fnum(r["time_s"]), "path_cm": fnum(r["path_cm"]),
                "err_cm": fnum(r["err_cm"]), "success": r["success"] == "1",
                "src": (fnum(r["src_x"]), fnum(r["src_y"])),
            })
    return rows, "MO PHONG"


def load_runs(d="data/runs"):
    files = sorted(glob.glob(os.path.join(d, "*.csv")))
    files = [f for f in files if os.path.basename(f) != "meta.csv"]
    if not files:
        sys.exit(f"Khong co file nao trong {d}/. Ghi du lieu bang tools/receiver.py truoc.")

    meta_path = os.path.join(d, "meta.csv")
    meta = {}
    if os.path.exists(meta_path):
        with open(meta_path, newline="") as fh:
            for r in csv.DictReader(fh):
                meta[r["file"]] = r

    rows, missing = [], []
    for f in files:
        base = os.path.basename(f)
        with open(f, newline="") as fh:
            recs = list(csv.DictReader(fh))
        if not recs:
            continue
        last = recs[-1]

        # Ten file do receiver.py sinh: <ngay>_<gio>_<ALGO>_<ENV>_t<N>.csv
        # Tach tu CUOI len: ten thuat toan co the chua dau '_' (SURGE_CAST).
        parts = base.rsplit(".", 1)[0].split("_")
        algo = ALGO_SHORT.get(last.get("algo", ""), last.get("algo", "?"))
        env, trial = "?", "?"
        if len(parts) >= 5 and parts[-1].startswith("t"):
            env = parts[-2]
            trial = parts[-1][1:]
        elif len(parts) >= 4:
            env = parts[-1]

        m = meta.get(base)
        if not m or not m.get("src_x"):
            missing.append(base)
            src, err, ok = None, float("nan"), False
        else:
            src = (fnum(m["src_x"]), fnum(m["src_y"]))
            err = math.dist((fnum(last["x_cm"]), fnum(last["y_cm"])), src)
            ok = err <= SUCCESS_RADIUS_CM
        rows.append({
            "algo": algo, "env": m["env"] if m and m.get("env") else env,
            "trial": trial, "time_s": fnum(last["t_s"]),
            "path_cm": fnum(last["dist_cm"]), "err_cm": err,
            "success": ok, "src": src, "file": f,
        })

    if missing:
        write_meta_template(meta_path, rows, meta)
        print(f"!! {len(missing)} lan chay chua co vi tri nguon that.")
        print(f"   Da tao/cap nhat {meta_path} - dien cot src_x, src_y (cm) roi chay lai.")
        print("   (Sai so va ti le thanh cong khong tinh duoc neu thieu.)\n")
    return rows, "THUC NGHIEM"


def write_meta_template(path, rows, existing):
    with open(path, "w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow(["file", "algo", "env", "trial", "src_x", "src_y", "ghi_chu"])
        for r in rows:
            base = os.path.basename(r["file"])
            old = existing.get(base, {})
            w.writerow([base, r["algo"], r["env"], r["trial"],
                        old.get("src_x", ""), old.get("src_y", ""),
                        old.get("ghi_chu", "")])


# --------------------------------------------------------------------------- #
def summarize(rows, label):
    groups = defaultdict(list)
    for r in rows:
        groups[(r["algo"], r["env"])].append(r)

    # Duyet DUNG cac nhom co trong du lieu. Tuyet doi khong duyet theo danh sach
    # moi truong cung: mot moi truong dat ten khac se bi bo qua am tham.
    envs_found = sorted({r["env"] for r in rows})
    order = {a: i for i, a in enumerate(ALGO_ORDER)}
    keys = sorted(groups.keys(), key=lambda k: (order.get(k[0], 99), envs_found.index(k[1])))

    table = []
    for algo, env in keys:
        g = groups[(algo, env)]
        t, ts = mean_sd([r["time_s"] for r in g])
        p, ps = mean_sd([r["path_cm"] / 100.0 for r in g])
        errs = [r["err_cm"] for r in g if not math.isnan(r["err_cm"])]
        e, es = mean_sd(errs)
        ok = sum(1 for r in g if r["success"])
        table.append({
            "algo": algo, "env": env, "n": len(g),
            "time": t, "time_sd": ts, "path": p, "path_sd": ps,
            "err": e, "err_sd": es, "n_err": len(errs), "ok": ok,
        })

    w = 92
    print("=" * w)
    print(f" BANG KET QUA - SO LIEU {label}")
    print("=" * w)
    print(f" {'Thuat toan':<15}{'Moi truong':<26}{'N':>3}  {'Thoi gian (s)':>15}"
          f"  {'Quang duong (m)':>16}  {'Sai so (cm)':>13}  {'Th.cong':>8}")
    print("-" * w)
    for r in table:
        # Thieu vi tri nguon -> in dau gach, KHONG in 0.0 (de khong bi hieu la
        # sai so bang khong).
        err_txt = (f"{r['err']:>5.1f} +/-{r['err_sd']:>5.1f}" if r["n_err"]
                   else f"{'(chua co)':>13}")
        ok_txt = f"{r['ok']:>3}/{r['n']:<4}" if r["n_err"] else f"{'-':>8}"
        print(f" {ALGO_VN.get(r['algo'], r['algo']):<15}"
              f"{ENV_VN.get(r['env'], r['env']):<26}{r['n']:>3}  "
              f"{r['time']:>7.1f} +/-{r['time_sd']:>5.1f}  "
              f"{r['path']:>8.2f} +/-{r['path_sd']:>5.2f}  "
              f"{err_txt}  {ok_txt}")
    print("=" * w)
    print(f" Thanh cong = dung cach nguon <= {SUCCESS_RADIUS_CM:.0f} cm.")
    if label == "MO PHONG":
        print(" LUU Y: so lieu mo phong. Bao cao phai tach rieng khoi so lieu do that.")
    print()

    os.makedirs(FIG_DIR, exist_ok=True)
    out = os.path.join(FIG_DIR, "bang_ketqua.csv")
    with open(out, "w", newline="") as fh:
        w2 = csv.writer(fh)
        w2.writerow(["thuat_toan", "moi_truong", "n", "thoi_gian_tb_s", "thoi_gian_sd",
                     "quang_duong_tb_m", "quang_duong_sd", "sai_so_tb_cm", "sai_so_sd",
                     "so_lan_thanh_cong", "ti_le_thanh_cong"])
        for r in table:
            w2.writerow([ALGO_VN.get(r["algo"], r["algo"]), ENV_VN.get(r["env"], r["env"]),
                         r["n"], f"{r['time']:.1f}", f"{r['time_sd']:.1f}",
                         f"{r['path']:.2f}", f"{r['path_sd']:.2f}",
                         f"{r['err']:.1f}" if r["n_err"] else "",
                         f"{r['err_sd']:.1f}" if r["n_err"] else "",
                         r["ok"] if r["n_err"] else "",
                         f"{r['ok'] / r['n']:.2f}" if r["n_err"] else ""])
    print(f"Da ghi {out}")
    return table


# --------------------------------------------------------------------------- #
def bar_chart(table, key, sdkey, ylabel, title, fname):
    if not HAVE_PLT or not table:
        return
    envs = sorted({r["env"] for r in table})
    algos = [a for a in ALGO_ORDER if any(r["algo"] == a for r in table)]
    width = 0.8 / max(1, len(envs))

    fig, ax = plt.subplots(figsize=(8, 4.6))
    for i, env in enumerate(envs):
        xs, ys, es = [], [], []
        for j, a in enumerate(algos):
            r = next((r for r in table if r["algo"] == a and r["env"] == env), None)
            xs.append(j + i * width - 0.4 + width / 2)
            ys.append(r[key] if r else 0)
            es.append(r[sdkey] if r else 0)
        ax.bar(xs, ys, width * 0.92, yerr=es, capsize=4,
               label=ENV_VN.get(env, env))
    ax.set_xticks(range(len(algos)))
    ax.set_xticklabels([ALGO_VN.get(a, a) for a in algos])
    ax.set_ylabel(ylabel)
    ax.set_title(title)
    ax.legend()
    ax.grid(axis="y", alpha=0.3)
    fig.tight_layout()
    path = os.path.join(FIG_DIR, fname)
    fig.savefig(path, dpi=150)
    plt.close(fig)
    print(f"Da ghi {path}")


def success_chart(table):
    if not HAVE_PLT or not table:
        return
    envs = sorted({r["env"] for r in table})
    algos = [a for a in ALGO_ORDER if any(r["algo"] == a for r in table)]
    width = 0.8 / max(1, len(envs))
    fig, ax = plt.subplots(figsize=(8, 4.6))
    for i, env in enumerate(envs):
        xs, ys = [], []
        for j, a in enumerate(algos):
            r = next((r for r in table if r["algo"] == a and r["env"] == env), None)
            xs.append(j + i * width - 0.4 + width / 2)
            ys.append(100.0 * r["ok"] / r["n"] if r else 0)
        ax.bar(xs, ys, width * 0.92, label=ENV_VN.get(env, env))
    ax.set_xticks(range(len(algos)))
    ax.set_xticklabels([ALGO_VN.get(a, a) for a in algos])
    ax.set_ylabel("Ti le thanh cong (%)")
    ax.set_ylim(0, 105)
    ax.set_title("Ti le thanh cong")
    ax.legend()
    ax.grid(axis="y", alpha=0.3)
    fig.tight_layout()
    path = os.path.join(FIG_DIR, "fig_thanhcong.png")
    fig.savefig(path, dpi=150)
    plt.close(fig)
    print(f"Da ghi {path}")


# --------------------------------------------------------------------------- #
def plot_traj_sim(d="data/sim", per_group=1):
    if not HAVE_PLT:
        return
    # Ve dai dien moi to hop (thuat toan x moi truong), khong ve het cho roi.
    seen = defaultdict(int)
    chosen = []
    for f in sorted(glob.glob(os.path.join(d, "traj_*.csv"))):
        key = "_".join(os.path.basename(f).split("_")[1:3])
        if seen[key] < per_group:
            seen[key] += 1
            chosen.append(f)
    for f in chosen:
        src = None
        pts, gas = [], []
        with open(f) as fh:
            lines = fh.read().splitlines()
        header = lines[0].split(",")
        for ln in lines[1:]:
            if ln.startswith("#"):
                for tok in ln[1:].split():
                    if tok.startswith("source_x="):
                        sx = float(tok.split("=")[1])
                    elif tok.startswith("source_y="):
                        src = (sx, float(tok.split("=")[1]))
                continue
            v = ln.split(",")
            if len(v) < len(header):
                continue
            pts.append((float(v[1]), float(v[2])))
            gas.append(float(v[7]))
        if not pts:
            continue
        name = os.path.basename(f).replace("traj_", "").replace(".csv", "")
        _draw_traj(pts, gas, src, name, f"Quy dao mo phong - {name}")


def plot_traj_runs(rows, limit=6):
    if not HAVE_PLT:
        return
    for r in rows[:limit]:
        pts, gas = [], []
        with open(r["file"], newline="") as fh:
            for rec in csv.DictReader(fh):
                pts.append((fnum(rec["x_cm"]), fnum(rec["y_cm"])))
                gas.append(fnum(rec["norm"]))
        if not pts:
            continue
        name = os.path.basename(r["file"]).rsplit(".", 1)[0]
        _draw_traj(pts, gas, r["src"], name, f"Quy dao thuc nghiem - {name}")


def _draw_traj(pts, gas, src, name, title):
    xs = [p[0] for p in pts]
    ys = [p[1] for p in pts]

    fig, ax = plt.subplots(figsize=(6.2, 6))
    ax.plot(xs, ys, "-", lw=1, color="0.6", zorder=1)
    sc = ax.scatter(xs, ys, c=gas, cmap="inferno", s=14, zorder=2)
    fig.colorbar(sc, ax=ax, label="gas_normalized (dem ADC)")
    ax.plot(xs[0], ys[0], "o", color="tab:blue", ms=11, label="xuat phat", zorder=3)
    ax.plot(xs[-1], ys[-1], "s", color="tab:green", ms=11, label="ket luan", zorder=3)
    if src:
        ax.plot(src[0], src[1], "*", color="tab:red", ms=20, label="nguon that", zorder=4)
        ax.add_patch(plt.Circle(src, SUCCESS_RADIUS_CM, fill=False,
                                ls="--", color="tab:red", alpha=0.6))
    ax.set_xlim(0, ARENA_W)
    ax.set_ylim(0, ARENA_H)
    ax.set_aspect("equal")
    ax.set_xlabel("x (cm)")
    ax.set_ylabel("y (cm)")
    ax.set_title(title)
    ax.legend(loc="upper left", fontsize=8)
    ax.grid(alpha=0.25)
    fig.tight_layout()
    path = os.path.join(FIG_DIR, f"fig_quydao_{name}.png")
    fig.savefig(path, dpi=150)
    plt.close(fig)
    print(f"Da ghi {path}")

    # --- ban do nhiet nong do tho theo o luoi ---
    cell = 25.0
    nx, ny = int(ARENA_W / cell), int(ARENA_H / cell)
    acc = [[[] for _ in range(nx)] for _ in range(ny)]
    for (x, y), g in zip(pts, gas):
        i, j = int(x / cell), int(y / cell)
        if 0 <= i < nx and 0 <= j < ny:
            acc[j][i].append(g)
    grid = [[(max(c) if c else float("nan")) for c in row] for row in acc]

    fig, ax = plt.subplots(figsize=(6.2, 5.4))
    im = ax.imshow(grid, origin="lower", extent=[0, ARENA_W, 0, ARENA_H],
                   cmap="inferno", interpolation="nearest")
    fig.colorbar(im, ax=ax, label="gas_normalized cao nhat trong o")
    if src:
        ax.plot(src[0], src[1], "*", color="cyan", ms=20, label="nguon that")
        ax.legend(loc="upper left", fontsize=8)
    ax.set_xlabel("x (cm)")
    ax.set_ylabel("y (cm)")
    ax.set_title(f"Ban do nhiet nong do tho - {name}")
    fig.tight_layout()
    path = os.path.join(FIG_DIR, f"fig_bandonhiet_{name}.png")
    fig.savefig(path, dpi=150)
    plt.close(fig)
    print(f"Da ghi {path}")


# --------------------------------------------------------------------------- #
def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--sim", action="store_true", help="phan tich ket qua mo phong")
    ap.add_argument("--dir", default=None)
    ap.add_argument("--no-traj", action="store_true")
    a = ap.parse_args()

    os.makedirs(FIG_DIR, exist_ok=True)
    if a.sim:
        rows, label = load_sim(a.dir or "data/sim")
    else:
        rows, label = load_runs(a.dir or "data/runs")

    table = summarize(rows, label)
    bar_chart(table, "time", "time_sd", "Thoi gian dinh vi (s)",
              "Thoi gian dinh vi", "fig_thoigian.png")
    bar_chart(table, "path", "path_sd", "Quang duong (m)",
              "Chieu dai quang duong", "fig_quangduong.png")
    bar_chart(table, "err", "err_sd", "Sai so (cm)",
              "Sai so dinh vi", "fig_saiso.png")
    success_chart(table)

    if not a.no_traj:
        if a.sim:
            plot_traj_sim(a.dir or "data/sim")
        else:
            plot_traj_runs([r for r in rows if r.get("file")])

    if not HAVE_PLT:
        print("\n!! Thieu matplotlib nen khong ve duoc bieu do. Cai bang:")
        print("   python3 -m pip install matplotlib")


if __name__ == "__main__":
    main()
