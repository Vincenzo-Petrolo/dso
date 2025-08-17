#!/usr/bin/env python3
import json, os, re
from pathlib import Path

HIST_PATH = Path("bench_history.json")
RES_PATH = Path("bench_results.json")
MD_PATH = Path("OLYMPICS.md")
README_PATH = Path("README.md")

START_MARK = "<!-- OLYMPICS START -->"
END_MARK = "<!-- OLYMPICS END -->"

def load_json(p, default):
    if p.exists():
        return json.loads(p.read_text())
    return default

def render_markdown(res, hist):
    # Compute best per structure (by minimal median total_ms)
    best = {}
    for entry in hist["history"]:
        agg = entry["aggregate"]
        for name, g in agg.items():
            total = g["total_ms"]["median"]
            if name not in best or total < best[name]["total_ms_median"]:
                best[name] = {
                    "total_ms_median": total,
                    "sha": entry.get("git_sha","local")[:12],
                    "timestamp": entry["timestamp"]
                }

    lines = []
    lines.append("# 🏅 Data Structures Olympics\n")
    lines.append("Automatic performance comparison (insert/search) executed via GitHub Actions.\n")
    lines.append("This section is updated on every push.\n")
    lines.append("\n## Last run\n")
    ts = res["timestamp"]
    sha = res.get("git_sha","local")[:12]
    wl = res["workload"]
    lines.append(f"- Commit: `{sha}`  \n- Timestamp (UTC): `{ts}`  \n- Workload: insert={wl['n_insert']}, hit={wl['n_hit']}, miss={wl['n_miss']}, reps={res['repetitions']}")
    lines.append("\n### Results (medians)\n")
    lines.append("| Structure | Insert (ms) | Search hit (ms) | Search miss (ms) | Total (ms) | OPS/s hit | OPS/s miss |")
    lines.append("|---|---:|---:|---:|---:|---:|---:|")
    for name, g in res["aggregate"].items():
        ops = g["ops_per_sec"]
        lines.append(f"| {name} | {g['insert_ms']['median']:.3f} | {g['search_hit_ms']['median']:.3f} | {g['search_miss_ms']['median']:.3f} | {g['total_ms']['median']:.3f} | {ops['search_hit']:.0f} | {ops['search_miss']:.0f} |")

    lines.append("\n## Hall of Fame (best total – median ms)\n")
    lines.append("| Structure | Best total (ms) | Commit | Timestamp (UTC) |")
    lines.append("|---|---:|---|---|")
    for name, b in best.items():
        lines.append(f"| {name} | {b['total_ms_median']:.3f} | `{b['sha']}` | {b['timestamp']} |")

    return "\n".join(lines)

def update_readme_section(content):
    if README_PATH.exists():
        text = README_PATH.read_text()
        if START_MARK in text and END_MARK in text and text.index(START_MARK) < text.index(END_MARK):
            # replace between markers
            start = text.index(START_MARK) + len(START_MARK)
            end = text.index(END_MARK)
            new_text = text[:start] + "\n\n" + content + "\n\n" + text[end:]
        else:
            # append a new section with markers at the end
            new_text = text.rstrip() + "\n\n" + START_MARK + "\n\n" + content + "\n\n" + END_MARK + "\n"
    else:
        # create a basic README with markers
        new_text = "# Project\n\n" + START_MARK + "\n\n" + content + "\n\n" + END_MARK + "\n"
    README_PATH.write_text(new_text)

def main():
    res = load_json(RES_PATH, None)
    if res is None:
        raise SystemExit("bench_results.json not found, run run_bench.py first")
    hist = load_json(HIST_PATH, {"history":[]})

    hist["history"].append(res)
    HIST_PATH.write_text(json.dumps(hist, indent=2))

    md = render_markdown(res, hist)

    # Write canonical full page
    MD_PATH.write_text(md)

    # Inject into README between markers
    update_readme_section(md)

    # Also append to job summary
    with open(os.environ.get("GITHUB_STEP_SUMMARY","/dev/null"), "a") as f:
        f.write("# Leaderboard updated\nSee **OLYMPICS.md** and README section.\n")

    print("[ok] Updated OLYMPICS.md, README.md and bench_history.json")

if __name__ == "__main__":
    main()
