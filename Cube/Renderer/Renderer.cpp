#include "Renderer.h"
const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif
namespace Cube::Renderer
{
	
	void createDummyTexture() {
		std::vector<unsigned char> dummy = { 0 };
		ImagecreateImage(1, 1, 1, VK_FORMAT_R8_UINT,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			textureImage, textureImageMemory);

		ImagetransitionImageLayout(textureImage, VK_FORMAT_R8_UINT,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkBuffer staging; VkDeviceMemory stagingMem;
		createBuffer(1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			staging, stagingMem);

		void* data;
		vkMapMemory(device, stagingMem, 0, 1, 0, &data);
		memcpy(data, dummy.data(), 1);
		vkUnmapMemory(device, stagingMem);

		ImagecopyBufferToImage(staging, textureImage, 1, 1, 1);
		ImagetransitionImageLayout(textureImage, VK_FORMAT_R8_UINT,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);

		vkDestroyBuffer(device, staging, nullptr);
		vkFreeMemory(device, stagingMem, nullptr);

		ImageCreateImageView();
	}


	VkCommandBuffer beginSingleTimeCommands() {
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = computeCommandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		VkResult result = vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

		if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate command buffer! Error code: " + std::to_string(result));
		}

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to begin command buffer! Error code: " + std::to_string(result));
		}

		return commandBuffer;
	}


	void ImagecreateImage(uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
		VkImageCreateInfo imageInfo{};



		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_3D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = depth;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = VK_FORMAT_R8_UINT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT |
			VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

		if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
			throw std::runtime_error("image failed to load");

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
			throw std::runtime_error("failed to allocfate memory for image");

		vkBindImageMemory(device, image, imageMemory, 0);
	}

	void ImagetransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
			newLayout == VK_IMAGE_LAYOUT_GENERAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_GENERAL &&
			newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			sourceStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
			newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		}
		else {
			throw std::invalid_argument("unsupported layout transition!");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
		endSingleTimeCommands(commandBuffer);
	}



	void ImageCreateImageView() {
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = textureImage;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
		viewInfo.format = VK_FORMAT_R8_UINT;

		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.levelCount = 1;

		vkCreateImageView(device, &viewInfo, nullptr, &textureImageView);
	}


	void createVoxelSampler() {
		VkSamplerCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		info.magFilter = VK_FILTER_NEAREST;
		info.minFilter = VK_FILTER_NEAREST;
		info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		info.minLod = 0.0f;
		info.maxLod = VK_LOD_CLAMP_NONE;
		vkCreateSampler(device, &info, nullptr, &voxelSampler);
	}


	void ImagecreateDescriptorPool() {
		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		poolSize.descriptorCount = 1;

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;
		poolInfo.maxSets = 1;

		if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &ImagedescriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	void ImageDescriptorSet() {
		VkDescriptorSetLayoutBinding imageLayoutBinding{};
		imageLayoutBinding.binding = 1;
		imageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		imageLayoutBinding.descriptorCount = 1;
		imageLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		imageLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &imageLayoutBinding;

		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &ImagedescriptorSetLayout) != VK_SUCCESS)
			throw std::runtime_error("failed to create descriptor set layout!");

		ImagecreateDescriptorPool();

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = ImagedescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &ImagedescriptorSetLayout;

		if (vkAllocateDescriptorSets(device, &allocInfo, &ImagedescriptorSet) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor set!");
		}

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		imageInfo.imageView = textureImageView;
		imageInfo.sampler = VK_NULL_HANDLE;

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = ImagedescriptorSet;
		descriptorWrite.dstBinding = 1;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
	}




	//voxel materials pallette

	
	void createMaterialBuffer(const std::array<GPUMaterial, 256>& materials) {
			VkDeviceSize size = sizeof(GPUMaterial) * 256;

			VkBuffer staging;
			VkDeviceMemory stagingMem;
			createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				staging, stagingMem);

			void* data;
			vkMapMemory(device, stagingMem, 0, size, 0, &data);
			memcpy(data, materials.data(), size);
			vkUnmapMemory(device, stagingMem);

			createBuffer(size,
				VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				materialBuffer, materialBufferMemory);

			copyBuffer(staging, materialBuffer, size);
			vkDestroyBuffer(device, staging, nullptr);
			vkFreeMemory(device, stagingMem, nullptr);

			std::cout << "[Materials] Uploaded " << size << " bytes\n";
	}
	
	void createVoxelTexture(uint32_t index,
		const std::vector<unsigned char>& voxels,
		uint32_t w, uint32_t h, uint32_t d)
	{

		std::cout << "creatingImage" << std::endl;
		// Grow vectors to fit this index
		if (voxelImages.size() <= index) {
			voxelImages.resize(index + 1, VK_NULL_HANDLE);
			voxelImageMemories.resize(index + 1, VK_NULL_HANDLE);
			voxelImageViews.resize(index + 1, VK_NULL_HANDLE);
		}

		std::cout << "voxelImages Size" << voxelImages.size() << std::endl;

		// Destroy old resources at this slot if they exist
		if (voxelImages[index] != VK_NULL_HANDLE) {
			vkDestroyImageView(device, voxelImageViews[index], nullptr);
			vkDestroyImage(device, voxelImages[index], nullptr);
			vkFreeMemory(device, voxelImageMemories[index], nullptr);
			std::cout << "Destroyed old resources" << std::endl;
		}

		VkDeviceSize imageSize = (VkDeviceSize)w * h * d;
		std::cout << "imageSize" << imageSize << std::endl;

		// Staging buffer
		VkBuffer staging; VkDeviceMemory stagingMem;
		createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			staging, stagingMem);
		std::cout << "created Buffer" << std::endl;

		void* data;
		vkMapMemory(device, stagingMem, 0, imageSize, 0, &data);
		memcpy(data, voxels.data(), imageSize);
		vkUnmapMemory(device, stagingMem);

		// Create the 3D image (re use existing helper)
		ImagecreateImage(w, h, d, VK_FORMAT_R8_UINT,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			voxelImages[index], voxelImageMemories[index]);
		std::cout << "created 3D image" << std::endl;

		ImagetransitionImageLayout(voxelImages[index], VK_FORMAT_R8_UINT,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		ImagecopyBufferToImage(staging, voxelImages[index], w, h, d);
		ImagetransitionImageLayout(voxelImages[index], VK_FORMAT_R8_UINT,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);

		// Create image view
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = voxelImages[index];
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
		viewInfo.format = VK_FORMAT_R8_UINT;
		viewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		vkCreateImageView(device, &viewInfo, nullptr, &voxelImageViews[index]);
		std::cout << "Create image view" << std::endl;

		vkDestroyBuffer(device, staging, nullptr);
		vkFreeMemory(device, stagingMem, nullptr);

		std::cout << "[VoxelTexture] Slot " << index
			<< " created " << w << "x" << h << "x" << d << "\n";
	}


	void endSingleTimeCommands(VkCommandBuffer commandBuffer) {
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(computeQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(computeQueue);

		vkFreeCommandBuffers(device, computeCommandPool, 1, &commandBuffer);
	}
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) &&
				(memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("Failed to find suitable memory type!");
	}

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties, VkBuffer& buffer,
		VkDeviceMemory& bufferMemory) {

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate buffer memory!");
		}

		vkBindBufferMemory(device, buffer, bufferMemory, 0);
	}

	std::vector<const char*> getRequiredExtensions() {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}
	void createInstance() {
		if (enableValidationLayers && !checkValidationLayerSupport()) {
			throw std::runtime_error("Validation layers requested, but not available!");
		}

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Vulkan Swapchain";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else {
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create Vulkan instance!");
		}
	}

	void createSurface() {
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create window surface!");
		}
		std::cout << "Surface created successfully!" << std::endl;
	}



	bool checkValidationLayerSupport() {
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}

		return true;
	}

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
		return VK_FALSE;
	}

	void setupDebugMessenger() {
		if (!enableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);

		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			if (func(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
				throw std::runtime_error("Failed to set up debug messenger!");
			}
		}
		else {
			throw std::runtime_error("Failed to get vkCreateDebugUtilsMessengerEXT function!");
		}
	}

	void pickPhysicalDevice() {
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		if (deviceCount == 0) {
			throw std::runtime_error("Failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		for (const auto& device : devices) {
			if (isDeviceSuitable(device)) {
				physicalDevice = device;
				break;
			}
		}

		if (physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("Failed to find a suitable GPU!");
		}
	}

	bool isDeviceSuitable(VkPhysicalDevice device) {
		QueueFamilyIndices indices = findQueueFamilies(device);

		bool extensionsSupported = checkDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() &&
				!swapChainSupport.presentModes.empty();
		}

		return indices.isComplete() && extensionsSupported && swapChainAdequate;
	}

	bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}

			if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
				indices.computeFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (presentSupport) {
				indices.presentFamily = i;
			}

			if (indices.isComplete()) {
				break;
			}
			i++;
		}

		return indices;
	}
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
				availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
		for (const auto& mode : availablePresentModes) {
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
				std::cout << "Using MAILBOX present mode (triple buffering)\n";
				return mode;
			}
		}
		for (const auto& mode : availablePresentModes) {
			if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
				std::cout << "Using IMMEDIATE present mode (uncapped)\n";
				return mode;
			}
		}
		std::cout << "Using FIFO present mode (vsync)\n";
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {

		if (capabilities.currentExtent.width != UINT32_MAX) {
			return capabilities.currentExtent;
		}
		else {

			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width,
				capabilities.minImageExtent.width,
				capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height,
				capabilities.minImageExtent.height,
				capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

	void createSwapChain() {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 &&
			imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage =
			VK_IMAGE_USAGE_TRANSFER_DST_BIT |
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;


		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}


		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;


		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create swap chain!");
		}

		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;

		std::cout << "Swapchain created successfully!\n";
		std::cout << "  Images: " << swapChainImages.size() << "\n";
		std::cout << "  Format: " << swapChainImageFormat << "\n";
		std::cout << "  Extent: " << swapChainExtent.width << "x" << swapChainExtent.height << "\n";
	}

	void createLogicalDevice() {
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = {
			indices.graphicsFamily.value(),
			indices.computeFamily.value(),
			indices.presentFamily.value()
		};

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.shaderStorageImageWriteWithoutFormat = VK_TRUE;

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();


		VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{};
		indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
		indexingFeatures.shaderStorageImageArrayNonUniformIndexing = VK_TRUE;
		indexingFeatures.runtimeDescriptorArray = VK_TRUE;

		VkPhysicalDeviceFeatures2 features2{};
		features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		features2.features = deviceFeatures;
		features2.pNext = &indexingFeatures;


		createInfo.pEnabledFeatures = nullptr;
		createInfo.pNext = &features2;

		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create logical device!");
		}

		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(device, indices.computeFamily.value(), 0, &computeQueue);
		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	}

	void cleanupSwapChain() {
		vkDestroySwapchainKHR(device, swapChain, nullptr);
		std::cout << "Swapchain destroyed\n";
	}
	void cleanup() {
		vkDeviceWaitIdle(device);


		vkWaitForFences(device, 1, &graphicsFence, VK_TRUE, UINT64_MAX);
		vkWaitForFences(device, 1, &computeFence, VK_TRUE, UINT64_MAX);

		if (BrickNodeBuffer != VK_NULL_HANDLE) {
			vkDestroyBuffer(device, BrickNodeBuffer, nullptr);
			BrickNodeBuffer = VK_NULL_HANDLE;
		}
		if (BrickNodeBufferMemory != VK_NULL_HANDLE) {
			vkFreeMemory(device, BrickNodeBufferMemory, nullptr);
			BrickNodeBufferMemory = VK_NULL_HANDLE;
		}
		if (materialBuffer != VK_NULL_HANDLE) {
			vkDestroyBuffer(device, materialBuffer, nullptr);
			vkFreeMemory(device, materialBufferMemory, nullptr);
			materialBuffer = VK_NULL_HANDLE;
			materialBufferMemory = VK_NULL_HANDLE;
		}


		if (brickElementsBuffer != VK_NULL_HANDLE) {
			vkDestroyBuffer(device, brickElementsBuffer, nullptr);
			brickElementsBuffer = VK_NULL_HANDLE;
		}
		if (brickElementsBufferMemory != VK_NULL_HANDLE) {
			vkFreeMemory(device, brickElementsBufferMemory, nullptr);
			brickElementsBufferMemory = VK_NULL_HANDLE;
		}
		cleanupImGui();

		std::cout << "[Cleanup] Starting voxel image cleanup, count: " << voxelImages.size() << "\n";
		for (uint32_t i = 0; i < voxelImages.size(); i++) {
			std::cout << "[Cleanup] Destroying voxel image " << i << "\n";
			if (voxelImageViews[i] != VK_NULL_HANDLE) {
				vkDestroyImageView(device, voxelImageViews[i], nullptr);
				voxelImageViews[i] = VK_NULL_HANDLE;
			}
			if (voxelImages[i] != VK_NULL_HANDLE) {
				vkDestroyImage(device, voxelImages[i], nullptr);
				voxelImages[i] = VK_NULL_HANDLE;
			}
			if (voxelImageMemories[i] != VK_NULL_HANDLE) {
				vkFreeMemory(device, voxelImageMemories[i], nullptr);
				voxelImageMemories[i] = VK_NULL_HANDLE;
			}
			std::cout << "[Cleanup] Voxel image " << i << " done\n";
		}

		if (voxelObjectBuffer != VK_NULL_HANDLE) {
			vkDestroyBuffer(device, voxelObjectBuffer, nullptr);
			vkFreeMemory(device, voxelObjectBufferMemory, nullptr);
			voxelObjectBuffer = VK_NULL_HANDLE;
			voxelObjectBufferMemory = VK_NULL_HANDLE;
		}

		if (voxelSampler != VK_NULL_HANDLE) {
			vkDestroySampler(device, voxelSampler, nullptr);
			voxelSampler = VK_NULL_HANDLE;
		}

		std::cout << "[Cleanup] Deleting physics world...\n";
		//delete g_PhysicsWorld;
		//g_PhysicsWorld = nullptr;
		std::cout << "[Cleanup] Physics world deleted\n";


		std::cout << "[Cleanup] Destroying texture image view...\n";
		vkDestroyImageView(device, textureImageView, nullptr);
		std::cout << "[Cleanup] Destroying texture image...\n";
		vkDestroyImage(device, textureImage, nullptr);
		vkFreeMemory(device, textureImageMemory, nullptr);
		std::cout << "[Cleanup] Texture image destroyed\n";

		std::cout << "[Cleanup] Destroying image descriptor resources...\n";
		if (ImagedescriptorPool != VK_NULL_HANDLE) {
			vkDestroyDescriptorPool(device, ImagedescriptorPool, nullptr);
			ImagedescriptorPool = VK_NULL_HANDLE;
		}
		if (ImagedescriptorSetLayout != VK_NULL_HANDLE) {
			vkDestroyDescriptorSetLayout(device, ImagedescriptorSetLayout, nullptr);
			ImagedescriptorSetLayout = VK_NULL_HANDLE;
		}

		vkDestroySemaphore(device, computeFinishedSemaphore, nullptr);
		vkDestroyFence(device, computeFence, nullptr);
		std::cout << "Synchronization objects destroyed\n";

		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
		std::cout << "Descriptor pool destroyed\n";

		vkFreeCommandBuffers(device, computeCommandPool, 1, &computeCommandBuffer);
		vkDestroyCommandPool(device, computeCommandPool, nullptr);

		vkDestroyPipeline(device, computePipeline, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
		vkDestroyShaderModule(device, computeShaderModule, nullptr);

		vkDestroyImageView(device, storageImageView, nullptr);
		vkDestroyImage(device, storageImage, nullptr);
		vkFreeMemory(device, storageImageMemory, nullptr);

		cleanupSwapChain();

		vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
		vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
		vkDestroyFence(device, graphicsFence, nullptr);
		vkFreeCommandBuffers(device, graphicsCommandPool, 1, &graphicsCommandBuffer);
		vkDestroyCommandPool(device, graphicsCommandPool, nullptr);



		vkDestroyDevice(device, nullptr);
		std::cout << "Logical device destroyed\n";

		vkDestroySurfaceKHR(instance, surface, nullptr);




		if (enableValidationLayers) {
			auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
				instance, "vkDestroyDebugUtilsMessengerEXT");
			if (func != nullptr)
				func(instance, debugMessenger, nullptr);
		}

		vkDestroyInstance(instance, nullptr);
		glfwDestroyWindow(window);
		glfwTerminate();
		std::cout << "Cleanup complete\n";
	}



	void createImage(uint32_t width, uint32_t height, VkFormat format,
		VkImageUsageFlags usage, VkImage& image, VkDeviceMemory& imageMemory) {

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0;

		if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate image memory!");
		}

		vkBindImageMemory(device, image, imageMemory, 0);
	}

	void createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
		VkImageView& imageView) {

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create texture image view!");
		}
	}




	void createStorageImage() {

		VkFormat storageImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

		createImage(swapChainExtent.width, swapChainExtent.height,
			storageImageFormat,
			VK_IMAGE_USAGE_STORAGE_BIT |
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
			VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			storageImage, storageImageMemory);

		createImageView(storageImage, storageImageFormat,
			VK_IMAGE_ASPECT_COLOR_BIT, storageImageView);

		std::cout << "Storage image created: "
			<< swapChainExtent.width << "x" << swapChainExtent.height
			<< " format: " << storageImageFormat << "\n";
	}

	void createDescriptorSetLayout() {

		VkDescriptorSetLayoutBinding storageImageBinding{};
		storageImageBinding.binding = 0;
		storageImageBinding.descriptorCount = 1;
		storageImageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		storageImageBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		VkDescriptorSetLayoutBinding imageBinding{};
		imageBinding.binding = 1;
		imageBinding.descriptorCount = MAX_VOXEL_OBJECTS;  // <-- was 1
		imageBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		imageBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		VkDescriptorSetLayoutBinding octreeNodeBufferBunding{};
		octreeNodeBufferBunding.binding = 2;
		octreeNodeBufferBunding.descriptorCount = 1;
		octreeNodeBufferBunding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		octreeNodeBufferBunding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;


		VkDescriptorSetLayoutBinding materialBinding{};
		materialBinding.binding = 3;
		materialBinding.descriptorCount = 1;
		materialBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		materialBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;


		// Add binding 4 for VoxelObject SSBO
		VkDescriptorSetLayoutBinding voxelObjectBinding{};
		voxelObjectBinding.binding = 4;
		voxelObjectBinding.descriptorCount = 1;
		voxelObjectBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		voxelObjectBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		VkDescriptorSetLayoutBinding brickElementsBufferBunding{};
		brickElementsBufferBunding.binding = 5;
		brickElementsBufferBunding.descriptorCount = 1;
		brickElementsBufferBunding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		brickElementsBufferBunding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		std::array<VkDescriptorSetLayoutBinding, 6> bindings = {
			storageImageBinding, imageBinding,
			octreeNodeBufferBunding, materialBinding, voxelObjectBinding, brickElementsBufferBunding
		};

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create descriptor set layout!");
		}

	}
	static std::vector<char> readFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("Failed to open file: " + filename);
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		return buffer;
	}

	void createComputePipeline() {

		auto computeShaderCode = readFile("fill.comp.spv");


		std::cout << "Shader code size: " << computeShaderCode.size() << " bytes\n";

		if (computeShaderCode.empty()) {
			throw std::runtime_error("Failed to read shader file or file is empty!");
		}


		VkShaderModuleCreateInfo shaderModuleInfo{};
		shaderModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleInfo.codeSize = computeShaderCode.size();
		shaderModuleInfo.pCode = reinterpret_cast<const uint32_t*>(computeShaderCode.data());

		if (vkCreateShaderModule(device, &shaderModuleInfo, nullptr, &computeShaderModule) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create compute shader module!");
		}

		std::cout << "Compute shader module created\n";


		VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
		computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		computeShaderStageInfo.module = computeShaderModule;
		computeShaderStageInfo.pName = "main";


		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(CameraData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create pipeline layout!");
		}

		std::cout << "Pipeline layout created with push constants\n";

		VkComputePipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.stage = computeShaderStageInfo;
		pipelineInfo.layout = pipelineLayout;

		if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &computePipeline) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create compute pipeline!");
		}

		std::cout << "Compute pipeline created\n";
	}


	///step 11
	///step 11
	///step 11
	///step 11
	/// 
	/// 

	void createCommandPool() {
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = indices.computeFamily.value();
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (vkCreateCommandPool(device, &poolInfo, nullptr, &computeCommandPool) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create command pool!");
		}

		std::cout << "Compute command pool created\n";
	}

	void allocateCommandBuffers() {
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = computeCommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(device, &allocInfo, &computeCommandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate command buffers!");
		}

		std::cout << "Compute command buffer allocated\n";
	}

	void createDescriptorResources() {
		std::cout << "\n=== DEBUG: Creating Descriptor Resources ===\n";

		std::array<VkDescriptorPoolSize, 3> poolSizes{};

		poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		poolSizes[0].descriptorCount = 1; // just binding 0

		VkDescriptorPoolSize samplerPoolSize{};
		samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerPoolSize.descriptorCount = voxelImages.size();


		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(voxelImages.size());

		poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		poolSizes[2].descriptorCount = 3;

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 3;
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = 1;

		if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create descriptor pool!");
		}

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &descriptorSetLayout;

		if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate descriptor set!");
		}



		VkDescriptorImageInfo imageInfo0{};
		imageInfo0.imageView = storageImageView;
		imageInfo0.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		VkWriteDescriptorSet imageWrite0{};
		imageWrite0.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		imageWrite0.dstSet = descriptorSet;
		imageWrite0.dstBinding = 0;
		imageWrite0.dstArrayElement = 0;
		imageWrite0.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		imageWrite0.descriptorCount = 1;
		imageWrite0.pImageInfo = &imageInfo0;


		VkDescriptorImageInfo imageInfo1{};
		imageInfo1.imageView = textureImageView;
		imageInfo1.imageLayout = VK_IMAGE_LAYOUT_GENERAL;






		VkDescriptorBufferInfo BrickNodeBufferInfo{};
		BrickNodeBufferInfo.buffer = BrickNodeBuffer;
		BrickNodeBufferInfo.offset = 0;
		BrickNodeBufferInfo.range = VK_WHOLE_SIZE;

		VkWriteDescriptorSet BrickNodeBufferWrite{};
		BrickNodeBufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		BrickNodeBufferWrite.dstSet = descriptorSet;
		BrickNodeBufferWrite.dstBinding = 2;
		BrickNodeBufferWrite.dstArrayElement = 0;
		BrickNodeBufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		BrickNodeBufferWrite.descriptorCount = 1;
		BrickNodeBufferWrite.pBufferInfo = &BrickNodeBufferInfo;

		VkDescriptorBufferInfo materialInfo{};
		materialInfo.buffer = materialBuffer;
		materialInfo.offset = 0;
		materialInfo.range = VK_WHOLE_SIZE;

		VkWriteDescriptorSet materialWrite{};
		materialWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		materialWrite.dstSet = descriptorSet;
		materialWrite.dstBinding = 3;
		materialWrite.dstArrayElement = 0;
		materialWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		materialWrite.descriptorCount = 1;
		materialWrite.pBufferInfo = &materialInfo;


		VkDescriptorBufferInfo brickElementsBufferInfo{};
		brickElementsBufferInfo.buffer = brickElementsBuffer;
		brickElementsBufferInfo.offset = 0;
		brickElementsBufferInfo.range = VK_WHOLE_SIZE;

		VkWriteDescriptorSet brickElementsBufferWrite{};
		brickElementsBufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		brickElementsBufferWrite.dstSet = descriptorSet;
		brickElementsBufferWrite.dstBinding = 5;
		brickElementsBufferWrite.dstArrayElement = 0;
		brickElementsBufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		brickElementsBufferWrite.descriptorCount = 1;
		brickElementsBufferWrite.pBufferInfo = &brickElementsBufferInfo;

		std::vector<VkDescriptorImageInfo> voxImageInfos(voxelImages.size());
		for (uint32_t i = 0; i < voxelImages.size(); i++) {


			VkImageView fallback = (i < voxelImageViews.size() && voxelImageViews[i] != VK_NULL_HANDLE)
				? voxelImageViews[i]
				: textureImageView;
				voxImageInfos[i].imageView = fallback;
				voxImageInfos[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
				voxImageInfos[i].sampler = voxelSampler;
				voxImageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


		}


		VkWriteDescriptorSet imageWrite1{};
		imageWrite1.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		imageWrite1.dstSet = descriptorSet;
		imageWrite1.dstBinding = 1;
		imageWrite1.dstArrayElement = 0;
		imageWrite1.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;;
		imageWrite1.descriptorCount = voxelImages.size();
		imageWrite1.pImageInfo = voxImageInfos.data();


		VkDescriptorBufferInfo voxObjInfo{};
		voxObjInfo.buffer = voxelObjectBuffer;
		voxObjInfo.offset = 0;
		voxObjInfo.range = VK_WHOLE_SIZE;

		VkWriteDescriptorSet voxObjWrite{};
		voxObjWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		voxObjWrite.dstSet = descriptorSet;
		voxObjWrite.dstBinding = 4;
		voxObjWrite.dstArrayElement = 0;
		voxObjWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		voxObjWrite.descriptorCount = 1;
		voxObjWrite.pBufferInfo = &voxObjInfo;



		std::array<VkWriteDescriptorSet, 6> descriptorWrites = {
			imageWrite0, imageWrite1, BrickNodeBufferWrite, materialWrite, voxObjWrite, brickElementsBufferWrite
		};

		std::cout << "Updating descriptor sets...\n";
		std::cout << "  Binding 0: storageImageView\n";
		std::cout << "  Binding 1: textureImageView (texture image)\n";

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()),
			descriptorWrites.data(), 0, nullptr);

	}
	void createSyncObjects() {
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &computeFinishedSemaphore) != VK_SUCCESS ||
			vkCreateFence(device, &fenceInfo, nullptr, &computeFence) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create synchronization objects!");
		}

		std::cout << "Synchronization objects created\n";
	}

	
	void recordComputeCommandBuffer() {


		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = nullptr;

		if (vkBeginCommandBuffer(computeCommandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("Failed to begin recording compute command buffer!");
		}




		VkImageMemoryBarrier imageBarrier{};
		imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
		imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageBarrier.image = storageImage;
		imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBarrier.subresourceRange.baseMipLevel = 0;
		imageBarrier.subresourceRange.levelCount = 1;
		imageBarrier.subresourceRange.baseArrayLayer = 0;
		imageBarrier.subresourceRange.layerCount = 1;
		imageBarrier.srcAccessMask = 0;
		imageBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;


		vkCmdPipelineBarrier(
			computeCommandBuffer,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageBarrier
		);







		vkCmdBindPipeline(computeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);


		
		CameraData cameraData;


		glm::mat4 view = camera.getViewMatrix();
		glm::mat4 projection = camera.getProjectionMatrix(
			static_cast<float>(swapChainExtent.width) /
			static_cast<float>(swapChainExtent.height)
		);

		projection[1][1] *= -1.0f;

		glm::mat4 viewProj = projection * view;

		glm::mat4 invViewProj = glm::inverse(viewProj);

		cameraData.view = invViewProj;
		cameraData.projection = camera.getProjectionMatrix(
			static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height)
		);

		glm::dvec3 camWorldPos = glm::floor(camera.getPositionPrecise());

		glm::dvec3 camLocalPos = camera.getPositionPrecise() - camWorldPos;


		cameraData.cameraPos = camera.getPosition();
		cameraData.frameCount = frameCount;
		cameraData.time = static_cast<float>(glfwGetTime());
		cameraData.worldMin = glm::vec3(camLocalPos);
		cameraData.worldMax = glm::vec3(camWorldPos);
		cameraData.gridSize = glm::ivec3(0);
		cameraData.invViewProj = invViewProj;
		cameraData.selectedObject = 0.0f;
		cameraData.voxelObjectCount = voxelObjects.size();

		vkCmdPushConstants(
			computeCommandBuffer,
			pipelineLayout,
			VK_SHADER_STAGE_COMPUTE_BIT,
			0,
			sizeof(CameraData),
			&cameraData
		);

		vkCmdBindDescriptorSets(
			computeCommandBuffer,
			VK_PIPELINE_BIND_POINT_COMPUTE,
			pipelineLayout,
			0,
			1,
			&descriptorSet,
			0,
			nullptr
		);

		uint32_t groupSize = 16;
		uint32_t scaledWidth = (swapChainExtent.width + RESOLUTION_SCALE - 1) / RESOLUTION_SCALE;
		uint32_t scaledHeight = (swapChainExtent.height + RESOLUTION_SCALE - 1) / RESOLUTION_SCALE;
		uint32_t groupCountX = (scaledWidth + groupSize - 1) / groupSize;
		uint32_t groupCountY = (scaledHeight + groupSize - 1) / groupSize;

		vkCmdDispatch(computeCommandBuffer, groupCountX, groupCountY, 1);


		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imageBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(
			computeCommandBuffer,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageBarrier
		);
		

		if (vkEndCommandBuffer(computeCommandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to record compute command buffer!");
		}

	}
	
	



	///step 12
	///step 12
	///step 12
	///step 12
	///step 12



	void createSwapChainImageViews() {
		swapChainImageViews.resize(swapChainImages.size());

		for (size_t i = 0; i < swapChainImages.size(); i++) {
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = swapChainImageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create image views!");
			}
		}

		std::cout << "Swapchain image views created: " << swapChainImageViews.size() << "\n";
	}


	void createGraphicsCommandPool() {
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = indices.graphicsFamily.value();
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (vkCreateCommandPool(device, &poolInfo, nullptr, &graphicsCommandPool) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create graphics command pool!");
		}

		std::cout << "Graphics command pool created\n";
	}

	void allocateGraphicsCommandBuffer() {
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = graphicsCommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(device, &allocInfo, &graphicsCommandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate graphics command buffer!");
		}

		std::cout << "Graphics command buffer allocated\n";
	}

	void createGraphicsSyncObjects() {
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;  // Start signaled

		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
			vkCreateFence(device, &fenceInfo, nullptr, &graphicsFence) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create graphics synchronization objects!");
		}

		std::cout << "Graphics synchronization objects created\n";
	}
	void recordGraphicsCommandBuffer(uint32_t imageIndex) {
		vkResetCommandBuffer(graphicsCommandBuffer, 0);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vkBeginCommandBuffer(graphicsCommandBuffer, &beginInfo);

		VkImageMemoryBarrier swapBarrier{};
		swapBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		swapBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		swapBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		swapBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		swapBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		swapBarrier.image = swapChainImages[imageIndex];
		swapBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		swapBarrier.srcAccessMask = 0;
		swapBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		vkCmdPipelineBarrier(graphicsCommandBuffer,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0, 0, nullptr, 0, nullptr, 1, &swapBarrier);

		VkImageCopy copyRegion{};
		copyRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		copyRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		copyRegion.extent = { swapChainExtent.width, swapChainExtent.height, 1 };

		vkCmdCopyImage(graphicsCommandBuffer,
			storageImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			swapChainImages[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &copyRegion);

		VkImageMemoryBarrier imguiBarrier{};
		imguiBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imguiBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imguiBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		imguiBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imguiBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imguiBarrier.image = swapChainImages[imageIndex];
		imguiBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		imguiBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imguiBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		vkCmdPipelineBarrier(graphicsCommandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			0, 0, nullptr, 0, nullptr, 1, &imguiBarrier);

		VkRenderPassBeginInfo rpInfo{};
		rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rpInfo.renderPass = imguiRenderPass;
		rpInfo.framebuffer = imguiFramebuffers[imageIndex];
		rpInfo.renderArea.offset = { 0, 0 };
		rpInfo.renderArea.extent = swapChainExtent;

		vkCmdBeginRenderPass(graphicsCommandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), graphicsCommandBuffer);

		vkCmdEndRenderPass(graphicsCommandBuffer);

		VkImageMemoryBarrier presentBarrier{};
		presentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		presentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		presentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		presentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		presentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		presentBarrier.image = swapChainImages[imageIndex];
		presentBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		presentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		presentBarrier.dstAccessMask = 0;

		vkCmdPipelineBarrier(graphicsCommandBuffer,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			0, 0, nullptr, 0, nullptr, 1, &presentBarrier);

		vkEndCommandBuffer(graphicsCommandBuffer);
	}

	void drawFrame() {
		vkWaitForFences(device, 1, &graphicsFence, VK_TRUE, UINT64_MAX);

		vkWaitForFences(device, 1, &computeFence, VK_TRUE, UINT64_MAX);
		vkResetFences(device, 1, &computeFence);


		vkResetCommandBuffer(computeCommandBuffer, 0);
		recordComputeCommandBuffer();


		VkSubmitInfo computeSubmitInfo{};
		computeSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		computeSubmitInfo.commandBufferCount = 1;
		computeSubmitInfo.pCommandBuffers = &computeCommandBuffer;
		computeSubmitInfo.signalSemaphoreCount = 1;
		computeSubmitInfo.pSignalSemaphores = &computeFinishedSemaphore;

		VkResult result = vkQueueSubmit(computeQueue, 1, &computeSubmitInfo, computeFence);
		if (result != VK_SUCCESS) {
			std::cerr << "vkQueueSubmit Failed! VkResult" << (result) << std::endl;
			throw std::runtime_error("Failed to submit compute command buffer!");
		}

		uint32_t imageIndex;
		VkResult acquireResult = vkAcquireNextImageKHR(
			device, swapChain, UINT64_MAX,
			imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex
		);

		if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return;
		}
		else if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("Failed to acquire swap chain image!");
		}

		recordGraphicsCommandBuffer(imageIndex);

		VkSemaphore waitSemaphores[] = { computeFinishedSemaphore, imageAvailableSemaphore };
		VkPipelineStageFlags waitStages[] = {
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT
		};

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 2;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &graphicsCommandBuffer;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &renderFinishedSemaphore;

		vkResetFences(device, 1, &graphicsFence);

		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, graphicsFence) != VK_SUCCESS) {
			throw std::runtime_error("Failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapChain;
		presentInfo.pImageIndices = &imageIndex;

		VkResult presentResult = vkQueuePresentKHR(presentQueue, &presentInfo);

		if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR) {
			recreateSwapChain();
		}
		else if (presentResult != VK_SUCCESS) {
			throw std::runtime_error("Failed to present swap chain image!");
		}
	}

	void recreateSwapChain() {
		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(device);


		cleanupSwapChain();
		for (auto imageView : swapChainImageViews) {
			vkDestroyImageView(device, imageView, nullptr);
		}

		for (auto fb : imguiFramebuffers)
			vkDestroyFramebuffer(device, fb, nullptr);
		createSwapChain();
		createSwapChainImageViews();


		vkDestroyImageView(device, storageImageView, nullptr);
		vkDestroyImage(device, storageImage, nullptr);
		vkFreeMemory(device, storageImageMemory, nullptr);
		createStorageImage();
		createImGuiFramebuffers();

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageView = storageImageView;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSet;
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);


		recordComputeCommandBuffer();

		std::cout << "Swapchain recreated\n";
	}
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		endSingleTimeCommands(commandBuffer);
	}
	
	std::vector<BrickNode> flatten(const std::vector<std::vector<BrickNode>>& objBrickMaps)
	{
		std::vector<BrickNode> ret;
		for (const auto& v : objBrickMaps)
			ret.insert(ret.end(), v.begin(), v.end());
		return ret;
	}

	std::vector<int> flattenInt(const std::vector<std::vector<int>>& objBrickMaps)
	{
		std::vector<int> ret;
		for (const auto& v : objBrickMaps)
			ret.insert(ret.end(), v.begin(), v.end());
		return ret;
	}

	void createBrickBuffer() {
		std::vector<BrickNode>flattenedBricks = flatten(BrickNodes);

		std::cout << "size of flattened bricks" << flattenedBricks.size()<< std::endl;
		if (flattenedBricks.empty()) return;

		VkDeviceSize bufferSize = sizeof(BrickNode) * MAX_BRICKNODES;


		// Single host-visible buffer, no staging needed
		createBuffer(bufferSize,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			BrickNodeBuffer, BrickNodeBufferMemory);
		VkDeviceSize uploadSize = sizeof(BrickNode) * flattenedBricks.size();

		void* data;
		vkMapMemory(device, BrickNodeBufferMemory, 0, uploadSize, 0, &data);
		memcpy(data, flattenedBricks.data(), uploadSize);
		vkUnmapMemory(device, BrickNodeBufferMemory);

		flattenedBricks.clear();
	}
	void resizeBrickBuffer(size_t brickCount)
	{

		vkWaitForFences(device, 1, &graphicsFence, VK_TRUE, UINT64_MAX);
		vkWaitForFences(device, 1, &computeFence, VK_TRUE, UINT64_MAX);

		VkDeviceSize newSize = sizeof(BrickNode) * brickCount;


		VkBuffer newBuffer;
		VkDeviceMemory newMemory;

		createBuffer(newSize,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			newBuffer, newMemory);

		if (BrickNodeBuffer != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(device, BrickNodeBuffer, nullptr);
			vkFreeMemory(device, BrickNodeBufferMemory, nullptr);
		}
		BrickNodeBuffer = newBuffer;
		brickElementsBufferMemory = newMemory;

	}
	void updateBrickDescriptor()
	{
		VkDescriptorBufferInfo BrickNodeBufferInfo{};
		BrickNodeBufferInfo.buffer = BrickNodeBuffer;
		BrickNodeBufferInfo.offset = 0;
		BrickNodeBufferInfo.range = sizeof(BrickNode) * 2;

		VkWriteDescriptorSet BrickNodeBufferWrite{};
		BrickNodeBufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		BrickNodeBufferWrite.dstSet = descriptorSet;
		BrickNodeBufferWrite.dstBinding = 2;
		BrickNodeBufferWrite.dstArrayElement = 0;
		BrickNodeBufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		BrickNodeBufferWrite.descriptorCount = 1;
		BrickNodeBufferWrite.pBufferInfo = &BrickNodeBufferInfo;


		vkUpdateDescriptorSets(device, 1, &BrickNodeBufferWrite, 0, nullptr);

	}
	void updateBrickBuffer() {


		std::vector<BrickNode>flattenedBricks = flatten(BrickNodes);
		std::cout << "flattenedBricks size" << flattenedBricks.size() << std::endl;
		if (flattenedBricks.empty()) return;

		vkWaitForFences(device, 1, &graphicsFence, VK_TRUE, UINT64_MAX);
		vkWaitForFences(device, 1, &computeFence, VK_TRUE, UINT64_MAX);

		VkDeviceSize size = sizeof(BrickNode) * flattenedBricks.size();

		void* data;

		vkMapMemory(device, BrickNodeBufferMemory, 0, size, 0, &data);
		memcpy(data, flattenedBricks.data(), size);
		vkUnmapMemory(device, BrickNodeBufferMemory);
	
		flattenedBricks.clear();
	}
	



	void ImagecopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevel) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = mipLevel;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			width,
			height,
			depth
		};

		vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		endSingleTimeCommands(commandBuffer);
	}



	
	void createVoxelObjectBuffer() {
		if (voxelObjects.empty()) return;

		VkDeviceSize bufferSize = sizeof(VoxelObject) * MAX_VOXEL_OBJECTS;

		// Single host-visible buffer, no staging needed
		createBuffer(bufferSize,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			voxelObjectBuffer, voxelObjectBufferMemory);

		VkDeviceSize uploadSize = sizeof(VoxelObject) * voxelObjects.size();

		void* data;
		vkMapMemory(device, voxelObjectBufferMemory, 0, uploadSize, 0, &data);
		memcpy(data, voxelObjects.data(), uploadSize);
		vkUnmapMemory(device, voxelObjectBufferMemory);
	}
	

	void cleanupImGui() {
		vkDeviceWaitIdle(device);

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		vkDestroyDescriptorPool(device, imguiPool, nullptr);

		for (auto fb : imguiFramebuffers)
			vkDestroyFramebuffer(device, fb, nullptr);


		vkDestroyRenderPass(device, imguiRenderPass, nullptr);
	}




	void recordImGuiPass(VkCommandBuffer cmdBuffer, uint32_t imageIndex) {
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = swapChainImages[imageIndex];
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		vkCmdPipelineBarrier(
			cmdBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		VkRenderPassBeginInfo rpInfo{};
		rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rpInfo.renderPass = imguiRenderPass;
		rpInfo.framebuffer = imguiFramebuffers[imageIndex];
		rpInfo.renderArea.offset = { 0, 0 };
		rpInfo.renderArea.extent = swapChainExtent;

		vkCmdBeginRenderPass(cmdBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdBuffer);

		vkCmdEndRenderPass(cmdBuffer);

		barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = 0;

		vkCmdPipelineBarrier(
			cmdBuffer,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
	}


	void createImGuiRenderPass() {
		VkAttachmentDescription attachment{};
		attachment.format = swapChainImageFormat;
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorRef{};
		colorRef.attachment = 0;
		colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorRef;

		VkSubpassDependency dep{};
		dep.srcSubpass = VK_SUBPASS_EXTERNAL;
		dep.dstSubpass = 0;
		dep.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dep.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.attachmentCount = 1;
		info.pAttachments = &attachment;
		info.subpassCount = 1;
		info.pSubpasses = &subpass;
		info.dependencyCount = 1;
		info.pDependencies = &dep;

		vkCreateRenderPass(device, &info, nullptr, &imguiRenderPass);
	}
	void createImGuiFramebuffers() {
		imguiFramebuffers.resize(swapChainImageViews.size());
		for (size_t i = 0; i < swapChainImageViews.size(); i++) {
			VkFramebufferCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			info.renderPass = imguiRenderPass;
			info.attachmentCount = 1;
			info.pAttachments = &swapChainImageViews[i];
			info.width = swapChainExtent.width;
			info.height = swapChainExtent.height;
			info.layers = 1;
			vkCreateFramebuffer(device, &info, nullptr, &imguiFramebuffers[i]);
		}
	}
	void createImGuiDescriptorPool(VkDevice device) {
		VkDescriptorPoolSize pool_sizes[] = {
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo pool_info{};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000 * 11;
		pool_info.poolSizeCount = 11;
		pool_info.pPoolSizes = pool_sizes;

		if (vkCreateDescriptorPool(device, &pool_info, nullptr, &imguiPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create ImGui descriptor pool!");
		}
	}

	void initImGui() {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		createImGuiRenderPass();
		createImGuiFramebuffers();
		createImGuiDescriptorPool(device);


		ImGui_ImplGlfw_InitForVulkan(window, true);


		ImGui_ImplVulkan_InitInfo init_info{};
		init_info.ApiVersion = VK_API_VERSION_1_0;
		init_info.Instance = instance;
		init_info.PhysicalDevice = physicalDevice;
		init_info.Device = device;
		init_info.QueueFamily = findQueueFamilies(physicalDevice).graphicsFamily.value();
		init_info.Queue = graphicsQueue;
		init_info.DescriptorPool = imguiPool;
		init_info.MinImageCount = 2;
		init_info.ImageCount = static_cast<uint32_t>(swapChainImages.size());

		init_info.PipelineInfoMain.RenderPass = imguiRenderPass;
		init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

		if (!ImGui_ImplVulkan_Init(&init_info)) {
			throw std::runtime_error("Failed to init ImGui Vulkan");
		}

	}

	void updateVoxelObjectDescriptor() {
		std::vector<VkDescriptorImageInfo> voxImageInfos(voxelImages.size());
		for (uint32_t i = 0; i < voxelImages.size(); i++) {
			VkImageView fallback = (i < voxelImageViews.size() && voxelImageViews[i] != VK_NULL_HANDLE)
				? voxelImageViews[i] : textureImageView;
			voxImageInfos[i].imageView = fallback;
			voxImageInfos[i].sampler = voxelSampler;
			voxImageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = descriptorSet;
		write.dstBinding = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.descriptorCount = voxelImages.size();
		write.pImageInfo = voxImageInfos.data();
		vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
	}


	void createBrickElementsBuffer() {

		if (brickElements.empty()) return;

		VkDeviceSize bufferSize = sizeof(int) * MAX_BRICKELEMENTS;


		createBuffer(bufferSize,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			brickElementsBuffer, brickElementsBufferMemory);

		VkDeviceSize uploadSize = sizeof(int) * brickElements.size();


		void* data;
		vkMapMemory(device, brickElementsBufferMemory, 0, uploadSize, 0, &data);
		memcpy(data, brickElements.data(), uploadSize);
		vkUnmapMemory(device, brickElementsBufferMemory);

	}

	void updateBrickElementsBuffer() {
		if (brickElements.empty()) return;

		vkWaitForFences(device, 1, &graphicsFence, VK_TRUE, UINT64_MAX);
		vkWaitForFences(device, 1, &computeFence, VK_TRUE, UINT64_MAX);

		VkDeviceSize size = sizeof(int) * brickElements.size();

		void* data;
		vkMapMemory(device, brickElementsBufferMemory, 0, size, 0, &data);
		memcpy(data, brickElements.data(), size);
		vkUnmapMemory(device, brickElementsBufferMemory);

	}
	void initPalette()
	{

		std::cout << "Current Path: " << current_path << std::endl;
		auto path = current_path / "assets/minecraft.vox";

		std::cout << "Loading VOX file from: " << path << std::endl;
		std::string pathStr = path.string();

		auto voxModels = VoxLoader::loadAll(pathStr, 1);
		if (!voxModels.empty()) {
			auto gpuMaterials = materialRegistry.buildGPUBuffer(voxModels[0]);
			globalPalette = gpuMaterials;
			std::cout << "Loaded " << voxModels.size() << " models from VOX file.\n";
			createMaterialBuffer(gpuMaterials);
		}
		else {
			// Fallback: create a default all white material buffer
			std::array<GPUMaterial, 256> defaultMaterials{};
			for (int i = 1; i < 256; i++) {
				defaultMaterials[i].color = glm::vec4(1.0f);
			}
			createMaterialBuffer(defaultMaterials);
		}
		glm::quat rot90Y = glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 0.0f));
		ObjectManager::addVoxelObjectFromFile(
			(current_path / "assets/floor2.vox").string(),
			glm::vec3(0.0f, -8.0f, 0.0f),
			rot90Y,
			glm::vec3(1.0f),
			false,
			glm::vec3(2),
			2
		);

		std::cout << "Voxel world initialized with file: " << pathStr << std::endl;
	}


	void initVulkan() {
		createInstance();
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createSwapChainImageViews();
		createStorageImage();
		createCommandPool();
		allocateCommandBuffers();


		initPalette();

		createVoxelObjectBuffer();

		createDummyTexture();

		createBrickBuffer();

		createBrickElementsBuffer();


		createDescriptorSetLayout();
		createComputePipeline();

		createVoxelSampler();

		createDescriptorResources();

		createSyncObjects();

		createImGuiRenderPass();
		createImGuiFramebuffers();
		initImGui();

		recordComputeCommandBuffer();
		createGraphicsCommandPool();
		allocateGraphicsCommandBuffer();
		createGraphicsSyncObjects();

		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);


		std::cout << "--- GPU Resource Limits ---" << std::endl;
		std::cout << "GPU Name: " << properties.deviceName << std::endl;

		// The maximum number of sampled images a compute shader can access
		std::cout << "Max Compute Sampled Images: "
			<< properties.limits.maxPerStageDescriptorSampledImages << std::endl;

		// The maximum number of samplers a compute shader can access
		std::cout << "Max Compute Samplers: "
			<< properties.limits.maxPerStageDescriptorSamplers << std::endl;

		// Total individual memory allocations allowed
		std::cout << "Max Memory Allocations: "
			<< properties.limits.maxMemoryAllocationCount << std::endl;

	}
	
}