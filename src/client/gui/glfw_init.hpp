//
// Created by shriller44 on 12/31/23.
//

#ifndef CHAT_GLFW_INIT_HPP
#define CHAT_GLFW_INIT_HPP

#include "client/state.hpp"
#include "gui_include.hpp"
#include "client/gui/client_gui.hpp"

namespace gui {

    inline bool toggle = true;
    inline void processInput(GLFWwindow* window)
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
        if ((glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) && toggle) {
            std::cout << "you pressed q " << std::endl;
            toggle = false;
            return;
        }
        toggle = true;
    }
    inline void framebuffer_size_callback(GLFWwindow* window, int width, int height){glViewport(0, 0, width, height);}

    constexpr unsigned SCR_WIDTH = 800;
    constexpr unsigned SCR_HEIGHT = 600;

    class glfw_init {
    public:
        glfw_init(state::state_ptr state);
        void run_gui();
    private:
        GLFWwindow *window_;
        client_gui gui_;
        std::jthread gui_thread_;
    };
}

#endif //CHAT_GLFW_INIT_HPP
