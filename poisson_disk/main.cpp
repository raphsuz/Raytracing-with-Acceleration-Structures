#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <limits>

struct Vec3 {
    float x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3 &other) const {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }

    Vec3 operator-(const Vec3 &other) const {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }

    Vec3 operator*(float scalar) const {
        return Vec3(x * scalar, y * scalar, z * scalar);
    }

    Vec3 operator/(float scalar) const {
        return Vec3(x / scalar, y / scalar, z / scalar);
    }

    float dot(const Vec3 &other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    Vec3 cross(const Vec3 &other) const {
        return Vec3(y * other.z - z * other.y,
                    z * other.x - x * other.z,
                    x * other.y - y * other.x);
    }

    Vec3 normalize() const {
        float length = std::sqrt(x * x + y * y + z * z);
        return Vec3(x / length, y / length, z / length);
    }
};

struct Ray {
    Vec3 origin, direction;
    Ray(const Vec3 &origin, const Vec3 &direction) : origin(origin), direction(direction) {}
};

struct Sphere {
    Vec3 center;
    float radius;
    Sphere(const Vec3 &center, float radius) : center(center), radius(radius) {}
};

bool sphereIntersection(const Ray &ray, const Sphere &sphere, float &t) {
    Vec3 oc = ray.origin - sphere.center;
    float a = ray.direction.dot(ray.direction);
    float b = 2.0 * oc.dot(ray.direction);
    float c = oc.dot(oc) - sphere.radius * sphere.radius;
    float discriminant = b * b - 4 * a * c;

    if (discriminant < 0) {
        return false;
    } else {
        t = (-b - std::sqrt(discriminant)) / (2.0 * a);
        return true;
    }
}

Vec3 poissonDiskSample(int num_samples, float radius, std::mt19937 &rng) {
    std::uniform_real_distribution<float> dist(0, 1);
    float angle = 2 * M_PI * dist(rng);
    float r = radius * std::sqrt(dist(rng));
    float x = r * std::cos(angle);
    float y = r * std::sin(angle);

    return Vec3(x, y, 0);
}

Vec3 trace(const Ray &ray, const Sphere &sphere, int num_samples, std::mt19937 &rng) {
    float t;
    if (sphereIntersection(ray, sphere, t)) {
        Vec3 point = ray.origin + ray.direction * t;
        Vec3 normal = (point - sphere.center).normalize();
Vec3 light_direction = Vec3(1, 1, 1).normalize();
float dot = std::max(0.0f, normal.dot(light_direction));
    // 使用 Poisson 分佈計算光線散射
    Vec3 scattered_light(0, 0, 0);
    for (int i = 0; i < num_samples; ++i) {
        Vec3 sample = poissonDiskSample(num_samples, 1.0f, rng);
        Vec3 sample_direction = (normal + sample).normalize();
        float sample_dot = std::max(0.0f, normal.dot(sample_direction));
        scattered_light = scattered_light + sample_direction * sample_dot;
    }
    scattered_light = scattered_light / static_cast<float>(num_samples);

    return Vec3(dot, dot, dot) + scattered_light * 0.5f;
    } 
    else 
    {  
        return Vec3(0, 0, 0); // 背景顏色
    }

}

int main() {
    int width = 400;
    int height = 300;
    int num_samples = 64;
    Vec3 camera_origin(0, 0, -5);
    Sphere sphere(Vec3(0, 0, 0), 1.0);
    std::random_device rd;
    std::mt19937 rng(rd());

    std::cout << "P3\n" << width << " " << height << "\n255\n";
    for (int j = height - 1; j >= 0; --j) {
        for (int i = 0; i < width; ++i) {
            float u = float(i) / float(width);
            float v = float(j) / float(height);
            Vec3 direction = Vec3(4 * (u - 0.5), 3 * (v - 0.5), 5).normalize();
            Ray ray(camera_origin, direction);
            Vec3 color = trace(ray, sphere, num_samples, rng);
            int ir = static_cast<int>(255.99 * color.x);
            int ig = static_cast<int>(255.99 * color.y);
            int ib = static_cast<int>(255.99 * color.z);
            std::cout << ir << " " << ig << " " << ib << "\n";
        }
    }

    return 0;
}