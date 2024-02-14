#include "ParticleSpawner.h"

bool ParticleSpawner::Tick(float delta_time)
{
    last_spawn_delta += delta_time;
    if (last_spawn_delta > spawn_delta) {
        last_spawn_delta = 0.0f;
        return true;
    }
    return false;
}

void ParticleSpawner::Spawn(Float2* positions, Float2* velocities, size_t offset, float particle_size, float aspect_ratio)
{
    const float SEPARATION_FACTOR = 1.5f;

    // Take a perpendicular direction over the direction
    // And spawn the particles alongside it
    Float2 offset_direction = { direction.y, -direction.x };
    float total_size = particle_size * spawn_count * SEPARATION_FACTOR;
    float half_size = total_size * 0.5f;
    Float2 initial_spawn_point = spawn_point * Float2(aspect_ratio, 1.0f) + offset_direction * half_size;
    offset_direction = { -offset_direction.x, -offset_direction.y };

    Float2 spawn_location = initial_spawn_point;
    Float2 spawn_velocity = direction * initial_velocity;
    for (unsigned int index = 0; index < spawn_count; index++) {
        positions[offset + index] = spawn_location;
        velocities[offset + index] = spawn_velocity;

        spawn_location += offset_direction * particle_size * SEPARATION_FACTOR;
    }
}
