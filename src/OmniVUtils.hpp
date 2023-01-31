#pragma once

#include "defines.hpp"
#include "OmniVCamera.hpp"

namespace OmniV {

	struct CascadesGroup {
		std::array<float, SHADOWMAP_CASCADE_COUNT> splitDepths;
		std::array<glm::mat4, SHADOWMAP_CASCADE_COUNT> viewProjMats;
	};

	// from: https://stackoverflow.com/a/57595105
	template <typename T, typename... Rest>
	void hashCombine(std::size_t& seed, const T& v, const Rest&... rest) {
		seed ^= std::hash<T>{}(v)+0x9e3779b9 + (seed << 6) + (seed >> 2);
		(hashCombine(seed, rest), ...);
	};

	CascadesGroup shadowmapCascadesMatrices(glm::mat4 viewMat, const OmniVCamera& camera, float aspectRatio);

	// from: https://cplusplus.com/forum/beginner/275937/
	inline void print_mat4(glm::mat4 matrix) {
		for (int i = 0; i < 4; i++) {
			std::cout << std::endl;
			std::cout << "|";
			for (int j = 0; j < 4; j++) {
				if (j > 0) std::cout << " ";
				std::cout << matrix[i][j];
				if (j == 3) std::cout << "|";
			}
		}

		std::cout << std::endl;
		std::cout << "---------------------";
	}

}