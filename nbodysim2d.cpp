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


bool NBodySim2D::init(const std::vector<std::string>& sources, cl_GLuint opengl_vertex_buffer_id,
    uint32_t num_points, float attraction, float radius, float time_step, std::string& error_message)
{
    // find OpenCL platforms
    cl_int ocl_err = CL_SUCCESS;
    std::vector<cl::Platform> ocl_platforms;
    cl::Platform::get(&ocl_platforms);

    if (ocl_platforms.empty()) {
        error_message = "No OpenCL platforms found.";
        return false;
    }

    // find compatible OpenCL device and create OpenCL context
    for (const cl::Platform& ocl_platform : ocl_platforms) {
        ocl_err = CL_SUCCESS;

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
            continue;
        }

        break;
    }

    if (ocl_err != CL_SUCCESS) {
        error_message = "No compatible OpenCL devices found.";
        return false;
    }

    // compile OpenCL program
    cl::Program ocl_program(m_ocl_context, sources, &ocl_err);
    if (ocl_err != CL_SUCCESS) {
        error_message = "Cannot create OpenCL program. Error: " + std::to_string(ocl_err);
        return false;
    }

    ocl_err = ocl_program.build("-cl-std=CL1.1");
    if (ocl_err != CL_SUCCESS) {
        error_message = "OpenCL build error: " + std::to_string(ocl_err) + "\n";
        auto build_info = ocl_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>();
        for (auto& device_log_pair : build_info) {
            error_message += device_log_pair.second + "\n";
        }
        return false;
    }

    // create OpenCL kernels
    m_ocl_kernel_gravity_accelerations = cl::Kernel(ocl_program, "accelerations", &ocl_err);
    if (ocl_err != CL_SUCCESS) {
        error_message = "Cannot create OpenCL kernel (accelerations). Error: " + std::to_string(ocl_err);
        return false;
    }

    m_ocl_kernel_leapfrog_positions = cl::Kernel(ocl_program, "positions", &ocl_err);
    if (ocl_err != CL_SUCCESS) {
        error_message = "Cannot create OpenCL kernel (positions). Error: " + std::to_string(ocl_err);
        return false;
    }

    m_ocl_kernel_leapfrog_velocities = cl::Kernel(ocl_program, "velocities", &ocl_err);
    if (ocl_err != CL_SUCCESS) {
        error_message = "Cannot create OpenCL kernel (velocities). Error: " + std::to_string(ocl_err);
        return false;
    }

    // create OpenCL buffers
    m_ocl_buffer_pos = cl::BufferGL(m_ocl_context, CL_MEM_READ_WRITE, opengl_vertex_buffer_id, &ocl_err);
    if (ocl_err != CL_SUCCESS) {
        error_message = "Cannot create OpenCL buffer (positions). Error: " + std::to_string(ocl_err);
        return false;
    }

    std::vector<float> velocities = generateRandomLocations(num_points);
    std::transform(velocities.begin(), velocities.end(), velocities.begin(), [](float& value) { return value * 1000.0f; });
    m_ocl_buffer_vel = cl::Buffer(m_ocl_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, velocities.size() * sizeof(float), velocities.data(), &ocl_err);
    if (ocl_err != CL_SUCCESS) {
        error_message = "Cannot create OpenCL buffer (velocities). Error: " + std::to_string(ocl_err);
        return false;
    }

    m_ocl_buffer_acc = cl::Buffer(m_ocl_context, CL_MEM_READ_WRITE, num_points * sizeof(float), nullptr, &ocl_err);
    if (ocl_err != CL_SUCCESS) {
        error_message = "Cannot create OpenCL buffer (accelerations). Error: " + std::to_string(ocl_err);
        return false;
    }

    // add arguments to "accelerations" kernel
    ocl_err = m_ocl_kernel_gravity_accelerations.setArg<cl::BufferGL>(0, m_ocl_buffer_pos);
    if (ocl_err != CL_SUCCESS) {
        error_message = "Cannot add argument to OpenCL kernel (pos->accelerations). Error: " + std::to_string(ocl_err);
        return false;
    }

    ocl_err = m_ocl_kernel_gravity_accelerations.setArg<cl::Buffer>(1, m_ocl_buffer_acc);
    if (ocl_err != CL_SUCCESS) {
        error_message = "Cannot add argument to OpenCL kernel (acc->accelerations). Error: " + std::to_string(ocl_err);
        return false;
    }

    ocl_err = m_ocl_kernel_gravity_accelerations.setArg<float>(2, attraction);
    if (ocl_err != CL_SUCCESS) {
        error_message = "Cannot add argument to OpenCL kernel (attr->accelerations). Error: " + std::to_string(ocl_err);
        return false;
    }

    ocl_err = m_ocl_kernel_gravity_accelerations.setArg<float>(3, radius);
    if (ocl_err != CL_SUCCESS) {
        error_message = "Cannot add argument to OpenCL kernel (rad->accelerations). Error: " + std::to_string(ocl_err);
        return false;
    }

    // add arguments to "positions" kernel
    ocl_err = m_ocl_kernel_leapfrog_positions.setArg<cl::BufferGL>(0, m_ocl_buffer_pos);
    if (ocl_err != CL_SUCCESS) {
        error_message = "Cannot add argument to OpenCL kernel (pos->positions). Error: " + std::to_string(ocl_err);
        return false;
    }

    ocl_err = m_ocl_kernel_leapfrog_positions.setArg<cl::Buffer>(1, m_ocl_buffer_vel);
    if (ocl_err != CL_SUCCESS) {
        error_message = "Cannot add argument to OpenCL kernel (vel->positions). Error: " + std::to_string(ocl_err);
        return false;
    }

    ocl_err = m_ocl_kernel_leapfrog_positions.setArg<cl::Buffer>(2, m_ocl_buffer_acc);
    if (ocl_err != CL_SUCCESS) {
        error_message = "Cannot add argument to OpenCL kernel (acc->positions). Error: " + std::to_string(ocl_err);
        return false;
    }

    ocl_err = m_ocl_kernel_leapfrog_positions.setArg<float>(3, time_step);
    if (ocl_err != CL_SUCCESS) {
        error_message = "Cannot add argument to OpenCL kernel (dt->positions). Error: " + std::to_string(ocl_err);
        return false;
    }

    ocl_err = m_ocl_kernel_leapfrog_positions.setArg<float>(4, 1.0f);
    if (ocl_err != CL_SUCCESS) {
        error_message = "Cannot add argument to OpenCL kernel (max_pos->positions). Error: " + std::to_string(ocl_err);
        return false;
    }

    ocl_err = m_ocl_kernel_leapfrog_positions.setArg<float>(5, 1.0f);
    if (ocl_err != CL_SUCCESS) {
        error_message = "Cannot add argument to OpenCL kernel (max_vel->positions). Error: " + std::to_string(ocl_err);
        return false;
    }

    // add arguments to "velocities" kernel
    ocl_err = m_ocl_kernel_leapfrog_velocities.setArg<cl::Buffer>(0, m_ocl_buffer_vel);
    if (ocl_err != CL_SUCCESS) {
        error_message = "Cannot add argument to OpenCL kernel (vel->velocities). Error: " + std::to_string(ocl_err);
        return false;
    }

    ocl_err = m_ocl_kernel_leapfrog_velocities.setArg<cl::Buffer>(1, m_ocl_buffer_acc);
    if (ocl_err != CL_SUCCESS) {
        error_message = "Cannot add argument to OpenCL kernel (acc->velocities). Error: " + std::to_string(ocl_err);
        return false;
    }

    ocl_err = m_ocl_kernel_leapfrog_velocities.setArg<float>(2, time_step);
    if (ocl_err != CL_SUCCESS) {
        error_message = "Cannot add argument to OpenCL kernel (dt->velocities). Error: " + std::to_string(ocl_err);
        return false;
    }

    ocl_err = m_ocl_kernel_leapfrog_velocities.setArg<float>(3, 1000.0f);
    if (ocl_err != CL_SUCCESS) {
        error_message = "Cannot add argument to OpenCL kernel (max_vel->velocities). Error: " + std::to_string(ocl_err);
        return false;
    }

    // create OpenCL command queue
    m_ocl_cmd_queue = cl::CommandQueue(m_ocl_context, cl::QueueProperties::None, &ocl_err);
    if (ocl_err != CL_SUCCESS) {
        error_message = "Cannot create OpenCL command queue. Error: " + std::to_string(ocl_err);
        return false;
    }

    return true;
}


bool NBodySim2D::updateLocations(uint32_t num_points, std::string& error_message)
{
    std::vector<cl::Memory> ogl_objects{ m_ocl_buffer_pos };
    cl_int ocl_err = m_ocl_cmd_queue.enqueueAcquireGLObjects(&ogl_objects, nullptr, nullptr);
    if (ocl_err != CL_SUCCESS) {
        error_message = "Cannot acquire OpenGL objects. Error: " + std::to_string(ocl_err);
        return false;
    }

    ocl_err = cl::finish();
    if (ocl_err != CL_SUCCESS) {
        error_message = "Cannot execute OpenCL finish. Error: " + std::to_string(ocl_err);
        return false;
    }

    ocl_err = m_ocl_cmd_queue.enqueueNDRangeKernel(m_ocl_kernel_gravity_accelerations, cl::NDRange(0), cl::NDRange(num_points), cl::NullRange, nullptr, nullptr);
    if (ocl_err != CL_SUCCESS) {
        error_message = "Cannot run OpenCL kernel (accelerations). Error: " + std::to_string(ocl_err);
        return false;
    }

    ocl_err = cl::finish();
    if (ocl_err != CL_SUCCESS) {
        error_message = "Cannot execute OpenCL finish. Error: " + std::to_string(ocl_err);
        return false;
    }

    ocl_err = m_ocl_cmd_queue.enqueueNDRangeKernel(m_ocl_kernel_leapfrog_positions, cl::NDRange(0), cl::NDRange(num_points), cl::NullRange, nullptr, nullptr);
    if (ocl_err != CL_SUCCESS) {
        error_message = "Cannot run OpenCL kernel (positions). Error: " + std::to_string(ocl_err);
        return false;
    }

    ocl_err = cl::finish();
    if (ocl_err != CL_SUCCESS) {
        error_message = "Cannot execute OpenCL finish. Error: " + std::to_string(ocl_err);
        return false;
    }

    ocl_err = m_ocl_cmd_queue.enqueueNDRangeKernel(m_ocl_kernel_gravity_accelerations, cl::NDRange(0), cl::NDRange(num_points), cl::NullRange, nullptr, nullptr);
    if (ocl_err != CL_SUCCESS) {
        error_message = "Cannot run OpenCL kernel (accelerations). Error: " + std::to_string(ocl_err);
        return false;
    }

    ocl_err = cl::finish();
    if (ocl_err != CL_SUCCESS) {
        error_message = "Cannot execute OpenCL finish. Error: " + std::to_string(ocl_err);
        return false;
    }

    ocl_err = m_ocl_cmd_queue.enqueueReleaseGLObjects(&ogl_objects, nullptr, nullptr);
    if (ocl_err != CL_SUCCESS) {
        error_message = "Cannot release OpenGL objects. Error: " + std::to_string(ocl_err);
        return false;
    }

    ocl_err = cl::finish();
    if (ocl_err != CL_SUCCESS) {
        error_message = "Cannot execute OpenCL finish. Error: " + std::to_string(ocl_err);
        return false;
    }

    ocl_err = m_ocl_cmd_queue.enqueueNDRangeKernel(m_ocl_kernel_leapfrog_velocities, cl::NDRange(0), cl::NDRange(num_points), cl::NullRange, nullptr, nullptr);
    if (ocl_err != CL_SUCCESS) {
        error_message = "Cannot run OpenCL kernel (velocities). Error: " + std::to_string(ocl_err);
        return false;
    }

    ocl_err = cl::finish();
    if (ocl_err != CL_SUCCESS) {
        error_message = "Cannot execute OpenCL finish. Error: " + std::to_string(ocl_err);
        return false;
    }

    return true;
}
