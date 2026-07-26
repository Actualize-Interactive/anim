#include <anim.hpp>
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

// Example animation containing curves
static anim::Animation animation("Example Animation");
// Storage for keyframe visibility toggles
static std::vector<std::vector<bool>> s_keyframe_visibilities;
static bool s_vis_data_initialized = false; // To track if visibility data is synced with curves

// Selection and interaction state
struct Selection {
    int curve_idx = -1;
    int keyframe_idx = -1;
    bool is_handle = false;
    bool is_in_handle = false; // true for in-handle, false for out-handle
    bool is_dragging = false;
};
static Selection s_selection;
static const float SELECTION_RADIUS = 8.0f; // Pixel radius for selection

double eval_step = 0.01;

// Enum string mappers - ensure these match your anim::Function and anim::HandleMode enum order and values
const char* const c_func_items[] = { "Constant", "Linear", "Bezier" }; // Assuming anim::Function::Constant=0, linear=1, bezier=2
const char* const c_hmode_items[] = { "Flat", "Smooth", "Aligned", "Free", "AlignStrict", "AlignFlex", "AlignAdjustable" }; // anim::HandleMode::Flat=0, smooth=1, aligned=2, free=3


void CreateExampleCurves() {
    animation.clear(); // Clear existing channels
    s_keyframe_visibilities.clear(); 
    
    // Reset selection when curves change
    s_selection = Selection{};

    // The same sine wave once per interpolation style, offset in value so the
    // curves stack legibly in the plot rather than overlapping.
    struct CurveSpec {
        const char*      name;
        double           value_offset;
        anim::Function   function;
        anim::HandleMode handle_mode;
    };
    static const CurveSpec curve_specs[] = {
        {"Sine Wave",         0.00, anim::Function::Bezier, anim::HandleMode::Smooth},
        {"Sine Wave Linear",  0.25, anim::Function::Linear, anim::HandleMode::Smooth},
        {"Sine Wave Flat",    0.50, anim::Function::Bezier, anim::HandleMode::Flat},
        {"Sine Wave Aligned", 0.75, anim::Function::Bezier, anim::HandleMode::Aligned},
        {"Sine Wave Free",    1.00, anim::Function::Bezier, anim::HandleMode::Free},
    };

    for (const CurveSpec& spec : curve_specs) {
        anim::Channel& sine_curve = animation.create_channel(spec.name);
        for (double t = 0.0; t <= 32.0; t += 8.0) {
            sine_curve.create_keyframe(t, sin(t) + spec.value_offset,
                                       spec.function, spec.handle_mode);
        }
    }

    // Initialize visibility data after curves are created
    s_keyframe_visibilities.resize(animation.num_channels());
    for(size_t i = 0; i < animation.num_channels(); ++i) {
        s_keyframe_visibilities[i].assign(animation.channel(i).num_keyframes(), true); // All keyframes visible by default
    }
    s_vis_data_initialized = true;
}

// Helper function to check if a point is near another point in plot coordinates
bool IsPointNear(ImVec2 plot_pos, ImVec2 mouse_plot_pos, float radius_pixels) {
    ImVec2 plot_size = ImPlot::GetPlotSize();
    ImPlotRect limits = ImPlot::GetPlotLimits();
    
    // Convert pixel radius to plot coordinates
    float x_range = limits.X.Max - limits.X.Min;
    float y_range = limits.Y.Max - limits.Y.Min;
    float radius_x = (radius_pixels / plot_size.x) * x_range;
    float radius_y = (radius_pixels / plot_size.y) * y_range;
    
    float dx = plot_pos.x - mouse_plot_pos.x;
    float dy = plot_pos.y - mouse_plot_pos.y;
    
    return (dx*dx)/(radius_x*radius_x) + (dy*dy)/(radius_y*radius_y) <= 1.0f;
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


        // Give both windows a sensible first-run size and position. Without
        // this the plot window auto-fits to its content, but its content is a
        // plot sized ImVec2(-1,-1) ("fill the available space"), so on the
        // first frame the two resolve to nothing and the window collapses to a
        // few pixels — which ImGui then persists to imgui.ini. FirstUseEver
        // means a layout the user has arranged is still respected.
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        const ImVec2 work_pos = viewport->WorkPos;
        const ImVec2 work_size = viewport->WorkSize;
        const float editor_width = std::max(320.0f, work_size.x * 0.22f);
        const float pad = 12.0f;

        ImGui::SetNextWindowPos(ImVec2(work_pos.x + pad, work_pos.y + pad), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(work_size.x - editor_width - pad * 3.0f,
                                        work_size.y - pad * 2.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("Curves Plot — Ctrl-click to move keyframes & handles (panning disabled)###CurvesPlot");

        // Disable ImPlot's click-drag panning so it doesn't fight with dragging
        // keyframes/handles. This must be set BEFORE BeginPlot to take effect for
        // the current frame; it is restored after EndPlot.
        ImPlotInputMap& plot_input = ImPlot::GetInputMap();
        const ImGuiMouseButton original_pan = plot_input.Pan;
        plot_input.Pan = ImGuiMouseButton_COUNT; // no mouse button pans

        if (ImPlot::BeginPlot("Animation Curves", ImVec2(-1,-1), ImPlotFlags_NoLegend)) {
            ImPlot::SetupAxes("Time (s)", "Value");
            ImPlot::SetupAxesLimits(-2, 34, -2, 3, ImGuiCond_Once);

            // Handle mouse interaction
            bool plot_hovered = ImPlot::IsPlotHovered();
            bool mouse_clicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
            bool mouse_down = ImGui::IsMouseDown(ImGuiMouseButton_Left);
            bool mouse_released = ImGui::IsMouseReleased(ImGuiMouseButton_Left);
            bool ctrl_pressed = ImGui::GetIO().KeyCtrl;
            
            auto mouse_pnt = ImPlot::GetPlotMousePos();
            ImVec2 mouse_plot_pos = ImVec2(mouse_pnt.x, mouse_pnt.y);

            // Check if mouse is over any interactive element
            bool mouse_over_interactive_element = false;
            if (plot_hovered) {
                for (size_t i = 0; i < animation.size() && !mouse_over_interactive_element; ++i) {
                    anim::Channel& curve = animation[i];
                    
                    for (size_t k = 0; k < curve.num_keyframes(); ++k) {
                        // Skip invisible keyframes
                        if (!s_vis_data_initialized || i >= s_keyframe_visibilities.size() || 
                            k >= s_keyframe_visibilities[i].size() || !s_keyframe_visibilities[i][k]) {
                            continue;
                        }
                        
                        const auto& kf = curve.keyframe(k);
                        ImVec2 kf_pos = ImVec2(static_cast<float>(kf.time()), static_cast<float>(kf.value()));
                        
                        // Check keyframe
                        if (IsPointNear(kf_pos, mouse_plot_pos, SELECTION_RADIUS)) {
                            mouse_over_interactive_element = true;
                            break;
                        }
                        
                        // Check handles for bezier animation
                        if (kf.function == anim::Function::Bezier) {
                            ImVec2 in_handle_pos = ImVec2(static_cast<float>(kf.in_handle.time), 
                                                         static_cast<float>(kf.in_handle.value));
                            ImVec2 out_handle_pos = ImVec2(static_cast<float>(kf.out_handle.time), 
                                                          static_cast<float>(kf.out_handle.value));
                            
                            if (IsPointNear(in_handle_pos, mouse_plot_pos, SELECTION_RADIUS) ||
                                IsPointNear(out_handle_pos, mouse_plot_pos, SELECTION_RADIUS)) {
                                mouse_over_interactive_element = true;
                                break;
                            }
                        }
                    }
                }
            }

            // Start selection/dragging. Requires Ctrl held (the title documents
            // this); panning is disabled above so the drag won't be hijacked.
            if (plot_hovered && mouse_clicked && ctrl_pressed && mouse_over_interactive_element) {
                s_selection = Selection{}; // Reset selection
                
                // Check for keyframe/handle selection
                for (size_t i = 0; i < animation.size() && s_selection.curve_idx == -1; ++i) {
                    anim::Channel& curve = animation[i];
                    
                    for (size_t k = 0; k < curve.num_keyframes(); ++k) {
                        // Skip invisible keyframes
                        if (!s_vis_data_initialized || i >= s_keyframe_visibilities.size() || 
                            k >= s_keyframe_visibilities[i].size() || !s_keyframe_visibilities[i][k]) {
                            continue;
                        }
                        
                        const auto& kf = curve.keyframe(k);
                        ImVec2 kf_pos = ImVec2(static_cast<float>(kf.time()), static_cast<float>(kf.value()));
                        
                        // Check handles first (for bezier curves) - they have priority over keyframes
                        if (kf.function == anim::Function::Bezier) {
                            ImVec2 in_handle_pos = ImVec2(static_cast<float>(kf.in_handle.time), 
                                                         static_cast<float>(kf.in_handle.value));
                            ImVec2 out_handle_pos = ImVec2(static_cast<float>(kf.out_handle.time), 
                                                          static_cast<float>(kf.out_handle.value));
                            
                            if (IsPointNear(in_handle_pos, mouse_plot_pos, SELECTION_RADIUS)) {
                                s_selection = {static_cast<int>(i), static_cast<int>(k), true, true, false};
                                break;
                            } else if (IsPointNear(out_handle_pos, mouse_plot_pos, SELECTION_RADIUS)) {
                                s_selection = {static_cast<int>(i), static_cast<int>(k), true, false, false};
                                break;
                            }
                        }
                        
                        // Check keyframe
                        if (IsPointNear(kf_pos, mouse_plot_pos, SELECTION_RADIUS)) {
                            s_selection = {static_cast<int>(i), static_cast<int>(k), false, false, false};
                            break;
                        }
                    }
                }
                
                if (s_selection.curve_idx != -1) {
                    s_selection.is_dragging = true;
                }
            }
            
            // Handle dragging
            if (s_selection.is_dragging && mouse_down && s_selection.curve_idx != -1) {
                anim::Channel& curve = animation[s_selection.curve_idx];
                
                if (s_selection.is_handle) {
                    // Move handle
                    anim::Point new_handle_pos(static_cast<double>(mouse_plot_pos.x), 
                                             static_cast<double>(mouse_plot_pos.y));
                    
                    if (s_selection.is_in_handle) {
                        curve.set_keyframe_in_handle(s_selection.keyframe_idx, new_handle_pos);
                    } else {
                        curve.set_keyframe_out_handle(s_selection.keyframe_idx, new_handle_pos);
                    }
                } else {
                    // Move keyframe
                    anim::Point new_kf_pos(static_cast<double>(mouse_plot_pos.x), 
                                           static_cast<double>(mouse_plot_pos.y));
                    curve.set_keyframe_position(s_selection.keyframe_idx, new_kf_pos);
                }
            }
            
            // End dragging
            if (mouse_released) {
                s_selection.is_dragging = false;
            }
            
            // Plot curves
            for (size_t i = 0; i < animation.size(); ++i) {
                anim::Channel& curve = animation[i];
                std::vector<double> x_data, y_data;
  
                // Sample the curve for plotting
                if (curve.num_keyframes() > 0) { // Use num_keyframes()
                    double start_time = curve.start_time(); // Use channel's start_time()
                    double end_time = curve.end_time();   // Use channel's end_time()

                    if (curve.num_keyframes() == 1) {
                        x_data.push_back(start_time);
                        y_data.push_back(curve.evaluate(start_time));
                    }
                    else if (start_time < end_time) {
                        // A closed range, so the plotted line reaches the last
                        // keyframe rather than stopping a step short of it. The
                        // count-based overload is what guarantees that: sampling
                        // by rate would only land on end_time when the span
                        // happens to be a whole number of steps.
                        const double duration = end_time - start_time;
                        const int num_points =
                            static_cast<int>(std::ceil(duration / eval_step)) + 1;
                        y_data = curve.evaluate_range(start_time, end_time, num_points,
                                                      anim::RangeEnd::Inclusive);

                        // evaluate_range returns values only, so rebuild the
                        // times it sampled at for the x axis.
                        const double step = duration / (num_points - 1);
                        x_data.reserve(y_data.size());
                        for (size_t s = 0; s < y_data.size(); ++s) {
                            x_data.push_back(start_time + static_cast<double>(s) * step);
                        }
                    }
                }
                
                if (!x_data.empty()) {
                    // ImPlot expects float pointers, so convert the sampled doubles.
                    std::vector<float> x_data_f, y_data_f;
                    x_data_f.reserve(x_data.size());
                    y_data_f.reserve(y_data.size());
                    for (double v : x_data) x_data_f.push_back(static_cast<float>(v));
                    for (double v : y_data) y_data_f.push_back(static_cast<float>(v));
                    ImPlot::PlotLine(curve.name().c_str(), x_data_f.data(), y_data_f.data(), x_data_f.size());
                }

                // Plot keyframes as points & handles
                std::vector<float> kf_x_f, kf_y_f;
                std::vector<float> selected_kf_x, selected_kf_y;
                
                for (size_t k_idx = 0; k_idx < curve.num_keyframes(); ++k_idx) {
                    if (!s_vis_data_initialized || i >= s_keyframe_visibilities.size() || 
                        k_idx >= s_keyframe_visibilities[i].size() || !s_keyframe_visibilities[i][k_idx]) {
                        continue;
                    }

                    const auto& kf = curve.keyframe(k_idx);
                    float kf_t = static_cast<float>(kf.time());
                    float kf_v = static_cast<float>(kf.value());
                    
                    // Check if this keyframe is selected
                    bool is_selected = (s_selection.curve_idx == static_cast<int>(i) && 
                                       s_selection.keyframe_idx == static_cast<int>(k_idx) && 
                                       !s_selection.is_handle);
                    
                    if (is_selected) {
                        selected_kf_x.push_back(kf_t);
                        selected_kf_y.push_back(kf_v);
                    } else {
                        kf_x_f.push_back(kf_t);
                        kf_y_f.push_back(kf_v);
                    }

                    // Plot handles for bezier curves
                    if (kf.function == anim::Function::Bezier) {
                        anim::Point in_handle_abs = kf.in_handle;
                        anim::Point out_handle_abs = kf.out_handle;

                        float in_h_t = static_cast<float>(in_handle_abs.time);
                        float in_h_v = static_cast<float>(in_handle_abs.value);
                        float out_h_t = static_cast<float>(out_handle_abs.time);
                        float out_h_v = static_cast<float>(out_handle_abs.value);
                        
                        std::string handle_label_base = curve.name() + "_KF" + std::to_string(k_idx);

                        // Handle lines
                        float line_in_x[] = { kf_t, in_h_t };
                        float line_in_y[] = { kf_v, in_h_v };
                        ImPlot::PlotLine((handle_label_base + "_InLine").c_str(), line_in_x, line_in_y, 2);
                        
                        float line_out_x[] = { kf_t, out_h_t };
                        float line_out_y[] = { kf_v, out_h_v };
                        ImPlot::PlotLine((handle_label_base + "_OutLine").c_str(), line_out_x, line_out_y, 2);

                        // Handle points with selection highlighting
                        bool in_handle_selected = (s_selection.curve_idx == static_cast<int>(i) && 
                                                  s_selection.keyframe_idx == static_cast<int>(k_idx) && 
                                                  s_selection.is_handle && s_selection.is_in_handle);
                        bool out_handle_selected = (s_selection.curve_idx == static_cast<int>(i) && 
                                                   s_selection.keyframe_idx == static_cast<int>(k_idx) && 
                                                   s_selection.is_handle && !s_selection.is_in_handle);
                        
                        if (in_handle_selected) {
                            ImPlot::SetNextMarkerStyle(ImPlotMarker_Diamond, 8.0f, ImVec4(1,1,0,1)); // Yellow for selected
                            float selected_in_x[] = { in_h_t };
                            float selected_in_y[] = { in_h_v };
                            ImPlot::PlotScatter((handle_label_base + "_InSelected").c_str(), selected_in_x, selected_in_y, 1);
                        }
                        if (out_handle_selected) {
                            ImPlot::SetNextMarkerStyle(ImPlotMarker_Diamond, 8.0f, ImVec4(1,1,0,1)); // Yellow for selected
                            float selected_out_x[] = { out_h_t };
                            float selected_out_y[] = { out_h_v };
                            ImPlot::PlotScatter((handle_label_base + "_OutSelected").c_str(), selected_out_x, selected_out_y, 1);
                        }
                        
                        // Plot non-selected handles
                        std::vector<float> normal_handle_x, normal_handle_y;
                        if (!in_handle_selected) {
                            normal_handle_x.push_back(in_h_t);
                            normal_handle_y.push_back(in_h_v);
                        }
                        if (!out_handle_selected) {
                            normal_handle_x.push_back(out_h_t);
                            normal_handle_y.push_back(out_h_v);
                        }
                        if (!normal_handle_x.empty()) {
                            ImPlot::SetNextMarkerStyle(ImPlotMarker_Diamond);
                            ImPlot::PlotScatter((handle_label_base + "_Handles").c_str(), 
                                              normal_handle_x.data(), normal_handle_y.data(), normal_handle_x.size());
                        }
                    }
                }
                
                // Plot normal keyframes
                if (!kf_x_f.empty()) {
                    ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
                    ImPlot::PlotScatter((curve.name() + " Keyframes").c_str(), kf_x_f.data(), kf_y_f.data(), kf_x_f.size());
                }
                
                // Plot selected keyframe with different style
                if (!selected_kf_x.empty()) {
                    ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 8.0f, ImVec4(1,1,0,1)); // Yellow for selected
                    ImPlot::PlotScatter((curve.name() + " Selected").c_str(), selected_kf_x.data(), selected_kf_y.data(), selected_kf_x.size());
                }
            }
            // Restore input map after all plot operations are complete
            plot_input.Pan = original_pan;
            
            ImPlot::EndPlot();
        }
        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(work_pos.x + work_size.x - editor_width - pad,
                                       work_pos.y + pad), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(editor_width, work_size.y - pad * 2.0f),
                                 ImGuiCond_FirstUseEver);
        ImGui::Begin("Curve Editor");
        // No CollapsingHeader for "Edit Curves", content is directly in the curve's TreeNode
        for (size_t i = 0; i < animation.size(); ++i) {
            anim::Channel& curve = animation[i];
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


