#pragma once

enum class DataType {
    Float,
    Float2,
    Float3,
    Float4,
    Uint,
    Uint2,
    Uint3,
    Uint4,
    Int,
    Int2,
    Int3,
    Int4,
    UNorm,
    UNorm2,
    UNorm3,
    UNorm4,
    Norm,
    Norm2,
    Norm3,
    Norm4,
    Byte,
    UByte,
    UByte3,
    UByte4,
    // These are UNORM values for the GPU but unsigned char on the CPU
    UByteNorm3,
    UByteNorm4
};

unsigned int GetDataTypeElemCount(DataType type);

unsigned int GetDataTypeNativeType(DataType type);

bool IsDataTypeNormalized(DataType type);

bool IsDataTypeUNorm(DataType type);

bool IsDataTypeSNorm(DataType type);

size_t GetDataTypeByteSize(DataType type);

void GetDataTypeInts(DataType type, int& internal_format, int& format, int& gl_type);
