# Class F / Class $\text{F}^{-1}$ Multi-Harmonic Matching Network Synthesis Tool

[![DOI](https://zenodo.org/badge/1394778314.svg)](https://doi.org/10.5281/zenodo.23456609)

---

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
S. Santos et al., **"A 3.7 GHz Class-F Power Amplifier Designed using an Algorithmic Harmonic Matching"**, to submit.

---
---

# Complete Analytical Synthesis Methodology

The synthesis engine determines the **exact electrical lengths** of main transmission lines and open stubs required to match a target power amplifier optimum load impedance $Z_{\text{opt}}$ to a standard termination $Z_L = 50 \Omega$ across $N$ frequencies (fundamental and $N-1$ harmonics).

---

## Network Topology & Strategy

The matching network consists of $N$ cascaded stages. Each stage $k \in \{1, 2, \dots, N\}$ comprises:
- A main transmission line of electrical length $\theta_{\text{line}, k}$
- A parallel OC stub of electrical length $\theta_{\text{stub}, k}$

All elements share a common characteristic impedance $Z_0$.

---

> **Reverse-Sequential Decoupling Procedure:**  
> Higher-harmonic terminations (e.g., $N$-th harmonic) are synthesized **first**, progressing down to the fundamental frequency $f_1$. Once a stage synthesizes the impedance for harmonic $k$, subsequent stages maintain the reflection coefficient magnitude at $k f_1$, decoupling it from further transformations.

---

## 1. Transmission Line Length Synthesis

Consider a targeted node impedance $Z = R + jX$ transformed by a main line of electrical length $\theta_{\text{line}} = \beta l$. The voltage reflection coefficient $\rho_{\text{in}}$ at the line input is defined as:

$$\rho_{\text{in}} = \frac{Z - Z_0}{Z + Z_0} = \vert{}\rho_{\text{in}}\vert{} e^{j \theta_{\text{in}}}$$

As the signal propagates along the transmission line toward the load, the reflection coefficient undergoes a phase rotation:

$$\rho(l) = \rho_{\text{in}} e^{-j 2 \beta l}$$

### Closed-Form Solution (`lineCalc()`)
To match the real part of the admittance looking into the subsequent network node, the line electrical length $\theta_{\text{line}}$ is analytically determined as:

$$\theta_{\text{line}} = \frac{1}{2} \left( \theta_{\text{in}} \pm \cos^{-1}\left( K \right) \right) + n\pi, \quad n \in \mathbb{Z}$$

Where $n$ ensures physical realizability ($\theta_{\text{line}} \ge 0$), and $K$ is the normalized mapping factor:

$$K = \frac{1}{2 \vert{}\rho_{\text{in}}\vert{}} \left[ \left(\frac{Y_0}{G} - 1\right) - \vert{}\rho_{\text{in}}\vert{}^2 \left(\frac{Y_0}{G} + 1\right) \right]$$

In the matched termination condition ($G = Y_0 = 1/Z_0$), this factor simplifies directly to:

$$K = -\vert{}\rho_{\text{in}}\vert{}$$

> **Dual-Branch Selection:**  
> The C function `lineCalc()` implements dual-branch root selection ($\pm \cos^{-1}$) for the fundamental matching stage, allowing the designer to choose between two valid physical configurations to optimize PCB layout footprint and avoid overlapping lines.

---

## 2. Parallel Open-Circuited Stub Synthesis

After transforming $Z$ through the main line length $\theta_{\text{line}}$, the resulting node impedance $Z_{\text{node}}$ is given by:

$$Z_{\text{node}} = Z_0 \frac{Z + j Z_0 \tan(\theta_{\text{line}})}{Z_0 + j Z \tan(\theta_{\text{line}})}$$

The corresponding node admittance is $Y_{\text{node}} = 1 / Z_{\text{node}} = G_{\text{node}} + j B_{\text{node}}$.

### Closed-Form Solution (`stubCalc()`)
To achieve the target susceptance $B_{\text{target}} = \text{Im}\\{ 1/Z_L \\}$, the required stub susceptance is $B_{\text{stub}} = B_{\text{target}} - B_{\text{node}}$. For an open-circuited stub, the electrical length $\theta_{\text{stub}}$ is computed analytically as:

$$\theta_{\text{stub}} = \tan^{-1}\left( \frac{B_{\text{stub}}}{Y_0} \right) + m\pi, \quad m \in \mathbb{Z}$$

Where $m$ is selected such that $\theta_{\text{stub}} \in [0, \pi)$.

---

## 3. Sequential Network Synthesis Algorithm

The main synthesis engine, executed in `calcCompleteNetwork()`, tracks the evolution of all $N+1$ node impedances across all $N$ frequencies using a dynamic complex matrix $\mathbf{Z}_{\text{nodes}} \in \mathbb{C}^{N \times (N+1)}$.

### Synthesis Steps

1. **Initialization (`initCalc`):** The conjugate optimum device impedances $Z_i^{(0)} = Z_{\text{opt}, i}^*$ for frequencies $f_i$ ($i=1, \dots, N$) populate the first column of $\mathbf{Z}_{\text{nodes}}$. Ideal short/open boundary limits are bounded to prevent division-by-zero singularities.
2. **Frequency Scaling:** Electrical lengths scale linearly with frequency relative to $f_1$:

$$f_{\text{ratio}, i} = \frac{f_i}{f_1}$$

3. **Forward Iterative Loop:** For each stage $k$ associated with target harmonic order $o_k \in \{N, N-1, \dots, 1\}$:
   - Compute electrical length pair $\theta_k = [\theta_{\text{line}, k}, \theta_{\text{stub}, k}]$ at $f_{\text{ratio}, o_k}$ via `electricalLengthCalc()`.
   - Store synthesized lengths in $\mathbf{L}_{\text{matrix}} \in \mathbb{R}^{N \times 2}$.
   - Transform impedance for **all** $N$ frequencies in parallel:

$$Z_i^{(k)} = \left( Y_{\text{line}, i}^{(k)} + Y_{\text{stub}, i}^{(k)} \right)^{-1}$$

Where:

$$Y_{\text{line}, i}^{(k)} = \left[ Z_0 \frac{Z_i^{(k-1)} + j Z_0 \tan(\theta_{\text{line}, k} \cdot f_{\text{ratio}, i})}{Z_0 + j Z_i^{(k-1)} \tan(\theta_{\text{line}, k} \cdot f_{\text{ratio}, i})} \right]^{-1}$$

$$Y_{\text{stub}, i}^{(k)} = \frac{j}{Z_0} \tan\left(\theta_{\text{stub}, k} \cdot f_{\text{ratio}, i}\right)$$

---

## 4. Reverse Verification Algorithm

To mathematically validate the synthesized network and evaluate the exact physical impedance presented to active device terminals, the software performs a full reverse propagation in `calcCompleteNetworkInv()`.

Starting from $Z_L = 50 \Omega$ at stage $N$, the algorithm propagates backward toward the transistor terminals ($k = N$ down to $1$):

$$Z_{\text{in}, i}^{(k-1)} = Z_0 \frac{\left( Z_{\text{in}, i}^{(k)} \parallel Z_{\text{stub}, i}^{(k)} \right) + j Z_0 \tan\left( \theta_{\text{line},k,i} \right)}{Z_0 + j \left( Z_{\text{in}, i}^{(k)} \parallel Z_{\text{stub}, i}^{(k)} \right) \tan\left( \theta_{\text{line},k,i} \right)}$$

Where:
$$Z_{\text{stub}, i}^{(k)} = -j Z_0 \cot\left( \theta_{\text{stub}, k} \cdot f_{\text{ratio}, i} \right)$$

> ✅ **Zero Algorithmic Error:**  
> The reconstructed input impedances $Z_{\text{in}, i}^{(0)}$ strictly match $Z_{\text{opt}, i}$ across all $N$ harmonics, confirming zero algorithmic synthesis error before microstrip translation and CAD export.
