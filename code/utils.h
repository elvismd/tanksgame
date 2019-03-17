#pragma once 

#include "agnostic.h"

inline bool point_overlap_rect(const vec2& point, const vec4& rect)
{
    return (point.x >= (rect.x) && point.x <= (rect.x + rect.z)
            && point.y >= (rect.y) && point.y <= (rect.y + rect.w));
}

inline bool point_overlap_rect_middle_anchor(const vec2& point, const vec4& rect)
{
    return (point.x >= (rect.x - rect.z * 0.5f) && point.x <= (rect.x + rect.z* 0.5f)
            && point.y >= (rect.y - rect.w* 0.5f) && point.y <= (rect.y + rect.w* 0.5f));
}

inline bool value_in_range(float value, float min, float max)
{
    return (value >= min) && (value <= max);
}

inline bool aabbb_overlaps(const vec4& r0, const vec4& r1)
{
    vec2 center = vec2(r0.x, r0.y);
    vec2 half_size = vec2(r0.z, r0.w);
    
    vec2 other_center = vec2(r1.x, r1.y);
    vec2 other_half_size = vec2(r1.z, r1.w);
    
    if (abs(center.x - other_center.x) > half_size.x + other_half_size.x) return false;
    if (abs(center.y - other_center.y) > half_size.y + other_half_size.y) return false;
    
    return true;
}

inline bool rect_overlap_rect(const vec4& r0, const vec4& r1)
{
    bool x_overlap = value_in_range(r0.x, r1.x, r1.x + r1.z) ||
        value_in_range(r1.x, r0.x, r0.x + r0.z);
    
    bool y_overlap = value_in_range(r0.y, r1.y, r1.y + r1.w) ||
        value_in_range(r1.y, r0.y, r0.y + r0.w);
    
    return x_overlap && y_overlap;
}

internal bool selecting_rect(vec2 world_mouse_pos, vec2 pos, float w, float h)
{
	vec4 bounds = vec4(pos.x, pos.y, w, h);
	bool overlapping = (point_overlap_rect(world_mouse_pos, bounds));

	return overlapping;
}

internal bool selecting_rect(vec2 world_mouse_pos, vec4 rect)
{
	return selecting_rect(world_mouse_pos, { rect.x, rect.y }, rect.z, rect.w);
}