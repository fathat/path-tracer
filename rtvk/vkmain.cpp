#include <iostream>
#include <SDL.h>
#include <SDL_vulkan.h>
#include <vk_types.h>

class vulkan_engine_t {
public:

	bool _isInitialized{ false };
	int _frameNumber {0};

	VkExtent2D _windowExtent{ 1700 , 900 };

	struct SDL_Window* _window{ nullptr };

	//initializes everything in the engine
	void init() {
	    // We initialize SDL and create a window with it. 
	    SDL_Init(SDL_INIT_VIDEO);

	    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);
	    
	    _window = SDL_CreateWindow(
		    "Vulkan Engine",
		    SDL_WINDOWPOS_UNDEFINED,
		    SDL_WINDOWPOS_UNDEFINED,
		    _windowExtent.width,
		    _windowExtent.height,
		    window_flags
	    );
	    
	    //everything went fine
	    _isInitialized = true;
	}

	//shuts down the engine
	void cleanup() {
	    if (_isInitialized) {
		    SDL_DestroyWindow(_window);
	    }
	}

	//draw loop
	void draw() {
	    
	}

	//run main loop
	void run() {
	    SDL_Event e;
	    bool bQuit = false;

	    //main loop
	    while (!bQuit)
	    {
		    //Handle events on queue
		    while (SDL_PollEvent(&e) != 0)
		    {
			    //close the window when user alt-f4s or clicks the X button			
			    if (e.type == SDL_QUIT) bQuit = true;
		    }

		    draw();
	    }
	}
};


int main(int argc, char *args[])
{
	vulkan_engine_t engine;
	engine.init();
	engine.run();
	engine.cleanup();
    return 0;
}