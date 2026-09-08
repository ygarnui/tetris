#include "asset_manager.h"
#include "camera.h"
#include "scene.h"

#include <vulkan_render_base.h>

#include <buffers/creator_buffer.h>
#include <manager_device.h>
#include <manager_swapchain.h>
#include <manager_window.h>
#include <raytracing/manager_ray_tracing.h>

#include <logger_instance.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <chrono>
#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>
#include <vector>

namespace
{
	constexpr uint32_t initial_width = 1280;
	constexpr uint32_t initial_height = 720;

	/*!
	\brief What the GLFW resize callback needs to reach the renderer.
	*/
	struct WindowContext
	{
		render::RenderBase* render_base = nullptr;
		render::GraphicsWindowId window_id;
	};

	/*!
	\brief Collect the instance extensions GLFW needs to present into its window.
	*/
	std::vector<const char*> glfwInstanceExtensions()
	{
		uint32_t count = 0;
		const char** extensions = glfwGetRequiredInstanceExtensions(&count);

		if (!extensions)
		{
			throw std::runtime_error("GLFW could not report the required Vulkan instance extensions");
		}

		return std::vector<const char*>(extensions, extensions + count);
	}
}

int main()
{
	try
	{
		if (!glfwInit())
		{
			std::cerr << "failed to initialise GLFW" << std::endl;
			return 1;
		}

		if (!glfwVulkanSupported())
		{
			std::cerr << "no Vulkan loader found" << std::endl;
			glfwTerminate();
			return 1;
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		GLFWwindow* window = glfwCreateWindow(initial_width, initial_height, "Tetris 3D", nullptr, nullptr);
		if (!window)
		{
			std::cerr << "failed to create the window" << std::endl;
			glfwTerminate();
			return 1;
		}

		// The crosshair in the centre of the screen is the pointer, so the cursor is captured.
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		std::filesystem::create_directories(TETRIS_SHADER_CACHE_PATH);

		auto assetManager = std::make_shared<tetris::AssetManager>();
		auto renderBase = std::make_shared<render::VulkanRenderBase>(glfwInstanceExtensions(), assetManager);

		const render::GraphicsWindowId windowId = renderBase->AddWindow(
			[window](std::shared_ptr<render::DataInstance> instance, std::shared_ptr<render::DataSurface> surface)
			{
				render::DataResult result{};
				result.result = glfwCreateWindowSurface(instance->instance, window, nullptr, &surface->surface);
				return result;
			},
			initial_width,
			initial_height,
			1.0f,
			false);

		const render::SwapchainId swapchainId = render::ManagerWindow::Get()->GetSwapchainId(windowId);
		const render::LogicalDeviceId logicalDeviceId = render::ManagerWindow::Get()->GetLogicalDeviceId(windowId);
		const render::PhysicalDeviceId physicalDeviceId = render::ManagerWindow::Get()->GetPhysicalDeviceId(windowId);

		auto managerDevice = render::ManagerDevice::Get();
		auto managerSwapchain = render::ManagerSwapchain::Get();

		render::BuildContext buildContext{};
		buildContext.device = managerDevice->GetLogicalDevice(logicalDeviceId);
		buildContext.physical_device = managerDevice->GetPhysicalDevice(physicalDeviceId);
		buildContext.command_pool = managerDevice->GetCommandPool(logicalDeviceId)->command_pool;
		buildContext.queue = managerDevice->GetGraphicsQueue(logicalDeviceId);

		std::cout << "device: " << renderBase->GetNamePhysicalDevice(windowId) << std::endl;

		tetris::Scene scene;
		scene.Build(buildContext);

		tetris::Camera camera;
		camera.SetBlockers(scene.GetBlockers());

		const std::vector<description::ShaderDescription> shaderDescriptions = {
			{ "raytracing.rgen",  description::ShaderType::RAY_GENERATION,  {} },
			{ "raytracing.rmiss", description::ShaderType::RAY_MISS,        {} },
			{ "raytracing.rchit", description::ShaderType::RAY_CLOSEST_HIT, {} },
		};

		const render::ShaderProgramId shaderProgramId =
			renderBase->GetManagerShaderProgram()->CreateShaderProgram(windowId, shaderDescriptions);

		render::RayTracingPassDescription passDescription{};
		passDescription.window_id = windowId;
		passDescription.shader_program_id = shaderProgramId;
		passDescription.top_level = scene.GetTopLevel();
		// The pass owns one buffer of this size per swapchain image and reallocates them
		// itself if a resize changes how many images there are.
		passDescription.uniform_buffer_size = sizeof(shaders::RtCamera);
		passDescription.instance_buffer = scene.GetInstanceBuffer();
		passDescription.max_recursion_depth = 1;

		render::ManagerRayTracing::Get()->CreatePass(passDescription);

		// The whole frame is one command: bind the pipeline and trace into the swapchain image.
		const render::CommandBufferId commandBufferId = renderBase->CreateCommandBuffer(windowId);

		auto managerDrawcall = renderBase->GetManagerDrawcall();
		const render::DrawcallId drawcallId = managerDrawcall->CreateDrawCall();
		managerDrawcall->AddCommandInDrawCall(std::make_shared<render::CmdTraceRays>(windowId), drawcallId);

		auto managerCommandBuffer = renderBase->GetManagerCommandBuffer();
		managerCommandBuffer->AddDrawcallInCommandBuffer(windowId, drawcallId, commandBufferId);
		managerCommandBuffer->AddToDrawingQueue(windowId, commandBufferId, render::DrawPriority::First);

		std::cout << "scene: " << scene.GetBoxes().size() << " boxes, "
			<< "bounces: " << MAX_BOUNCES << std::endl;
		std::cout << "WASD to walk, mouse to look, Esc to quit" << std::endl;

		// Tell the renderer the new size so the swapchain is rebuilt against it. The actual
		// recreation happens inside the draw when the swapchain reports itself out of date.
		WindowContext windowContext{ renderBase.get(), windowId };
		glfwSetWindowUserPointer(window, &windowContext);
		glfwSetFramebufferSizeCallback(window, [](GLFWwindow* resized, int width, int height)
			{
				if (width <= 0 || height <= 0)
				{
					return;
				}

				auto* context = static_cast<WindowContext*>(glfwGetWindowUserPointer(resized));
				context->render_base->Resize(
					context->window_id,
					static_cast<uint32_t>(width),
					static_cast<uint32_t>(height));
			});

		auto previousTime = std::chrono::steady_clock::now();

		// Toggled by the 1 key; camera.MakeUniform still needs to be told about this so it can
		// feed a sample count through to the shader.
		bool aaEnabled = true;
		bool aaKeyWasDown = false;
		std::cout << "1 toggles anti-aliasing (currently " << (aaEnabled ? "on" : "off") << ")" << std::endl;

		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();

			if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			{
				glfwSetWindowShouldClose(window, GLFW_TRUE);
			}

			// Edge detected so holding the key down does not flip the state every frame.
			const bool aaKeyIsDown = glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS;
			if (aaKeyIsDown && !aaKeyWasDown)
			{
				aaEnabled = !aaEnabled;
				std::cout << "anti-aliasing: " << (aaEnabled ? "on" : "off") << std::endl;
			}
			aaKeyWasDown = aaKeyIsDown;

			const auto currentTime = std::chrono::steady_clock::now();
			const float deltaSeconds = std::chrono::duration<float>(currentTime - previousTime).count();
			previousTime = currentTime;

			int width = 0;
			int height = 0;
			glfwGetFramebufferSize(window, &width, &height);
			if (width == 0 || height == 0)
			{
				// Minimised: nothing to draw.
				continue;
			}

			camera.Update(window, deltaSeconds);

			// Handed over now, uploaded inside the draw once the swapchain image is acquired.
			const shaders::RtCamera cameraUniform = camera.MakeUniform(renderBase->GetAspect(windowId), aaEnabled);
			render::ManagerRayTracing::Get()->SetUniform(windowId, &cameraUniform, sizeof(cameraUniform));

			renderBase->DrawFrame();
		}

		renderBase->DeviceWaitIdle();

		glfwDestroyWindow(window);
		glfwTerminate();

		return 0;
	}
	catch (const std::exception& exception)
	{
		std::cerr << "fatal: " << exception.what() << std::endl;
		return 1;
	}
}
