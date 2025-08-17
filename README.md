# Project

<!-- OLYMPICS START -->

# 🏅 Data Structures Olympics

Automatic performance comparison (insert/search) executed via GitHub Actions.

This section is updated on every push.


## Last run

- Commit: `8891fa357854`  
- Timestamp (UTC): `2025-08-17T10:25:20Z`  
- Workload: insert=100000, hit=100000, miss=100000, reps=5

### Results (medians)

| Structure | Insert (ms) | Search hit (ms) | Search miss (ms) | Total (ms) | OPS/s hit | OPS/s miss |
|---|---:|---:|---:|---:|---:|---:|
| skiplist | 40.049 | 32.984 | 2.707 | 76.361 | 3031773 | 36941263 |
| list | 1.430 | 6446.237 | 12851.716 | 19299.994 | 15513 | 7781 |

## Hall of Fame (best total – median ms)

| Structure | Best total (ms) | Commit | Timestamp (UTC) |
|---|---:|---|---|
| skiplist | 76.361 | `8891fa357854` | 2025-08-17T10:25:20Z |
| list | 19299.994 | `8891fa357854` | 2025-08-17T10:25:20Z |

<!-- OLYMPICS END -->
