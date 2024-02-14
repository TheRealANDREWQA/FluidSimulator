// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "fluidSimulatorWindow.h"
#include "GPU/Simulation.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include "ButtonState.h"

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Main code
int main(int, char**)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(2560, 1200, "Fluid Simulator", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwSwapInterval(0); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCanvasResizeCallback("#canvas");
#endif
    ImGui_ImplOpenGL3_Init(glsl_version);


    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.2f, 0.2f, 0.2f, 1.00f);
    FluidSimulatorWindow fluid_simulator_window;
    ButtonStates button_states{ 500 };

    // Main loop
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!glfwWindowShouldClose(window))
#endif
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        ImGui::Begin("Fluid Simulator Main Window", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse
            | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        static bool hide_ui = false;
        auto update_key_entry = [&button_states, window](int key) {
            button_states.UpdateEntry(key, glfwGetKey(window, key) == GLFW_RELEASE);
        };

        ImGui::SetWindowSize({ (float)display_w, (float)display_h });

        // Update the button states, for those that we are interested in
        update_key_entry(GLFW_KEY_H);
        update_key_entry(GLFW_KEY_P);
        update_key_entry(GLFW_KEY_G);

        if (button_states.IsPressed(GLFW_KEY_H)) {
            hide_ui = !hide_ui;
        }
        if (button_states.IsPressed(GLFW_KEY_P)) {
            fluid_simulator_window.simulation.InvertPauseStatus();
        }

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        bool interacting_with_ui = false;
        auto convert_ndc_to_imgui = [&](Float2 position) {
            Float2 multiply_factor = { display_h / (POSITION_FACTOR * 2.0f), display_h / (POSITION_FACTOR * 2.0f) };
            position.y = -position.y;
            position += Float2(POSITION_FACTOR * (float)display_w / (float)display_h, POSITION_FACTOR);
            return position * multiply_factor;
        };

        GeneralSettings* general_settings = fluid_simulator_window.simulation.GetGeneralSettings();
        if (!hide_ui) {
            interacting_with_ui |= ImGui::SliderFloat("Gravity", &general_settings->gravity, -500.0f, 500.0f);
            interacting_with_ui |= ImGui::IsItemActive();
            interacting_with_ui |= ImGui::SliderFloat("Collision Damping", &general_settings->collision_damping, 0.0f, 1.0f);
            interacting_with_ui |= ImGui::IsItemActive();
            interacting_with_ui |= ImGui::SliderFloat("Smoothing Radius", &general_settings->smoothing_radius, 0.005f, 100.0f);
            interacting_with_ui |= ImGui::IsItemActive();
            interacting_with_ui |= ImGui::SliderFloat("Target Density", &general_settings->target_density, 0.005f, 500.0f);
            interacting_with_ui |= ImGui::IsItemActive();
            interacting_with_ui |= ImGui::SliderFloat("Pressure Multiplier", &general_settings->pressure_multiplier, 0.0f, 100.0f);
            interacting_with_ui |= ImGui::IsItemActive();
            interacting_with_ui |= ImGui::SliderFloat("Near Pressure Multiplier", &general_settings->near_pressure_multiplier, 0.0f, 100.0f);
            interacting_with_ui |= ImGui::IsItemActive();
            interacting_with_ui |= ImGui::SliderFloat("Viscosity Strength", &general_settings->viscosity_strength, 0.0f, 10.0f);
            interacting_with_ui |= ImGui::IsItemActive();
            if (ImGui::Button("Restart")) {
                fluid_simulator_window.simulation.Reset();
            }
            if (ImGui::Button("Set Default")) {
                fluid_simulator_window.simulation.SetInitialSettingsData();
            }
            interacting_with_ui |= ImGui::Checkbox("Mouse pull", fluid_simulator_window.simulation.GetUseMousePullPtr());
            interacting_with_ui |= ImGui::IsItemActive();;
            interacting_with_ui |= ImGui::SliderFloat("Interaction Input Radius", &general_settings->interaction_input_radius, 0.0f, 1000.0f);
            interacting_with_ui |= ImGui::IsItemActive();
            interacting_with_ui |= ImGui::SliderFloat("Interaction Input Strength", fluid_simulator_window.simulation.GetMouseClickStrength(), 0.0f, 10000.0f);
            interacting_with_ui |= ImGui::IsItemActive();
            interacting_with_ui |= ImGui::Checkbox("Paint collision", fluid_simulator_window.simulation.GetPaintCollisionPtr());
            interacting_with_ui |= ImGui::IsItemActive();;
            bool size_interaction = ImGui::SliderInt("Paint size", (int*)fluid_simulator_window.simulation.GetPaintCollisionSizePtr(), 25, 500);
            size_interaction |= ImGui::IsItemActive();
            interacting_with_ui |= size_interaction;
            if (size_interaction) {
                fluid_simulator_window.simulation.GetPaintCollisionSizePtr()->y = fluid_simulator_window.simulation.GetPaintCollisionSizePtr()->x;
            }

            auto convert_float4_to_color = [&](Float4 color) {
                return IM_COL32(color.x * 255.0f, color.y * 255.0f, color.z * 255.0f, color.w * 255.0f);
            };

            auto display_gradient = [&](Float2 top_left, Float2 bottom_right, Float4 left_color, Float4 right_color, bool imgui_positions) {
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                if (!imgui_positions) {
                    top_left = convert_ndc_to_imgui(top_left);
                    bottom_right = convert_ndc_to_imgui(bottom_right);
                }

                ImU32 imgui_left = convert_float4_to_color(left_color);
                ImU32 imgui_right = convert_float4_to_color(right_color);
                draw_list->AddRectFilledMultiColor(
                    top_left,
                    bottom_right,
                    imgui_left,
                    imgui_right,
                    imgui_right,
                    imgui_left
                );
            };

            auto display_gradient4 = [&](
                Float2 top_left,
                Float2 bottom_right,
                HeatmapEntry first,
                HeatmapEntry second,
                HeatmapEntry third,
                HeatmapEntry fourth,
                bool imgui_positions
                ) {
                    Float2 current_gradient_start = top_left;
                    if (first.percentage > 0.0f) {
                        Float2 size = bottom_right - top_left;
                        size.x *= first.percentage;
                        display_gradient(current_gradient_start, current_gradient_start + size, first.color, first.color, imgui_positions);
                        current_gradient_start.x += size.x;
                    }

                    Float2 size = bottom_right - top_left;
                    size.x *= second.percentage - first.percentage;
                    display_gradient(current_gradient_start, current_gradient_start + size, first.color, second.color, imgui_positions);
                    current_gradient_start.x += size.x;

                    size = bottom_right - top_left;
                    size.x *= third.percentage - second.percentage;
                    display_gradient(current_gradient_start, current_gradient_start + size, second.color, third.color, imgui_positions);
                    current_gradient_start.x += size.x;

                    size = bottom_right - top_left;
                    size.x *= fourth.percentage - third.percentage;
                    display_gradient(current_gradient_start, current_gradient_start + size, third.color, fourth.color, imgui_positions);
                    current_gradient_start.x += size.x;

                    if (fourth.percentage < 1.0f) {
                        size = bottom_right - top_left;
                        size.x *= 1.0f - fourth.percentage;
                        display_gradient(current_gradient_start, current_gradient_start + size, fourth.color, fourth.color, imgui_positions);
                    }
            };

            ImColor colorTopLeft = IM_COL32(255, 0, 0, 255);
            ImColor colorTopRight = IM_COL32(255, 0, 0, 255);
            ImColor colorBottomRight = IM_COL32(0, 255, 0, 255);
            ImColor colorBottomLeft = IM_COL32(0, 255, 0, 255);

            std::vector<HeatmapEntry>& heatmap_entries = fluid_simulator_window.simulation.GetHeatmapEntries();

            if (ImGui::ColorEdit4("First", (float*)&heatmap_entries[0].color, ImGuiColorEditFlags_NoInputs)) {
                fluid_simulator_window.simulation.RecalculateHeatmap();
                interacting_with_ui = true;
            }
            if (ImGui::ColorEdit4("Second", (float*)&heatmap_entries[1].color, ImGuiColorEditFlags_NoInputs)) {
                fluid_simulator_window.simulation.RecalculateHeatmap();
                interacting_with_ui = true;
            }
            if (ImGui::ColorEdit4("Third", (float*)&heatmap_entries[2].color, ImGuiColorEditFlags_NoInputs)) {
                fluid_simulator_window.simulation.RecalculateHeatmap();
                interacting_with_ui = true;
            }
            if (ImGui::ColorEdit4("Fourth", (float*)&heatmap_entries[3].color, ImGuiColorEditFlags_NoInputs)) {
                fluid_simulator_window.simulation.RecalculateHeatmap();
                interacting_with_ui = true;
            }

            ImVec2 draw_position = ImGui::GetCursorPos();
            Float2 float_draw_position = { draw_position.x, draw_position.y };
            display_gradient4(
                float_draw_position,
                float_draw_position + Float2{ 800, 50 },
                heatmap_entries[0],
                heatmap_entries[1],
                heatmap_entries[2],
                heatmap_entries[3],
                true
            );
        }

        Float2 obstacle_pos = general_settings->obstacle_centre;
        Float2 obstacle_size = general_settings->obstacle_size;
        draw_list->AddRect(convert_ndc_to_imgui(obstacle_pos - obstacle_size), convert_ndc_to_imgui(obstacle_pos + obstacle_size), IM_COL32(20, 180, 30, 255));

        if (*fluid_simulator_window.simulation.GetPaintCollisionPtr()) {
            ImVec2 mouse_pos = ImGui::GetMousePos();
            Int2 paint_size = *fluid_simulator_window.simulation.GetPaintCollisionSizePtr();
            draw_list->AddRectFilled(
                { mouse_pos.x - (paint_size.x / 2), mouse_pos.y - (paint_size.y / 2) },
                { mouse_pos.x + (paint_size.x / 2), mouse_pos.y + (paint_size.y / 2) },
                IM_COL32(20, 180, 30, 255)
            );
        }
        else if (*fluid_simulator_window.simulation.GetUseMousePullPtr()) {
            draw_list->AddCircle(ImGui::GetMousePos(), general_settings->interaction_input_radius / (POSITION_FACTOR * 2.0f) * display_w / 2.0f, IM_COL32(255, 30, 30, 255));
        }

        ImGui::End();

        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        bool is_left_mouse_pressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        bool is_right_mouse_pressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
        fluid_simulator_window.SetWindowDimensions(display_w, display_h);
        if (interacting_with_ui) {
            is_left_mouse_pressed = false;
            is_right_mouse_pressed = false;
        }
        fluid_simulator_window.Draw(is_left_mouse_pressed, is_right_mouse_pressed, io);

        // Rendering
        ImGui::Render();       
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
