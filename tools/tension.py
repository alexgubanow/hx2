#!/usr/bin/env python3
"""HX2 tension calculator: Hz <-> N, and fixture force <-> belt tension.

Examples:
  python tools/tension.py hz 120 --span 150 --mu 7.7     # Hz -> N
  python tools/tension.py n 10 --span 150 --mu 7.7       # N -> Hz
  python tools/tension.py fix 1.2 --s 80 --d 2.5         # probe force N -> tension N
  python tools/tension.py design 15 --s 80 --d 2.5       # tension N -> expected probe force

Units: span/s/d in mm, mu in g/m (weigh 1 m of your belt), forces in N.
"""
import argparse
import math

G = 9.80665


def hz_to_n(f, span_mm, mu_gpm):
    L = span_mm / 1000
    mu = mu_gpm / 1000
    return 4 * L * L * f * f * mu


def n_to_hz(t, span_mm, mu_gpm):
    L = span_mm / 1000
    mu = mu_gpm / 1000
    return math.sqrt(t / mu) / (2 * L)


def sin_theta(s_mm, d_mm):
    return d_mm / math.hypot(d_mm, s_mm / 2)


def force_to_tension(F, s_mm, d_mm):
    return F / (2 * sin_theta(s_mm, d_mm))


def tension_to_force(T, s_mm, d_mm):
    return 2 * T * sin_theta(s_mm, d_mm)


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers(dest="cmd", required=True)
    a = sub.add_parser("hz"); a.add_argument("f", type=float)
    a.add_argument("--span", type=float, required=True); a.add_argument("--mu", type=float, required=True)
    b = sub.add_parser("n"); b.add_argument("t", type=float)
    b.add_argument("--span", type=float, required=True); b.add_argument("--mu", type=float, required=True)
    c = sub.add_parser("fix"); c.add_argument("F", type=float)
    c.add_argument("--s", type=float, required=True); c.add_argument("--d", type=float, required=True)
    d = sub.add_parser("design"); d.add_argument("T", type=float)
    d.add_argument("--s", type=float, required=True); d.add_argument("--d", type=float, required=True)
    args = p.parse_args()

    if args.cmd == "hz":
        t = hz_to_n(args.f, args.span, args.mu)
        print(f"{args.f:.1f} Hz over {args.span:.0f} mm, mu={args.mu} g/m  ->  T = {t:.2f} N ({t / G * 1000:.0f} gf)")
    elif args.cmd == "n":
        print(f"T = {args.t:.2f} N over {args.span:.0f} mm, mu={args.mu} g/m  ->  f = {n_to_hz(args.t, args.span, args.mu):.1f} Hz")
    elif args.cmd == "fix":
        t = force_to_tension(args.F, args.s, args.d)
        print(f"F = {args.F:.3f} N, s={args.s} mm, d={args.d} mm  ->  T = {t:.2f} N")
    elif args.cmd == "design":
        F = tension_to_force(args.T, args.s, args.d)
        stretch = 2 * math.hypot(args.d, args.s / 2) - args.s
        print(f"T = {args.T:.2f} N, s={args.s} mm, d={args.d} mm  ->  F = {F:.3f} N ({F / G * 1000:.0f} g on the cell)")
        print(f"  F/T = {F / args.T:.4f}   extra belt path pushed in = {stretch:.3f} mm")


if __name__ == "__main__":
    main()
