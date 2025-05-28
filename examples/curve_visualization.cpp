#include <anim.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"
#include <vector>
#include <string>
#include <iostream>
#include <cmath> // For sin and acos
#include <algorithm> // For std::min/max if used, not directly in this diff for axis

// Helper to display a little (?) mark which shows a tooltip when hovered.
static void ShowHelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

// Example curves
std::vector<anim::Channel> curves;
// Storage for keyframe visibility toggles
static std::vector<std::vector<bool>> s_keyframe_visibilities;
static bool s_vis_data_initialized = false; // To track if visibility data is synced with curves
double eval_step = 0.01;

// Enum string mappers - ensure these match your anim::Function and anim::HandleMode enum order and values
const char* const c_func_items[] = { "Constant", "Linear", "Bezier" }; // Assuming anim::Function::constant=0, linear=1, bezier=2
const char* const c_hmode_items[] = { "Flat", "Smooth", "Aligned", "Free" }; // anim::HandleMode::flat=0, smooth=1, aligned=2, free=3


void CreateExampleCurves() {
    curves.clear(); 
    s_keyframe_visibilities.clear(); 

    if (false) { // Placeholder for future curves, currently only sine wave
        // Curve 1: Simple Sine Wave (8 points) - only curve for now
        anim::Channel sine_curve("Sine Wave");
        for (float t = 0; t <= 32.f; t += 8.f) { 
            sine_curve.create_keyframe(static_cast<double>(t), static_cast<double>(sin(t)));
        }
        curves.push_back(sine_curve);
    }

    if (false){
        // Curve 1: Simple Sine Wave (8 points) - only curve for now
        anim::Channel sine_curve("Sine Wave");
        for (float t = 0; t <= 32.f; t += 8.f) { 
            sine_curve.create_keyframe(static_cast<double>(t), static_cast<double>(sin(t)), anim::Function::linear);
        }
        curves.push_back(sine_curve);
    }

    if (false){
        // Curve 1: Simple Sine Wave (8 points) - only curve for now
        anim::Channel sine_curve("Sine Wave");
        for (float t = 0; t <= 32.f; t += 8.f) { 
            sine_curve.create_keyframe(static_cast<double>(t), static_cast<double>(sin(t)), anim::Function::bezier, anim::HandleMode::flat);
        }
        curves.push_back(sine_curve);
    }

    if (true){
        // Curve 1: Simple Sine Wave (8 points) - only curve for now
        anim::Channel sine_curve("Sine Wave");
        for (float t = 0; t <= 32.f; t += 8.f) { 
            sine_curve.create_keyframe(static_cast<double>(t), static_cast<double>(sin(t)), anim::Function::bezier, anim::HandleMode::aligned);
        }
        curves.push_back(sine_curve);
    }   

    if (false){
        // Curve 1: Simple Sine Wave (8 points) - only curve for now
        anim::Channel sine_curve("Sine Wave");
        for (float t = 0; t <= 32.f; t += 8.f) { 
            sine_curve.create_keyframe(static_cast<double>(t), static_cast<double>(sin(t)), anim::Function::bezier, anim::HandleMode::free);
        }
        curves.push_back(sine_curve);
    }   

    // Initialize visibility data after curves are created
    s_keyframe_visibilities.resize(curves.size());
    for(size_t i = 0; i < curves.size(); ++i) {
        s_keyframe_visibilities[i].assign(curves[i].num_keyframes(), true); // All keyframes visible by default
    }
    s_vis_data_initialized = true;
}


int main() {
    // Setup window
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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif    // Create window with graphics context
    // Get primary monitor size to center the window
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primary);
    
    int windowWidth = 3200;
    int windowHeight = 1280;
    
    // Calculate centered position
    int xpos = (mode->width - windowWidth) / 2;
    int ypos = (mode->height - windowHeight) / 2;
    
    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Anim Curve Visualizer", NULL, NULL);
    if (window == NULL)
        return 1;
    
    // Set window position to center it
    glfwSetWindowPos(window, xpos, ypos);
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize OpenGL loader" << std::endl;
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    
    // Enable docking and viewports if supported by the ImGui build
    #ifdef IMGUI_HAS_DOCK
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    #endif
    #ifdef IMGUI_HAS_VIEWPORT
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    #endif

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    #ifdef IMGUI_HAS_VIEWPORT
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
    #endif

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    CreateExampleCurves(); // Curves and visibility data are initialized here
    
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    static bool show_app_metrics = false;
    static bool first_time_docking_layout = true;


    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // DockSpace setup for initial layout
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
            ImGui::DockSpaceOverViewport(0u, ImGui::GetMainViewport());
        }


        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Exit")) { glfwSetWindowShouldClose(window, true); }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {
                ImGui::MenuItem("Metrics/Debugger", NULL, &show_app_metrics);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
        
        if (show_app_metrics) {
             ImGui::ShowMetricsWindow(&show_app_metrics);
        }


        ImGui::Begin("Curves Plot");
        // Help marker can be removed or kept if desired
        // ImGui::Text("Displaying example animation curves.");
        // ImGui::SameLine(); ShowHelpMarker("Right-click to configure plot.\nCtrl+Click to box select.");

        if (ImPlot::BeginPlot("Animation Curves", ImVec2(-1,-1), ImPlotFlags_NoLegend)) { // Fill parent window
            ImPlot::SetupAxes("Time (s)", "Value");
            ImPlot::SetupAxesLimits(-2, 34, -2, 3, ImGuiCond_Once); // Adjusted limits for new data range

            for (size_t i = 0; i < curves.size(); ++i) {
                anim::Channel& curve = curves[i];
                std::vector<double> x_data, y_data; // Use double for anim library consistency
  
                // Sample the curve for plotting
                if (curve.num_keyframes() > 0) { // Use num_keyframes()
                    double start_time = curve.start_time(); // Use channel's start_time()
                    double end_time = curve.end_time();   // Use channel's end_time()

                    if (curve.num_keyframes() == 1) {
                        x_data.push_back(start_time);
                        y_data.push_back(curve.evaluate(start_time));
                    }
                    else if (start_time < end_time) {
                         for (double t = start_time; t <= end_time; t += eval_step) {
                            x_data.push_back(t);
                            y_data.push_back(curve.evaluate(t));
                        }
                        // Ensure the last point is plotted if not caught by the loop condition
                        if (x_data.empty() || x_data.back() < end_time) {
                             x_data.push_back(end_time);
                             y_data.push_back(curve.evaluate(end_time));
                        }
                    }
                }
                
                if (!x_data.empty()) {
                    // ImPlot expects float pointers, so we need to convert or use a wrapper if available
                    // For simplicity, let's prepare float vectors for ImPlot
                    std::vector<float> x_data_f(x_data.begin(), x_data.end());
                    std::vector<float> y_data_f(y_data.begin(), y_data.end());
                    ImPlot::PlotLine(curve.name().c_str(), x_data_f.data(), y_data_f.data(), x_data_f.size());
                }

                // Plot keyframes as points & handles
                std::vector<float> kf_x_f, kf_y_f;
                for (size_t k_idx = 0; k_idx < curve.num_keyframes(); ++k_idx) {
                    // Check visibility
                    if (!s_vis_data_initialized || i >= s_keyframe_visibilities.size() || k_idx >= s_keyframe_visibilities[i].size() || !s_keyframe_visibilities[i][k_idx]) {
                        continue; // Skip plotting this keyframe if not visible or data is out of sync
                    }

                    const auto& kf = curve.keyframe(k_idx); // Access keyframe by index
                    kf_x_f.push_back(static_cast<float>(kf.time()));
                    kf_y_f.push_back(static_cast<float>(kf.value()));

                    // Plot handles for bezier curves if keyframe is visible
                    if (kf.function == anim::Function::bezier) {
                        // Access absolute handle positions
                        anim::Point in_handle_abs = kf.in_handle;
                        anim::Point out_handle_abs = kf.out_handle;

                        float kf_t_plot = static_cast<float>(kf.time());
                        float kf_v_plot = static_cast<float>(kf.value());

                        // Absolute coordinates for handle points
                        float in_h_t_abs_plot = static_cast<float>(in_handle_abs.time);
                        float in_h_v_abs_plot = static_cast<float>(in_handle_abs.value);
                        float out_h_t_abs_plot = static_cast<float>(out_handle_abs.time);
                        float out_h_v_abs_plot = static_cast<float>(out_handle_abs.value);
                        
                        std::string handle_label_base = curve.name() + "_KF" + std::to_string(k_idx);

                        // Line from keyframe to in-handle
                        float line_in_x[] = { kf_t_plot, in_h_t_abs_plot };
                        float line_in_y[] = { kf_v_plot, in_h_v_abs_plot };
                        ImPlot::PlotLine((handle_label_base + "_InLine").c_str(), line_in_x, line_in_y, 2);
                        
                        // Line from keyframe to out-handle
                        float line_out_x[] = { kf_t_plot, out_h_t_abs_plot };
                        float line_out_y[] = { kf_v_plot, out_h_v_abs_plot };
                        ImPlot::PlotLine((handle_label_base + "_OutLine").c_str(), line_out_x, line_out_y, 2);

                        // Scatter for handle points
                        float handle_pts_x[] = { in_h_t_abs_plot, out_h_t_abs_plot };
                        float handle_pts_y[] = { in_h_v_abs_plot, out_h_v_abs_plot };
                        ImPlot::SetNextMarkerStyle(ImPlotMarker_Diamond);
                        ImPlot::PlotScatter((handle_label_base + "_Handles").c_str(), handle_pts_x, handle_pts_y, 2);
                    }
                }
                if (!kf_x_f.empty()) {
                    ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
                    ImPlot::PlotScatter((curve.name() + " Keyframes").c_str(), kf_x_f.data(), kf_y_f.data(), kf_x_f.size());
                }
            }
            ImPlot::EndPlot();
        }
        ImGui::End();
        
        ImGui::Begin("Curve Editor");
        // No CollapsingHeader for "Edit Curves", content is directly in the curve's TreeNode
        for (size_t i = 0; i < curves.size(); ++i) {
            anim::Channel& curve = curves[i];
            std::string curve_node_label = curve.name() + "##curve_" + std::to_string(i);
            if (ImGui::TreeNode(curve_node_label.c_str())) {
                // Optional: Edit curve name (InputText here if needed)

                // Keyframe editing table header (optional, for alignment reference)
                // ImGui::Text("Vis | Time | Value | In-Handle (dx,dy) | Out-Handle (dx,dy) | Interpolation | Handle Mode");
                // ImGui::Separator();

                for (size_t k = 0; k < curve.num_keyframes(); ++k) {
                    ImGui::PushID(static_cast<int>(k)); // Unique ID scope for widgets of keyframe k
                    const anim::Keyframe& current_kf = curve.keyframe(k);
                    
                    // Visibility Toggle
                    if (s_vis_data_initialized && i < s_keyframe_visibilities.size() && k < s_keyframe_visibilities[i].size()) {
                        bool vis = s_keyframe_visibilities[i][k];
                        if (ImGui::Checkbox("##Visible", &vis)) {
                            s_keyframe_visibilities[i][k] = vis;
                        }
                    } else {
                        ImGui::TextDisabled("N/A"); // Fallback if visibility data is not ready
                    }
                    ImGui::SameLine();

                    // Time and Value
                    float t_edit = static_cast<float>(current_kf.time());
                    float v_edit = static_cast<float>(current_kf.value());

                    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 4.0f);
                    if (ImGui::DragFloat("T", &t_edit, 0.05f, 0, 0, "%.2f")) {
                        curve.set_keyframe_time(k, static_cast<double>(t_edit));
                        // Note: Changing time might re-sort keyframes. The visibility flag s_keyframe_visibilities[i][k]
                        // will stick to the *index* k, not the specific keyframe instance if it moves.
                        // For robust state preservation on sort, keyframe IDs would be needed.
                    }
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 4.0f);
                    if (ImGui::DragFloat("V", &v_edit, 0.05f, 0, 0, "%.2f")) {
                        curve.set_keyframe_value(k, static_cast<double>(v_edit));
                    }
                    ImGui::SameLine();

                    // Handles (absolute positions)
                    anim::Point current_in_handle = current_kf.in_handle;
                    float in_h_edit[2] = { static_cast<float>(current_in_handle.time), static_cast<float>(current_in_handle.value) };
                    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 7.0f); 
                    if (ImGui::DragFloat2("In", in_h_edit, 0.05f, 0, 0, "%.2f")) {
                        // Assuming Channel has set_keyframe_in_handle taking absolute Point
                        curve.set_keyframe_in_handle(k, anim::Point(static_cast<double>(in_h_edit[0]), static_cast<double>(in_h_edit[1])));
                    }
                    ImGui::SameLine();

                    anim::Point current_out_handle = current_kf.out_handle;
                    float out_h_edit[2] = { static_cast<float>(current_out_handle.time), static_cast<float>(current_out_handle.value) };
                    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 7.0f);
                    if (ImGui::DragFloat2("Out", out_h_edit, 0.05f, 0, 0, "%.2f")) {
                        // Assuming Channel has set_keyframe_out_handle taking absolute Point
                        curve.set_keyframe_out_handle(k, anim::Point(static_cast<double>(out_h_edit[0]), static_cast<double>(out_h_edit[1])));
                    }
                    ImGui::SameLine();
                    
                    // Interpolation Function
                    int func_type_idx = static_cast<int>(current_kf.function);
                    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 7.0f);
                    if (ImGui::Combo("Func", &func_type_idx, c_func_items, IM_ARRAYSIZE(c_func_items))) {
                        curve.set_keyframe_function(k, static_cast<anim::Function>(func_type_idx));
                    }
                    ImGui::SameLine();

                    // Handle Mode
                    int hmode_idx = static_cast<int>(current_kf.handle_mode);
                    ImGui::SetNextItemWidth(ImGui::GetFontSize() * 7.0f);
                    if (ImGui::Combo("HMode", &hmode_idx, c_hmode_items, IM_ARRAYSIZE(c_hmode_items))) {
                        curve.set_keyframe_handle_mode(k, static_cast<anim::HandleMode>(hmode_idx));
                    }
                    
                    ImGui::PopID();
                } // End keyframes loop

                if (ImGui::Button(("Add Keyframe to " + curve.name()).c_str())) {
                    double new_time = curve.num_keyframes() > 0 ? curve.end_time() + 1.0 : 0.0;
                    curve.create_keyframe(new_time, 0.0); // Add with default settings
                    // Simplest way to update visibility: re-assign for this curve, making all new/existing keyframes visible.
                    // This approach loses any specific on/off states the user might have set for this curve's keyframes.
                    if (s_vis_data_initialized && i < s_keyframe_visibilities.size()) {
                        s_keyframe_visibilities[i].assign(curve.num_keyframes(), true);
                    }
                }
                ImGui::TreePop();
            } // End TreeNode for current curve
        } // End curves loop
        ImGui::End();


        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        #ifdef IMGUI_HAS_VIEWPORT
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
        #endif

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

// Add implementations for Keyframe::set_time, Keyframe::set_value, Channel::sort_keyframes, Channel::keyframes_mutable if they don't exist
// For example, in anim.hpp or relevant .cpp files:
/*/
/ In Keyframe.hpp or similar
void Keyframe::set_time(double new_time) { position.time = new_time; } // If Point has time/value members
void Keyframe::set_value(double new_value) { position.value = new_value; } // If Point has time/value members

 In Channel.hpp or similar
std::vector<Keyframe>& Channel::keyframes_mutable() { return m_keyframes; } // Unlikely based on search
void Channel::sort_keyframes() { // Likely handled internally by insert/update methods
    std::sort(m_keyframes.begin(), m_keyframes.end(), [](const Keyframe& a, const Keyframe& b) {
        return a.time() < b.time();
    });
}
*/


