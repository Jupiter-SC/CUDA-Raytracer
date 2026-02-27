// Jupiter Sinclair Chong
// CUDA Raytracer

#pragma once

#include "core.h"

// C++ Standard
#include <vector>

//struct Material;

/// <summary>
/// Information stored from a hit
/// </summary>
struct HitInfo {
	point3 p;
	vec3 normal;
	double t = 0;
	bool frontFace = false;
	//shared_ptr<Material> mat;

	// Sets the hit record normal vector. Normalized outward normal
	__device__ void setFaceNormal(const ray& r, const vec3& outwardNormal) {
		vec3 outNormalNorm = unitVector(outwardNormal);
		frontFace = dot(r.direction(), outNormalNorm) < 0;
		normal = frontFace ? outNormalNorm : -outNormalNorm;
	}
};

/// <summary>
/// Interface. Things that rays can hit
/// </summary>
struct Hitable {
public:
	virtual ~Hitable() = default;

	// temp
	__device__ virtual bool hit(const ray& r, float t_min, float t_max, HitInfo& rec) const = 0;

	__device__ virtual bool hit(const ray& r, interval rayT, HitInfo& info) const = 0;
};

//typedef shared_ptr<Hitable> HitableObject;
//typedef Hitable* HitableObject;
//typedef std::vector<HitableObject> HitableObjects;

/// <summary>
/// Many things that rays can hit
/// </summary>
struct HitableList : public Hitable {
	//HitableObjects objs;

	// Cringe C
	Hitable** list;
	int listSize;

	__device__ HitableList() {};
	__device__ HitableList(Hitable** input, int size) { 
		list = input; 
		listSize = size; 

		// Not gonna work cuz this doesn't exist on device
		//for (int i = 0; i < size; i++)
		//	objs.push_back(input[i]);
	};

	//__device__ HitableList(HitableList* list) { objs = list->objs; };
	//__device__ HitableList(HitableObject obj) { add(obj); };

	//void clear() { objs.clear(); }
	//
	//__device__
	//void add(HitableObject obj) {
	//	objs.push_back(obj);
	//}
	
	// TODO Fix this
	//__device__
	//void add(Hitable obj) {
	//	add(make_shared<Hitable>(obj));
	//}

	__device__ bool hit(const ray& r, float t_min, float t_max, HitInfo& rec) const {
		HitInfo temp_rec;
		bool hit_anything = false;
		float closest_so_far = t_max;
		for (int i = 0; i < listSize; i++) {
			if (list[i]->hit(r, t_min, closest_so_far, temp_rec)) {
				hit_anything = true;
				closest_so_far = temp_rec.t;
				rec = temp_rec;
			}
		}
		return hit_anything;
	}

	__device__ bool hit(const ray& r, interval rayT, HitInfo& info) const {
		HitInfo tempInfo;
		bool hitSomething = false;
		double currentClosest = rayT.max;

		for (int i = 0; i < listSize; i++) {
			if (list[i]->hit(r, interval(rayT.min, currentClosest), tempInfo)) {
				hitSomething = true;
				currentClosest = tempInfo.t;
				info = tempInfo;
			}
		}

		return hitSomething;
	}
	
	//bool hit(const ray& r, interval rayT, HitInfo& info) const {
	//	HitInfo tempInfo;
	//	bool hitSomething = false;
	//	double currentClosest = rayT.max;

	//	for (const HitableObject& obj : objs)
	//	{
	//		if (obj->hit(r, interval(rayT.min, currentClosest), tempInfo)) {
	//			hitSomething = true;
	//			currentClosest = tempInfo.t;
	//			info = tempInfo;
	//		}
	//	}

	//	return hitSomething;
	//}

};