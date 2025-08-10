import os
import matplotlib.pyplot as plt
import numpy as np

folder = "./branchPredOutputs"

predictors = [
    "Nbit-16K-1",
    "Nbit-8K-2",
    "Static-NotTaken",
    "Static-BTFNT",
    "Gshare-13b",
    "BTB-512-1-16",
    "BTB-64-8-16"  
]

mpki_data = {}

for filename in sorted(os.listdir(folder)):
    if not filename.endswith(".bp.out"):
        continue

    path = os.path.join(folder, filename)
    benchmark = filename.replace(".bp.out", "")
    total_instructions = 1  # avoid div by 0
    predictor_mpki = {}

    with open(path) as f:
        for line in f:
            line = line.strip()
            tokens = line.split()
            if line.startswith("Total Instructions:"):
                total_instructions = int(tokens[2])
            else:
                for predictor in predictors:
                    if line.startswith(predictor + ":"):
                        try:
                            incorrect_idx = tokens.index("Incorrect:")
                            incorrect = int(tokens[incorrect_idx + 1])
                            mpki = (incorrect / total_instructions) * 1000
                            predictor_mpki[predictor] = mpki
                        except (ValueError, IndexError):
                            continue
    mpki_data[benchmark] = predictor_mpki

# Plot
benchmarks = sorted(mpki_data.keys())
x = np.arange(len(benchmarks))
width = 0.11 

fig, ax = plt.subplots(figsize=(16, 6))

for i, predictor in enumerate(predictors):
    mpkis = [mpki_data[b].get(predictor, 0) for b in benchmarks]
    ax.bar(x + i * width, mpkis, width, label=predictor)

ax.set_xlabel("Benchmark")
ax.set_ylabel("MPKI")
ax.set_title("MPKI per Branch Predictor Across Benchmarks")
ax.set_xticks(x + width * (len(predictors) - 1) / 2)
ax.set_xticklabels(benchmarks, rotation=45)
ax.legend()
ax.grid(True)

plt.tight_layout()
plt.savefig("summary_mpki.png")
print("Saved: summary_mpki.png")
