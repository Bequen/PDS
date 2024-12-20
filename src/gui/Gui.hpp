#include "imgui.h"

#include <stdio.h>


struct GLFWwindow;

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}


class Gui {
private:
    GLFWwindow *window;

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

public:
    Gui() : window(nullptr){}

    ~Gui();

    void start();

    bool should_close();

    void begin_frame();

    void end_frame();

    void run();
};
