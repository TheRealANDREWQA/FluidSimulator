#include "Simulation.h"
#include "glad.h"
#include <iostream>
#include "../Vec2.h"
#include "ShaderLocation.h"
#include "GeneralSettings.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <GLFW\glfw3.h>
#include "std_image.h"
#include <intrin.h>

extern "C" {
    _declspec(dllexport) unsigned int NvOptimusEnablement = 1;
    _declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

#define PARTICLE_SIZE 0.008f
#define SIMULATION_FILE ".sim"

static void DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
    std::cout << "Source: ";
    switch (source) {
    case GL_DEBUG_SOURCE_API:
        std::cout << "API | ";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        std::cout << "APPLICATION | ";
        break;
    case GL_DEBUG_SOURCE_OTHER:
        std::cout << "OTHER | ";
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        std::cout << "SHADER COMPILER | ";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        std::cout << "THIRD PARTY | ";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        std::cout << "WINDOW SYSTEM | ";
        break;
    }

    std::cout << "Type: ";
    switch (type) {
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        std::cout << "DEPRECATED | ";
        break;
    case GL_DEBUG_TYPE_ERROR:
        std::cout << "ERROR | ";
        break;
    case GL_DEBUG_TYPE_MARKER:
        std::cout << "TYPE MARKER | ";
        break;
    case GL_DEBUG_TYPE_OTHER:
        std::cout << "OTHER | ";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        std::cout << "PERFORMANCE | ";
        break;
    case GL_DEBUG_TYPE_POP_GROUP:
        std::cout << "POP GROUP | ";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        std::cout << "PORTABILITY | ";
        break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        std::cout << "PUSH GROUP | ";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        std::cout << "UNDEFINED BEHAVIOUR | ";
        break;
    }

    std::cout << " Severity: ";
    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
        std::cout << "HIGH | ";
        break;
    case GL_DEBUG_SEVERITY_LOW:
        std::cout << "LOW | ";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        std::cout << "MEDIUM | ";
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        std::cout << "NOTIFICATION | ";
        break;
    }

    std::cout << "Message: " << message << "\n";
}

void Simulation::ChangeParticleCount(size_t _particle_count)
{
    particle_count = _particle_count;
    position_buffer.SetNewDataSize(sizeof(Float2), particle_count);
    predicted_position_buffer.SetNewDataSize(sizeof(Float2), particle_count);
    velocity_buffer.SetNewDataSize(sizeof(Float2), particle_count);
    density_buffer.SetNewDataSize(sizeof(Float2), particle_count);
    spatial_indices.SetNewDataSize(sizeof(unsigned int) * 3, particle_count);
    spatial_offsets.SetNewDataSize(sizeof(unsigned int), particle_count);
}

void Simulation::ChangeParticleCountPreserve(size_t new_particle_count, const Float2* add_positions, const Float2* add_velocities)
{
    // Retrieve the data for all buffers
    std::vector<Float2> position_data(new_particle_count);
    std::vector<Float2> predicted_position_data(new_particle_count);
    std::vector<Float2> velocity_data(new_particle_count);
    std::vector<Float2> density_data(new_particle_count);
    struct SpatialIndex {
        unsigned int values[3];
    };
    std::vector<SpatialIndex> spatial_indices_data(new_particle_count);
    std::vector<unsigned int> spatial_offsets_data(new_particle_count);

    position_buffer.RetrieveData(sizeof(Float2), particle_count, position_data.data());
    predicted_position_buffer.RetrieveData(sizeof(Float2), particle_count, predicted_position_data.data());
    velocity_buffer.RetrieveData(sizeof(Float2), particle_count, velocity_data.data());
    density_buffer.RetrieveData(sizeof(Float2), particle_count, density_data.data());
    spatial_indices.RetrieveData(sizeof(SpatialIndex), particle_count, spatial_indices_data.data());
    spatial_offsets.RetrieveData(sizeof(unsigned int), particle_count, spatial_offsets_data.data());

    if (new_particle_count > particle_count) {
        size_t difference = new_particle_count - particle_count;
        memset(density_data.data() + particle_count, 0, sizeof(Float2) * difference);
        memset(spatial_indices_data.data() + particle_count, 0, sizeof(SpatialIndex) * difference);
        memset(spatial_offsets_data.data() + particle_count, 0, sizeof(unsigned int) * difference);
        if (add_positions != nullptr) {
            memcpy(position_data.data() + particle_count, add_positions, sizeof(Float2) * difference);
            memcpy(predicted_position_data.data() + particle_count, add_positions, sizeof(Float2) * difference);
        }
        else {
            memset(position_data.data() + particle_count, 0, sizeof(Float2) * difference);
            memset(predicted_position_data.data() + particle_count, 0, sizeof(Float2) * difference);
        }

        if (add_velocities != nullptr) {
            memcpy(velocity_data.data() + particle_count, add_velocities, sizeof(Float2) * difference);
        }
        else {
            memset(velocity_data.data() + particle_count, 0, sizeof(Float2) * difference);
        }
    }

    particle_count = new_particle_count;
    position_buffer.SetNewData(sizeof(Float2), new_particle_count, position_data.data());
    predicted_position_buffer.SetNewData(sizeof(Float2), new_particle_count, predicted_position_data.data());
    velocity_buffer.SetNewData(sizeof(Float2), new_particle_count, velocity_data.data());
    density_buffer.SetNewData(sizeof(Float2), new_particle_count, density_data.data());
    spatial_indices.SetNewData(sizeof(unsigned int) * 3, new_particle_count, spatial_indices_data.data());
    spatial_offsets.SetNewData(sizeof(unsigned int), new_particle_count, spatial_offsets_data.data());
}

void Simulation::DoFrame(Float2 normalized_mouse_pos, bool is_left_mouse_pressed, bool is_right_mouse_pressed, float delta_time)
{
    if (!pause_simulation) {
        delta_time = std::min(0.007f, delta_time);
        if (image_mode) {
            if (image_mode_delta_time_index < image_mode_delta_time.size()) {
                delta_time = image_mode_delta_time[image_mode_delta_time_index];
                image_mode_delta_time_index++;
            }
            else {
                /*std::vector<Float2> values(particle_count);
                FILE* pos_file = fopen(".pos", "rb");
                fread(values.data(), sizeof(Float2), values.size(), pos_file);
                fclose(pos_file);

                std::vector<Float2> runtime_values(particle_count);
                position_buffer.RetrieveData(sizeof(Float2), particle_count, runtime_values.data());

                bool are_the_same = memcmp(values.data(), runtime_values.data(), sizeof(Float2) * runtime_values.size()) == 0;
                __debugbreak();*/
            }
        }

        if (particle_spawner.Tick(delta_time)) {
            if (particle_count < max_particle_count) {
                Float2 spawn_positions[1000];
                Float2 spawn_velocities[1000];
                if (particle_spawner.spawn_count > std::size(spawn_positions)) {
                    abort();
                }
                particle_spawner.Spawn(spawn_positions, spawn_velocities, 0, PARTICLE_SIZE * POSITION_FACTOR, aspect_ratio);
                ChangeParticleCountPreserve(particle_count + particle_spawner.spawn_count, spawn_positions, spawn_velocities);
            }
        }

        SetFrameParameters(normalized_mouse_pos, is_left_mouse_pressed, is_right_mouse_pressed, delta_time);
        FrameCompute();

        if (image_mode) {
            //if (image_mode_delta_time_index < image_mode_delta_time.size()) {
            //    std::string pos_file_path = ".pos";
            //    pos_file_path = pos_file_path + std::to_string(image_mode_delta_time_index);

            //    std::string index_file_path = ".sp";
            //    index_file_path = index_file_path + std::to_string(image_mode_delta_time_index);

            //    std::string predicted_file_path = ".predictedpos";
            //    predicted_file_path = predicted_file_path + std::to_string(image_mode_delta_time_index);

            //    std::string velocity_file_path = ".velocity";
            //    velocity_file_path = velocity_file_path + std::to_string(image_mode_delta_time_index);

            //    std::string density_file_path = ".density";
            //    density_file_path = density_file_path + std::to_string(image_mode_delta_time_index);

            //    std::string offset_file_path = ".offset";
            //    offset_file_path = offset_file_path + std::to_string(image_mode_delta_time_index);

            //    //if (image_mode_delta_time_index == 1600) {
            //    std::vector<Float2> values(particle_count);
            //    FILE* pos_file = fopen(pos_file_path.c_str(), "rb");
            //    fread(values.data(), sizeof(Float2), values.size(), pos_file);
            //    fclose(pos_file);

            //    std::vector<Float2> runtime_values(particle_count);
            //    position_buffer.RetrieveData(sizeof(Float2), particle_count, runtime_values.data());

            //    bool are_the_same = memcmp(values.data(), runtime_values.data(), sizeof(Float2) * runtime_values.size()) == 0;
            //    size_t index = 0;
            //    for (; index < runtime_values.size(); index++) {
            //        if (runtime_values[index].x != values[index].x) {
            //            break;
            //        }
            //        if (runtime_values[index].y != values[index].y) {
            //            break;
            //        }
            //    }

            //    struct SpatialIndex {
            //        unsigned int values[3];
            //    };
            //    std::vector<SpatialIndex> runtime_indices(particle_count);
            //    spatial_indices.RetrieveData(sizeof(SpatialIndex), particle_count, runtime_indices.data());

            //    std::vector<SpatialIndex> file_indices(particle_count);
            //    FILE* spatial_file = fopen(index_file_path.c_str(), "rb");
            //    fread(file_indices.data(), sizeof(SpatialIndex), file_indices.size(), spatial_file);
            //    fclose(spatial_file);

            //    bool are_the_same_indices = memcmp(file_indices.data(), runtime_indices.data(), sizeof(SpatialIndex) * runtime_indices.size()) == 0;
            //    size_t subindex = 0;
            //    for (; subindex < runtime_indices.size(); subindex++) {
            //        if (runtime_indices[subindex].values[0] != file_indices[subindex].values[0]) {
            //            break;
            //        }
            //        if (runtime_indices[subindex].values[1] != file_indices[subindex].values[1]) {
            //            break;
            //        }
            //        if (runtime_indices[subindex].values[2] != file_indices[subindex].values[2]) {
            //            break;
            //        }
            //    }

            //    std::vector<Float2> predicted_values(particle_count);
            //    FILE* predicted_pos_file = fopen(predicted_file_path.c_str(), "rb");
            //    fread(predicted_values.data(), sizeof(Float2), predicted_values.size(), predicted_pos_file);
            //    fclose(predicted_pos_file);

            //    std::vector<Float2> runtime_predicted_values(particle_count);
            //    predicted_position_buffer.RetrieveData(sizeof(Float2), particle_count, runtime_predicted_values.data());

            //    bool are_the_same_predicted = memcmp(predicted_values.data(), runtime_predicted_values.data(), sizeof(Float2) * runtime_predicted_values.size()) == 0;
            //    size_t index2 = 0;
            //    for (; index2 < runtime_predicted_values.size(); index2++) {
            //        if (runtime_predicted_values[index2].x != predicted_values[index2].x) {
            //            break;
            //        }
            //        if (runtime_predicted_values[index2].y != predicted_values[index2].y) {
            //            break;
            //        }
            //    }

            //    std::vector<Float2> velocity_values(particle_count);
            //    FILE* velocity_file = fopen(velocity_file_path.c_str(), "rb");
            //    fread(velocity_values.data(), sizeof(Float2), velocity_values.size(), velocity_file);
            //    fclose(velocity_file);

            //    std::vector<Float2> runtime_velocity_values(particle_count);
            //    velocity_buffer.RetrieveData(sizeof(Float2), particle_count, runtime_velocity_values.data());

            //    bool are_the_same_velocity = memcmp(velocity_values.data(), runtime_velocity_values.data(), sizeof(Float2) * runtime_velocity_values.size()) == 0;
            //    size_t index3 = 0;
            //    for (; index3 < runtime_velocity_values.size(); index3++) {
            //        if (runtime_velocity_values[index3].x != velocity_values[index3].x) {
            //            break;
            //        }
            //        if (runtime_velocity_values[index3].y != velocity_values[index3].y) {
            //            break;
            //        }
            //    }

            //    std::vector<Float2> density_values(particle_count);
            //    FILE* density_file = fopen(density_file_path.c_str(), "rb");
            //    fread(density_values.data(), sizeof(Float2), density_values.size(), density_file);
            //    fclose(density_file);

            //    std::vector<Float2> runtime_density_values(particle_count);
            //    density_buffer.RetrieveData(sizeof(Float2), particle_count, runtime_density_values.data());

            //    bool are_the_same_density = memcmp(density_values.data(), runtime_density_values.data(), sizeof(Float2) * runtime_density_values.size()) == 0;
            //    size_t index4 = 0;
            //    for (; index4 < runtime_density_values.size(); index4++) {
            //        if (runtime_density_values[index4].x != density_values[index4].x) {
            //            break;
            //        }
            //        if (runtime_density_values[index4].y != density_values[index4].y) {
            //            break;
            //        }
            //    }

            //    std::vector<unsigned int> offset_values(particle_count);
            //    FILE* offset_file = fopen(offset_file_path.c_str(), "rb");
            //    fread(offset_values.data(), sizeof(unsigned int), offset_values.size(), offset_file);
            //    fclose(offset_file);

            //    std::vector<unsigned int> runtime_offset_values(particle_count);
            //    spatial_offsets.RetrieveData(sizeof(unsigned int), particle_count, runtime_offset_values.data());

            //    bool are_the_same_offsets = memcmp(offset_values.data(), runtime_offset_values.data(), sizeof(unsigned int) * runtime_offset_values.size()) == 0;
            //    size_t index6 = 0;
            //    for (; index6 < runtime_offset_values.size(); index6++) {
            //        if (runtime_offset_values[index6] != offset_values[index6]) {
            //            break;
            //        }
            //    }

            //    if (!are_the_same || !are_the_same_indices || !are_the_same_predicted || !are_the_same_velocity || !are_the_same_density || !are_the_same_offsets) {
            //        __debugbreak();
            //    }
            //    //}
            //}
        }

        HandleRecordSimulation(delta_time);
    }
}

void Simulation::Initialize()
{
    glDebugMessageCallback(DebugCallback, nullptr);

    glDisable(GL_CULL_FACE);

    // Use the simulation early compute to hold the general settings
    simulation_early_compute = ComputeShader(SHADER_LOCATION(simulation_early.comp), 128, 1, 1);
    calculate_density_compute = ComputeShader(SHADER_LOCATION(calculate_density.comp), 128, 1, 1);
    calculate_pressure_compute = ComputeShader(SHADER_LOCATION(calculate_pressure.comp), 128, 1, 1);
    calculate_viscosity_update_pos_compute = ComputeShader(SHADER_LOCATION(calculate_viscosity_update_pos.comp), 128, 1, 1);

    simulation_early_compute.CreateUniformBlock("Settings", sizeof(GeneralSettings));

    render_shader = Shader(SHADER_LOCATION(sprite.vert), SHADER_LOCATION(sprite.frag));
    image_render_shader = Shader(SHADER_LOCATION(sprite_image.vert), SHADER_LOCATION(sprite_image.frag));
    collision_render_shader = Shader(SHADER_LOCATION(whole_quad.vert), SHADER_LOCATION(draw_collision.frag));

    render_vertex_buffer = VertexBuffer(DataType::Float2, 6);

    Float2 ndc_coordinates[6] = {
        { -1.0f, -1.0f },
        { 1.0f, -1.0f },
        { -1.0f, 1.0f },
        { -1.0f, 1.0f },
        { 1.0f, -1.0f },
        { 1.0f, 1.0f }
    };
    collision_render_vertex_buffer = VertexBuffer(DataType::Float2, 6, ndc_coordinates);
    circle_alpha_texture = CreateCircleAlphaTexture(256, 256, 1.0f);

    heatmap_entries = {
        { Float4{28 / 255.0f, 70 / 255.0f, 158 / 255.0f, 1.0f}, 0.15f },
        { Float4{94 / 255.0f, 190 / 255.0f, 149 / 255.0f, 1.0f}, 0.5f},
        { Float4{200 / 255.0f, 200 / 255.0f, 17 / 255.0f, 1.0f}, 0.70f},
        { Float4{200 / 255.0f, 73 / 255.0f, 43 / 255.0f, 1.0f}, 1.0f}
    };
    RecalculateHeatmap();
    circle_alpha_texture.Bind(0);
    heatmap_texture.Bind(1);
    particle_count = 25'000;
    max_particle_count = 32'500;
    SetWindowSize(2500, 1200);
    aspect_ratio_change = 1.0f;
    image_mode_delta_time_index = 0;

    position_buffer = StructuredBuffer(sizeof(Float2), particle_count);
    predicted_position_buffer = StructuredBuffer(sizeof(Float2), particle_count);
    velocity_buffer = StructuredBuffer(sizeof(Float2), particle_count);
    density_buffer = StructuredBuffer(sizeof(Float2), particle_count);
    spatial_indices = StructuredBuffer(sizeof(unsigned int) * 3, particle_count);
    spatial_offsets = StructuredBuffer(sizeof(unsigned int), particle_count);
    SetInitialBufferData(particle_count);
    SetInitialSettingsData();

    gpu_sort.Initialize();
    pause_simulation = false;
    image_mode = false;
    record_simulation = false;
    particle_spawner.spawn_point = Float2(0.0f, POSITION_FACTOR - 50.0f);
    particle_spawner.spawn_delta = FLT_MAX;
    particle_spawner.initial_velocity = -7.5f;
    particle_spawner.spawn_count = 60;
    particle_spawner.direction = Float2(0.0f, -1.0f);

    //SetRecordMode();
    //SetImageDisplayMode("ancient_rome.jpg");
}

void Simulation::PaintCollision(Int2 center, Int2 rectangle_size, bool is_set)
{
    center.x -= rectangle_size.x * 0.5f;
    center.y += rectangle_size.y * 0.5f;

    center.x = std::max(0, center.x);
    center.y = std::max(0, center.y);
    center.x = std::min(center.x, (int)window_width);
    center.y = std::min(center.y, (int)window_height);

    // It must be inverted, such that it respects the OpenGL texture layout
    center.y = window_height - center.y;
    if (center.x + rectangle_size.x >= window_width) {
        rectangle_size.x = window_width - center.x;
    }
    if (center.y + rectangle_size.y >= window_height) {
        rectangle_size.y = window_height - center.y;
    }
    for (size_t row = 0; row < rectangle_size.y; row++) {
        for (size_t column = 0; column < rectangle_size.x; column++) {
            SetCollisionPixel(center + Int2(column, row), is_set);
        }
    }
}

void Simulation::RecalculateHeatmap()
{
    std::vector<UChar4> heatmap_values = ConstructHeatmap(1024, heatmap_entries);
    heatmap_texture.SetData(DataType::UNorm4, 1024, heatmap_values.data(), TextureSampling::Point);
    heatmap_texture.Bind(1);
}

void Simulation::SetImageDisplayMode(const char* texture_path)
{
    // Load the texture data
    int texture_x, texture_y;
    int channel_count;
    unsigned char* image_data = stbi_load(texture_path, &texture_x, &texture_y, &channel_count, 0);
    if (image_data == NULL) {
        std::cout << "Failed to load the image\n";
    }
    else {
        DataType data_type = DataType::UByteNorm3;
        if (channel_count == 4) {
            data_type = DataType::UByteNorm4;
        }
        image_mode_texture.SetData(data_type, texture_x, texture_y, image_data, TextureSampling::Bilinear);
        stbi_image_free(image_data);

        // Load the simulation file
        FILE* simulation_file = fopen(SIMULATION_FILE, "rb");
        if (simulation_file != NULL) {
            // Read the parameters
            GeneralSettings* general_settings = (GeneralSettings*)simulation_early_compute.GetUniformBlockData("Settings");
            if (fread(general_settings, sizeof(*general_settings), 1, simulation_file) != 1) {
                std::cout << "Failed to read the simulation file\n";
            }
            else {
                // Read the spawner data
                if (fread(&particle_spawner, sizeof(particle_spawner), 1, simulation_file) != 1) {
                    std::cout << "Failed to read the simulation file\n";
                }
                else {
                    // Allocate a very large buffer to read the entire file. We can take advantage of virtual
                    // Memory for not having the entire buffer be physically paged
                    const size_t ALLOCATION_SIZE = 1024 * 1024 * 50;
                    void* simulation_file_allocation = malloc(ALLOCATION_SIZE); // 50MB
                    size_t read_amount = fread(simulation_file_allocation, sizeof(char), ALLOCATION_SIZE, simulation_file);
                    if (read_amount == 0) {
                        std::cout << "Failed to read the simulation file\n";
                    }
                    else {
                        // Count how many delta times there are
                        float* delta_times = (float*)simulation_file_allocation;
                        size_t delta_time_count = 0;
                        while (delta_times[delta_time_count] != 0.0f) {
                            delta_time_count++;
                        }
                        image_mode_delta_time.resize(delta_time_count);
                        memcpy(image_mode_delta_time.data(), delta_times, sizeof(float) * delta_time_count);

                        // Create the UVs structured buffer
                        delta_times += delta_time_count + 1;
                        void* current_data_ptr = delta_times;
                        size_t uv_entry_count = *(size_t*)current_data_ptr;
                        current_data_ptr = (size_t*)current_data_ptr + 1;
                        image_mode_uvs = StructuredBuffer(sizeof(Float2), uv_entry_count, current_data_ptr);

                        image_mode_delta_time_index = 0;
                        image_mode = true;
                    }
                    fclose(simulation_file);
                    free(simulation_file_allocation);
                }
            }
        }
        else {
            std::cout << "Failed to read the simulation file\n";
        }
    }
}

void Simulation::SetRecordMode()
{
    record_simulation = true;
    record_file = fopen(SIMULATION_FILE, "wb");
    if (record_file == NULL) {
        std::cout << "Failed to open record file\n";
        abort();
    }

    // Write the simulation parameters firstly
    const GeneralSettings* general_settings = (const GeneralSettings*)simulation_early_compute.GetUniformBlockData("Settings");
    if (fwrite(general_settings, sizeof(*general_settings), 1, record_file) != 1) {
        std::cout << "Failed to write record file\n";
        abort();
    }

    // Write into the record file the spawner parameters
    if (fwrite(&particle_spawner, sizeof(particle_spawner), 1, record_file) != 1) {
        std::cout << "Failed to write record file\n";
        abort();
    }
}

void Simulation::SetWindowSize(size_t width, size_t height)
{
    // Resize the collision texture, if necessary
    if (width != window_width || height != window_height) {
        size_t reduced_width = (width + 7) / 8;
        collision_map_data = (unsigned char*)calloc(sizeof(unsigned char), reduced_width * height);
        collision_map.SetData(DataType::UByte, reduced_width, height, nullptr, TextureSampling::Point);
        collision_map.Bind(2);
        aspect_ratio_change = ((float)width / (float)height) / aspect_ratio;
    }
    aspect_ratio = (float)width / (float)height;
    window_width = width;
    window_height = height;
}

void Simulation::SetInitialSettingsData()
{
    const float FACTOR = 2.0f;

    GeneralSettings* settings = (GeneralSettings*)simulation_early_compute.GetUniformBlockData("Settings");
    settings->collision_damping = 0.3f;
    settings->gravity = 50.0f * FACTOR;
    settings->interaction_input_radius = 100.0f;
    settings->interaction_input_strength = 0.0f;
    settings->near_pressure_multiplier = 0.0f * FACTOR;
    settings->pressure_multiplier = 25.0f * FACTOR;
    settings->num_particles = particle_count;
    settings->smoothing_radius = 3.25f * FACTOR;
    settings->target_density = 30.0f * FACTOR;
    settings->viscosity_strength = 0.5f * FACTOR;
    mouse_click_strength = 1000.0f;

    settings->obstacle_centre = Float2(1.0f * POSITION_FACTOR, -0.3f * POSITION_FACTOR);
    //settings->obstacle_size = Float2(0.0f, 0.0f);
    settings->obstacle_size = Float2(0.2f * POSITION_FACTOR, 0.4f * POSITION_FACTOR);

    simulation_early_compute.SetUniformBlockDirty("Settings");

    use_mouse_pull = true;
    paint_collision = false;
    paint_collision_size = Int2(30, 30);
}

void Simulation::FrameCompute()
{
    const size_t ITERATION_COUNT = 1;
    GeneralSettings* general_settings = (GeneralSettings*)simulation_early_compute.GetUniformBlockData("Settings");
    general_settings->delta_time /= ITERATION_COUNT;
    simulation_early_compute.SetUniformBlockDirty("Settings");

    for (size_t index = 0; index < ITERATION_COUNT; index++) {
        // Early dispatch
        simulation_early_compute.BindUniformBlock(0);
        position_buffer.Bind(0);
        velocity_buffer.Bind(1);
        predicted_position_buffer.Bind(2);
        spatial_offsets.Bind(3);
        spatial_indices.Bind(4);
        simulation_early_compute.Bind(false);
        simulation_early_compute.SetFloat("aspect_ratio", aspect_ratio);
        simulation_early_compute.Dispatch(particle_count, 1, 1);

        // GPU spatial sorting
        gpu_sort.Execute(spatial_indices, spatial_offsets, particle_count);

        // We need to rebing the uniform block with the general settings for the rest of the pipeline
        simulation_early_compute.BindUniformBlock(0);
        // The density dispatch
        density_buffer.Bind(0);
        predicted_position_buffer.Bind(1);
        spatial_offsets.Bind(2);
        spatial_indices.Bind(3);
        calculate_density_compute.BindAndDispatch(particle_count, 1, 1);

        // The bindings for the pressure include those from the density
        velocity_buffer.Bind(4);
        calculate_pressure_compute.BindAndDispatch(particle_count, 1, 1);

        // The bindings for the final dispatch include those from the pressure dispatch
        calculate_viscosity_update_pos_compute.Bind(false);
        position_buffer.Bind(5);
        calculate_viscosity_update_pos_compute.SetUInt("window_width", window_width);
        calculate_viscosity_update_pos_compute.SetUInt("window_height", window_height);
        calculate_viscosity_update_pos_compute.SetFloat("aspect_ratio_change", aspect_ratio_change);
        if (aspect_ratio_change != 1.0f) {
            aspect_ratio_change = 1.0f;
        }
        collision_map.Bind(2);
        calculate_viscosity_update_pos_compute.SetTexture("CollisionMap", 2);
        calculate_viscosity_update_pos_compute.Dispatch(particle_count, 1, 1);
   }
}

void Simulation::HandleRecordSimulation(float delta_time)
{
    if (record_simulation) {
        if (record_file != NULL) {
            // Write the delta time
            if (fwrite(&delta_time, sizeof(delta_time), 1, record_file) != 1) {
                std::cout << "Failed to write delta time\n";
                abort();
            }
        }
        if (particle_count >= max_particle_count && record_file != NULL) {
            static int wait_count = 0;
            const int threshold = 3000;

            if (wait_count > threshold) {
                // The recording is finished, read the positions buffer and
                // And assign the UV values for each particle
                std::vector<Float2> final_positions(particle_count);
                position_buffer.RetrieveData(sizeof(Float2), particle_count, final_positions.data());

                FILE* pos_file = fopen(".pos", "wb");
                fwrite(final_positions.data(), sizeof(Float2), final_positions.size(), pos_file);
                fclose(pos_file);

                // Map the positions to the texture values
                std::vector<Float2> texture_uvs(particle_count);
                for (size_t index = 0; index < particle_count; index++) {
                    // Transform into ndc coordinates, and then into UV, and then into texel indices
                    Float2 ndc = final_positions[index] / POSITION_FACTOR;
                    // The x component needs to be adjusted to the aspect ratio
                    ndc.x /= aspect_ratio;

                    Float2 uv = (ndc + 1.0f) * 0.5f;
                    texture_uvs[index] = uv;
                }

                // Write a delta time of 0.0f to indicate that the simulation has ended
                float delta_zero = 0.0f;
                if (fwrite(&delta_zero, sizeof(delta_zero), 1, record_file) != 1) {
                    std::cout << "Failed to write record file\n";
                    abort();
                }

                // Write the number of particles
                if (fwrite(&particle_count, sizeof(particle_count), 1, record_file) != 1) {
                    std::cout << "Failed to write record file\n";
                    abort();
                }

                // Write the uvs now
                if (fwrite(texture_uvs.data(), sizeof(Float2), texture_uvs.size(), record_file) != texture_uvs.size()) {
                    std::cout << "Failed to write record file\n";
                    abort();
                }

                // It can be closed now
                fclose(record_file);
                record_file = NULL;
            }
            else {
                // Write the delta time
                if (fwrite(&delta_time, sizeof(delta_time), 1, record_file) != 1) {
                    std::cout << "Failed to write delta time\n";
                    abort();
                }
                wait_count++;
            }
        }
    }
}

void Simulation::SetInitialBufferData(size_t particle_count)
{
    if (particle_count > 0) {
        const float center_x = 0.0f;
        const float center_y = 0.0f;

        const float REDUCE_FACTOR = 0.65f;

        size_t rows = sqrt(particle_count);
        size_t per_row_count = particle_count / rows;

        float row_x_start = (center_x - ((float)per_row_count / 2) * PARTICLE_SIZE * REDUCE_FACTOR) * POSITION_FACTOR;
        float row_y = (center_y - ((float)rows / 2) * PARTICLE_SIZE) * POSITION_FACTOR;

        // At first, initialize the positions. The predicted positions will be the same as these positions
        std::vector<Float2> data;
        data.reserve(particle_count);
        for (size_t index = 0; index < rows; index++) {
            for (size_t column = 0; column < per_row_count; column++) {
                data.push_back({ row_x_start + (float)column * PARTICLE_SIZE * REDUCE_FACTOR * POSITION_FACTOR, row_y });
            }
            row_y += PARTICLE_SIZE * POSITION_FACTOR;
        }
        for (size_t index = rows * per_row_count; index < particle_count; index++) {
            data.push_back({ row_x_start + (float)(index - rows * per_row_count) * PARTICLE_SIZE * REDUCE_FACTOR * POSITION_FACTOR, row_y });
        }
        position_buffer.SetNewData(sizeof(Float2), particle_count, data.data());
        predicted_position_buffer.SetNewData(sizeof(Float2), particle_count, data.data());

        // Set the initial velocities to 0.0f
        // We can reuse the buffer from the positions
        for (size_t index = 0; index < particle_count; index++) {
            data[index] = { 0.0f };
        }
        velocity_buffer.SetNewData(sizeof(Float2), particle_count, data.data());
    }
}

void Simulation::SetFrameParameters(Float2 normalized_mouse_pos, bool is_left_mouse_pressed, bool is_right_mouse_pressed, float delta_time)
{
    GeneralSettings* settings = (GeneralSettings*)simulation_early_compute.GetUniformBlockData("Settings");
    float smoothing_radius = settings->smoothing_radius;
    settings->poly6_scaling_factor = 4 / (M_PI * pow(smoothing_radius, 8));
    settings->spiky_pow3_scaling_factor = 10 / (M_PI * pow(smoothing_radius, 5));
    settings->spiky_pow2_scaling_factor = 6 / (M_PI * pow(smoothing_radius, 4));
    settings->spiky_pow3_derivative_scaling_factor = 30 / (pow(smoothing_radius, 5) * M_PI);
    settings->spiky_pow2_derivative_scaling_factor = 12 / (pow(smoothing_radius, 4) * M_PI);
    settings->delta_time = delta_time;
    settings->num_particles = particle_count;

    float interaction_strength = 0;
    if (paint_collision) {
        Int2 mouse_pixel_position = (normalized_mouse_pos + Float2(1.0f, 1.0f)) * Float2(0.5f, 0.5f) * Float2(window_width, window_height);
        if (is_left_mouse_pressed) {
            PaintCollision(mouse_pixel_position, paint_collision_size, true);
            ReuploadCollisionData();
        }
        else if (is_right_mouse_pressed) {
            PaintCollision(mouse_pixel_position, paint_collision_size, false);
            ReuploadCollisionData();
        }
    }
    else if (use_mouse_pull) {
        if (is_left_mouse_pressed || is_right_mouse_pressed)
        {
            interaction_strength = is_right_mouse_pressed ? mouse_click_strength : -mouse_click_strength;
        }
    }

    settings->interaction_input_point = normalized_mouse_pos * POSITION_FACTOR;
    settings->interaction_input_point.x *= ((float)window_width / (float)window_height);
    // This must be inverted because of OpenGL order
    settings->interaction_input_point.y = -settings->interaction_input_point.y;
    settings->interaction_input_strength = interaction_strength;
    if (image_mode) {
        settings->viscosity_strength = 0.0f;
    }

    simulation_early_compute.SetUniformBlockDirty("Settings");
}

void Simulation::SetCollisionPixel(Int2 position, bool is_set)
{
    unsigned char bit_index = position.x & 7;
    position.x /= 8;
    size_t flat_index = (size_t)position.y * ((window_width + 7) / 8) + (size_t)position.x;
    if (is_set) {
        collision_map_data[flat_index] |= 1 << bit_index;
    }
    else {
        collision_map_data[flat_index] &= ~(1 << bit_index);
    }
}

void Simulation::RenderCollisionObjects() {
    collision_render_shader.Use();
    collision_render_shader.SetFloatColor("draw_color", 0.3, 0.7f, 0.2f, 1.0f);
    collision_render_shader.SetUInt("window_width", window_width);
    collision_render_shader.SetTexture("CollisionMap", 2);
    collision_render_vertex_buffer.Bind();
    collision_render_vertex_buffer.Draw(1);
}

void Simulation::ReuploadCollisionData()
{
    size_t reduced_width = (window_width + 7) / 8;
    collision_map.SetData(DataType::UByte, reduced_width, window_height, collision_map_data, TextureSampling::Point);
    collision_map.Bind(2);
}

void Simulation::Render() {
    RenderParticles();
    RenderCollisionObjects();
}

void Simulation::RenderParticles()
{
    const float REDUCTION_FACTOR = 0.45f;

    if (image_mode) {
        image_render_shader.Use();
        image_render_shader.SetFloat("scale", PARTICLE_SIZE * REDUCTION_FACTOR);
        image_render_shader.SetFloat("aspect_ratio", aspect_ratio);
        image_render_shader.SetTexture("circle_alpha", 0);
        image_render_shader.SetTexture("color_texture", 4);

        position_buffer.Bind(0);
        image_mode_uvs.Bind(1);

        circle_alpha_texture.Bind(0);
        image_mode_texture.Bind(4);
    }
    else {
        render_shader.Use();
        render_shader.SetFloat("scale", PARTICLE_SIZE * REDUCTION_FACTOR);
        render_shader.SetFloat("aspect_ratio", aspect_ratio);
        render_shader.SetFloat("max_speed", 400.0f);
        render_shader.SetTexture("circle_alpha", 0);
        render_shader.SetTexture("Heatmap", 1);
        render_vertex_buffer.Bind();

        position_buffer.Bind(0);
        velocity_buffer.Bind(1);

        circle_alpha_texture.Bind(0);
        heatmap_texture.Bind(1);
    }

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    render_vertex_buffer.Draw(particle_count);
}
