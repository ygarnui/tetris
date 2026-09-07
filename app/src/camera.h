#pragma once

#include <raytracing.h>

#include <glm/glm.hpp>

#include <vector>

struct GLFWwindow;

namespace tetris
{
	/*!
	\brief An axis aligned box the player cannot walk into.
	*/
	struct Blocker
	{
		glm::vec3 min;
		glm::vec3 max;
	};

	/*!
	\brief First person camera walking on the floor.

	Mouse look plus WASD, with the eye height fixed: the player walks around the table, and
	the crosshair in the centre of the screen is what presses the buttons.
	*/
	class Camera
	{
	public:
		/*!
		\brief Take one frame of input and move the camera.
		\param[in] window the window the input is read from
		\param[in] deltaSeconds time since the previous frame
		*/
		void Update(GLFWwindow* window, const float deltaSeconds);

		/*!
		\brief Fill the constants the ray generation shader needs.
		\param[in] aspect width divided by height of the output image
		\return the camera block, ready to be uploaded
		*/
		[[nodiscard]] shaders::RtCamera MakeUniform(const float aspect, const bool aaEnabled) const;

		/*!
		\brief Set the boxes the player collides with, in world space.
		*/
		void SetBlockers(std::vector<Blocker> blockers);

		[[nodiscard]] glm::vec3 GetPosition() const noexcept { return position_; }
		[[nodiscard]] glm::vec3 GetForward() const noexcept;

		/*!
		\brief Direction the sun light travels towards.
		*/
		[[nodiscard]] static glm::vec3 GetLightDirection() noexcept;

	private:
		[[nodiscard]] bool isBlocked(const glm::vec3& position) const;

		glm::vec3 position_ = { 0.0f, 1.7f, 3.5f };

		float yaw_ = -90.0f;
		float pitch_ = -5.0f;

		double last_cursor_x_ = 0.0;
		double last_cursor_y_ = 0.0;
		bool has_cursor_ = false;

		/*! \brief Half the width of the player, used to keep them out of the blockers. */
		float radius_ = 0.25f;

		std::vector<Blocker> blockers_;
	};
}
