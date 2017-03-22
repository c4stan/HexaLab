#include "plane.h"

namespace HexaLab {
    Plane::Plane(Vector3f& position, Vector3f& normal) {
        this->normal = normal.normalized();
        this->d = -position.dot(this->normal);
    }

    Plane::Plane(float x, float y, float z, float nx, float ny, float nz) {
        this->normal = Vector3f(nx, ny, nz).normalized();
        Vector3f position = Vector3f(x, y, z);
        this->d = -position.dot(this->normal);
    }

    Plane::Plane() {
        this->normal = Vector3f(1, 0, 0);
        this->d = 0;
    }

    void Plane::move(float x, float y, float z) {
        Vector3f position = Vector3f(x, y, z);
        this->d = -position.dot(this->normal);
    }

    void Plane::orient(float nx, float ny, float nz) {
        this->normal = Vector3f(nx, ny, nz).normalized();
    }

    float Plane::solve(Vector3f& point) {
        return this->normal.dot(point) + this->d;
    }
}
