#include "creator_swapchain.h"
#include "creator_swapchain_support_details.h"
#include "selector_swapchain_settings.h"
#include "struct_base_data.h"

#include <logger_instance.h>

#include <stdexcept>

namespace render
{
std::shared_ptr<DataSwapchain> CreatorSwapchain::CreateSwapchain(
	const DataSwapchainSupportDetails& swapChainSupport,
	std::shared_ptr<DataDevice> device,
	VkPhysicalDevice physicalDevice,
	std::shared_ptr<DataSurface> surface,
	const VkExtent2D& extent,
	const QueueFamilyIndices& indices)
{
	VkSurfaceFormatKHR surfaceFormat;
	bool find = SelectorSwapchainSettings::ChooseSwapSurfaceFormat(
		swapChainSupport.formats,
		VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
		VK_FORMAT_R8G8B8A8_UNORM,
		surfaceFormat);

	if (!find)
	{
		LOG(Loglvl::info, "[CreatorSwapchain::CreateSwapchain] surface don't have format: VK_FORMAT_R8G8B8A8_SRGB and don't supports color space: VK_COLOR_SPACE_SRGB_NONLINEAR_KHR");
	}

	// TO DO
	//const VkPresentModeKHR presentMode = SelectorSwapchainSettings::ChooseSwapPresentMode(swapChainSupport.present_modes);
	const VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

	VkSwapchainCreateInfoKHR createInfo = createSwapchainCreateInfoKHR(
		surface,
		swapChainSupport.capabilities,
		surfaceFormat,
		extent,
		indices,
		presentMode);

	std::shared_ptr<DataSwapchain> dataSwapchain(
		new DataSwapchain{
			surfaceFormat,
			{},
			device,
			//todo surface deletion
			surface},
		[](DataSwapchain* p) {
			vkDestroySwapchainKHR(p->device->device, p->swapchain, nullptr);
			delete p;
		}
	);

	const VkResult result = vkCreateSwapchainKHR(device->device, &createInfo, nullptr, &dataSwapchain->swapchain);
	if (result != VK_SUCCESS)
	{
		LOGEXC(std::runtime_error, "[CreatorSwapchain::CreateSwapchain] Failed to create swap chain!");
	}

	return dataSwapchain;
}

VkSwapchainCreateInfoKHR CreatorSwapchain::createSwapchainCreateInfoKHR(
	std::shared_ptr<DataSurface> surface,
	VkSurfaceCapabilitiesKHR capabilities,
	VkSurfaceFormatKHR surfaceFormat,
	const VkExtent2D& extent,
	const QueueFamilyIndices& indices,
	const VkPresentModeKHR presentMode)
{
	VkSwapchainCreateInfoKHR createInfo{};

	uint32_t imageCount = capabilities.minImageCount + 1;

	if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
		imageCount = capabilities.maxImageCount;
	}

	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface->surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	// The ray generation shader writes its result straight into the swapchain image, so the
	// images are storage capable. This is why the swapchain format above must stay a UNORM
	// one: storage images may not use an sRGB format.
	createInfo.imageUsage =
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
		VK_IMAGE_USAGE_STORAGE_BIT;


	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily) 
	{
		// TO DO process this case
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
		LOGEXC(std::runtime_error, "[CreatorSwapchain::createSwapchainCreateInfoKHR]");
	}
	else 
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	createInfo.preTransform = capabilities.currentTransform;

	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = VK_NULL_HANDLE;


	return createInfo;
}

}
