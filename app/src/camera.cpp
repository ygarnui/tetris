#include "camera.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>

namespace tetris
{

namespace
{
	constexpr float move_speed = 3.0f;
	constexpr float mouse_sensitivity = 0.12f;
	constexpr float pitch_limit = 89.0f;
	constexpr float eye_height = 1.7f;

	constexpr float field_of_view_degrees = 60.0f;
	constexpr float near_plane = 0.05f;
	constexpr float far_plane = 200.0f;
}

glm::vec3 Camera::GetLightDirection() noexcept
{
	// Slightly off vertical, so the scene gets readable shadows instead of flat lighting.
	return glm::normalize(glm::vec3(-0.45f, -1.0f, -0.35f));
}

glm::vec3 Camera::GetForward() const noexcept
{
	const float yaw = glm::radians(yaw_);
	const float pitch = glm::radians(pitch_);

	return glm::normalize(glm::vec3(
		std::cos(yaw) * std::cos(pitch),
		std::sin(pitch),
		std::sin(yaw) * std::cos(pitch)));
}

void Camera::SetBlockers(std::vector<Blocker> blockers)
{
	blockers_ = std::move(blockers);
}

bool Camera::isBlocked(const glm::vec3& position) const
{
	for (const Blocker& blocker : blockers_)
	{
		const bool insideX = position.x > blocker.min.x - radius_ && position.x < blocker.max.x + radius_;
		const bool insideZ = position.z > blocker.min.z - radius_ && position.z < blocker.max.z + radius_;

		// The player never leaves the floor, so only the footprint of the blocker matters.
		if (insideX && insideZ)
		{
			return true;
		}
	}

	return false;
}

void Camera::Update(GLFWwindow* window, const float deltaSeconds)
{
	double cursorX = 0.0;
	double cursorY = 0.0;
	glfwGetCursorPos(window, &cursorX, &cursorY);

	if (!has_cursor_)
	{
		last_cursor_x_ = cursorX;
		last_cursor_y_ = cursorY;
		has_cursor_ = true;
	}

	const float deltaX = static_cast<float>(cursorX - last_cursor_x_);
	const float deltaY = static_cast<float>(cursorY - last_cursor_y_);

	last_cursor_x_ = cursorX;
	last_cursor_y_ = cursorY;

	yaw_ += deltaX * mouse_sensitivity;
	pitch_ = std::clamp(pitch_ - deltaY * mouse_sensitivity, -pitch_limit, pitch_limit);

	const glm::vec3 forward = GetForward();
	// Movement stays in the floor plane even when looking up or down.
	const glm::vec3 flatForward = glm::normalize(glm::vec3(forward.x, 0.0f, forward.z));
	const glm::vec3 right = glm::normalize(glm::cross(flatForward, glm::vec3(0.0f, 1.0f, 0.0f)));

	glm::vec3 movement(0.0f);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { movement += flatForward; }
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) { movement -= flatForward; }
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { movement += right; }
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { movement -= right; }

	if (glm::dot(movement, movement) > 0.0f)
	{
		float speed = move_speed;
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		{
			speed *= 2.0f;
		}

		const glm::vec3 step = glm::normalize(movement) * speed * deltaSeconds;

		// Each axis is resolved on its own, so walking into the table slides along it
		// instead of stopping dead.
		const glm::vec3 alongX = { position_.x + step.x, position_.y, position_.z };
		if (!isBlocked(alongX))
		{
			position_.x = alongX.x;
		}

		const glm::vec3 alongZ = { position_.x, position_.y, position_.z + step.z };
		if (!isBlocked(alongZ))
		{
			position_.z = alongZ.z;
		}
	}

	position_.y = eye_height;
}

shaders::RtCamera Camera::MakeUniform(const float aspect, const bool aaEnabled) const
{
	const glm::vec3 forward = GetForward();

	const glm::mat4 view = glm::lookAt(position_, position_ + forward, glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 projection = glm::perspective(
		glm::radians(field_of_view_degrees),
		aspect,
		near_plane,
		far_plane);

	// Vulkan's clip space has y pointing down compared to the OpenGL convention glm uses.
	projection[1][1] *= -1.0f;

	shaders::RtCamera uniform{};
	uniform.view_inverse = glm::inverse(view);
	uniform.projection_inverse = glm::inverse(projection);
	uniform.light_direction = glm::vec4(GetLightDirection(), 0.0f);
	uniform.light_color = glm::vec4(1.0f, 0.97f, 0.90f, 0.0f);
	uniform.sky_color = glm::vec4(0.45f, 0.62f, 0.85f, 0.0f);
	uniform.ground_color = glm::vec4(0.12f, 0.12f, 0.14f, 0.0f);
	uniform.aa_samples = aaEnabled ? AA_SAMPLES : 1;
	
	return uniform;
}

}
