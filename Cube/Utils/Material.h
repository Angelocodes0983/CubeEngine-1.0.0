#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
namespace Cube
{
	struct Material {
		float mass;
		float friction;

		Material()
			: mass(1.0f), friction(0.5f) {
		}  // default white, medium mass, normal friction

		Material(float m, float f)
			: mass(m), friction(f) {
		}
	};

	namespace Materials {
		static const Material Rubber = Material(0.3f, 0.5);
		static const Material Metal = Material(3.0f, 0.9f);
		static const Material Wood = Material(0.6f, 0.8f);
		static const Material Player = Material(0.9f, 2.0f);
		static const Material Ice = Material(0.5f, 0.01f);
		static const Material HeavyMetal = Material(100.0f, 0.5f);
	};
}
