#pragma once

#include <array>
#include <cstdint>

namespace voxskel
{

using Vec3f = std::array<float, 3>;

using Vec2u = std::array<uint32_t, 2>;
using Vec3u = std::array<uint32_t, 3>;
using Vec3i = std::array<int32_t, 3>;
using Vec4i = std::array<int32_t, 4>;

inline Vec3u toVec3u(const Vec3i & v)
{
    return{ uint32_t(v[0]), uint32_t(v[1]), uint32_t(v[2]) };
}

using Mat4f = std::array<std::array<float, 4>, 4>;

}