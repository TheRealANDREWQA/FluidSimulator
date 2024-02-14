#pragma once
#include "DataType.h"
#include "../Vec4.h"
#include <vector>

enum class TextureSampling : unsigned char {
    Point,
    Bilinear,
    Trilinear
};

class Texture1D {
public:
    Texture1D();

    void Bind(unsigned int texture_unit) const;

    void SetData(DataType data_type, size_t width, const void* data, TextureSampling sampling_mode);

private:
    unsigned int ID;
};

class Texture2D {
public:
    Texture2D();

    void Bind(unsigned int texture_unit) const;

    void SetData(DataType data_type, size_t width, size_t height, const void* data, TextureSampling sampling_mode);

private:
    unsigned int ID;
};

Texture2D CreateCircleAlphaTexture(size_t width, size_t height, float radius);

struct HeatmapEntry {
    Float4 color;
    float percentage;
};

std::vector<UChar4> ConstructHeatmap(size_t numberOfEntries, const std::vector<HeatmapEntry>& entries);
