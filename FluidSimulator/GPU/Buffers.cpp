#include "Buffers.h"
#include "glad.h"
#include <string.h>

StructuredBuffer::StructuredBuffer(size_t element_byte_size, size_t element_count)
{
    glGenBuffers(1, &id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, element_byte_size * element_count, nullptr, GL_DYNAMIC_DRAW);
}

StructuredBuffer::StructuredBuffer(size_t element_byte_size, size_t element_count, const void* data)
{
    glGenBuffers(1, &id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, element_byte_size * element_count, data, GL_DYNAMIC_DRAW);
}

void StructuredBuffer::Bind(unsigned int index) const
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, id);
}

void StructuredBuffer::RetrieveData(size_t element_byte_size, size_t element_count, void* buffer) const
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, element_byte_size * element_count, buffer);
}

void StructuredBuffer::SetNewDataSize(size_t element_byte_size, size_t element_count) const
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, element_byte_size * element_count, nullptr, GL_DYNAMIC_DRAW);
}

void StructuredBuffer::SetNewData(size_t element_byte_size, size_t element_count, const void* data) const
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, element_byte_size * element_count, data, GL_DYNAMIC_DRAW);
}
