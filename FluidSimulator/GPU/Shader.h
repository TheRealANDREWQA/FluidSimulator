#pragma once

class Shader {
public:
    Shader() { ID = -1; }
    Shader(const char* vertex_path, const char* pixel_path);

    void Use() const;

    void SetBool(const char* name, bool value) const;

    void SetInt(const char* name, int value) const;

    void SetUInt(const char* name, unsigned int value) const;

    void SetFloat(const char* name, float value) const;

    void SetTexture(const char* name, unsigned int slot) const;

    void SetFloatColor(const char* name, float r, float g, float b, float alpha);

private:
    unsigned int ID;
};
