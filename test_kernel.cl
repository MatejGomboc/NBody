kernel void test_kernel(global float* data)
{
	data[get_global_id(0)] += 1.0f;
}
