# SPH Fluid Simulation

A real-time, 2D/(3D Work in Progress) **Smoothed Particle Hydrodynamics (SPH)** fluid simulation written in C++ with an OpenGL renderer. The simulation is a weakly compressible SPH(WCSPH) using OpenMp parallelization and spatial hashing for improved performance.

The SPH implementation follows the paper *"A Survey on SPH Methods in Computer Graphics"* (Koschier et al.). Used sections are referenced in the source comments.

<img width="910" height="730" alt="image" src="https://github.com/user-attachments/assets/8571f1c2-020e-45e9-bd55-b9e462347cf2" />

## Features

- **Weakly compressible SPH** 
- **Spatial hashing** (cell size = smoothing radius) for O(n) neighbour search
- **Adaptive time step** based on the CFL condition
- **OpenMP-parallelised** density, pressure and force passes
- **Interactive mouse forces**: repel particles with the right mouse button
- **Optional "soft glow" rendering** mode (additive blending)

## How It Works

Each simulation step runs the following stages:

| Stage | Description |
|---|---|
| 0. Time step | `dt = 0.4 · h / v_max`, capped at `0.016 s` (CFL condition) |
| 1. Neighbour search | Particles are hashed to grid cells, sorted by key, and a start-index table is built |
| 2. Density | `ρᵢ = Σ mⱼ · W(rᵢⱼ, h)` using a 2D-normalised smoothing kernel |
| 3. Pressure | `pᵢ = k · ((ρᵢ / ρ₀)⁷ − 1)` |
| 4. Forces | Symmetric pressure gradient (spiky kernel derivative), viscosity, gravity, optional mouse force |
| 5. Integration | Semi-implicit Euler, then boundary collisions with damping (`0.95`) |
 
### Rendering

Own OpenGL rendering for maximum performance. Particles are rendered with one  draw-call as 2D points. Geometry shader transforms poins into quads and fragment shader discards pixels outside of specified radius to turn them into circles. Optinal blurring of particles to achieve more fluid like look.

## Configuration

| Parameter | Default | Description |
|---|---|---|
| `simulation_size` | `1200` | Number of fluid particles |
| `obstacle_particle_count` | `250` | Buffer size reserved for static obstacle particles  |
| `particle_radius` | `0.16` | Visual radius and initial spacing (m) |
| `smoothing_radius` | `0.5` | SPH kernel support radius `h` (m) |
| `target_density` | `6.0` | Rest density `ρ₀` |
| `k` | `10` | Pressure stiffness |
| `viscosity` | `1.0` | Viscosity coefficient |
| `gravity` | `6` | Gravitational acceleration (m/s²) |
| `render_complex` | `false` | Enables soft-glow rendering with additive blending |
| `radius_increase` | `4.0` | Sprite size multiplier for glow rendering |


## Further Improvments

- Currently only using Cpu for calculation -> More particles would require Gpu support(Compute Shader)
- Possible 3D adaptation
- More realistic Models(IISPH,DFSPH)

## References

- Koschier, D., Bender, J., Solenthaler, B., Teschner, M. *A Survey on SPH Methods in Computer Graphics.* Computer Graphics Forum, 2022.
- Sebastian Lague's fluid simulation series (inspiration for the spatial hashing and mouse-interaction approach).
