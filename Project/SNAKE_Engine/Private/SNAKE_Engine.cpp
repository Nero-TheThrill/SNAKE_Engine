#define GLAD_GL_IMPLEMENTATION
#include "gl.h"
#include "glfw3.h"
#ifdef _DEBUG
//#include<vld.h>//TODO: remove this and directories before release (VC++ Directories -> Include Directories & Library Directories)
#endif
#include "Engine.h"


void SNAKE_Engine::SetEngineContext()
{
    engineContext.stateManager = &stateManager;
    engineContext.windowManager = &windowManager;
    engineContext.inputManager = &inputManager;
    engineContext.renderManager = &renderManager;
    engineContext.soundManager = &soundManager;
    engineContext.engine = this;
}


bool SNAKE_Engine::Init(int windowWidth, int windowHeight)
{
    if (!windowManager.Init(windowWidth, windowHeight, *this))
    {
        SNAKE_ERR("Window Initialization failed.");
        return false;
    }
    SetEngineContext();
    inputManager.Init(windowManager.GetHandle());
    soundManager.Init();
    renderManager.Init(engineContext);

    return true;
}


void SNAKE_Engine::Run()
{
    EngineTimer timer;
    timer.Start();
    while (shouldRun && !glfwWindowShouldClose(windowManager.GetHandle()))
    {
        float dt = timer.Tick();

        float fps = 0.0f;
        if (timer.ShouldUpdateFPS(fps))
        {
            windowManager.SetTitle("SNAKE_Engine - FPS: " + std::to_string(static_cast<int>(fps)));
        }

        windowManager.PollEvents();
        inputManager.Update();
        windowManager.ClearScreen();

        stateManager.Update(dt, engineContext);
        stateManager.Draw(engineContext);

        soundManager.Update();

        windowManager.SwapBuffers();
    }

    soundManager.Free();
    stateManager.Free(engineContext);
    windowManager.Free();
    Free();
}

void SNAKE_Engine::Free() const
{
    glfwTerminate();
}

void SNAKE_Engine::RequestQuit()
{
    shouldRun = false;
}
