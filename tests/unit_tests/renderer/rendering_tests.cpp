/*
 * Copyright 2025 Rive
 */

#include "common/testing_window.hpp"
#include "rive/renderer.hpp"
#include "rive/factory.hpp"
#include <catch.hpp>

namespace rive::gpu
{
// Factories to manually instantiate real rendering contexts, for unit testing
// the full pipeline.
struct FactoryWrapper
{
    const char* displayName;
    std::function<std::unique_ptr<TestingWindow>()> function;
};
static FactoryWrapper testingWindowFactories[] = {
    {"Vulkan",
     []() {
         return rivestd::adopt_unique(TestingWindow::MakeVulkanTexture({
#ifdef RIVE_ANDROID
             // Android doesn't support validation layers for command line
             // apps like the unit_tests.
             .disableValidationLayers = true,
             // The OnePlus7 doesn't support debug callbacks either for
             // command line apps.
             .disableDebugCallbacks = true,
#endif
         }));
     }},
#if defined(__APPLE__)
    {"Metal",
     []() {
         return rivestd::adopt_unique(TestingWindow::MakeMetalTexture({}));
     }},
#endif
#ifdef _WIN32
    {"D3D12",
     []() {
         return rivestd::adopt_unique(TestingWindow::MakeFiddleContext(
             TestingWindow::Backend::d3d12,
             {},
             TestingWindow::Visibility::headless,
             nullptr));
     }},
    {"D3D12 atomic",
     []() {
         return rivestd::adopt_unique(TestingWindow::MakeFiddleContext(
             TestingWindow::Backend::d3d12,
             {.atomic = true},
             TestingWindow::Visibility::headless,
             nullptr));
     }},
    {"D3D11",
     []() {
         return rivestd::adopt_unique(TestingWindow::MakeFiddleContext(
             TestingWindow::Backend::d3d,
             {},
             TestingWindow::Visibility::headless,
             nullptr));
     }},
    {"D3D11 atomic",
     []() {
         return rivestd::adopt_unique(TestingWindow::MakeFiddleContext(
             TestingWindow::Backend::d3d,
             {.atomic = true},
             TestingWindow::Visibility::headless,
             nullptr));
     }},
    {"OpenGL",
     []() {
         return rivestd::adopt_unique(TestingWindow::MakeFiddleContext(
             TestingWindow::Backend::gl,
             {},
             TestingWindow::Visibility::headless,
             nullptr));
     }},
    {"OpenGL atomic",
     []() {
         return rivestd::adopt_unique(TestingWindow::MakeFiddleContext(
             TestingWindow::Backend::gl,
             {.atomic = true},
             TestingWindow::Visibility::headless,
             nullptr));
     }},
#endif
#ifdef RIVE_ANDROID
    {"EGL (GL backend)",
     []() {
         return rivestd::adopt_unique(
             TestingWindow::MakeEGL(TestingWindow::Backend::gl, {}, nullptr));
     }},
#endif
};

// Ensure that rendering still succeeds when compilations fail (e.g., by falling
// back on an uber shader or at least not crashing). Valid compilations may fail
// in the real world if the device is pressed for resources or in a bad state.
TEST_CASE("synthesizedFailureType", "[rendering]")
{
    // TODO: There are potentially stronger ways to build some of these
    // synthesized failures if we were to pass SynthesizedFailureType as a
    // creation option instead of on beginFrame
    for (auto failureType : {SynthesizedFailureType::shaderCompilation,
                             SynthesizedFailureType::ubershaderLoad,
                             SynthesizedFailureType::pipelineCreation})
    {
        switch (failureType)
        {
            case SynthesizedFailureType::shaderCompilation:
                printf("testing synthesied shader compilation failure\n");
                break;
            case SynthesizedFailureType::ubershaderLoad:
                printf("testing synthesized ubershader load failure\n");
                break;
            case SynthesizedFailureType::pipelineCreation:
                printf("testing synthesized pipeline creation failure\n");
                break;
            case SynthesizedFailureType::none:
                // android compiler complains (rightly) if this case isn't here.
                RIVE_UNREACHABLE();
        }
        for (auto& testingWindowFactory : testingWindowFactories)
        {
            printf("  testing with '%s' factory\n",
                   testingWindowFactory.displayName);
            std::unique_ptr<TestingWindow> window =
                testingWindowFactory.function();
            if (window == nullptr)
            {
                continue;
            }
            Factory* factory = window->factory();

            window->resize(32, 32);

            // Expected colors after we draw a cyan rectangle.
            std::vector<uint8_t> drawColors;
            drawColors.reserve(32 * 32 * 4);
            for (size_t i = 0; i < 32 * 32; ++i)
                drawColors.insert(drawColors.end(), {0x00, 0xff, 0xff, 0xff});

            // Expected colors when only the clear happens (because even the
            // uber shader failed to compile).
            std::vector<uint8_t> clearColors;
            clearColors.reserve(32 * 32 * 4);
            for (size_t i = 0; i < 32 * 32; ++i)
                clearColors.insert(clearColors.end(), {0xff, 0x00, 0x00, 0xff});

            for (bool disableRasterOrdering : {false, true})
            {
                auto renderer = window->beginFrame({
                    .clearColor = 0xffff0000,
                    .doClear = true,
                    .disableRasterOrdering = disableRasterOrdering,
                    .synthesizedFailureType = failureType,
                });

                rcp<RenderPath> path =
                    factory->makeRenderPath(AABB{0, 0, 32, 32});
                rcp<RenderPaint> paint = factory->makeRenderPaint();
                paint->color(0xff00ffff);
                renderer->drawPath(path.get(), paint.get());

                std::vector<uint8_t> pixels;
                window->endFrame(&pixels);

                // There are two acceptable results to this test:
                //
                // 1) The draw happens anyway because we fell back on a
                //    precompiled uber shader.
                //
                // 2) The uber shader also synthesizes a compilation faiulre, so
                //    only the clear color makes it through.
                if (pixels != drawColors && pixels != clearColors)
                {
                    printf("Expected {%02x, %02x, %02x, %02x} or {%02x, %02x, "
                           "%02x, %02x}, got {%02x, %02x, %02x, %02x}",
                           drawColors[0],
                           drawColors[1],
                           drawColors[2],
                           drawColors[3],
                           clearColors[0],
                           clearColors[1],
                           clearColors[2],
                           clearColors[3],
                           pixels[0],
                           pixels[1],
                           pixels[2],
                           pixels[3]);
                }
                CHECK((pixels == drawColors || pixels == clearColors));
            }
        }
    }
}
} // namespace rive::gpu
