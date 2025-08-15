# branch-predictor-simulator

##  Overview
This repository contains the implementation and evaluation of various **branch predictors** using the [Intel PIN Tool](https://software.intel.com/content/www/us/en/develop/articles/pin-a-dynamic-binary-instrumentation-tool.html) and the [PARSEC Benchmark Suite](https://github.com/bamos/parsec-benchmark).

The main goal was to:
- Implement both **static** and **dynamic** branch predictors.
- Compare their performance across different workloads.
- Visualize results using Python scripts.

---

## 🧩 Implemented Predictors

| Predictor Name          | Type      | Description |
|------------------------|-----------|-------------|
| **Static-NotTaken**    | Static    | Always predicts branch as **not taken**. |
| **Static-BTFNT**       | Static    | Predicts *backward* branches as **taken** and *forward* branches as **not taken**. |
| **Nbit-16K-1**         | Dynamic   | 1-bit counter per entry, 16K entries. Predicts based on last branch outcome. |
| **Nbit-8K-2**          | Dynamic   | 2-bit saturating counters (4 states), 8K entries — requires two consecutive mispredictions to change prediction. |
| **Gshare-13b**         | Dynamic   | Uses **Global History Register (GHR)** XORed with PC bits to index into a Pattern History Table (PHT) of 2-bit counters. |
| **BTB-512-1-16**       | Hybrid    | **Branch Target Buffer** with LRU replacement + **Return Address Stack** for `call` / `ret` prediction. |
| **BTB-64-8-16**        | Hybrid    | Smaller BTB with 64 lines, 8-way associativity, and 16-entry RAS. |

---

##  Technical Details

###  Gshare
The **Gshare** predictor combines **global branch history** with the branch’s **program counter (PC)** to reduce Pattern History Table collisions.

**How it works:**
1. **Global History Register (GHR)** stores the outcomes of the last *N* branches (`1` = taken, `0` = not taken).
2. The lower *N* bits of the PC are **XORed** with the GHR to form the PHT index.
3. The PHT contains **2-bit saturating counters** for prediction:
   - `00` = Strongly Not Taken
   - `01` = Weakly Not Taken
   - `10` = Weakly Taken
   - `11` = Strongly Taken
4. After each branch, both the **GHR** and the **counter** at the indexed PHT entry are updated.

---

###  BTB + RAS
The **Branch Target Buffer (BTB)** predicts the **target address** of taken branches, while the **Return Address Stack (RAS)** improves prediction of `ret` instructions.

**Structure:**
- **BTB**: Set-associative cache storing `(PC → Target)` pairs.
- **RAS**: Stack storing return addresses for function calls.

**Operation:**
1. **Prediction phase**:
   - If branch is predicted **taken** and found in BTB → use stored target.
   - If instruction is `call` → push return address to RAS.
   - If instruction is `ret` → pop RAS and compare to actual target.
2. **Update phase**:
   - On incorrect target → increment `Incorrect_Targets`.
   - On incorrect RAS return → increment `Incorrect_RAS`.
   - Replace BTB entries using **LRU**.

---

##  Benchmarking

Benchmarks used from the PARSEC suite:

blackscholes, bodytrack, canneal, ferret, fluidanimate,
freqmine, rtview, streamcluster, swaptions

The execution commands for **simlarge** inputs are stored in [`cmds_simlarge.txt`](./scripts/cmds_simlarge.txt).

---

##  Results Visualization

Ι use custom Python scripts to parse `.bp.out` files and generate **MPKI plots** for each predictor and benchmark.

Example:
```bash
python3 plot_summary_mpki.py ./branchPredOutputs/

