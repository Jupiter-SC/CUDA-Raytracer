// Jupiter Sinclair Chong
// CUDA Raytracer

#pragma once

#include "core.h"
#include "hitable.h"

struct Sphere : public Hitable {
    Sphere() = default;

    __device__ 
    Sphere(const point3& center, double radius)
        : center(center), radius(radius)
    {
        // TODO Init mat
    }

    __device__ bool hit(const ray& r, float t_min, float t_max, HitInfo& rec) const {
        vec3 oc = r.origin() - center;
        float a = dot(r.direction(), r.direction());
        float b = dot(oc, r.direction());
        float c = dot(oc, oc) - radius * radius;
        float discriminant = b * b - a * c;
        if (discriminant > 0) {
            float temp = (-b - sqrt(discriminant)) / a;
            if (temp < t_max && temp > t_min) {
                rec.t = temp;
                rec.p = r.at(rec.t);
                rec.normal = (rec.p - center) / radius;
                return true;
            }
            temp = (-b + sqrt(discriminant)) / a;
            if (temp < t_max && temp > t_min) {
                rec.t = temp;
                rec.p = r.at(rec.t);
                rec.normal = (rec.p - center) / radius;
                return true;
            }
        }
        return false;
    }

    // Inherited via Hitable
    __device__ virtual bool Sphere::hit(const ray& r, interval rayT, HitInfo& info) const
    {
        vec3 originToCenter = center - r.origin();

        double a = r.direction().lengthSquared();
        double h = dot(r.direction(), originToCenter);
        double c = originToCenter.lengthSquared() - radius * radius;
        double discriminant = h * h - a * c;

        if (discriminant < 0) return false;

        double sqrtDiscriminant = std::sqrt(discriminant);

        // Find the nearest root that lies in the acceptable range.
        double root = (h - sqrtDiscriminant) / a;
        if (!rayT.surrounds(root)) {
            root = (h + sqrtDiscriminant) / a;
            if (!rayT.surrounds(root))
                return false;
        }

        // Populate info struct
        info.t = root;
        info.p = r.at(info.t);

        vec3 outwardNormal = (info.p - center) / radius;
        info.setFaceNormal(r, outwardNormal);
        //info.mat = mat;
        return true;
    }

private:
    point3 center;
    double radius;
    //shared_ptr<Material> mat;
};