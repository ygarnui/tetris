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
	\brief A world space ray, for CPU-side picking (e.g. clicking a cabinet button).
	*/
	struct Ray
	{
		glm::vec3 origin;
		glm::vec3 direction;
	};

	/*!
	\brief First person camera walking on the floor.

	WASD to walk, eye height fixed. Mouse look is decoupled from the cursor: holding the
	right mouse button drags the view, while the cursor is otherwise free to roam the screen
	so it can click the cabinet's buttons (see the picking code in main.cpp).
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

		/*!
		\brief The world space ray through a point on the screen, for CPU-side picking.

		Mirrors the primary ray computed per pixel in raytracing.rgen, evaluated here for one
		specific point (the cursor) instead of every pixel.
		\param[in] pixel cursor position in the same coordinate space as \p windowSize (top-left
		origin, y down - what glfwGetCursorPos/glfwGetWindowSize report)
		\param[in] windowSize width/height of that same coordinate space
		\param[in] aspect width divided by height of the rendered image
		*/
		[[nodiscard]] Ray ScreenPointToRay(const glm::vec2& pixel, const glm::vec2& windowSize, const float aspect) const;

		/*!
		\brief Whether the view is currently being dragged (right mouse button held).

		While looking around, the cursor is captured for mouse-look deltas rather than
		pointing at anything on screen, so button picking is suppressed during this.
		*/
		[[nodiscard]] bool IsLooking() const noexcept { return right_mouse_was_down_; }

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
		bool right_mouse_was_down_ = false;

		/*! \brief Half the width of the player, used to keep them out of the blockers. */
		float radius_ = 0.25f;

		std::vector<Blocker> blockers_;
	};
}
