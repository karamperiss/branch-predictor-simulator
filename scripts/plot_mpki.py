#!/usr/bin/env python3

import sys
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
import os

plt.style.use('seaborn-v0_8-darkgrid') 

included_predictors = ["Nbit", "BTB", "Gshare", "Static"]

x_Axis = []
mpki_Axis = []
total_ins = 1 

if len(sys.argv) < 2:
    print("Usage: python3 plot_mpki.py <file.bp.out>")
    sys.exit(1)

input_path = sys.argv[1]
benchmark_name = os.path.basename(input_path).replace(".bp.out", "")

with open(input_path, 'r') as fp:
    for line in fp:
        line = line.strip()
        tokens = line.split()
        if line.startswith("Total Instructions:"):
            total_ins = int(tokens[2])
        elif any(p in line for p in included_predictors) and "Incorrect:" in line:
            predictor_name = tokens[0].rstrip(':')
            try:
                incorrect_idx = tokens.index("Incorrect:")
                incorrect_predictions = int(tokens[incorrect_idx + 1])
                x_Axis.append(predictor_name)
                mpki = (incorrect_predictions / total_ins) * 1000.0
                mpki_Axis.append(mpki)
            except (ValueError, IndexError):
                continue

# Plot
fig, ax = plt.subplots(figsize=(10, 6))
bars = ax.bar(x_Axis, mpki_Axis, color=plt.cm.tab10.colors)

for bar in bars:
    height = bar.get_height()
    ax.annotate(f'{height:.2f}',
                xy=(bar.get_x() + bar.get_width() / 2, height),
                xytext=(0, 5),
                textcoords="offset points",
                ha='center', va='bottom',
                fontsize=10)

ax.set_xlabel("Branch Predictors", fontsize=12)
ax.set_ylabel("MPKI", fontsize=12)
ax.set_title(f"MPKI per Predictor for {benchmark_name}", fontsize=14)
ax.set_ylim(0, max(mpki_Axis) * 1.25)
plt.xticks(rotation=45)
plt.tight_layout()

output_file = f"{benchmark_name}_mpki.png"
plt.savefig(output_file, bbox_inches="tight")
print(f"Saved plot to {output_file}")
