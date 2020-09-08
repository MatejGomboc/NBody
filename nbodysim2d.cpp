#include <random>
#include <algorithm>
#include <functional>
#include "nbodysim2d.h"

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
