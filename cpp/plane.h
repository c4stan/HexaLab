#pragma once

#include "common.h"
#include <eigen/dense>

namespace HexaLab {
    using namespace Eigen;

	// -------------------------------------------------------------------------------------
	// Simple plane class, used to cull mesh parts
    class Plane {
		Vector3f normal;
		float d;

	public:
		Plane(Vector3f& position, Vector3f& normal);
		Plane(float x, float y, float z, float nx, float ny, float nz);
		Plane();

		void move(float x, float y, float z);
		void orient(float nx, float ny, float nz);
		float solve(Vector3f& point);
	};
}