#pragma once

#include <filesystem>
#include <functional>

#include "buffer_description.h"
#include "cmd_calls.h"
#include "data_shader_module_reflection.h"
#include "draw_priority.h"
#include "interface_image.h"
#include "interface_manager_asset_render.h"
#include "manager_buffer.h"
#include "manager_drawcall.h"
#include "manager_command_buffer.h"
#include "manager_frame_buffer.h"
#include "manager_textures.h"
#include "manager_uniform_buffer.h"
#include "manager_pipeline.h"
#include "manager_render_pass.h"
#include "manager_shader_program.h"
#include "renderpass_create_info.h"

namespace render
{
    struct DataInstance;
    struct DataResult;
    struct DataSurface;

    class RenderBase
    {
    public:

        RenderBase() {};

        virtual ~RenderBase() {}

        virtual std::shared_ptr<DataInstance> GetInstance() = 0;

        virtual void DrawFrame() = 0;

        virtual void DeviceWaitIdle() = 0;

        virtual void Resize(const GraphicsWindowId& windowId, const uint32_t width, const uint32_t height) = 0;

        virtual GraphicsWindowId AddWindow(
            const std::function<DataResult(std::shared_ptr<DataInstance>, std::shared_ptr<DataSurface>)>& createWindowSurface,
            uint32_t width,
            uint32_t height,
            float windowRatio,
            bool externalDeleter) = 0;

        virtual GraphicsWindowId AddWindow(
            uint32_t width,
            uint32_t height,
            uint32_t numImages) = 0;

        virtual void DeleteWindow(const GraphicsWindowId& windowId) = 0;

        virtual void SetWindowRatio(const GraphicsWindowId& idWindow, float windowRatio) = 0;

        virtual bool HasOneDevice(const GraphicsWindowId& windowId1, const GraphicsWindowId& windowId2) = 0;

        [[nodiscard]] virtual CommandBufferId CreateCommandBuffer(const GraphicsWindowId& windowId) = 0;

        [[nodiscard]] virtual std::shared_ptr<ManagerDrawcall> GetManagerDrawcall() = 0;
        [[nodiscard]] virtual std::shared_ptr<ManagerFrameBuffer> GetManagerFrameBuffer() = 0;
        [[nodiscard]] virtual std::shared_ptr<ManagerCommandBuffer> GetManagerCommandBuffer() = 0;
        [[nodiscard]] virtual std::shared_ptr<ManagerTextures> GetManagerTextures() = 0;
        [[nodiscard]] virtual std::shared_ptr<ManagerBuffer> GetManagerBuffer() = 0;
        [[nodiscard]] virtual std::shared_ptr<ManagerUniformBuffer> GetManagerUniformBuffer() = 0;
        [[nodiscard]] virtual std::shared_ptr<ManagerPipeline> GetManagerPipeline() = 0;
        [[nodiscard]] virtual std::shared_ptr<ManagerShaderProgram> GetManagerShaderProgram() = 0;
        [[nodiscard]] virtual std::shared_ptr<ManagerRenderPass> GetManagerRenderPass() = 0;

        [[nodiscard]] virtual const float GetAspect(const GraphicsWindowId& windowId) const = 0;

        [[nodiscard]] virtual const std::pair<uint32_t, uint32_t> GetWindowSize(const GraphicsWindowId& windowId) const = 0;

        [[nodiscard]] virtual const std::shared_ptr<general::InterfaceManagerAssetRender> GetManagerAsset() const = 0;

        [[nodiscard]] virtual const std::string GetNamePhysicalDevice(const GraphicsWindowId& windowId) const = 0;

        /*!
        \brief Get pixel data of the window framebuffer.
        \param windowId is the id of an existing window.
        \param alpha indicates whether the texture data will contain the alpha channel of the texture.
        \param frameBufferId where data comes from
        \return the info struct, which contains all information about the texture and a pointer to an array of pixel bytes.
        */
        [[nodiscard]] virtual std::shared_ptr<image::InterfaceImage> GetWindowPixelData(const GraphicsWindowId& windowId, bool alpha, FrameBufferId frameBufferId) = 0;
    };
}
