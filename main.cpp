#include <iostream>
#include <fstream>
#include <CL/opencl.hpp>

void CL_CALLBACK on_ocl_context_error(const char* errinfo, const void* private_info, rsize_t cb, void* user_data)
{
    std::cerr << __FILE__ << ":" << __LINE__ << ", errinfo: " << errinfo << std::endl;
}

int main()
{
    std::vector<cl::Platform> ocl_platforms;
    cl_int ocl_err = cl::Platform::get(&ocl_platforms);
    if (ocl_err != CL_SUCCESS) {
        std::cerr << __FILE__ << ":" << __LINE__ << ", ocl_error: " << ocl_err << std::endl;
        return EXIT_FAILURE;
    }

    for (size_t i = 0; i < ocl_platforms.size(); i++) {
        std::string ocl_version = ocl_platforms[i].getInfo<CL_PLATFORM_VERSION>(&ocl_err);
        if (ocl_err != CL_SUCCESS) {
            std::cerr << __FILE__ << ":" << __LINE__ << ", ocl_error: " << ocl_err << std::endl;
            return EXIT_FAILURE;
        }

        if ((ocl_version.find("OpenCL 1.0") != std::string::npos) ||
            (ocl_version.find("OpenCL 1.1") != std::string::npos)){
            ocl_platforms.erase(ocl_platforms.begin() + i);
            i--;
        }
    }

    if (ocl_platforms.empty()) {
        std::cerr << "No compatible OpenCL platforms found." << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<cl::Context> ocl_contexts;
    std::vector<cl::CommandQueue> ocl_command_queues;
    for (const cl::Platform& ocl_platform : ocl_platforms) {
        std::vector<cl::Device> ocl_devices;
        ocl_err = ocl_platform.getDevices(CL_DEVICE_TYPE_ALL, &ocl_devices);
        if (ocl_err != CL_SUCCESS) {
            std::cerr << __FILE__ << ":" << __LINE__ << ", ocl_error: " << ocl_err << std::endl;
            return EXIT_FAILURE;
        }

        for (size_t i = 0; i < ocl_devices.size(); i++) {
            std::string ocl_version = ocl_devices[i].getInfo<CL_DEVICE_VERSION>(&ocl_err);
            if (ocl_err != CL_SUCCESS) {
                std::cerr << __FILE__ << ":" << __LINE__ << ", ocl_error: " << ocl_err << std::endl;
                return EXIT_FAILURE;
            }

            if ((ocl_version.find("OpenCL 1.0") != std::string::npos) ||
                (ocl_version.find("OpenCL 1.1") != std::string::npos)) {
                ocl_devices.erase(ocl_devices.begin() + i);
                i--;
            }
        }

        if (ocl_devices.empty()) {
            continue;
        }

        cl_context_properties ocl_context_props[] = {
            CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>(ocl_platform()),
            0
        };

        cl::Context ocl_context(ocl_devices, ocl_context_props, on_ocl_context_error, nullptr, &ocl_err);
        if (ocl_err != CL_SUCCESS) {
            std::cerr << __FILE__ << ":" << __LINE__ << ", ocl_error: " << ocl_err << std::endl;
            return EXIT_FAILURE;
        }

        ocl_contexts.push_back(ocl_context);

        std::ifstream ocl_source_file("test_kernel.cl", std::ios::in);
        if (!ocl_source_file.is_open()) {
            std::cerr << __FILE__ << ":" << __LINE__ << ", failed to open test_kernel.cl" << std::endl;
            return EXIT_FAILURE;
        }

        std::string ocl_source((std::istreambuf_iterator<char>(ocl_source_file)), std::istreambuf_iterator<char>());
        ocl_source_file.close();
        std::vector<std::string> ocl_sources;
        ocl_sources.push_back(ocl_source);

        cl::Program ocl_program(ocl_context, ocl_sources, &ocl_err);
        if (ocl_err != CL_SUCCESS) {
            std::cerr << __FILE__ << ":" << __LINE__ << ", ocl_error: " << ocl_err << std::endl;
            return EXIT_FAILURE;
        }

        bool build_failed = false;
        ocl_err = ocl_program.build(ocl_devices, "-cl-std=CL1.2", nullptr, nullptr);
        if (ocl_err != CL_SUCCESS) {
            std::cerr << __FILE__ << ":" << __LINE__ << ", ocl_error: " << ocl_err << std::endl;
            build_failed = true;
        }

        std::vector<std::pair<cl::Device, std::string>> ocl_program_build_logs = ocl_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(&ocl_err);
        if (ocl_err != CL_SUCCESS) {
            std::cerr << __FILE__ << ":" << __LINE__ << ", ocl_error: " << ocl_err << std::endl;
            return EXIT_FAILURE;
        }

        for (const std::pair<cl::Device, std::string>& ocl_program_build_log : ocl_program_build_logs) {
            std::string ocl_device_name = ocl_program_build_log.first.getInfo<CL_DEVICE_NAME>(&ocl_err);
            if (ocl_err != CL_SUCCESS) {
                std::cerr << __FILE__ << ":" << __LINE__ << ", ocl_error: " << ocl_err << std::endl;
                return EXIT_FAILURE;
            }

            std::cout << ocl_device_name << ", build_log: {" << ocl_program_build_log.second << "}" << std::endl;
        }

        if (build_failed) {
            return EXIT_FAILURE;
        }

        for (const cl::Device& ocl_device : ocl_devices) {
            cl::CommandQueue ocl_command_queue(ocl_context, ocl_device, cl::QueueProperties::None, &ocl_err);
            if (ocl_err != CL_SUCCESS) {
                std::cerr << __FILE__ << ":" << __LINE__ << ", ocl_error: " << ocl_err << std::endl;
                return EXIT_FAILURE;
            }

            ocl_command_queues.push_back(ocl_command_queue);
        }
    }

    if (ocl_contexts.empty()) {
        std::cerr << "No compatible OpenCL devices found." << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
