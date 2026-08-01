#!/usr/bin/env python3
"""
mq3_fit.py - hai viec cua Ngay 4 (de cuong muc 11.1b va 11.1c).

1) FIT duong dac tuyen  ppm = A * (Rs/R0)^B  tu HAI diem doc tren do thi
   ethanol trong datasheet MQ-3. Truc log-log la duong thang nen hai diem la du.

       python3 tools/mq3_fit.py fit --p1 0.1 2.6 --p2 10 0.22 --unit mgl

   --p1/--p2 la cap  <nong do> <ti so Rs/R0>  doc tren do thi.
   --unit mgl neu do thi ghi mg/L (mac dinh cua MQ-3), ppm neu ghi ppm.

2) DE XUAT ba nguong canh bao T1/T2/T3 tu mot file log do thuc te.

       python3 tools/mq3_fit.py levels --csv data/runs/xxx.csv

   Doc cot ppm, lay cac phan vi de goi y nguong. Van phai tu quyet dinh cuoi.
"""
import argparse
import csv
import math
import sys

# 1 mg/L ethanol o 25 C, 1 atm  ->  ppm the tich.
# ppm = (mg/m3) * 24.45 / M  voi M(ethanol) = 46.07 g/mol, 1 mg/L = 1000 mg/m3.
MGL_TO_PPM = 1000.0 * 24.45 / 46.07  # ~ 530.7


def cmd_fit(a):
    c1, r1 = a.p1
    c2, r2 = a.p2
    if a.unit == "mgl":
        ppm1, ppm2 = c1 * MGL_TO_PPM, c2 * MGL_TO_PPM
        print(f"Quy doi mg/L -> ppm (he so {MGL_TO_PPM:.1f}):")
        print(f"   {c1} mg/L = {ppm1:.0f} ppm     {c2} mg/L = {ppm2:.0f} ppm")
    else:
        ppm1, ppm2 = c1, c2

    if r1 <= 0 or r2 <= 0 or ppm1 <= 0 or ppm2 <= 0 or r1 == r2:
        sys.exit("Hai diem khong hop le (phai duong va khac nhau).")

    B = math.log10(ppm2 / ppm1) / math.log10(r2 / r1)
    A = ppm1 / (r1 ** B)

    print()
    print("Ket qua fit:")
    print(f"   A = {A:.4g}")
    print(f"   B = {B:.4g}")
    print()
    print("Dan hai dong nay vao src/core/config.h (muc 5):")
    print(f"   constexpr float MQ3_CURVE_A = {A:.4g}f;")
    print(f"   constexpr float MQ3_CURVE_B = {B:.4g}f;")
    print()
    print("Kiem tra lai bang vai gia tri ti so Rs/R0:")
    print(f"   {'Rs/R0':>8}  {'ppm uoc luong':>14}")
    for r in (3.0, 2.0, 1.0, 0.7, 0.5, 0.3, 0.2, 0.1):
        print(f"   {r:>8.2f}  {A * r ** B:>14.0f}")
    print()
    print("Luu y: day chi la uoc luong BAC DO LON. Bao cao phai ghi ro dieu do")
    print("(de cuong muc 11.1d) va khong duoc dua ppm vao logic dieu khien.")


def cmd_levels(a):
    vals = []
    with open(a.csv, newline="") as fh:
        for row in csv.DictReader(fh):
            try:
                vals.append(float(row.get("ppm") or row.get("ppm_est") or 0))
            except ValueError:
                pass
    if not vals:
        sys.exit("Khong doc duoc cot 'ppm' nao trong file.")
    vals.sort()

    def q(p):
        return vals[min(len(vals) - 1, int(p * len(vals)))]

    print(f"Doc {len(vals)} mau tu {a.csv}")
    print(f"   nho nhat {vals[0]:.0f} | trung vi {q(0.5):.0f} | lon nhat {vals[-1]:.0f} ppm")
    print()
    print("De xuat nguong (lam tron), CAN kiem tra lai bang mat:")
    print(f"   constexpr float PPM_T1 = {round(q(0.70), -1):.0f}f;   // SAFE -> DETECTED")
    print(f"   constexpr float PPM_T2 = {round(q(0.90), -1):.0f}f;   // DETECTED -> HIGH")
    print(f"   constexpr float PPM_T3 = {round(q(0.98), -1):.0f}f;   // HIGH -> CRITICAL")
    print()
    print("Trong san pham that voi khi doc, ba nguong nay phai dat theo gioi han")
    print("phoi nhiem nghe nghiep (TLV-TWA, IDLH) chu khong theo phan vi do dac.")


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)

    f = sub.add_parser("fit", help="fit A, B tu hai diem tren datasheet")
    f.add_argument("--p1", nargs=2, type=float, required=True, metavar=("NONGDO", "TISO"))
    f.add_argument("--p2", nargs=2, type=float, required=True, metavar=("NONGDO", "TISO"))
    f.add_argument("--unit", choices=["mgl", "ppm"], default="mgl")
    f.set_defaults(func=cmd_fit)

    l = sub.add_parser("levels", help="de xuat T1/T2/T3 tu mot file log")
    l.add_argument("--csv", required=True)
    l.set_defaults(func=cmd_levels)

    a = ap.parse_args()
    a.func(a)


if __name__ == "__main__":
    main()
