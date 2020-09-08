#ifndef NBODYSIM2D_H
#define NBODYSIM2D_H

#include <string>

#define CL_HPP_MINIMUM_OPENCL_VERSION 100
#define CL_HPP_TARGET_OPENCL_VERSION 100
#include <CL/cl2.hpp>

class NBodySim2D {
public:
    static std::vector<float> generateRandomLocations(uint32_t num_points);

private:
    uint32_t m_num_points;
    cl::Context m_ocl_context;
    cl::CommandQueue m_ocl_cmd_queue;
    cl::Kernel m_ocl_kernel;
    cl::BufferGL m_ocl_buffer_pos;
    cl::Buffer m_ocl_buffer_vel;
    cl::Buffer m_ocl_buffer_acc;
};

#endif // NBODYSIM2D_H
