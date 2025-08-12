# branch-predictor-simulator

##  Overview
This repository contains the implementation and evaluation of various **branch predictors** using the [Intel PIN Tool](https://software.intel.com/content/www/us/en/develop/articles/pin-a-dynamic-binary-instrumentation-tool.html) and the [PARSEC Benchmark Suite](https://github.com/bamos/parsec-benchmark).

The goal of this project is to **implement, run, and compare** different branch prediction strategies by measuring their accuracy in terms of **MPKI** (Mispredictions Per 1000 Instructions) across multiple benchmarks.

**Implemented predictors:**
- **Static Not Taken**
- **Static BTFNT**
- **Dynamic 1-bit (16K entries)**
- **Dynamic 2-bit (8K entries)**
- **Gshare (13-bit global history)**
- **BTB + RAS** (Branch Target Buffer with Return Address Stack)
- **BTB-64-8-16** (BTB with alternative configuration)

---

## ⚙️ How It Works
### Intel PIN Tool
The **Intel PIN Tool** is a dynamic binary instrumentation framework that allows us to insert custom analysis code into programs at runtime. In this project, a custom pintool was developed to:
- Intercept branch instructions
- Predict their outcome based on the selected predictor
- Update predictor state after the branch is resolved
- Collect statistics on prediction accuracy

### PARSEC Benchmark Suite
PARSEC is a collection of realistic multithreaded applications used for architectural and systems research.  
Each benchmark was run with the **simlarge** input set to provide enough instruction counts for meaningful MPKI measurements.

---

## 📂 Repository Structure
---

## 📦 Requirements
- **Intel PIN Tool** (tested with version 3.31)
- **PARSEC Benchmark Suite** (tested with PARSEC 3.0)
- **Python 3.8+**
