#pragma once

#include "engine/render/texture_manager/texture_mng.h"

#include <memory>


class RenderSystem
{
    friend bool engInitRenderSystem() noexcept;
    friend void engTerminateRenderSystem() noexcept;
    friend bool engIsRenderSystemInitialized() noexcept;

public:
    static RenderSystem& GetInstance() noexcept;

public:
    ~RenderSystem();

    void RunDepthPrepass() noexcept;
    void RunGBufferPass() noexcept;
    void RunColorPass() noexcept;
    void RunPostprocessingPass() noexcept;

private:
    RenderSystem() = default;
    
    bool Init() noexcept; 
    void Terminate() noexcept;

    bool IsInitialized() const noexcept;

private:
    // GBuffer textures, framebuffer
    // ...
};


bool engInitRenderSystem() noexcept;
void engTerminateRenderSystem() noexcept;
bool engIsRenderSystemInitialized() noexcept;