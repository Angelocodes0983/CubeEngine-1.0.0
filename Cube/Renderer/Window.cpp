#include "Window.h"

namespace Cube::Window
{
	static void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
		if (mouseCaptured) {
			Renderer::camera.processMouse(xpos, ypos);
		}
	}

	static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
		//processScroll(xoffset, yoffset);
	}
	WindowProps selectResolution() {
		struct ResOption {
			uint32_t w, h;
			bool fullscreen;
			const char* label;
		};

		const ResOption options[] = {
			{ 1280,  720,  true,  "1280 x 720   (720p fullscreen)"  },
			{ 1600,  900,  true,  "1600 x 900   (900p fullscreen)"  },
			{ 1920, 1080,  true,  "1920 x 1080  (1080p fullscreen)" },
			{ 2560, 1440,  true,  "2560 x 1440  (1440p fullscreen)" },
			{ 3840, 2160,  true,  "3840 x 2160  (4K fullscreen)"    },
			{  800,  600, false,  "800 x 600    (windowed)"         },
			{ 1280,  720, false,  "1280 x 720   (windowed)"         },
			{ 1920, 1080, false,  "1920 x 1080  (windowed)"         },
		};
		const int COUNT = sizeof(options) / sizeof(options[0]);
		const int DEFAULT = 2; // 1920x1080 fullscreen

		std::cout << "\n=== Select Resolution ===\n";
		for (int i = 0; i < COUNT; i++) {
			std::cout << "  [" << i << "] " << options[i].label;
			if (i == DEFAULT) std::cout << "  <-- default";
			std::cout << "\n";
		}
		std::cout << "Enter choice (0-" << (COUNT - 1)
			<< ", or press Enter for default): ";

		std::string input;
		std::getline(std::cin, input);

		int choice = DEFAULT;
		if (!input.empty()) {
			try {
				int parsed = std::stoi(input);
				if (parsed >= 0 && parsed < COUNT)
					choice = parsed;
				else
					std::cout << "Out of range, using default.\n";
			}
			catch (...) {
				std::cout << "Invalid input, using default.\n";
			}
		}
		WindowProps properties;

		properties.Width = options[choice].w;
		properties.Height = options[choice].h;
		properties.FullScreen = options[choice].fullscreen;
		properties.label = options[choice].label;

		WIDTH = options[choice].w;
		HEIGHT = options[choice].h;

		return properties;
		std::cout << "width1" << options[choice].w << std::endl;
		std::cout << "Selected: " << options[choice].label << "\n\n";
	}

	void initWindow(const WindowProps& properties) {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		GLFWmonitor* monitor = nullptr;
		if (properties.FullScreen) {
			monitor = glfwGetPrimaryMonitor();

			const GLFWvidmode* mode = glfwGetVideoMode(monitor);
			glfwWindowHint(GLFW_RED_BITS, mode->redBits);
			glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
			glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
			glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
		}
		std::cout << "width" << properties.Width << std::endl;
		Cube::Renderer::window = glfwCreateWindow(properties.Width, properties.Height, "Vulkan Swapchain", monitor, nullptr);
		if (!Cube::Renderer::window)
			throw std::runtime_error("Failed to create GLFW window!");


		glfwSetCursorPosCallback(Cube::Renderer::window, mouseCallback);
		glfwSetScrollCallback(Cube::Renderer::window, scrollCallback);
		//glfwSetMouseButtonCallback(Cube::Renderer::window, mouseButtonCallback);
		glfwSetInputMode(Cube::Renderer::window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		mouseCaptured = true;


	}



}