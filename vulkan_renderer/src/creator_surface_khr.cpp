#include "creator_surface_khr.h"
#include "struct_base_data.h"

#include <logger_instance.h>

namespace render
{
std::shared_ptr<DataSurface> CreatorSurfaceKHR::CreateSurfaceKHR(
    std::shared_ptr<DataInstance> instance,
    const std::function<DataResult(std::shared_ptr<DataInstance>, std::shared_ptr<DataSurface>)>& createWindowSurface,
    bool externalDeleter)
{
    std::shared_ptr<DataSurface> surface;
    if (externalDeleter)
    {
        surface = std::make_shared<DataSurface>(DataSurface({}, instance ));
    }
    else
    {
        std::shared_ptr<DataSurface> newSurface(
            new DataSurface{{}, instance }, 
            [](DataSurface* p) 
            {
                vkDestroySurfaceKHR(p->instance->instance, p->surface, nullptr);
                delete p;
            });
        surface = newSurface;
    }

    auto res = createWindowSurface(instance, surface);
    if (res.result != VK_SUCCESS)
    {
        LOGEXC(std::runtime_error, "[CreatorSurfaceKHR::CreateSurfaceKHR]");
    }

    return surface;
}
}