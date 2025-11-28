
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

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

    bool bgfxInitialised = false;
};
