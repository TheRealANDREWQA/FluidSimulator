#include "Texture.h"
#include "glad.h"
#include <vector>

static void SetTextureSampling(int texture_type, int filter_type, TextureSampling sampling) {
    int native_sampling = 0;
    switch (sampling) {
    case TextureSampling::Point:
        native_sampling = GL_NEAREST;
        break;
    case TextureSampling::Bilinear:
        native_sampling = GL_LINEAR;
        break;
    case TextureSampling::Trilinear:
        native_sampling = GL_LINEAR_MIPMAP_LINEAR;
        break;
    default:
        perror("Invalid sampling mode");
    }

    glTexParameteri(texture_type, filter_type, native_sampling);
}

Texture1D::Texture1D() : ID(-1) {}

void Texture1D::Bind(unsigned int texture_unit) const
{
    glActiveTexture(GL_TEXTURE0 + texture_unit);
    glBindTexture(GL_TEXTURE_1D, ID);
}

void Texture1D::SetData(DataType data_type, size_t width, const void* data, TextureSampling sampling)
{
    if (ID == -1) {
        glGenTextures(1, &ID);
        glBindTexture(GL_TEXTURE_1D, ID);
        SetTextureSampling(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, sampling);
        SetTextureSampling(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, sampling);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    }
    else {
        glBindTexture(GL_TEXTURE_1D, ID);
    }

    int internal_format, format, type;
    GetDataTypeInts(data_type, internal_format, format, type);
    glTexImage1D(GL_TEXTURE_1D, 0, internal_format, width, 0, format, type, data);
}


Texture2D::Texture2D() : ID(-1) {}

void Texture2D::Bind(unsigned int texture_unit) const
{
    glActiveTexture(GL_TEXTURE0 + texture_unit);
    glBindTexture(GL_TEXTURE_2D, ID);
}

void Texture2D::SetData(DataType data_type, size_t width, size_t height, const void* data, TextureSampling sampling_mode)
{
    if (ID == -1) {
        glGenTextures(1, &ID);
        glBindTexture(GL_TEXTURE_2D, ID);
        SetTextureSampling(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, sampling_mode);
        SetTextureSampling(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sampling_mode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
    else {
        glBindTexture(GL_TEXTURE_2D, ID);
    }

    int internal_format, format, type;
    GetDataTypeInts(data_type, internal_format, format, type);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, type, data);
}

Texture2D CreateCircleAlphaTexture(size_t width, size_t height, float radius)
{
    Texture2D texture;

    float squared_radius = radius * radius;

    std::vector<unsigned char> pixels;
    pixels.resize(width * height);

    float half_width = width * 0.5f;
    float half_height = height * 0.5f;
    float half_width_inverse = 1.0f / half_width;
    float half_height_inverse = 1.0f / half_height;

    for (size_t row = 0; row < height; row++) {
        float float_row = ((float)row - half_height) * half_height_inverse;
        for (size_t column = 0; column < width; column++) {
            float float_column = ((float)column - half_width) * half_width_inverse;
            float squared_distance = float_row * float_row + float_column * float_column;

            unsigned char byte_value = 0;
            if (squared_distance <= squared_radius) {
                byte_value = 255;
            }
            pixels[row * width + column] = byte_value;
        }
    }

    texture.SetData(DataType::UNorm, width, height, pixels.data(), TextureSampling::Bilinear);
    return texture;
}

std::vector<UChar4> ConstructHeatmap(size_t numberOfEntries, const std::vector<HeatmapEntry>& entries) {
    std::vector<UChar4> heatmap;
    heatmap.resize(numberOfEntries);

    auto fill_range = [&heatmap, numberOfEntries](Float4 first_color, Float4 second_color, float first_percentage, float second_percentage) {
        size_t range_start = floor(first_percentage * numberOfEntries);
        size_t range_end = floor(second_percentage * numberOfEntries);

        float step = 1.0f / (range_end - range_start);
        for (size_t index = 0; index < range_end - range_start; index++) {
            float interpolation_factor = 1.0f - index * step;
            float one_minus_factor = 1.0f - interpolation_factor;
            Float4 float_result = first_color * Float4(interpolation_factor, interpolation_factor, interpolation_factor, interpolation_factor) +
                second_color * Float4(one_minus_factor, one_minus_factor, one_minus_factor, one_minus_factor);
            heatmap[range_start + index] = float_result * 255.0f;
        }
    };

    if (entries[0].percentage > 0.0f) {
        fill_range(entries[0].color, entries[0].color, 0.0f, entries[0].percentage);
    }

    for (size_t index = 0; index < entries.size() - 1; index++) {
        fill_range(entries[index].color, entries[index + 1].color, entries[index].percentage, entries[index + 1].percentage);
    }

    if (entries[entries.size() - 1].percentage < 1.0f) {
        Float4 last_color = entries[entries.size() - 1].color;
        fill_range(last_color, last_color, entries[entries.size() - 1].percentage, 1.0f);
    }

    return heatmap;
}
