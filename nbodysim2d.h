#ifndef NBODYSIM2D_H
#define NBODYSIM2D_H

#include <string>
#include <vector>

#define CL_HPP_MINIMUM_OPENCL_VERSION 100
#define CL_HPP_TARGET_OPENCL_VERSION 100
#include <CL/cl2.hpp>

class NBodySim2D {
public:
    static std::vector<float> generateRandomLocations(uint32_t num_points);

    bool init(const std::vector<std::string>& sources, cl_GLuint opengl_vertex_buffer_id,
        uint32_t num_points, float attraction, float radius, float time_step, float max_pos,
        float max_vel, std::string& error_message);

    bool updateLocations(uint32_t num_points, std::string& error_message);

private:
    cl::Context m_ocl_context;
    cl::CommandQueue m_ocl_cmd_queue;
    cl::Kernel m_ocl_kernel_gravity_accelerations;
    cl::Kernel m_ocl_kernel_leapfrog_positions;
    cl::Kernel m_ocl_kernel_leapfrog_velocities;
    cl::BufferGL m_ocl_buffer_pos;
    cl::Buffer m_ocl_buffer_vel;
    cl::Buffer m_ocl_buffer_acc;
};

#endif // NBODYSIM2D_H
