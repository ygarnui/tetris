#pragma once

#include "render_id.h"

namespace render
{
class VertexBufferId : public RenderId
{
public:
    VertexBufferId() = default;
private:
    VertexBufferId(const RenderId& id) { id_ = id.GetId(); }
    friend class GeneratorId;
};

class IndexBufferId : public RenderId
{
public:
    IndexBufferId() = default;
private:
    IndexBufferId(const RenderId& id) { id_ = id.GetId(); }
    friend class GeneratorId;
};

class CommandBufferId : public RenderId
{
public:
    CommandBufferId() = default;
private:
    CommandBufferId(const RenderId& id) { id_ = id.GetId(); }
    friend class GeneratorId;
};

class UniformBufferId : public RenderId
{
public:
    UniformBufferId() = default;
private:
    UniformBufferId(const RenderId& id) { id_ = id.GetId(); }
    friend class GeneratorId;
};

class DescriptorSetId : public RenderId
{
public:
    DescriptorSetId() = default;
private:
    DescriptorSetId(const RenderId& id) { id_ = id.GetId(); }
    friend class GeneratorId;
};

class GraphicsWindowId : public RenderId
{
public:
    GraphicsWindowId() = default;
private:
    GraphicsWindowId(const RenderId& id) { id_ = id.GetId(); }
    friend class GeneratorId;
};

class LogicalDeviceId : public RenderId
{
public:
    LogicalDeviceId() = default;
private:
    LogicalDeviceId(const RenderId& id) { id_ = id.GetId(); }
    friend class GeneratorId;
};

class ShaderProgramId : public RenderId
{
public:
    ShaderProgramId() = default;
private:
	ShaderProgramId(const RenderId& id) { id_ = id.GetId(); }
    friend class GeneratorId;
};

class SurfaceId : public RenderId
{
public:
    SurfaceId() = default;
private:
    SurfaceId(const RenderId& id) { id_ = id.GetId(); }
    friend class GeneratorId;
};

class PhysicalDeviceId : public RenderId
{
public:
    PhysicalDeviceId() = default;
private:
    PhysicalDeviceId(const RenderId& id) { id_ = id.GetId(); }
    friend class GeneratorId;
};

class RenderPassId : public RenderId
{
public:
    RenderPassId() = default;
private:
    RenderPassId(const RenderId& id) { id_ = id.GetId(); }
    friend class GeneratorId;
};

class SwapchainId : public RenderId
{
public:
    SwapchainId() = default;
private:
    SwapchainId(const RenderId& id) { id_ = id.GetId(); }
    friend class GeneratorId;
};

class FrameBufferId : public RenderId
{
public:
    FrameBufferId() = default;
private:
    FrameBufferId(const RenderId& id) { id_ = id.GetId(); }
    friend class GeneratorId;
};

class TextureId : public RenderId
{
public:
    TextureId() = default;
private:
    TextureId(const RenderId& id) { id_ = id.GetId(); }
    friend class GeneratorId;
};

class SamplerId : public RenderId
{
public:
    SamplerId() = default;
private:
    SamplerId(const RenderId& id) { id_ = id.GetId(); }
    friend class GeneratorId;
};

class DrawcallId : public RenderId
{
public:
    DrawcallId() = default;
private:
    DrawcallId(const RenderId& id) { id_ = id.GetId(); }
    friend class GeneratorId;
};

}
