# 3D SPH Fluid Simulation 

A real-time, 3D **Smoothed Particle Hydrodynamics (SPH)** fluid simulation written in C++ with an OpenGL renderer. The simulation is a weakly compressible SPH(WCSPH) using Compute Shader and spatial hashing for improved performance.(70Fps @ 400k particles)

The SPH implementation follows the paper *"A Survey on SPH Methods in Computer Graphics"* (Koschier et al.). Used sections are referenced in the source comments.

<img width="1655" height="973" alt="image" src="https://github.com/user-attachments/assets/576fcdc4-23f5-4cdf-9323-5d8763438aca" />

<img width="1423" height="843" alt="image" src="https://github.com/user-attachments/assets/e8a4c971-5e18-4327-a935-081bbe80bb53" />




## Features

- **Weakly compressible SPH** 
- **Spatial hashing**  for O(n) neighbour search
- **Adaptive time step** based on the CFL condition
- **Compute Shader** OpenGL/GLSL Compute Shader for fast calculations on the Gpu
- **Free 3D Movement** Fly around and look at  the Simulation from every angle

## How It Works

Each frame is advanced in up to `max_substeps` simulation steps. All stages run as OpenGL compute shaders (work-group size 256) on the GPU.

| Stage | Description |
|---|---|
| 0. Time step | A `reduce_max` pass finds the maximum particle speed `v_max` and acceleration `a_max` on the GPU. The step is `dt = λ · h / (c + v_max)` with `λ = 0.6` and sound speed `c = √(7k / ρ0)`. The frame time is clamped to `0.05 s` and split into at most `max_substeps` substeps. |
| 1. Neighbour search | Uniform grid with cell size `h`, built by counting sort : `clear_grid` → `count_cells`  → `scan` → `scatter` . A neighbour query scans the 3×3 neighbouring rows of cells, each as one contiguous index range. |
| 2. Density | `ρi = Σ mj · W_poly6(rij, h)` (3D-normalised, self-contribution included). |
| 3. Pressure | Tait equation of state, clamped to non-negative values: `pi = max(0, k · ((ρi / ρ0)<sup>7</sup> − 1))` |
| 4. Forces | Symmetric pressure gradient `−Σ mj · (pi/ρi² + pj/ρj²) · ∇W_spiky`, an XSPH-style viscosity term `10 · μ/ρi · (vij · xij) / (ρj · (r² + 0.01 h²))` with the same spiky gradient |
| 5. Integration | Semi-implicit Euler: `v += a · dt`, then `x += v · dt`. |
 
### Rendering
Custom OpenGL 4.5 renderer. All particles are drawn in **one draw call** as camera-facing circles emulation real 3D spheres. No Data transfer between Cpu and Gpu because the SSBO´s are reused as Vertex buffers.

## Configuration

| Parameter | Default | Description |
|---|---|---|
| `n` | `400k` | Number of fluid particles |
| `particle_radius` | `0.16` | Visual radius |
| `smoothing_radius` | `0.736` | SPH kernel support radius `h`. |
| `target_density` | `20.76` | Rest density `ρ₀` |
| `k` | `24700` | Pressure stiffness |
| `viscosity` | `6.0` | Viscosity coefficient |
| `gravity` | `9.81` | Gravitational acceleration (m/s²) |
| `cfl_lamda` | `0.6` | CFL safety factor for the adaptive time step |
| `max_substeps` | `2` | Maximum simulation steps per frame |


## Further Improvments

- More realistic Models(IISPH,DFSPH)

## References

- Koschier, D., Bender, J., Solenthaler, B., Teschner, M. *A Survey on SPH Methods in Computer Graphics.* Computer Graphics Forum, 2022.
- Sebastian Lague's fluid simulation series.
