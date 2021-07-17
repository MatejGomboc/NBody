#include <iostream>
#include <CL/opencl.hpp>

int main()
{
    std::vector<cl::Platform> ocl_platforms;
    cl_int ocl_err = cl::Platform::get(&ocl_platforms);
    if (ocl_err != CL_SUCCESS) {
        std::cerr << "cl::Platform::get(), error: " << ocl_err << std::endl;
        return 0;
    }

    for (size_t i = 0; i < ocl_platforms.size(); i++) {
        std::string ocl_platform_version = ocl_platforms[i].getInfo<CL_PLATFORM_VERSION>();
        if (ocl_platform_version.find("OpenCL 1.2") == std::string::npos) {
            if (ocl_platform_version.find("OpenCL 2.") == std::string::npos) {
                if (ocl_platform_version.find("OpenCL 3.") == std::string::npos) {
                    ocl_platforms.erase(ocl_platforms.begin() + i);
                    i--;
                }
            }
        }
    }

    std::cout << "num of compatible OCL platforms: " << ocl_platforms.size() << std::endl;

    std::vector<cl::Context> ocl_contexts;
    for (const cl::Platform& ocl_platform : ocl_platforms) {
        cl_context_properties ocl_context_props[] = {
            CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>(ocl_platform()),
            0
        };

        cl::Context ocl_context = cl::Context(CL_DEVICE_TYPE_ALL, ocl_context_props, nullptr, nullptr, &ocl_err);
        if (ocl_err != CL_SUCCESS) {
            std::cerr << "cl::Context(), error: " << ocl_err << std::endl;
            return 0;
        }

        ocl_contexts.push_back(ocl_context);
    }

    std::cout << "num of OCL contexts: " << ocl_contexts.size() << std::endl;

    for (size_t i = 0; i < ocl_contexts.size(); i++) {
        std::vector<cl::Device> ocl_devices = ocl_contexts[i].getInfo<CL_CONTEXT_DEVICES>(&ocl_err);
        if (ocl_err != CL_SUCCESS) {
            std::cerr << "cl::Context::getInfo<CL_CONTEXT_DEVICES>(), error: " << ocl_err << std::endl;
            return 0;
        }

        std::cout << "OCL context " << i << " num of devices: " << ocl_devices.size() << std::endl;
    }

    return 0;
}
