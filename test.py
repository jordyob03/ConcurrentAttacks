import subprocess
import re
import matplotlib.pyplot as plt
from statistics import mean


hash_algos = ["SHA-1", "SHA-256", "SHA-512", "MD5"]
thread_counts = [1, 2, 4, 8, 16]
trials = 10


# results dict
results = {
    hash_name: {
        "seq_times": [],
        "par_times": {tc: [] for tc in thread_counts}
    } for hash_name in hash_algos
}

seq_pattern = r"=== Hash Algorithm: (.*?) ===\nTarget hash: .*\nSequential result: .*?, Time: ([\d.]+)s"
par_pattern = r"Parallel \((\d+) threads\) result: .*?, Time: ([\d.]+)s"

# Run attack 
for trial in range(trials):
    print(f"Running trial {trial + 1}...")
    try:
        result = subprocess.run(["./attack"], capture_output=True, text=True, check=True)
        output = result.stdout

        # sequential times
        for match in re.finditer(seq_pattern, output):
            hash_name = match.group(1).strip()
            time = float(match.group(2))
            if hash_name in results:
                results[hash_name]["seq_times"].append(time)

        # parallel times
        current_algo_index = -1
        lines = output.splitlines()
        for i, line in enumerate(lines):
            if line.startswith("=== Hash Algorithm:"):
                current_algo_index += 1
            match = re.match(par_pattern, line)
            if match and current_algo_index >= 0:
                threads = int(match.group(1))
                time = float(match.group(2))
                hash_name = hash_algos[current_algo_index]
                results[hash_name]["par_times"][threads].append(time)

    except subprocess.CalledProcessError as e:
        print("Error during subprocess execution:", e.stderr)

aggregated = {}

for hash_name in hash_algos:
    seq_avg = mean(results[hash_name]["seq_times"])
    par_avgs = []
    speedups = []
    efficiencies = []

    print(f"\n=== {hash_name} ===")
    print(f"Avg Sequential Time: {seq_avg:.4f}s")
    
    for tc in thread_counts:
        if results[hash_name]["par_times"][tc]:
            par_avg = mean(results[hash_name]["par_times"][tc])
            speedup = seq_avg / par_avg
            efficiency = speedup / tc
        else:
            par_avg = speedup = efficiency = 0.0

        par_avgs.append(par_avg)
        speedups.append(speedup)
        efficiencies.append(efficiency)

        print(f"Threads: {tc}, Avg Parallel: {par_avg:.4f}s, Speedup: {speedup:.2f}, Efficiency: {efficiency:.2f}")

    aggregated[hash_name] = {
        "seq_time": seq_avg,
        "parallel_times": par_avgs,
        "speedups": speedups,
        "efficiencies": efficiencies
    }

# --- Plotting ---
# 1. Time vs Threads
plt.figure(figsize=(10, 6))
for hash_name in hash_algos:
    plt.plot(thread_counts, aggregated[hash_name]["parallel_times"], marker='o', label=f"{hash_name} Parallel")
    plt.hlines(aggregated[hash_name]["seq_time"], min(thread_counts), max(thread_counts),
               linestyles='dashed', label=f"{hash_name} Sequential")

plt.xlabel("Number of Threads")
plt.ylabel("Time (seconds)")
plt.title("Execution Time vs Thread Count (Avg of 10 Runs)")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()

# 2. Speedup vs Threads
plt.figure(figsize=(10, 6))
for hash_name in hash_algos:
    plt.plot(thread_counts, aggregated[hash_name]["speedups"], marker='o', label=hash_name)

plt.xlabel("Number of Threads")
plt.ylabel("Speedup")
plt.title("Speedup vs Thread Count (Avg of 10 Runs)")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()

# 3. Efficiency vs Threads
plt.figure(figsize=(10, 6))
for hash_name in hash_algos:
    plt.plot(thread_counts, aggregated[hash_name]["efficiencies"], marker='o', label=hash_name)

plt.xlabel("Number of Threads")
plt.ylabel("Efficiency")
plt.title("Efficiency vs Thread Count (Avg of 10 Runs)")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()
