# Class F / Class $\text{F}^{-1}$ Multi-Harmonic Matching Network Synthesis Tool

A fast, exact, and deterministic C-based tool for synthesizing multi-harmonic matching networks (ideal lines) for Class F and Class $\text{F}^{-1}$ power amplifiers (PAs) without relying on numerical CAD optimization.

---

## Key Features

- **Exact Closed-Form Synthesis:** Computes electrical lengths of cascaded line-stub pairs using a sequential harmonic-decoupling algorithm.
- **Arbitrary Harmonic Order ($N$):** Synthesizes matching structures for fundamental and $N-1$ higher-order harmonics.
- **Zero Algorithmic Error:** Includes an integrated reverse-verification routine to mathematically confirm network impedance matching prior to layout conversion.
- **Optimization-Free:** Eliminates convergence issues, initial-guess dependency, and local minima common in numerical optimization techniques.
- **Fast Execution:** C-based engine delivers instantaneous results suitable for automated design sweeps.

---

## How It Works

The tool processes target load/source impedances derived from Harmonic Balance / Load-Pull simulations and computes the line-stub parameters:

1. **Input:** Class selection (Class F or Class $\text{F}^{-1}$), number of target harmonics ($N$), characteristic impedance ($Z_0$), and fundamental matching configuration using the dual solution ($\pm \cos^{-1}$ branch).
2. **Synthesis Engine:** Calculates electrical lengths ($\theta_{\text{line}, k}, \theta_{\text{stub}, k}$) through a deterministic, sequential harmonic-decoupling algorithm.
3. **Verification:** Reverse propagates the standard load termination ($Z_L = 50\,\Omega$) back through the synthesized pairs toward the transistor terminals to mathematically evaluate the resulting node impedances and quantify synthesis accuracy.
4. **Output:** Exact electrical dimensions ($\theta_{\text{line}}, \theta_{\text{stub}}$) for ideal transmission lines, ready for microstrip physical translation.

> **Note on Higher Harmonics ($N \ge 4$):** Because sequential decoupling assumes independent control of each harmonic at its respective stage, phase shifts introduced by previously cascaded line-stub pairs can constrain the achievable impedance transformation at higher orders. For large harmonic counts, the reverse-verification routine may reveal small residual mismatches at specific higher-order frequencies due to these cumulative phase interactions.

---

## Getting Started

### Prerequisites
A standard C compiler (e.g., `gcc`, `clang`, or MSVC).

### Compilation
Clone the repository and compile using `gcc`:

```bash
git clone https://github.com/SamuelAS03/classf-synthesis.git
cd classf-synthesis
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/classf_synthesis
```

---

## Institutional Context & Acknowledgments

This software was developed by Samuel Santos as part of his Master's Thesis in Electronics and Telecommunications Engineering at the Department of Electronics, Telecommunications and Informatics (DETI), University of Aveiro, Portugal.

If you use this tool or methodology in your academic work, please cite the associated paper:
S. Santos et al., **"An Analytical Harmonic-Matching Synthesis Tool for $\text{Class-F/F}^{-1}$ PAs: Design and Experimental Validation"**, to submit.
