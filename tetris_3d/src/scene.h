#pragma once

#include "camera.h"
#include "geometry.h"

#include <raytracing/creator_acceleration_structure.h>

#include <board.h>
#include <types.h>

#include <glm/glm.hpp>

#include <memory>
#include <string>
#include <unordered_map>
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
		\param[in] textureIndexByBoxName index into RayTracingPassDescription::textures for a
		box's top face, keyed by Box::name; a box not listed here shades from its albedo alone
		\throw runtime_error if any Vulkan resource cannot be created
		*/
		void Build(
			const render::BuildContext& context,
			const std::unordered_map<std::string, uint32_t>& textureIndexByBoxName = {});

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

		/*!
		\brief Rebuild the board, the active piece, the next piece preview and the score
		display as small cubes, and the top level acceleration structure to match.

		The caller must have waited for the device to go idle first (see the call site in
		main.cpp) and must follow this with ManagerRayTracing::UpdateTopLevel: the old top
		level is replaced here, and both it and the instance buffer's old contents must not
		still be read by an in-flight frame when that happens.
		\param[in] context the device the resources are (re)created on
		\param[in] board the current playfield
		\param[in] activeType shape of the currently falling piece
		\param[in] activeRotation orientation of the currently falling piece
		\param[in] activePosition top-left of the falling piece's 4x4 box, in board cell coordinates
		\param[in] nextType shape shown on the next-piece preview
		\param[in] score shown on the score display, clamped to what its digit count can hold
		*/
		void UpdateGameplay(
			const render::BuildContext& context,
			const tetris::game::Board& board,
			tetris::game::PieceType activeType,
			tetris::game::Rotation activeRotation,
			tetris::game::Point activePosition,
			tetris::game::PieceType nextType,
			int score);

	private:
		void createBoxes();
		void createSpheres();

		std::vector<Box> boxes_;
		std::vector<Blocker> blockers_;
		std::vector<Box> spheres_;

		/*! \brief World space placement of the "screen"/"screen_next"/"screen_score" boxes. */
		glm::vec3 screen_center_{};
		glm::vec3 screen_size_{};
		glm::vec3 next_screen_center_{};
		glm::vec3 next_screen_size_{};
		glm::vec3 score_screen_center_{};
		glm::vec3 score_screen_size_{};

		Mesh cube_;
		render::DataBottomLevel bottom_level_;

		Mesh sphere_;
		render::DataBottomLevel bottom_level_sphere_;

		/*! \brief The floor/table/console/buttons/lamp, rebuilt into every UpdateGameplay call. */
		std::vector<VkAccelerationStructureInstanceKHR> static_instances_;
		std::vector<shaders::RtInstance> static_instance_data_;

		std::shared_ptr<render::DataAccelerationStructure> top_level_;
		render::BufferWithMemory instance_buffer_;
	};
}
