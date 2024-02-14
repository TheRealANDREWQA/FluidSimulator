#include "DataType.h"
#include "glad.h"
#include <stdio.h>

unsigned int GetDataTypeElemCount(DataType type)
{
#define CASE(count) case DataType::Float##count:\
                    case DataType::Int##count:\
                    case DataType::Uint##count:\
                    case DataType::Norm##count:\
                    case DataType::UNorm##count:

    switch (type) {
        CASE()
            return 1;
        CASE(2)
            return 2;
        CASE(3)
            return 3;
        CASE(4)
            return 4;
    case DataType::Byte:
    case DataType::UByte:
        return 1;
    case DataType::UByte3:
    case DataType::UByteNorm3:
        return 3;
    case DataType::UByte4:
    case DataType::UByteNorm4:
        return 4;
    }

    return -1;

#undef CASE

}

unsigned int GetDataTypeNativeType(DataType type)
{
#define CASE(type)  case DataType::type:\
                    case DataType::type##2:\
                    case DataType::type##3:\
                    case DataType::type##4:

    switch (type) {
        CASE(Float)
            return GL_FLOAT;
        CASE(Int)
        CASE(Norm)
            return GL_INT;
        CASE(Uint)
        CASE(UNorm)
            return GL_UNSIGNED_INT;
    case DataType::Byte:
        return GL_BYTE;
    case DataType::UByte:
    case DataType::UByte3:
    case DataType::UByte4:
    case DataType::UByteNorm3:
    case DataType::UByteNorm4:
        return GL_UNSIGNED_BYTE;
    }

    return -1;

#undef CASE
}

bool IsDataTypeNormalized(DataType type)
{
    return IsDataTypeSNorm(type) || IsDataTypeUNorm(type);
}

bool IsDataTypeUNorm(DataType type) {
    switch (type) {
    case DataType::UNorm:
    case DataType::UNorm2:
    case DataType::UNorm3:
    case DataType::UNorm4:
    case DataType::UByteNorm3:
    case DataType::UByteNorm4:
        return true;
    }
    return false;
}

bool IsDataTypeSNorm(DataType type) {
    switch (type) {
    case DataType::Norm:
    case DataType::Norm2:
    case DataType::Norm3:
    case DataType::Norm4:
        return true;
    }
    return false;
}

size_t GetDataTypeByteSize(DataType type)
{
    // At the moment, all types are either floats or 32bit integers
    return GetDataTypeElemCount(type) * sizeof(float);
}

void GetDataTypeInts(DataType type, int& internal_format, int& format, int& gl_type)
{
    switch (type)
    {
    case DataType::Float:
    {
        internal_format = GL_R32F;
        format = GL_RED;
        gl_type = GL_FLOAT;
    }
        break;
    case DataType::Float2:
    {
        internal_format = GL_RG32F;
        format = GL_RG;
        gl_type = GL_FLOAT;
    }
        break;
    case DataType::Float3:
    {
        internal_format = GL_RGB32F;
        format = GL_RGB;
        gl_type = GL_FLOAT;
    }
        break;
    case DataType::Float4:
    {
        internal_format = GL_RGBA32F;
        format = GL_RGBA;
        gl_type = GL_FLOAT;
    }
        break;
    case DataType::Uint:
    {
        internal_format = GL_R32UI;
        format = GL_RED_INTEGER;
        gl_type = GL_UNSIGNED_INT;
    }
        break;
    case DataType::Uint2:
    {
        internal_format = GL_RG32UI;
        format = GL_RG_INTEGER;
        gl_type = GL_UNSIGNED_INT;
    }
        break;
    case DataType::Uint3:
    {
        internal_format = GL_RGB32UI;
        format = GL_RGB_INTEGER;
        gl_type = GL_UNSIGNED_INT;
    }
        break;
    case DataType::Uint4:
    {
        internal_format = GL_RGBA32UI;
        format = GL_RGBA_INTEGER;
        gl_type = GL_UNSIGNED_INT;
    }
        break;
    case DataType::Int:
    {
        internal_format = GL_R32I;
        format = GL_RED_INTEGER;
        gl_type = GL_INT;
    }
        break;
    case DataType::Int2:
    {
        internal_format = GL_RG32I;
        format = GL_RG_INTEGER;
        gl_type = GL_INT;
    }
        break;
    case DataType::Int3:
    {
        internal_format = GL_RGB32I;
        format = GL_RGB_INTEGER;
        gl_type = GL_INT;
    }
        break;
    case DataType::Int4:
    {
        internal_format = GL_RGBA32I;
        format = GL_RGBA_INTEGER;
        gl_type = GL_INT;
    }
        break;
    case DataType::UNorm:
    {
        internal_format = GL_R8;
        format = GL_RED;
        gl_type = GL_UNSIGNED_BYTE;
    }
        break;
    case DataType::UNorm2:
    {
        internal_format = GL_RG8;
        format = GL_RG;
        gl_type = GL_UNSIGNED_BYTE;
    }
        break;
    case DataType::UNorm3:
    {
        internal_format = GL_RGB8;
        format = GL_RGB;
        gl_type = GL_UNSIGNED_BYTE;
    }
        break;
    case DataType::UNorm4:
    {
        internal_format = GL_RGBA8;
        format = GL_RGBA;
        gl_type = GL_UNSIGNED_BYTE;
    }
        break;
    case DataType::Norm:
    {
        internal_format = GL_R8_SNORM;
        format = GL_RED;
        gl_type = GL_BYTE;
    }
        break;
    case DataType::Norm2:
    {
        internal_format = GL_RG8_SNORM;
        format = GL_RG;
        gl_type = GL_BYTE;
    }
        break;
    case DataType::Norm3:
    {
        internal_format = GL_RGB8_SNORM;
        format = GL_RGB;
        gl_type = GL_BYTE;
    }
        break;
    case DataType::Norm4:
    {
        internal_format = GL_RGBA8_SNORM;
        format = GL_RGBA;
        gl_type = GL_BYTE;
    }
        break;
    case DataType::Byte:
    {
        internal_format = GL_R8I;
        format = GL_RED_INTEGER;
        gl_type = GL_BYTE;
    }
        break;
    case DataType::UByte:
    {
        internal_format = GL_R8UI;
        format = GL_RED_INTEGER;
        gl_type = GL_UNSIGNED_BYTE;
    }
        break;
    case DataType::UByte3:
    {
        internal_format = GL_RGB8UI;
        format = GL_RGB_INTEGER;
        gl_type = GL_UNSIGNED_BYTE;
    }
        break;
    case DataType::UByte4:
    {
        internal_format = GL_RGBA8UI;
        format = GL_RGBA_INTEGER;
        gl_type = GL_UNSIGNED_BYTE;
    }
        break;
    case DataType::UByteNorm3:
    {
        internal_format = GL_RGB8;
        format = GL_RGB;
        gl_type = GL_UNSIGNED_BYTE;
    }
    break;
    case DataType::UByteNorm4:
    {
        internal_format = GL_RGBA8;
        format = GL_RGBA;
        gl_type = GL_UNSIGNED_BYTE;
    }
    break;
    default:
        perror("Invalid format");
        break;
    }
}
