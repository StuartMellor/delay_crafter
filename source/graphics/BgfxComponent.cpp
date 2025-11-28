#include "BgfxComponent.h"

BgfxComponent::BgfxComponent()
{
    startTimerHz(60); // drive bgfx at ~60 FPS
}

BgfxComponent::~BgfxComponent()
{
    shutdownBgfx();
}

void BgfxComponent::parentHierarchyChanged()
{
    // Peer might have changed; re-init if needed.
    if (isShowing())
        ensureBgfxInitialised();
}

void BgfxComponent::visibilityChanged()
{
    if (isShowing())
        ensureBgfxInitialised();
}

void BgfxComponent::paint(juce::Graphics&)
{
    // JUCE won't actually draw anything here; bgfx will.
    ensureBgfxInitialised();
}

void BgfxComponent::resized()
{
    if (bgfxInitialised)
    {
        bgfx::reset((uint32_t)getWidth(), (uint32_t)getHeight(), BGFX_RESET_VSYNC);
    }
}

void BgfxComponent::timerCallback()
{
    if (bgfxInitialised)
        renderFrame();
}

void BgfxComponent::ensureBgfxInitialised()
{
    if (bgfxInitialised)
        return;

    auto* peer = getPeer();
    if (peer == nullptr)
        return; // not on-screen yet

    bgfx::Init init;
    init.type = bgfx::RendererType::Count; // auto-select backend
    init.vendorId = BGFX_PCI_ID_NONE;

    init.resolution.width  = (uint32_t) getWidth();
    init.resolution.height = (uint32_t) getHeight();
    init.resolution.reset  = BGFX_RESET_VSYNC;

    bgfx::PlatformData pd {};
    pd.ndt = nullptr;
    pd.nwh = peer->getNativeHandle(); // JUCE gives us the OS window handle
    pd.context = nullptr;
    pd.backBuffer = nullptr;
    pd.backBufferDS = nullptr;

    init.platformData = pd;

    if (!bgfx::init(init))
        return;

    bgfx::setViewClear(0,
        BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH,
        0x303030ff, 1.0f, 0);

    bgfxInitialised = true;
}

void BgfxComponent::renderFrame()
{
    // Minimal frame: just clear.
    bgfx::touch(0);
    bgfx::frame();
}

void BgfxComponent::shutdownBgfx()
{
    if (bgfxInitialised)
    {
        bgfx::shutdown();
        bgfxInitialised = false;
    }
}
