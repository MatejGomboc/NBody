#include <random>
#include <algorithm>
#include <functional>
#include "nbodysim2d.h"

#ifdef _WIN32
#define WINDOWS_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

#ifdef __linux__
#include <GL/glx.h>
#endif

std::vector<float> NBodySim2D::generateRandomLocations(uint32_t num_points)
{
    std::vector<float> vertices_data(num_points * 2);
    std::random_device rand_device;
    std::seed_seq rand_seed{ rand_device(), rand_device(), rand_device(), rand_device(), rand_device() };
    std::mt19937 rand_gen(rand_seed);
    std::uniform_real_distribution<float> rand_dist(-1.0f, 1.0f);
    std::generate(vertices_data.begin(), vertices_data.end(), std::bind(rand_dist, rand_gen));
    return vertices_data;
}

bool NBodySim2D::init(const std::vector<std::string>& sources, cl_GLuint opengl_vertex_buffer_id, std::string& error_message)
{
    cl_int ocl_err;
    std::vector<cl::Platform> ocl_platforms;
    cl::Platform::get(&ocl_platforms);

    if (ocl_platforms.empty()) {
        error_message = "No OpenCL platforms found.";
        return false;
    }

    for (const cl::Platform& ocl_platform : ocl_platforms) {
        error_message.clear();

#ifdef _WIN32
        cl_context_properties ocl_context_props[] = {
            CL_GL_CONTEXT_KHR, reinterpret_cast<cl_context_properties>(wglGetCurrentContext()),
            CL_WGL_HDC_KHR, reinterpret_cast<cl_context_properties>(wglGetCurrentDC()),
            CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>(ocl_platform()),
            0
        };
#endif

#ifdef __linux__
        cl_context_properties ocl_context_props[] = {
            CL_GL_CONTEXT_KHR, reinterpret_cast<cl_context_properties>(glXGetCurrentContext()),
            CL_GLX_DISPLAY_KHR, reinterpret_cast<cl_context_properties>(glXGetCurrentDisplay()),
            CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>(ocl_platform()),
            0
        };
#endif

        m_ocl_context = cl::Context(CL_DEVICE_TYPE_ALL, ocl_context_props, nullptr, nullptr, &ocl_err);
        if (ocl_err != CL_SUCCESS) {
            error_message = "OpenCL error: " + std::to_string(ocl_err);
            continue;
        }

        break;
    }

    if (!error_message.empty()) {
        error_message = "No compatible OpenCL devices found.";
        return false;
    }

    m_ocl_cmd_queue = cl::CommandQueue(m_ocl_context, cl::QueueProperties::None, &ocl_err);
    if (ocl_err != CL_SUCCESS) {
        error_message = "OpenCL error: " + std::to_string(ocl_err);
        return false;
    }

    cl::Program ocl_program(m_ocl_context, sources, &ocl_err);
    if (ocl_err != CL_SUCCESS) {
        error_message = "OpenCL error: " + std::to_string(ocl_err);
        return false;
    }

    /*ocl_err = ocl_program.build("-cl-std=CL1.0");
    if (ocl_err != CL_SUCCESS) {
        error_message = "OpenCL error: " + std::to_string(ocl_err);
        return false;
    }*/

    return true;
}

bool NBodySim2D::updateLocations(std::string& error_message)
{
    return false;
}
