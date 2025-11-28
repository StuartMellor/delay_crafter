// #pragma once

// #include <juce_gui_basics/juce_gui_basics.h>
// #include <memory>
// #include "BinaryData.h"


// static std::unique_ptr<juce::Drawable> loadSVG(const char* fileName, int fileSize)
// {
//     auto svg = juce::Drawable::createFromImageData(fileName, static_cast<size_t>(fileSize));
    
//     if (svg != nullptr)
//     {
//         if (auto comp = dynamic_cast<juce::DrawableImage*>(svg.get()))
//             comp->setBoundingBox({ 0.0f, 0.0f, 100.0f, 100.0f });
//     }
    
//     return svg;
// }

// static std::unique_ptr<juce::Drawable> loadDelayCrafterSVG()
// {
//     return loadSVG(BinaryData::Delay_Crafter_svg, BinaryData::Delay_Crafter_svgSize);
// }
