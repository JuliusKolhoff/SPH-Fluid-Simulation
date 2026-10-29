# 3D SPH Fluid Simulation 

A real-time, 3D **Smoothed Particle Hydrodynamics (SPH)** fluid simulation written in C++ with an OpenGL renderer. The simulation is a weakly compressible SPH(WCSPH) using Compute Shader and spatial hashing for improved performance.(70Fps @ 400k particles)

The SPH implementation follows the paper *"A Survey on SPH Methods in Computer Graphics"* (Koschier et al.). Used sections are referenced in the source comments.

<img width="1655" height="973" alt="image" src="https://github.com/user-attachments/assets/576fcdc4-23f5-4cdf-9323-5d8763438aca" />

<img width="1288" height="840" alt="image" src="https://github.com/user-attachments/assets/55490221-72a0-4fa6-a44c-2abcedcc829c" />



## Features

- **Weakly compressible SPH** 
- **Spatial hashing** (cell size = smoothing radius) for O(n) neighbour search
- **Adaptive time step** based on the CFL condition
- **Compute Shader** OpenGL/GLSL Compute Shader for fast calculations on the Gpu
- **Free 3D Movement** Fly around and look at  the Simulation from every angle

## How It Works

Each simulation step runs the following stages:

| Stage | Description |
|---|---|
| 0. Time step | `dt = 0.4 · h / v_max`, capped at `0.016 s` (CFL condition) |
| 1. Neighbour search | Particles are hashed to grid cells, sorted by key, and a start-index table is built |
| 2. Density | `ρᵢ = Σ mⱼ · W(rᵢⱼ, h)` using a 3D-normalised smoothing kernel |
| 3. Pressure | `pᵢ = k · ((ρᵢ / ρ₀)⁷ − 1)` |
| 4. Forces | Symmetric pressure gradient (spiky kernel derivative), viscosity, gravity, optional mouse force |
| 5. Integration | Semi-implicit Euler, then boundary collisions with static particles and hard border |
 
### Rendering

Own OpenGL rendering for maximum performance. Particles are rendered with one  draw-call as 2D points. Geometry shader transforms poins into quads and fragment shader discards pixels outside of specified radius to turn them into circles. 3D sphere look achieved by always having circles pointed to camera and emulation a 3d Normalmap for lighting calculations.

## Configuration

| Parameter | Default | Description |
|---|---|---|
| `simulation_size` | `400k` | Number of fluid particles |
| `particle_radius` | `0.16` | Visual radius and initial spacing (m) |
| `smoothing_radius` | `0.57` | SPH kernel support radius `h` (m) |
| `target_density` | `30.0` | Rest density `ρ₀` |
| `k` | `27k` | Pressure stiffness |
| `viscosity` | `7` | Viscosity coefficient |
| `gravity` | `9.81` | Gravitational acceleration (m/s²) |


## Further Improvments

- More realistic Models(IISPH,DFSPH)

## References

- Koschier, D., Bender, J., Solenthaler, B., Teschner, M. *A Survey on SPH Methods in Computer Graphics.* Computer Graphics Forum, 2022.
- Sebastian Lague's fluid simulation series.
