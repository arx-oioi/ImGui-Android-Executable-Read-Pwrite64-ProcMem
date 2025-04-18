#include <draw.h>
#include <touch.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <iostream>
#include <cstring>
#include <stdint.h>
#include <thread>
#include <string>
#include "Font.h"
#include "mem.h"
EGLDisplay display = EGL_NO_DISPLAY;
EGLConfig config;
EGLSurface surface = EGL_NO_SURFACE;
ANativeWindow *native_window;
EGLContext context = EGL_NO_CONTEXT;
int FPSg = 100000;
Screen full_screen;
int Orientation = 0;
int screen_x = 0, screen_y = 0, density_ = 0;
int init_screen_x = 0, init_screen_y = 0;
bool g_Initialized = false;
ExternFunction externFunction;
MDisplayInfo displayInfo;
string exec(string command) {
    char buffer[128];
    string result;
    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return "popen failed!";
    }
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != nullptr)
            result += buffer;
    }
    pclose(pipe);
    return result;
}
int init_egl(int _screen_x, int _screen_y, bool log) {
    native_window = externFunction.createNativeWindow("Ssage", _screen_x, _screen_y, false);
    ANativeWindow_acquire(native_window);
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        printf("eglGetDisplay error=%u\n", glGetError());
        return -1;
    }
    if (log) {
        printf("eglGetDisplay ok\n");
    }
    if (eglInitialize(display, nullptr, nullptr) != EGL_TRUE) {
        printf("eglInitialize error=%u\n", glGetError());
        return -1;
    }
    if (log) {
        printf("eglInitialize ok\n");
    }
    EGLint num_config = 0;
    const EGLint attribList[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_BLUE_SIZE, 5,   //-->delete
            EGL_GREEN_SIZE, 6,  //-->delete
            EGL_RED_SIZE, 5,    //-->delete
            EGL_BUFFER_SIZE, 32,  //-->new field
            EGL_DEPTH_SIZE, 16,
            EGL_STENCIL_SIZE, 8,
            EGL_NONE
    };
    if (eglChooseConfig(display, attribList, nullptr, 0, &num_config) != EGL_TRUE) {
        printf("eglChooseConfig  error=%u\n", glGetError());
        return -1;
    }
    if (log) {
        printf("num_config=%d\n", num_config);
    }
    if (!eglChooseConfig(display, attribList, &config, 1, &num_config)) {
        printf("eglChooseConfig  error=%u\n", glGetError());
        return -1;
    }
    if (log) {
        printf("eglChooseConfig ok\n");
    }
    EGLint egl_format;
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &egl_format);
    ANativeWindow_setBuffersGeometry(native_window, 0, 0, egl_format);
    const EGLint attrib_list[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, attrib_list);
    if (context == EGL_NO_CONTEXT) {
        printf("eglCreateContext  error = %u\n", glGetError());
        return -1;
    }
    if (log) {
        printf("eglCreateContext ok\n");
    }
    surface = eglCreateWindowSurface(display, config, native_window, nullptr);
    if (surface == EGL_NO_SURFACE) {
        printf("eglCreateWindowSurface  error = %u\n", glGetError());
        return -1;
    }
    if (log) {
        printf("eglCreateWindowSurface ok\n");
    }
    if (!eglMakeCurrent(display, surface, surface, context)) {
        printf("eglMakeCurrent  error = %u\n", glGetError());
        return -1;
    }
    if (log) {
        printf("eglMakeCurrent ok\n");
    }
    return 1;
}
void screen_config() {
   // printf("Screen configraton");
    std::string window_size = exec("wm size");
    sscanf(window_size.c_str(), "Physical size: %dx%d", &screen_x, &screen_y);
   // printf("x:%d y: %d",screen_x,screen_y);

   std::string window_density = exec("wm density");
    sscanf(window_density.c_str(), "Physical density: %d", &density_);

    full_screen.ScreenX = screen_x;
    full_screen.ScreenY = screen_y;

    auto *orithread = new std::thread([&] {
        while (true) {
            displayInfo = externFunction.getDisplayInfo();
            if (displayInfo.orientation == 0 || displayInfo.orientation == 2) {
                screen_x = full_screen.ScreenX;
                screen_y = full_screen.ScreenY;
            }
            if (displayInfo.orientation == 1 || displayInfo.orientation == 3) {
                screen_x = full_screen.ScreenY;
                screen_y = full_screen.ScreenX;
            }
            std::this_thread::sleep_for(0.5s);
        }
    });
    orithread->detach();
}
void TouchThread() {
     Touch_Init(&screen_x, &screen_y);
}
void ImGui_init() {
    if (g_Initialized) {
        return;
    }
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;
  //  ImGui::StyleColorsDark();
	ImGui::StyleColorsClassic();
    ImGui_ImplAndroid_Init(native_window);
    ImGui_ImplOpenGL3_Init("#version 300 es");
    io.Fonts->AddFontFromMemoryTTF(const_cast<std::uint8_t*>(Custom), sizeof(Custom), 30.f, NULL, io.Fonts->GetGlyphRangesThai());
    ImGui::GetStyle().ScaleAllSizes(3.0f);
    io.Fonts->AddFontDefault();
    io.FontGlobalScale = 1.2f; 

    g_Initialized = true;
}
static bool Mode = false;
static bool Mode1 = false;
void tick() {
    ImGuiIO &io = ImGui::GetIO();
    if (display == EGL_NO_DISPLAY)
        return;

 
//static ImVec4 clear_color = ImVec4(0, 0, 0, 0);
 static ImVec4 window_bg_color = ImVec4(0.0f, 0.0f, 0.0f, 0.8f);
 static ImVec4 title_bg_color = ImVec4(0.0f, 0.0f, 0.0f, 0.8f);
 static ImVec4 title_bg_active_color = ImVec4(1.0f, 1.0f, 1.0f, 0.4f);
 static ImVec4 title_bg_collapsed_color = ImVec4(0.0f, 0.0f, 0.0f, 0.8f);
 static ImVec4 text_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

 static ImVec4 button_color = ImVec4(0.5f, 0.5f, 0.5f, 0.4f);
 static ImVec4 button_hovered_color = ImVec4(0.7f, 0.7f, 0.7f, 0.6f);
 static ImVec4 button_active_color = ImVec4(1.0f, 1.0f, 1.0f, 0.8f);

 static ImVec4 header_color = ImVec4(0.5f, 0.5f, 0.5f, 0.4f);
 static ImVec4 header_hovered_color = ImVec4(0.7f, 0.7f, 0.7f, 0.6f);
 static ImVec4 header_active_color = ImVec4(1.0f, 1.0f, 1.0f, 0.8f);

 static ImVec4 slider_color = ImVec4(0.5f, 0.5f, 0.5f, 0.4f); 
 static ImVec4 slider_grab_color = ImVec4(1.0f, 1.0f, 1.0f, 0.6f); 
 static ImVec4 slider_grab_hovered_color = ImVec4(1.0f, 1.0f, 1.0f, 0.8f); 
 static ImVec4 slider_grab_active_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); 

 static ImVec4 checkbox_color = ImVec4(0.5f, 0.5f, 0.5f, 0.4f);
 static ImVec4 checkbox_checked_color = ImVec4(1.0f, 1.0f, 1.0f, 0.8f); 


    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame(init_screen_x, init_screen_y);
    ImGui::NewFrame();

  
    ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = window_bg_color;
    ImGui::GetStyle().Colors[ImGuiCol_TitleBg] = title_bg_color;
    ImGui::GetStyle().Colors[ImGuiCol_TitleBgActive] = title_bg_active_color;
    ImGui::GetStyle().Colors[ImGuiCol_TitleBgCollapsed] = title_bg_collapsed_color;
    ImGui::GetStyle().Colors[ImGuiCol_Text] = text_color;
 //   ImGui::GetStyle().Colors[ImGuiCol_ChildBg] = clear_color;

    ImGui::GetStyle().Colors[ImGuiCol_Header] = header_color;
    ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered] = header_hovered_color;
    ImGui::GetStyle().Colors[ImGuiCol_HeaderActive] = header_active_color;

    ImGui::GetStyle().Colors[ImGuiCol_Button] = button_color;
    ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered] = button_hovered_color;
    ImGui::GetStyle().Colors[ImGuiCol_ButtonActive] = button_active_color;

    ImGui::GetStyle().Colors[ImGuiCol_SliderGrab] = slider_grab_color;
    ImGui::GetStyle().Colors[ImGuiCol_SliderGrabActive] = slider_grab_active_color;
    
    ImGui::GetStyle().Colors[ImGuiCol_CheckMark] = checkbox_checked_color;
    
	
  

    ImGui::Begin(OBFUSCATE("test debug"));

    if (ImGui::Checkbox(Mode ? OBFUSCATE("TEST1 [เปิดอยู่]") : OBFUSCATE("TEST1 [ปิดอยู่]"), &Mode)) {
    ARX(Mode ? 0 : 1); 
}


    if (ImGui::Checkbox(Mode1 ? OBFUSCATE("TEST2 [เปิดอยู่]") : OBFUSCATE("TEST2 [ปิดอยู่]"), &Mode1)) {
    ARX(Mode1 ? 2 : 3); 
}



 //  ImGui::Text(OBFUSCATE("FPS : %.3f ms/frame (%.1f FPS)"), 1000.0f / io.Framerate, io.Framerate);
    
    ImGui::End();

    
    ImGui::Render();
    glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    eglSwapBuffers(display, surface);
}
void shutdown() {
    if (!g_Initialized) {
        return;
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplAndroid_Shutdown();
    ImGui::DestroyContext();
    if (display != EGL_NO_DISPLAY) {
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (context != EGL_NO_CONTEXT) {
            eglDestroyContext(display, context);
        }
        if (surface != EGL_NO_SURFACE) {
            eglDestroySurface(display, surface);
        }
        eglTerminate(display);
    }
    display = EGL_NO_DISPLAY;
    context = EGL_NO_CONTEXT;
    surface = EGL_NO_SURFACE;
    ANativeWindow_release(native_window);
}
//@ankitrawatgit
