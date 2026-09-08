#include "asset_manager.h"
#include "camera.h"
#include "scene.h"

#include <vulkan_render_base.h>

#include <buffers/creator_buffer.h>
#include <manager_device.h>
#include <manager_swapchain.h>
#include <manager_window.h>
#include <raytracing/manager_ray_tracing.h>
#include <textures/vulkan_manager_textures.h>

#include <buffer_description.h>
#include <image_loader.h>
#include <logger_instance.h>

#include <game.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <chrono>
#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
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

	/*!
	\brief Fire the single-click action a cabinet button performs.

	button_down is handled separately (SetSoftDrop, driven by whether the mouse is currently
	held down over it, not a single click).
	*/
	void HandleButtonClick(tetris::game::Game& game, const std::string& buttonName)
	{
		if (buttonName == "button_left") { game.MoveLeft(); }
		else if (buttonName == "button_right") { game.MoveRight(); }
		else if (buttonName == "button_rotate") { game.Rotate(); }
		else if (buttonName == "button_pause") { game.TogglePause(); }
		else if (buttonName == "button_start") { game.Start(); }
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

		// Cursor starts free so it can click the cabinet's buttons; Camera::Update captures
		// it (GLFW_CURSOR_DISABLED) only while the right mouse button drags the view.
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

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

		// Each cabinet button gets the texture matching its name; every other box's
		// RtInstance::texture_index stays RT_NO_TEXTURE (Scene::Build's default), so it keeps
		// shading from its plain albedo.
		const std::vector<std::pair<std::string, std::string>> buttonTextureFiles = {
			{ "button_left",   "left.png" },
			{ "button_right",  "right.png" },
			{ "button_rotate", "rotate.png" },
			{ "button_down",   "down.png" },
			{ "button_pause",  "pause.png" },
			{ "button_start",  "play.png" },
		};

		std::vector<VkDescriptorImageInfo> buttonTextureInfos;
		std::unordered_map<std::string, uint32_t> textureIndexByBoxName;

		for (const auto& [boxName, fileName] : buttonTextureFiles)
		{
			const description::ImportImageDescription imageDescription(std::string(TETRIS_TEXTURE_PATH) + fileName);
			const auto image = image::ImageLoader::CreateTextureImage(imageDescription, true);

			const render::TextureId textureId = render::VulkanManagerTextures::Get()->CreateTexture(windowId, image);

			VkDescriptorImageInfo imageInfo{};
			imageInfo.sampler = render::VulkanManagerTextures::Get()->GetSamplerData(textureId)->sampler;
			imageInfo.imageView = render::VulkanManagerTextures::Get()->GetImageViewData(textureId)->image_view;
			imageInfo.imageLayout = render::VulkanManagerTextures::Get()->GetImageLayout(textureId);

			textureIndexByBoxName[boxName] = static_cast<uint32_t>(buttonTextureInfos.size());
			buttonTextureInfos.push_back(imageInfo);
		}

		tetris::Scene scene;
		scene.Build(buildContext, textureIndexByBoxName);

		tetris::Camera camera;
		camera.SetBlockers(scene.GetBlockers());

		tetris::game::Game game;

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
		passDescription.textures = buttonTextureInfos;
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
		std::cout << "WASD to walk, hold right mouse button to look, Esc to quit" << std::endl;

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

		// Edge detected the same way as the AA toggle, so a held click fires a cabinet button
		// once rather than every frame.
		bool leftMouseWasDown = false;
		std::cout << "left click a cabinet button to press it" << std::endl;

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

			// While the view is being dragged, the cursor is captured for mouse-look deltas
			// rather than pointing at anything on screen, so button picking is skipped.
			if (camera.IsLooking())
			{
				leftMouseWasDown = false;
				game.SetSoftDrop(false);
			}
			else
			{
				int windowWidth = 0;
				int windowHeight = 0;
				glfwGetWindowSize(window, &windowWidth, &windowHeight);

				double cursorX = 0.0;
				double cursorY = 0.0;
				glfwGetCursorPos(window, &cursorX, &cursorY);

				const tetris::Ray pickRay = camera.ScreenPointToRay(
					glm::vec2(cursorX, cursorY),
					glm::vec2(windowWidth, windowHeight),
					renderBase->GetAspect(windowId));

				const tetris::Box* hoveredBox = scene.PickBox(pickRay);

				const bool leftMouseIsDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
				const bool leftMouseClicked = leftMouseIsDown && !leftMouseWasDown;
				leftMouseWasDown = leftMouseIsDown;

				if (leftMouseClicked && hoveredBox != nullptr)
				{
					HandleButtonClick(game, hoveredBox->name);
				}

				game.SetSoftDrop(leftMouseIsDown && hoveredBox != nullptr && hoveredBox->name == "button_down");
			}

			game.Tick(deltaSeconds);

			// The board and the active piece are re-rendered as small cubes every tick, which
			// means rebuilding the top level acceleration structure and rewriting the shared
			// instance buffer every tick too. Both are read by whatever frame(s) the GPU still
			// has in flight, so this waits for the device to go idle first rather than risk a
			// rebuild racing a pending traceRaysKHR - simple and correct, at the cost of a full
			// CPU/GPU sync point every tick (a non-issue for a scene this small, but exactly
			// the kind of shortcut a fuller "double buffer the moving parts" version would
			// remove later).
			renderBase->DeviceWaitIdle();
			scene.UpdateBoard(
				buildContext,
				game.GetBoard(),
				game.GetActivePieceType(),
				game.GetActivePieceRotation(),
				game.GetActivePiecePosition());
			render::ManagerRayTracing::Get()->UpdateTopLevel(windowId, scene.GetTopLevel());

			// The command buffer was recorded once with vkCmdBindDescriptorSets baked in, so
			// simply rewriting the descriptor via UpdateTopLevel is not enough - validation
			// flags the recorded buffer as referencing a since-destroyed acceleration
			// structure. Marking it dirty makes the next GetCommandBufferForDraw (inside
			// DrawFrame) reset and re-record it, freshly binding whatever the descriptor set
			// currently points at.
			managerCommandBuffer->RecreateCommandBuffer(windowId, commandBufferId);

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
