#pragma once

#include "camera.h"
#include "geometry.h"

#include <raytracing/creator_acceleration_structure.h>

#include <glm/glm.hpp>

#include <memory>
#include <string>
#include <vector>

namespace tetris
{
	/*!
	\brief One axis aligned box of the scene.
	*/
	struct Box
	{
		std::string name;
		glm::vec3 center;
		glm::vec3 size;
		glm::vec3 albedo;
		float reflectivity = 0.0f;
	};

	/*!
	\brief The room: floor, table, the arcade cabinet on it, its screen and its buttons.

	Every object is a box, so the whole scene is one bottom level acceleration structure
	holding a unit cube, instanced once per box with a scaling transform. Rebuilding the
	scene is therefore only a top level rebuild, which is what moving pieces will need.
	*/
	class Scene
	{
	public:
		/*!
		\brief Create the geometry, the acceleration structures and the instance buffer.
		\param[in] context the device the resources are created on
		\throw runtime_error if any Vulkan resource cannot be created
		*/
		void Build(const render::BuildContext& context);

		[[nodiscard]] std::shared_ptr<render::DataAccelerationStructure> GetTopLevel() const { return top_level_; }

		[[nodiscard]] std::shared_ptr<render::DataBuffer> GetInstanceBuffer() const { return instance_buffer_.buffer; }

		/*!
		\brief Boxes the player is not allowed to walk through.
		*/
		[[nodiscard]] const std::vector<Blocker>& GetBlockers() const { return blockers_; }

		[[nodiscard]] const std::vector<Box>& GetBoxes() const { return boxes_; }

		/*!
		\brief The closest box the ray hits, for clicking a button under the cursor.
		\return the hit box, or nullptr if the ray hits none of them
		*/
		[[nodiscard]] const Box* PickBox(const Ray& ray) const;

	private:
		void createBoxes();
		void createSpheres();

		std::vector<Box> boxes_;
		std::vector<Blocker> blockers_;
		std::vector<Box> spheres_;

		Mesh cube_;
		render::DataBottomLevel bottom_level_;
		
		Mesh sphere_;
		render::DataBottomLevel bottom_level_sphere_;

		std::shared_ptr<render::DataAccelerationStructure> top_level_;
		render::BufferWithMemory instance_buffer_;
	};
}
