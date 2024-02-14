#pragma once
#include "ComputeShader.h"
#include "Buffers.h"
#include "Shader.h"
#include "VertexBuffer.h"
#include "Texture.h"
#include "GPUSort.h"
#include "GeneralSettings.h"
#include "ParticleSpawner.h"

#define POSITION_FACTOR 500.0f

class Simulation {
public:
    // This function doesn't retain the contents of the existing data
    void ChangeParticleCount(size_t particle_count);

    // This function will preserve the data. It will trim in case the particle count
    // Is smaller. If you add particles, you can specify their initial positions and/or velocities
    void ChangeParticleCountPreserve(size_t particle_count, const Float2* add_positions = nullptr, const Float2* add_velocities = nullptr);

    void DoFrame(Float2 normalized_mouse_pos, bool is_left_mouse_pressed, bool is_right_mouse_pressed, float delta_time);

    inline GeneralSettings* GetGeneralSettings() {
        return (GeneralSettings*)simulation_early_compute.GetUniformBlockData("Settings");
    }

    inline float* GetMouseClickStrength() {
        return &mouse_click_strength;
    }

    inline std::vector<HeatmapEntry>& GetHeatmapEntries() {
        return heatmap_entries;
    }

    inline bool* GetPaintCollisionPtr() {
        return &paint_collision;
    }

    inline bool* GetUseMousePullPtr() {
        return &use_mouse_pull;
    }

    inline Int2* GetPaintCollisionSizePtr() {
        return &paint_collision_size;
    }

    void Initialize();

    inline void InvertPauseStatus() {
        pause_simulation = !pause_simulation;
    }

    // If the set value is true, it will add collisions to those cells, else it will clear them
    void PaintCollision(Int2 center, Int2 rectangle_size, bool set_value);

    void RecalculateHeatmap();

    void Render();

    void RenderParticles();

    void RenderCollisionObjects();

    void ReuploadCollisionData();

    inline void Reset() {
        ChangeParticleCount(particle_count);
        SetInitialBufferData(particle_count);
    }

    void SetImageDisplayMode(const char* texture_path);

    void SetRecordMode();

    void SetWindowSize(size_t width, size_t height);

    void SetInitialSettingsData();

    void SetFrameParameters(Float2 normalized_mouse_pos, bool is_left_mouse_pressed, bool is_right_mouse_pressed, float delta_time);

    // If the set value is true, it will add collision to the cell, else it will clear it
    void SetCollisionPixel(Int2 position, bool is_set);

private:
    void FrameCompute();

    void HandleRecordSimulation(float delta_time);

    void SetInitialBufferData(size_t particle_count);

    std::vector<HeatmapEntry> heatmap_entries;

    Shader render_shader;
    Shader image_render_shader;
    // This contains the sprite square vertices
    VertexBuffer render_vertex_buffer;
    Shader collision_render_shader;
    // This is a whole quad in NDC
    VertexBuffer collision_render_vertex_buffer;
    Texture2D circle_alpha_texture;
    Texture1D heatmap_texture;
    Texture2D collision_map;
    unsigned char* collision_map_data;
    Texture2D image_mode_texture;

    ComputeShader simulation_early_compute;
    ComputeShader calculate_density_compute;
    ComputeShader calculate_pressure_compute;
    ComputeShader calculate_viscosity_update_pos_compute;

    StructuredBuffer position_buffer;
    StructuredBuffer predicted_position_buffer;
    StructuredBuffer velocity_buffer;
    StructuredBuffer density_buffer;
    StructuredBuffer spatial_indices;
    StructuredBuffer spatial_offsets;
    StructuredBuffer image_mode_uvs;

    GPUSort gpu_sort;

    size_t particle_count;
    size_t max_particle_count;
    size_t window_width;
    size_t window_height;
    float aspect_ratio;
    float aspect_ratio_change;
    float mouse_click_strength;
    bool paint_collision;
    bool use_mouse_pull;
    bool pause_simulation;
    bool record_simulation;
    bool image_mode;
    Int2 paint_collision_size;

    ParticleSpawner particle_spawner;

    // Data used by the record feature
    struct {
        FILE* record_file;
    };

    std::vector<float> image_mode_delta_time;
    size_t image_mode_delta_time_index;
};
