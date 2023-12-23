#include "VkFrame.h"
#include "SDL.h"
#include "SDL_vulkan.h"
#include "SDL_video.h"

#include <vulkan/vulkan.h>

#include <thread>
#include <chrono>


VkFrame::VkFrame()
	:
	context(0),
	handle(0),
	window(0)
{
	alive = true;
}


void VkFrame::Initialize(const char* title, float x, float y)
{
	std::thread t(&VkFrame::Thread, this, title, x, y);
	t.detach();
}

void VkFrame::InitVulkan()
{
	target.InitVulkan(window);
}

VkRenderTarget* VkFrame::GetTarget()
{
	return &target;
}



void VkFrame::Thread(const char* title, float x, float y)
{
	SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_VIDEO | SDL_INIT_EVENTS);

	/* Turn on double buffering with a 24bit Z buffer.
	* You may need to change this to 16 or 32 for your system */
	//SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	//SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	SDL_Window* window = SDL_CreateWindow(title,
		SDL_WINDOWPOS_UNDEFINED,           // initial x position
		SDL_WINDOWPOS_UNDEFINED,           // initial y position
		(int)x,                               // width, in pixels
		(int)y,                               // height, in pixels
		SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
	);
	this->window = window;

	SDL_ShowWindow((SDL_Window*)window);

	InitVulkan();

	while (alive)
	{
		//std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();

		SDL_Event Event;
		while (SDL_PollEvent(&Event))
		{
			if (Event.type == SDL_WINDOWEVENT)
			{
				if (Event.window.event == SDL_WINDOWEVENT_RESIZED || Event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
				{
					int w, h;
					w = Event.window.data1;
					h = Event.window.data2;
					//TODO: Resize
				}
				else if (Event.window.event == SDL_WINDOWEVENT_SHOWN)
				{
					//TODO: Shown
				}
			}
			else if (Event.type == SDL_QUIT)
			{
				alive = false;
			}
			else if (Event.type == SDL_KEYDOWN)
			{
				//Event.key.keysym.sym

			}
			else if (Event.type == SDL_KEYUP)
			{

				//Event.key.keysym.sym
			}
			else if (Event.type == SDL_MOUSEBUTTONDOWN || Event.type == SDL_MOUSEBUTTONUP)
			{
				//Event.button.button
				int vkButton = 0;

				/*
				switch (Event.button.button)
				{
				case SDL_BUTTON_LEFT:
					if ((Event.type == SDL_MOUSEBUTTONUP) != !mouseButtons[0])
					{
						//vkButton = VK_LBUTTON;
						mouseButtons[0] = Event.type == SDL_MOUSEBUTTONDOWN;
					}
					break;
				case SDL_BUTTON_RIGHT:
					if ((Event.type == SDL_MOUSEBUTTONUP) != !mouseButtons[1])
					{
						//vkButton = VK_RBUTTON;
						mouseButtons[1] = Event.type == SDL_MOUSEBUTTONDOWN;
					}
					break;
				case SDL_BUTTON_MIDDLE:
					if ((Event.type == SDL_MOUSEBUTTONUP) != !mouseButtons[2])
					{
						//vkButton = VK_MBUTTON;
						mouseButtons[2] = Event.type == SDL_MOUSEBUTTONDOWN;
					}
					break;
				}
				if (vkButton > 0)
				{
					//uiCon->HandleMouse(vkButton, Event.type == SDL_MOUSEBUTTONUP, Event.button.x, Event.button.y);
				}
				*/
			}
			else if (Event.type == SDL_MOUSEWHEEL)
			{
				//Int2D mPos = uiCon->GetUIModel()->getMPos();
				//uiCon->HandleScrollWheel(Event.wheel.y, mPos.x, mPos.y);
			}
			else if (Event.type == SDL_MOUSEMOTION)
			{
				//uiCon->HandleMouseMove(Int2D(Event.motion.x, Event.motion.y));
			}
			else
			{
				break;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

}