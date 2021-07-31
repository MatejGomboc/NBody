#include <iostream>
#include <fstream>
#include <chrono>
#include <algorithm>
#include <CL/opencl.hpp>

struct OclDeviceData {
    cl::Platform platform;
    cl::Device device;
    cl::Context context;
    cl::CommandQueue command_queue;
    cl::Kernel kernel;
    cl::Buffer buffer;
    cl::Event calculate_finish_event;
    cl::Event copy_from_gpu_finish_event;
};

static void CL_CALLBACK on_ocl_context_error(const char* errinfo, const void* private_info, rsize_t cb, void* user_data)
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

    if (ocl_platforms.empty()) {
        std::cerr << "No OpenCL platforms found." << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<OclDeviceData> ocl_devices_data;
    for (const cl::Platform& ocl_platform : ocl_platforms) {
        cl_int ocl_err;
        std::string ocl_version = ocl_platform.getInfo<CL_PLATFORM_VERSION>(&ocl_err);
        if (ocl_err != CL_SUCCESS) {
            std::cerr << __FILE__ << ":" << __LINE__ << ", ocl_error: " << ocl_err << std::endl;
            return EXIT_FAILURE;
        }

        if ((ocl_version.find("OpenCL 1.0") != std::string::npos) ||
            (ocl_version.find("OpenCL 1.1") != std::string::npos)) {
            continue;
        }

        std::vector<cl::Device> ocl_devices;
        ocl_err = ocl_platform.getDevices(CL_DEVICE_TYPE_ALL, &ocl_devices);
        if (ocl_err != CL_SUCCESS) {
            std::cerr << __FILE__ << ":" << __LINE__ << ", ocl_error: " << ocl_err << std::endl;
            return EXIT_FAILURE;
        }

        if (ocl_devices.empty()) {
            std::cerr << "No OpenCL devices found." << std::endl;
            return EXIT_FAILURE;
        }

        for (const cl::Device& ocl_device : ocl_devices) {
            cl_int ocl_err;
            std::string ocl_version = ocl_device.getInfo<CL_DEVICE_VERSION>(&ocl_err);
            if (ocl_err != CL_SUCCESS) {
                std::cerr << __FILE__ << ":" << __LINE__ << ", ocl_error: " << ocl_err << std::endl;
                return EXIT_FAILURE;
            }

            if ((ocl_version.find("OpenCL 1.0") != std::string::npos) ||
                (ocl_version.find("OpenCL 1.1") != std::string::npos)) {
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

            cl::CommandQueue ocl_command_queue(ocl_context, ocl_device, cl::QueueProperties::None, &ocl_err);
            if (ocl_err != CL_SUCCESS) {
                std::cerr << __FILE__ << ":" << __LINE__ << ", ocl_error: " << ocl_err << std::endl;
                return EXIT_FAILURE;
            }

            OclDeviceData ocl_device_data;
            ocl_device_data.platform = ocl_platform;
            ocl_device_data.device = ocl_device;
            ocl_device_data.context = ocl_context;
            ocl_device_data.command_queue = ocl_command_queue;
            ocl_devices_data.push_back(ocl_device_data);
        }
    }

    if (ocl_devices_data.empty()) {
        std::cerr << "No compatible OpenCL devices found." << std::endl;
        return EXIT_FAILURE;
    }

    /**********************************************************/

    std::fstream ocl_source_file("test_kernel.cl", std::ios::in);
    if (!ocl_source_file.is_open()) {
        std::cerr << __FILE__ << ":" << __LINE__ << ", failed to open test_kernel.cl" << std::endl;
        return EXIT_FAILURE;
    }

    ocl_source_file.seekg(0, std::ios::end);
    std::streampos ocl_source_file_size = ocl_source_file.tellg();
    ocl_source_file.seekg(0, std::ios::beg);

    std::string ocl_source(ocl_source_file_size, '\0');
    ocl_source_file.read(ocl_source.data(), ocl_source.size());
    ocl_source_file.close();

    std::vector<std::string> ocl_sources;
    ocl_sources.push_back(ocl_source);

    for (size_t i = 0; i < ocl_devices_data.size(); i++) {
        cl::Program ocl_program(ocl_devices_data[i].context, ocl_sources, &ocl_err);
        if (ocl_err != CL_SUCCESS) {
            std::cerr << __FILE__ << ":" << __LINE__ << ", ocl_error: " << ocl_err << std::endl;
            return EXIT_FAILURE;
        }

        bool build_failed = false;
        ocl_err = ocl_program.build(ocl_devices_data[i].device, "-cl-std=CL1.2", nullptr, nullptr);
        if (ocl_err != CL_SUCCESS) {
            std::cerr << __FILE__ << ":" << __LINE__ << ", ocl_error: " << ocl_err << std::endl;
            build_failed = true;
        }

        std::string ocl_program_build_log = ocl_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(ocl_devices_data[i].device, &ocl_err);
        if (ocl_err != CL_SUCCESS) {
            std::cerr << __FILE__ << ":" << __LINE__ << ", ocl_error: " << ocl_err << std::endl;
            return EXIT_FAILURE;
        }

        std::string ocl_device_name = ocl_devices_data[i].device.getInfo<CL_DEVICE_NAME>(&ocl_err);
        if (ocl_err != CL_SUCCESS) {
            std::cerr << __FILE__ << ":" << __LINE__ << ", ocl_error: " << ocl_err << std::endl;
            return EXIT_FAILURE;
        }

        std::fstream ocl_program_build_log_file(ocl_device_name + " - build log.txt", std::ios::out);
        if (!ocl_program_build_log_file.is_open()) {
            std::cerr << __FILE__ << ":" << __LINE__ << ", failed to create OpenCL program build log file" << std::endl;
            return EXIT_FAILURE;
        }

        ocl_program_build_log_file.write(ocl_program_build_log.data(), ocl_program_build_log.size());
        ocl_program_build_log_file.close();

        if (build_failed) {
            return EXIT_FAILURE;
        }

        cl::Kernel ocl_kernel(ocl_program, "test_kernel", &ocl_err);
        ocl_devices_data[i].kernel = ocl_kernel;
    }

    /**********************************************************/

    std::vector<float> test_data(10000000, 0.0f);
    size_t part_size = test_data.size() / ocl_devices_data.size();
    size_t last_part_size = test_data.size() - (ocl_devices_data.size() - 1) * part_size;

    for (size_t i = 0; i < ocl_devices_data.size(); i++) {
        size_t current_part_size;
        if (i == ocl_devices_data.size() - 1) {
            current_part_size = last_part_size;
        }
        else {
            current_part_size = part_size;
        }

        ocl_devices_data[i].buffer = cl::Buffer(ocl_devices_data[i].context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, current_part_size * sizeof(float),
            test_data.data() + part_size * i, &ocl_err);
        if (ocl_err != CL_SUCCESS) {
            std::cerr << __FILE__ << ":" << __LINE__ << ", ocl_error: " << ocl_err << std::endl;
            return EXIT_FAILURE;
        }

        ocl_err = ocl_devices_data[i].kernel.setArg<cl::Buffer>(0, ocl_devices_data[i].buffer);
        if (ocl_err != CL_SUCCESS) {
            std::cerr << __FILE__ << ":" << __LINE__ << ", ocl_error: " << ocl_err << std::endl;
            return EXIT_FAILURE;
        }
    }

    /***********************************************************/

    std::chrono::steady_clock::time_point time_start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < ocl_devices_data.size(); i++) {
        cl::NDRange size_range;
        if (i == ocl_devices_data.size() - 1) {
            size_range = cl::NDRange(last_part_size);
        } else {
            size_range = cl::NDRange(part_size);
        }

        ocl_err = ocl_devices_data[i].command_queue.enqueueNDRangeKernel(ocl_devices_data[i].kernel, cl::NullRange, size_range, cl::NullRange, nullptr,
            &ocl_devices_data[i].calculate_finish_event);
        if (ocl_err != CL_SUCCESS) {
            std::cerr << __FILE__ << ":" << __LINE__ << ", ocl_error: " << ocl_err << std::endl;
            return EXIT_FAILURE;
        }

        size_t current_part_size;
        if (i == ocl_devices_data.size() - 1) {
            current_part_size = last_part_size;
        } else {
            current_part_size = part_size;
        }

        std::vector<cl::Event> calculate_finish_events({ ocl_devices_data[i].calculate_finish_event });
        ocl_err = ocl_devices_data[i].command_queue.enqueueReadBuffer(ocl_devices_data[i].buffer, false, 0, current_part_size * sizeof(float),
            test_data.data() + part_size * i, &calculate_finish_events, &ocl_devices_data[i].copy_from_gpu_finish_event);
        if (ocl_err != CL_SUCCESS) {
            std::cerr << __FILE__ << ":" << __LINE__ << ", ocl_error: " << ocl_err << std::endl;
            return EXIT_FAILURE;
        }
    }

    for (size_t i = 0; i < ocl_devices_data.size(); i++) {
        ocl_err = cl::WaitForEvents({ ocl_devices_data[i].calculate_finish_event });
        if (ocl_err != CL_SUCCESS) {
            std::cerr << __FILE__ << ":" << __LINE__ << ", ocl_error: " << ocl_err << std::endl;
            return EXIT_FAILURE;
        }
    }

    std::chrono::steady_clock::time_point time_end = std::chrono::high_resolution_clock::now();

    std::cout << "Duration: " << std::chrono::duration<double, std::milli>(time_end - time_start).count() << std::endl;

    /***********************************************************/

    std::fstream results_file("results.bin", std::ios::out);
    if (!results_file.is_open()) {
        std::cerr << __FILE__ << ":" << __LINE__ << ", failed to create results file" << std::endl;
        return EXIT_FAILURE;
    }

    results_file.write(reinterpret_cast<char*>(test_data.data()), test_data.size() * sizeof(float));
    results_file.close();

    if (!std::all_of(test_data.cbegin(), test_data.cend(), [=] (float element) -> bool { return element == 1.0f; })) {
        std::cerr << __FILE__ << ":" << __LINE__ << ", wrong result" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
