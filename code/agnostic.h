#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stdarg.h> 
#include <ctime>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>

using namespace glm;

// TYPES
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef int32_t b32;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef size_t memory_index;

typedef unsigned char uchar;

typedef float r32;
typedef double r64;

#define internal static
#define local_persist static
#define global_variable static

#define Pi32 3.14159265359

#if HANDMADE_SLOW
#define Assert(Expression) \
	if(!(Expression)) { *(int*)0 = 0; }
#else 
#define Assert(Expression)
#endif

#define Kilobytes(Value) ((Value) * 1024)
#define Megabytes(Value) (Kilobytes(Value) * 1024)
#define Gigabytes(Value) (Megabytes(Value) * 1024)
#define Terabytes(Value) (Gigabytes(Value) * 1024)

#define array_count(a) (sizeof(a) / sizeof(a[0]))

#define str_equal_to(str1, str2) strcmp(str1, str2) == 0

inline u32 safe_truncate_uint64(u64 v)
{
	Assert(v <= 0xFFFFFFFF);
	u32 result = (u32)v;

	return (result);
}

struct Debug_Read_File_Result
{
	u32 contents_size;
	void *contents;
};

Debug_Read_File_Result DEBUG_platform_read_entire_file(char *file_name);
void DEBUG_platform_free_file_memory(void *memory);
b32 DEBUG_platform_write_entire_file(char *file_name, u64 memory_size, void *memory);

inline float magnitude_vector(const vec2& v)
{
	float x = pow(v.x, 2);
	float y = pow(v.y, 2);

	return sqrt(x + y);
}

inline float magnitude_vector(const vec3& v)
{
	float x = pow(v.x, 2);
	float y = pow(v.y, 2);
	float z = pow(v.z, 2);

	return sqrt(x + y + z);
}

inline vec3 midpoint_vec3(vec3 v1, vec3 v2)
{
	vec3 r = v1 + v2;
	r.x *= 0.5f;
	r.y *= 0.5f;
	r.z *= 0.5f;

	return r;
}

inline vec3 rotate_point(vec3 p, float angle)
{
    vec3 result = {};
    result.x = p.x * cos(angle) - p.y * sin(angle);
    result.y = p.x * sin(angle) + p.y * cos(angle);
    result.z = p.z;

    return result;
}

inline float lerp(float a, float b, float f)
{
    return a + f * (b - a);
}

inline vec2 lerp(vec2 a, vec2 b, float f)
{
    return vec2(lerp(a.x, b.x, f), lerp(a.y, b.y, f));
}