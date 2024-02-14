#pragma once
#include "../Vec2.h"

struct ParticleSpawner {
    // Returns true if it is ready to spawn some particles
    bool Tick(float delta_time);

    // It will fill in the values at the given buffers, they must have
    // At least spawn_count entries available
    void Spawn(Float2* positions, Float2* velocities, size_t offset, float particle_size, float aspect_ratio);

    Float2 direction;
    Float2 spawn_point;
    float spawn_delta;
    unsigned int spawn_count;
    float initial_velocity;
    float last_spawn_delta = 0.0f;
};
