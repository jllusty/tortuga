#ifndef VEC3_HPP

#include <cmath>
#include <array>
#include <ostream>

struct vec3 {
    std::array<float,3> data{0,0,0};
    // constructor
    inline vec3() {}
    inline vec3(float x, float y, float z) {
        data[0] = x; data[1] = y; data[2] = z;
    }
    // copy constructor
    inline vec3(const vec3& u) {
        data = u.data;
    }
    // operator overloads
    //  arithmetic
    inline vec3 operator+(const vec3& u) const {
        return vec3(data[0]+u.data[0],
                    data[1]+u.data[1],
                    data[2]+u.data[2]);
    }
    inline vec3 operator-(const vec3& u) const {
        return vec3(data[0]-u.data[0],
                    data[1]-u.data[1],
                    data[2]-u.data[2]);
    }
    inline vec3 operator*(const float& s) const {
        return vec3(s*data[0],s*data[1],s*data[2]);
    };
    //  assignment
    inline vec3& operator=(const vec3& u)  {
        data = u.data;
        return *this;
    }
    inline vec3& operator+=(const vec3& u) {
        data[0] += u.data[0];
        data[1] += u.data[1];
        data[2] += u.data[2];
        return *this;
    }
    inline vec3& operator-=(const vec3& u) {
        data[0] -= u.data[0];
        data[1] -= u.data[1];
        data[2] -= u.data[2];
        return *this;
    }
    inline vec3& operator*=(const float& s) {
        data[0] *= s;
        data[1] *= s;
        data[2] *= s;
        return *this;
    }
    inline vec3& operator/=(const float& s) {
        data[0] /= s;
        data[1] /= s;
        data[2] /= s;
        return *this;
    }
};
inline vec3 operator*(const float&s, const vec3& v) {
    return vec3(v.data[0] * s,v.data[1] * s,v.data[2] * s);
}
inline vec3 operator/(const float&s, const vec3& v) {
    return vec3(v.data[0] / s,v.data[1] / s,v.data[2] / s);
}
// debug only overloads below
std::ostream& operator<<(std::ostream& os, const vec3& v) {
    os << "(" << v.data[0] << ", " << v.data[1] << ", " << v.data[2] << ")";
    return os;
}


// dot product
inline float dot(const vec3& v, const vec3& u) {
    return v.data[0]*u.data[0] + v.data[1]*u.data[1] + v.data[2]*u.data[2];
}
// cross product
inline vec3 cross(const vec3& v, const vec3& u) {
    vec3 res;
    res.data[0] = v.data[1]*u.data[2] - v.data[2]*u.data[1],
    res.data[1] = v.data[2]*u.data[0] - v.data[0]*u.data[2],
    res.data[2] = v.data[0]*u.data[1] - v.data[1]*u.data[0];
    return res;
}
// rotate vector 'v' 'theta' radians about 'axis'
inline void rotate(vec3& v, const vec3& axis, const float theta) {
    // quaternion rotation
    float s = cosf(theta/2.0f);
    vec3  u = sinf(theta/2.0f)*axis;
    v = 2.0f*dot(u,v)*u + (s*s - dot(u,u))*v + 2.0f*s*cross(u,v);
}

#define VEC3_HPP
#endif