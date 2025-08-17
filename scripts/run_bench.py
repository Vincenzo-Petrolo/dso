#!/usr/bin/env python3
import json, os, re, subprocess, statistics, platform, time
from pathlib import Path

CFG_PATH = Path("bench_config.json")
DEFAULT_CFG = {
  "build": {
    "cc": "gcc",
    "cflags": "-O3 -std=c11",
    "sources": ["main.c", "skiplist.c", "list.c"],
    "output": "bench"
  },
  "workload": {"n_insert": 100000, "n_hit": 100000, "n_miss": 100000, "seed": 42},
  "repetitions": 5
}

def load_cfg():
    if CFG_PATH.exists():
        with open(CFG_PATH, "r") as f:
            return json.load(f)
    return DEFAULT_CFG

def build(cfg):
    cc = cfg["build"]["cc"]
    cflags = cfg["build"]["cflags"]
    sources = " ".join(cfg["build"]["sources"])
    out = cfg["build"]["output"]
    cmd = f"{cc} {cflags} {sources} -o {out}"
    print(f"[build] {cmd}")
    subprocess.run(cmd, shell=True, check=True)

LINE_RE = re.compile(
    r"^\[(?P<name>[^\]]+)\]\s+inserts:\s+(?P<ins>[\d.]+)\s+ms,\s+search hit:\s+(?P<hit>[\d.]+)\s+ms,\s+search miss:\s+(?P<miss>[\d.]+)\s+ms",
    re.MULTILINE
)

def run_once(cfg):
    out = cfg["build"]["output"]
    wl = cfg["workload"]
    args = f"{wl['n_insert']} {wl['n_hit']} {wl['n_miss']} {wl['seed']}"
    cmd = f"./{out} {args}"
    print(f"[run] {cmd}")
    cp = subprocess.run(cmd, shell=True, check=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    txt = cp.stdout
    print(txt)
    results = {}
    for m in LINE_RE.finditer(txt):
        name = m.group("name").strip()
        ins = float(m.group("ins"))
        hit = float(m.group("hit"))
        miss = float(m.group("miss"))
        total = ins + hit + miss
        results[name] = {"insert_ms": ins, "search_hit_ms": hit, "search_miss_ms": miss, "total_ms": total}
    if not results:
        raise RuntimeError("No benchmark lines parsed. Check the bench output format.")
    return results, txt

def aggregate(rep_results):
    # rep_results: list of dict(name->metrics)
    names = set().union(*[set(d.keys()) for d in rep_results])
    agg = {}
    for name in names:
        series = {k: [] for k in ["insert_ms","search_hit_ms","search_miss_ms","total_ms"]}
        for r in rep_results:
            if name in r:
                for k in series: series[k].append(r[name][k])
        agg[name] = {k: {
            "mean": statistics.mean(v),
            "median": statistics.median(v),
            "stdev": (statistics.pstdev(v) if len(v)>1 else 0.0),
            "min": min(v),
            "max": max(v),
            "samples": v
        } for k,v in series.items()}
    return agg

def main():
    cfg = load_cfg()
    build(cfg)
    wl = cfg["workload"]
    reps = int(cfg.get("repetitions", 3))
    rep_results = []
    for i in range(reps):
        print(f"=== repetition {i+1}/{reps} ===")
        r, _ = run_once(cfg)
        rep_results.append(r)
    agg = aggregate(rep_results)

    # Enrich with ops/s (using medians)
    for name, groups in agg.items():
        ins_s = groups["insert_ms"]["median"]/1000.0
        hit_s = groups["search_hit_ms"]["median"]/1000.0
        miss_s = groups["search_miss_ms"]["median"]/1000.0
        groups["ops_per_sec"] = {
            "insert": (wl["n_insert"]/ins_s) if ins_s>0 else None,
            "search_hit": (wl["n_hit"]/hit_s) if hit_s>0 else None,
            "search_miss": (wl["n_miss"]/miss_s) if miss_s>0 else None,
        }

    payload = {
        "timestamp": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
        "git_sha": os.environ.get("GITHUB_SHA","local"),
        "runner_os": platform.platform(),
        "workload": wl,
        "repetitions": reps,
        "aggregate": agg,
        "raw_runs": rep_results
    }

    with open("bench_results.json","w") as f:
        json.dump(payload, f, indent=2)

    # Job summary for Actions UI
    lines = ["# Olympics – Latest results\n"]
    for name, groups in agg.items():
        lines.append(f"## {name}")
        lines.append(f"- Insert (ms): median **{groups['insert_ms']['median']:.3f}** (mean {groups['insert_ms']['mean']:.3f} ± {groups['insert_ms']['stdev']:.3f})")
        lines.append(f"- Search hit (ms): median **{groups['search_hit_ms']['median']:.3f}**")
        lines.append(f"- Search miss (ms): median **{groups['search_miss_ms']['median']:.3f}**")
        lines.append(f"- Total (ms): median **{groups['total_ms']['median']:.3f}**")
        ops = groups["ops_per_sec"]
        lines.append(f"- OPS/s: insert ~{ops['insert']:.0f}, hit ~{ops['search_hit']:.0f}, miss ~{ops['search_miss']:.0f}\n")
    with open(os.environ.get("GITHUB_STEP_SUMMARY","/dev/null"), "a") as f:
        f.write("\n".join(lines))

    print("[ok] Wrote bench_results.json and job summary")

if __name__ == "__main__":
    main()
