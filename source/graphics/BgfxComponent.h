
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include "graphics/3d/mesh/Mesh.h"
#include "graphics/3d/camera/Camera.h"

// Simple component that owns a bgfx context and renders each frame.
class BgfxComponent : public juce::Component,
                      private juce::Timer
{
public:
    BgfxComponent();
    ~BgfxComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void parentHierarchyChanged() override;
    void visibilityChanged() override;

private:
    void ensureBgfxInitialised();
    void shutdownBgfx();
    void renderFrame();
    void timerCallback() override;
    void createTriangle();
    float getScaleFactor() const;

    bool bgfxInitialised = false;
    bgfx::VertexBufferHandle vertexBuffer = BGFX_INVALID_HANDLE;
    bgfx::ProgramHandle program = BGFX_INVALID_HANDLE;
    Mesh mesh;
    Camera camera;
    double startTimeMs = 0.0;
};
