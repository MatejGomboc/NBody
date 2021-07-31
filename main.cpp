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
    cl::Event copy_to_gpu_finish_event;
    cl::Event calculate_finish_event;
    cl::Event copy_from_gpu_finish_event;
    float load_fraction;
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

            cl::CommandQueue ocl_command_queue(ocl_context, ocl_device, cl::QueueProperties::Profiling, &ocl_err);
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

    for (size_t i = 0; i < ocl_devices_data.size(); i++) {
        cl_uint max_ocl_workgroups = ocl_devices_data[i].device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>(&ocl_err);
        if (ocl_err != CL_SUCCESS) {
            std::cerr << __FILE__ << ":" << __LINE__ << ", ocl_error: " << ocl_err << std::endl;
            return EXIT_FAILURE;
        }

        size_t max_workgroup_size = ocl_devices_data[i].device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>(&ocl_err);
        if (ocl_err != CL_SUCCESS) {
            std::cerr << __FILE__ << ":" << __LINE__ << ", ocl_error: " << ocl_err << std::endl;
            return EXIT_FAILURE;
        }

        cl_uint max_clk_frequency = ocl_devices_data[i].device.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>(&ocl_err);
        if (ocl_err != CL_SUCCESS) {
            std::cerr << __FILE__ << ":" << __LINE__ << ", ocl_error: " << ocl_err << std::endl;
            return EXIT_FAILURE;
        }

        bool is_integrated = ocl_devices_data[i].device.getInfo<CL_DEVICE_HOST_UNIFIED_MEMORY>(&ocl_err);
        if (ocl_err != CL_SUCCESS) {
            std::cerr << __FILE__ << ":" << __LINE__ << ", ocl_error: " << ocl_err << std::endl;
            return EXIT_FAILURE;
        }

        ocl_devices_data[i].load_fraction = max_clk_frequency * max_workgroup_size * max_ocl_workgroups;

        if (is_integrated) {
            ocl_devices_data[i].load_fraction *= 2.0f;
        }
    }

    float total_load = 0;
    for (size_t i = 0; i < ocl_devices_data.size(); i++) {
        total_load += ocl_devices_data[i].load_fraction;
    }

    for (size_t i = 0; i < ocl_devices_data.size(); i++) {
        ocl_devices_data[i].load_fraction /= total_load;
    }

    /**********************************************************/

    std::vector<float> test_data(1e6, 0.0f);

    for (size_t i = 0; i < ocl_devices_data.size(); i++) {
        size_t current_part_size = test_data.size() * ocl_devices_data[i].load_fraction;
        
        ocl_devices_data[i].buffer = cl::Buffer(ocl_devices_data[i].context, CL_MEM_READ_WRITE,
            current_part_size * sizeof(float), nullptr, &ocl_err);
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

    std::chrono::steady_clock::time_point time_total_start = std::chrono::high_resolution_clock::now();

    float* current_part_start = test_data.data();
    for (size_t i = 0; i < ocl_devices_data.size(); i++) {
        size_t current_part_size = test_data.size() * ocl_devices_data[i].load_fraction;

        ocl_err = ocl_devices_data[i].command_queue.enqueueWriteBuffer(ocl_devices_data[i].buffer, false, 0, current_part_size * sizeof(float), 
            current_part_start, nullptr, &ocl_devices_data[i].copy_to_gpu_finish_event);
        if (ocl_err != CL_SUCCESS) {
            std::cerr << __FILE__ << ":" << __LINE__ << ", ocl_error: " << ocl_err << std::endl;
            return EXIT_FAILURE;
        }

        std::vector<cl::Event> copy_to_gpu_finish_events({ ocl_devices_data[i].copy_to_gpu_finish_event });
        ocl_err = ocl_devices_data[i].command_queue.enqueueNDRangeKernel(ocl_devices_data[i].kernel, cl::NullRange,
            cl::NDRange(current_part_size), cl::NullRange, &copy_to_gpu_finish_events, &ocl_devices_data[i].calculate_finish_event);
        if (ocl_err != CL_SUCCESS) {
            std::cerr << __FILE__ << ":" << __LINE__ << ", ocl_error: " << ocl_err << std::endl;
            return EXIT_FAILURE;
        }

        std::vector<cl::Event> calculate_finish_events({ ocl_devices_data[i].calculate_finish_event });
        ocl_err = ocl_devices_data[i].command_queue.enqueueReadBuffer(ocl_devices_data[i].buffer, false, 0, current_part_size * sizeof(float),
            current_part_start, &calculate_finish_events, &ocl_devices_data[i].copy_from_gpu_finish_event);
        if (ocl_err != CL_SUCCESS) {
            std::cerr << __FILE__ << ":" << __LINE__ << ", ocl_error: " << ocl_err << std::endl;
            return EXIT_FAILURE;
        }

        current_part_start += current_part_size;
    }

    for (size_t i = 0; i < ocl_devices_data.size(); i++) {
        ocl_err = cl::WaitForEvents({ ocl_devices_data[i].copy_from_gpu_finish_event });
        if (ocl_err != CL_SUCCESS) {
            std::cerr << __FILE__ << ":" << __LINE__ << ", ocl_error: " << ocl_err << std::endl;
            return EXIT_FAILURE;
        }
    }

    std::chrono::steady_clock::time_point time_total_end = std::chrono::high_resolution_clock::now();

    std::cout << "total duration (ms): " << std::chrono::duration<double, std::milli>(time_total_end - time_total_start).count() << std::endl;

    for (size_t i = 0; i < ocl_devices_data.size(); i++) {
        cl_ulong time_part_start_ns = ocl_devices_data[i].calculate_finish_event.getProfilingInfo<CL_PROFILING_COMMAND_START>(&ocl_err);
        if (ocl_err != CL_SUCCESS) {
            std::cerr << __FILE__ << ":" << __LINE__ << ", ocl_error: " << ocl_err << std::endl;
            return EXIT_FAILURE;
        }

        cl_ulong time_part_end_ns = ocl_devices_data[i].calculate_finish_event.getProfilingInfo<CL_PROFILING_COMMAND_END>(&ocl_err);
        if (ocl_err != CL_SUCCESS) {
            std::cerr << __FILE__ << ":" << __LINE__ << ", ocl_error: " << ocl_err << std::endl;
            return EXIT_FAILURE;
        }

        std::cout << i << " duration (ms): " << (time_part_end_ns - time_part_start_ns) / 1.0e6f << std::endl;
    }

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
