#!/usr/bin/env python3
import argparse
import csv
import datetime as dt
import os
import sys
import time

try:
    import serial
    from serial.tools import list_ports
except ImportError:
    sys.exit("Thieu pyserial. Cai bang:  python3 -m pip install pyserial")

FIELDS = ["t_s", "algo", "state", "adc", "norm", "ppm", "level",
          "x_cm", "y_cm", "head_deg", "dist_cm", "cell_x", "cell_y",
          "best_norm", "finished", "rssi", "snr"]

COLOR = {"SAFE": "\033[92m", "DETECTED": "\033[93m",
         "HIGH": "\033[33m", "CRITICAL": "\033[91m"}
RESET = "\033[0m"


def checksum_ok(pkt: str) -> bool:
    if not pkt.startswith("$") or "*" not in pkt:
        return False
    body, _, ck = pkt[1:].rpartition("*")
    c = 0
    for ch in body:
        c ^= ord(ch)
    try:
        return c == int(ck[:2], 16)
    except ValueError:
        return False


def parse(line: str):
    line = line.strip()
    rssi = snr = ""
    if line.startswith("RX,"):
        parts = line.split(",", 3)
        if len(parts) < 4:
            return None, "dong RX khong du truong"
        rssi, snr, line = parts[1], parts[2], parts[3]
    if not line.startswith("$GS,"):
        return None, None
    if not checksum_ok(line):
        return None, "sai checksum"

    body = line[1:].rpartition("*")[0]
    f = body.split(",")
    if len(f) < 16:
        return None, "thieu truong"
    rec = dict(zip(FIELDS[:15], f[1:16]))
    rec["rssi"], rec["snr"] = rssi, snr
    return rec, None


def pick_port(explicit):
    if explicit:
        return explicit
    ports = [p for p in list_ports.comports()
             if "usb" in p.device.lower() or "USB" in (p.description or "")]
    if not ports:
        sys.exit("Khong tim thay cong USB nao. Dung --port de chi ro.")
    if len(ports) > 1:
        print("Co nhieu cong, chon cong dau tien. Danh sach:")
        for p in ports:
            print(f"   {p.device}  -  {p.description}")
    return ports[0].device


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--port")
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--algo", default="NA", help="ghi vao ten file, vd GRADIENT")
    ap.add_argument("--env", default="NA", help="khong-quat | co-quat")
    ap.add_argument("--trial", default="1")
    ap.add_argument("--out", default="data/runs")
    ap.add_argument("--no-write", action="store_true", help="chi xem, khong ghi file")
    ap.add_argument("--list", action="store_true", help="liet ke cong roi thoat")
    args = ap.parse_args()

    if args.list:
        for p in list_ports.comports():
            print(f"{p.device}  -  {p.description}")
        return

    args.algo = args.algo.replace("_", "-")
    args.env = args.env.replace("_", "-")

    port = pick_port(args.port)
    print(f"Mo {port} @ {args.baud}...")
    ser = serial.Serial(port, args.baud, timeout=1)
    time.sleep(0.3)

    writer = fh = path = None
    if not args.no_write:
        os.makedirs(args.out, exist_ok=True)
        stamp = dt.datetime.now().strftime("%Y%m%d_%H%M%S")
        path = os.path.join(args.out, f"{stamp}_{args.algo}_{args.env}_t{args.trial}.csv")
        fh = open(path, "w", newline="")
        writer = csv.DictWriter(fh, fieldnames=FIELDS)
        writer.writeheader()
        print(f"Ghi vao {path}")
    print("Ctrl-C de dung.\n")

    n = bad = 0
    peak_ppm = 0.0
    last = None
    try:
        while True:
            raw = ser.readline().decode("utf-8", "replace")
            if not raw.strip():
                continue
            rec, err = parse(raw)
            if rec is None:
                if err:
                    bad += 1
                    print(f"  [bo qua: {err}]")
                else:
                    print("  " + raw.rstrip())
                continue

            n += 1
            last = rec
            peak_ppm = max(peak_ppm, float(rec["ppm"] or 0))
            if writer:
                writer.writerow(rec)
                fh.flush()

            c = COLOR.get(rec["level"], "")
            print(f"  t={rec['t_s']:>6}s  {rec['algo']:<3} {rec['state']:<12} "
                  f"ADC={rec['adc']:>4} norm={rec['norm']:>5} "
                  f"{c}{rec['level']:<8}{RESET} ppm~{rec['ppm']:>5} "
                  f"o=({rec['cell_x']},{rec['cell_y']}) "
                  f"({rec['x_cm']},{rec['y_cm']}) {rec['dist_cm']}cm")
    except KeyboardInterrupt:
        pass
    finally:
        ser.close()
        if fh:
            fh.close()
        print("\n--------------------------------------------------")
        print(f"Nhan duoc {n} goi, bo {bad} goi loi.")
        if last:
            print(f"Thoi gian   : {last['t_s']} s")
            print(f"Quang duong : {last['dist_cm']} cm")
            print(f"Vi tri cuoi : ({last['x_cm']}, {last['y_cm']}) cm")
            print(f"ppm cao nhat: {peak_ppm:.0f}")
        if path:
            print(f"File        : {path}")


if __name__ == "__main__":
    main()
