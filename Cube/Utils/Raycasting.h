#pragma once
#include "Cube.h";


namespace Cube
{
	



	size_t raycastForPhysicsId(float maxDist = 500.0f) {

		auto hit = Physics::RaycastPhysics(
			Renderer::camera.getPosition(),
			Renderer::camera.getFront(),
			maxDist
		);

		if (!hit.hit) return 0;
		return hit.objectId;
	}

	size_t raycastFromMouseForPhysicsId(float maxDist = 5000.0f) {
	
		double xpos = 0;
		double ypos = 0;
		glfwGetCursorPos(Renderer::window, &xpos, &ypos);

		int lWidth, lHeight;
		glfwGetFramebufferSize(Renderer::window, &lWidth, &lHeight);

		int winWidth, winHeight;
		glfwGetWindowSize(Renderer::window, &winWidth, &winHeight);

		xpos *= (double)lWidth / winWidth;
		ypos *= (double)lHeight / winHeight;

		float x = (2.0f * xpos) / lWidth - 1.0f;
		float y = 1.0f - (2.0f * ypos) / lHeight;
		float z = 1.0f;
		glm::vec3 ray_nds = glm::vec3(x, y, z);
		glm::vec4 ray_clip = glm::vec4(glm::vec2(ray_nds.x, ray_nds.y), -1.0, 1.0);

		float aspect = (float)lWidth / (float)lHeight;
		glm::vec4 ray_eye = inverse(Renderer::camera.getProjectionMatrix(aspect)) * ray_clip;
		ray_eye = glm::vec4(glm::vec2(ray_eye.x, ray_eye.y), -1.0, 0.0);
		glm::vec3 ray_wor = glm::vec3((inverse(Renderer::camera.getViewMatrix()) * ray_eye));
		ray_wor = glm::normalize(ray_wor);
		glm::vec3 rayDir = ray_wor;


		glm::vec3 direction = rayDir;

		auto hit = Physics::RaycastPhysics(
			Renderer::camera.getPosition(),
			direction,
			maxDist
		);

		if (!hit.hit)
			return 0;

		if (hit.objectId == 0)
			return 0;
		


		return hit.objectId;
	}


	glm::vec3 voxelRayCast(size_t id, std::vector<unsigned char>& voxels)
	{



		double xpos = 0;
		double ypos = 0;
		glfwGetCursorPos(Renderer::window, &xpos, &ypos);

		int lWidth, lHeight;
		glfwGetFramebufferSize(Renderer::window, &lWidth, &lHeight);

		int winWidth, winHeight;
		glfwGetWindowSize(Renderer::window, &winWidth, &winHeight);

		xpos *= (double)lWidth / winWidth;
		ypos *= (double)lHeight / winHeight;


		float x = (2.0f * xpos) / lWidth - 1.0f;
		float y = 1.0f - (2.0f * ypos) / lHeight;
		float z = 1.0f;
		glm::vec3 ray_nds = glm::vec3(x, y, z);
		glm::vec4 ray_clip = glm::vec4(glm::vec2(ray_nds.x, ray_nds.y), -1.0, 1.0);

		float aspect = (float)lWidth / (float)lHeight;
		glm::vec4 ray_eye = inverse(Renderer::camera.getProjectionMatrix(aspect)) * ray_clip;
		ray_eye = glm::vec4(glm::vec2(ray_eye.x, ray_eye.y), -1.0, 0.0);
		glm::vec3 ray_wor = glm::vec3((inverse(Renderer::camera.getViewMatrix()) * ray_eye));
		ray_wor = glm::normalize(ray_wor);
		glm::vec3 rayDir = ray_wor;



		glm::vec3 hitPos = glm::vec3(-1);
		glm::vec3 gridSize = ObjectManager::physicsIdToGridSize[id];
		glm::vec3 cameraPosGlobal = Renderer::camera.getPosition();


		glm::vec3 direction = rayDir;


		glm::vec3 localRo = glm::vec3(Renderer::voxelObjects[id].invTransform * glm::vec4(cameraPosGlobal, 1.0f));

		glm::vec3 localRd = glm::vec3(Renderer::voxelObjects[id].invTransform * glm::vec4(direction, 0.0));

		float localRdLen = length(localRd);
		glm::vec3 localRdNorm = localRd / localRdLen;

		int maxSteps = 1000;
		glm::vec3 pos = localRo;




		for (int i = 0; i < maxSteps; i++)
		{
			pos = pos + (localRdNorm);
			//std::cout << "step "<<i<<" is " << "X " << pos.x << "Y " << pos.y << "Z " << pos.z << std::endl;
			if (pos.x < 0 || pos.y < 0 || pos.z < 0 ||
				pos.x >= gridSize.x || pos.y >= gridSize.y || pos.z >= gridSize.z)
				continue;
			size_t idx = (size_t)pos.z * gridSize.x * gridSize.y
				+ (size_t)pos.y * gridSize.x + pos.x;

			if (voxels[idx] != 0)
			{
				hitPos = pos;
				break;
			}


		}
		return hitPos;

	}

}