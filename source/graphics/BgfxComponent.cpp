#include "BgfxComponent.h"
#include "BinaryData.h"
#include "CompiledShaderData.h"
#include <bgfx/embedded_shader.h>
#include <bx/math.h>


// Vertex structure
struct PosColorVertex
{
    float x, y, z;
    uint32_t abgr;
};

static bgfx::VertexLayout s_PosColorLayout;

BgfxComponent::BgfxComponent() : mesh()
{
    setOpaque(true); // Tell JUCE we're fully opaque
    startTimerHz(60); // This timer drives the rendering
    mesh.load(bx::FilePath("assets/3d/ui-models-dial.glb"), true);
    startTimeMs = juce::Time::getMillisecondCounterHiRes();
    camera.setPositionTarget(bx::Vec3{0.0f, 0.0f, -6.0f}, bx::Vec3{0.0f, 0.0f, 0.0f});
    camera.setFov(60.0f);
    camera.setClipPlanes(0.01f, 100.0f);
}

BgfxComponent::~BgfxComponent()
{
    shutdownBgfx();
}

void BgfxComponent::parentHierarchyChanged()
{
    if (isShowing())
        ensureBgfxInitialised();
}

void BgfxComponent::visibilityChanged()
{
    if (isShowing())
        ensureBgfxInitialised();
}

// ---------------------------------------------------------
// FIX 1: Paint should be almost empty.
// We are rendering via the Timer, not via OS Paint messages.
// ---------------------------------------------------------
void BgfxComponent::paint(juce::Graphics&)
{
    ensureBgfxInitialised();
    
    // Do NOT call renderFrame() here.
    // Do NOT call bgfx::frame() here.
    // The Timer handles the frame update.
}

void BgfxComponent::resized()
{
    if (bgfxInitialised)
    {
        float scale = getScaleFactor();
        uint32_t w = (uint32_t)(getWidth() * scale);
        uint32_t h = (uint32_t)(getHeight() * scale);

        // Reset bgfx backbuffer size with PHYSICAL dimensions
        bgfx::reset(w, h, BGFX_RESET_VSYNC);
        camera.setViewportSize(w, h);
    }
}

void BgfxComponent::timerCallback()
{
    if (bgfxInitialised)
        renderFrame();
}

// ---------------------------------------------------------
// FIX 2: Restore renderFrame and apply DPI scaling here too
// ---------------------------------------------------------
void BgfxComponent::renderFrame()
{
    // Calculate Physical Pixels
    float scale = getScaleFactor();
    uint16_t w = (uint16_t)(getWidth() * scale);
    uint16_t h = (uint16_t)(getHeight() * scale);

    // Set view 0 viewport to match component size in PHYSICAL pixels
    bgfx::setViewRect(0, 0, 0, w, h);

    // Camera view/projection
    float view[16];
    float proj[16];
    camera.getView(view);
    camera.getProj(proj);
    bgfx::setViewTransform(0, view, proj);

    // Clear the view
    bgfx::touch(0);

    // Model transform (simple rotation over time)
    double now = juce::Time::getMillisecondCounterHiRes();
    float timeSec = float((now - startTimeMs) * 0.001);
    float model[16];
    bx::mtxRotateY(model, timeSec); // spin

    // Draw triangle (kept as reference)
    if (bgfx::isValid(vertexBuffer) && bgfx::isValid(program))
    {
        bgfx::setTransform(model);
        bgfx::setVertexBuffer(0, vertexBuffer);
        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z);
        bgfx::submit(0, program);
    }

    // Advance to next frame
    bgfx::frame();
}

void BgfxComponent::ensureBgfxInitialised()
{
    if (bgfxInitialised) return;

    auto* peer = getPeer();
    if (peer == nullptr) return;

    auto* topLevel = getTopLevelComponent();
    if (topLevel == nullptr) return;
    
    auto* topPeer = topLevel->getPeer();
    if (topPeer == nullptr) return;

    void* nativeHandle = topPeer->getNativeHandle();
    if (nativeHandle == nullptr) return;

    bgfx::Init init;
    init.type = bgfx::RendererType::Vulkan;
    init.vendorId = BGFX_PCI_ID_NONE;
    init.debug = false;
    init.profile = false;

    // Initial resolution (doesn't matter much as we resize immediately)
    init.resolution.width  = (uint32_t) topLevel->getWidth();
    init.resolution.height = (uint32_t) topLevel->getHeight();
    init.resolution.reset  = BGFX_RESET_VSYNC;

    init.platformData.nwh = nativeHandle;

    if (!bgfx::init(init))
    {
        DBG("bgfx init failed!");
        return;
    }

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x443355ff, 1.0f, 0);

    bgfxInitialised = true;

    // FIX 3: Force a resize immediately to sync Physical Pixels
    resized();
    
    createTriangle();
}

float BgfxComponent::getScaleFactor() const
{
    if (auto* peer = getPeer())
        return (float)peer->getPlatformScaleFactor();
    return 1.0f;
}

void BgfxComponent::createTriangle()
{
    // Define vertex layout
    s_PosColorLayout
        .begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
        .end();

    static PosColorVertex vertices[] = {
        { 0.0f,  0.5f, 0.0f, 0xff0000ff },
        { 0.5f, -0.5f, 0.0f, 0xff00ff00 },
        {-0.5f, -0.5f, 0.0f, 0xffff0000 },
    };

    vertexBuffer = bgfx::createVertexBuffer(
        bgfx::makeRef(vertices, sizeof(vertices)),
        s_PosColorLayout
    );

    // Ensure your ShaderData names match your BinaryData header
    const bgfx::Memory* vsMem = bgfx::copy(ShaderData::vs_triangle_bin, ShaderData::vs_triangle_binSize);
    const bgfx::Memory* fsMem = bgfx::copy(ShaderData::fs_triangle_bin, ShaderData::fs_triangle_binSize);

    bgfx::ShaderHandle vsh = bgfx::createShader(vsMem);
    bgfx::ShaderHandle fsh = bgfx::createShader(fsMem);

    program = bgfx::createProgram(vsh, fsh, true);
}

void BgfxComponent::shutdownBgfx()
{
    if (bgfxInitialised)
    {
        if (bgfx::isValid(vertexBuffer)) bgfx::destroy(vertexBuffer);
        if (bgfx::isValid(program)) bgfx::destroy(program);
            
        bgfx::shutdown();
        bgfxInitialised = false;
    }
}