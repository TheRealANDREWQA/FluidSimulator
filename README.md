# FluidSimulator
 Particle based 2D Fluid (Liquid) Simulator

 

https://github.com/TheRealANDREWQA/FluidSimulator/assets/68424250/5e36fb55-2aba-4512-b50d-957755b8e90d


https://github.com/TheRealANDREWQA/FluidSimulator/assets/68424250/f3497bff-9108-498a-9ad4-c346ae1778d5


# Summary
This project is based on the video made by Sebastian Lague, https://www.youtube.com/watch?v=rSKMYc1CQHE&t=1s. The simulation core is using his ideas of simulation a liquid based on particles. I have added 2 major new features: collisions with arbitrary shapes (these are painted on the screen) and displaying an image from a particle spawner. The implementation is using OpenGL for rendering and compute, as the simulation runs entirely on the GPU. Version 4.3 was used for OpenGL in order to access the compute capabilities of the GPU. The simulation has multiple parameters that can be controlled through the UI, including behaviour parameters (like gravity, pressure factor), mouse input interaction, painting collisions.

# External Dependencies (embedded in the project)
- GLFW
- ImGui (UI)
- stb_image (image loading)

# Arbitrary collisions
Collisions can be analytical (like the box in the preview), or using some other technique. Collisions with arbitrary shapes is trickier since there are no (direct) formulas to use to determine whether a particle is colliding or not. My approach is to use a 2D texture that has a bit set for each pixel to indicate a collision shape. To detect that a particle is colliding this frame with the shape, a crude approximation of CCD, continuous collision detection, is to sample
the location at fixed intervals and determine the first position that corresponds to a collision, and push back the particle a little. This works for high enough of a framerate and for reasonable velocities, which is the case here.

# Image formation
This feature is based on the deterministic nature of the simulation (the caveat is that the viscosity needs to be 0). A simulation is pre-run and its parameters are written to a file. When the simulation fills the screen, for each particle a set of UV coordinates is assigned. When the image display is activated, it will read those parameters and drive the simulation with it. The UV parameters are used to look into the texture, and since the simulation is deterministic, the particle will have the correct color when the simulation stops.

<p float="left">
  <img src="https://github.com/TheRealANDREWQA/FluidSimulator/assets/68424250/667ba3e2-86a8-4962-80d6-fc83b15c0a33" width="300" />
  <img src="https://github.com/TheRealANDREWQA/FluidSimulator/assets/68424250/25075dbd-383f-4f0d-8c0f-b3a65fd945f3" width="300" />
  <img src="https://github.com/TheRealANDREWQA/FluidSimulator/assets/68424250/1c0f70f1-2fdc-48a4-a531-7f16fbb311b2)" width="300" />
</p>
