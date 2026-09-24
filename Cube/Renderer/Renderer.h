#pragma once
#include "Dependencies.h"
#include "Camera.h"
#include "MaterialRegistry.h"
#include "ObjectManager.h"


namespace Cube::Renderer {
	struct VoxelObject {
		glm::mat4  transform;           // 64 bytes  offset 0
		glm::mat4  invTransform;        // 64 bytes  offset 64
		glm::vec3  boundsMin;           // 12 bytes  offset 128
		float      mass;                //  4 bytes  offset 140
		glm::vec3  boundsMax;           // 12 bytes  offset 144
		float      restitution;         //  4 bytes  offset 156
		glm::ivec4 gridSizeAndTex;      // 16 bytes  offset 160  xyz=gridSize, w=textureIndex
		glm::vec3  velocity;            // 12 bytes  offset 176
		float      pad0;                //  4 bytes  offset 188    
		glm::vec3  angularVel;          // 12 bytes  offset 192
		float      pad1;                //  4 bytes  offset 204
	};

	inline float deltaTime = 0.0f;
	inline float lastTime = glfwGetTime();
	inline int frameCount = 0;
	
	inline Cube::Camera camera;

	inline GLFWwindow* window;
	inline VkInstance instance;
	inline VkDebugUtilsMessengerEXT debugMessenger;
	inline VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	inline VkDevice device;
	inline VkQueue graphicsQueue;
	inline VkQueue presentQueue;
	inline VkSurfaceKHR surface;

	inline VkSwapchainKHR swapChain;
	inline std::vector<VkImage> swapChainImages;
	inline VkFormat swapChainImageFormat;
	inline VkExtent2D swapChainExtent;

	inline VkImage storageImage;
	inline VkDeviceMemory storageImageMemory;
	inline VkImageView storageImageView;

	inline VkQueue computeQueue;

	inline VkShaderModule computeShaderModule;
	inline VkDescriptorSetLayout descriptorSetLayout;
	inline VkPipelineLayout pipelineLayout;
	inline VkPipeline computePipeline;

	// Command buffer members
	inline VkCommandPool computeCommandPool;
	inline VkCommandBuffer computeCommandBuffer;

	// Descriptor resources
	inline VkDescriptorPool descriptorPool;
	inline VkDescriptorSet descriptorSet;

	// Synchronization
	inline VkSemaphore computeFinishedSemaphore;
	inline VkFence computeFence;

	inline VkCommandPool graphicsCommandPool;
	inline VkCommandBuffer graphicsCommandBuffer;

	// Synchronization for graphics
	inline VkSemaphore imageAvailableSemaphore;
	inline VkSemaphore renderFinishedSemaphore;
	inline VkFence graphicsFence;

	// Image views for swapchain images
	inline std::vector<VkImageView> swapChainImageViews;


	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;
		std::optional<uint32_t> computeFamily;

		bool isComplete() {
			return graphicsFamily.has_value() &&
				presentFamily.has_value() &&
				computeFamily.has_value();
		}
	};

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};


	inline std::vector<VoxelObject> voxelObjects;
	inline VkBuffer       voxelObjectBuffer = VK_NULL_HANDLE;
	inline VkDeviceMemory voxelObjectBufferMemory = VK_NULL_HANDLE;
	inline uint32_t MAX_VOXEL_OBJECTS = 1000;


	inline std::vector<VkImage>        voxelImages;
	inline std::vector<VkDeviceMemory> voxelImageMemories;
	inline std::vector<VkImageView>    voxelImageViews;

	inline uint32_t MAX_BRICKNODES = 2000000;
	inline uint32_t MAX_BRICKELEMENTS = 1000;

	inline const uint32_t BRICK_SIZE = 8;

	inline VkBuffer BrickNodeBuffer;
	inline VkDeviceMemory BrickNodeBufferMemory;

	struct BrickNode {
		uint32_t occupancy;
	};
	inline std::vector<std::vector<BrickNode>> BrickNodes;

	inline VkBuffer gridStagingBuffer;
	inline VkDeviceMemory gridStagingBufferMemory;
	inline VkDeviceSize gridStagingBufferSize = 0;

	inline std::vector<int> brickElements;
	inline VkBuffer brickElementsBuffer;
	inline VkDeviceMemory brickElementsBufferMemory;
	inline std::unordered_map<uint32_t, uint32_t> brickGridToBrickIndex;


	inline Cube::MaterialRegistry materialRegistry;
	inline VkBuffer       materialBuffer = VK_NULL_HANDLE;
	inline VkDeviceMemory materialBufferMemory = VK_NULL_HANDLE;
	inline std::unordered_map<size_t, btHingeConstraint*> motors;
	inline VkImageView textureImageView;
	inline VkSampler voxelSampler = VK_NULL_HANDLE;

	inline VkDescriptorPool ImagedescriptorPool;
	inline VkDescriptorSetLayout ImagedescriptorSetLayout;
	inline VkDescriptorSet ImagedescriptorSet;

	inline VkDescriptorPool imguiPool;
	inline VkRenderPass imguiRenderPass;
	inline std::vector<VkFramebuffer> imguiFramebuffers;

	inline VkImage textureImage;
	inline VkDeviceMemory textureImageMemory;


	inline static uint32_t RESOLUTION_SCALE = 2;
	inline static std::array<GPUMaterial, 256> globalPalette{};
	inline static std::filesystem::path current_path = std::filesystem::current_path();

	void createDummyTexture();
	void createInstance();
	void createSurface();
	bool checkValidationLayerSupport();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

	void setupDebugMessenger();
	void pickPhysicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	void createSwapChain();
	void createLogicalDevice();
	void cleanupSwapChain();
	void cleanup();
	void createImage(uint32_t width, uint32_t height, VkFormat format,
		VkImageUsageFlags usage, VkImage& image, VkDeviceMemory& imageMemory);
	void createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
		VkImageView& imageView);
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void createStorageImage();
	void createDescriptorSetLayout();
	void createComputePipeline();
	void allocateCommandBuffers();
	void createCommandPool();
	void createDescriptorResources();
	void createSyncObjects();
	void recordComputeCommandBuffer();
	void createSwapChainImageViews();
	void createGraphicsCommandPool();
	void allocateGraphicsCommandBuffer();
	void createGraphicsSyncObjects();
	void recordGraphicsCommandBuffer(uint32_t imageIndex);
	void drawFrame();
	void recreateSwapChain();
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties, VkBuffer& buffer,
		VkDeviceMemory& bufferMemory);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);

	std::vector<BrickNode> flatten(const std::vector<std::vector<BrickNode>>& objBrickMaps);
	std::vector<int> flattenInt(const std::vector<std::vector<int>>& objBrickMaps);
	void createBrickBuffer();
	void resizeBrickBuffer(size_t brickCount);
	void updateBrickDescriptor();
	void updateBrickBuffer();
	void ImagecreateImage(uint32_t width, uint32_t height, uint32_t depth,
		VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	void ImagetransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void ImagecopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevel = 0);


	void ImageCreateImageView();


	void ImagecreateDescriptorPool();
	void createVoxelSampler();
	void ImageDescriptorSet();

	void createMaterialBuffer(const std::array<GPUMaterial, 256>& materials);
	void createVoxelTexture(uint32_t index,
		const std::vector<unsigned char>& voxels,
		uint32_t w, uint32_t h, uint32_t d);

	void createVoxelObjectBuffer();


	void cleanupImGui();
	void createImGuiFramebuffers();
	void createImGuiDescriptorPool(VkDevice device);
	void initImGui();
	void createImGuiRenderPass();
	void recordImGuiPass(VkCommandBuffer cmdBuffer, uint32_t imageIndex);
	void updateVoxelObjectDescriptor();
	void createBrickElementsBuffer();
	void updateBrickElementsBuffer();
	void initVulkan();
	void initPalette();
	

}