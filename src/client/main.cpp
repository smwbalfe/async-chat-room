#include "client/gui/glfw_init.hpp"
#include "client_reader.hpp"
#include "client_writer.hpp"

int main()
{

    auto state_ptr = state::setup_state();

    client_reader reader {state_ptr};
    client_writer writer {state_ptr};
    gui::glfw_init glfw {state_ptr};

    return 0;
}
