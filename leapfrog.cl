kernel void positions(global float2* pos, global float2* vel, global float2* acc, const float dt) {
    unsigned long i = get_global_id(0);
    const float dt_2 = dt / 2.0f;

    vel[i] += dt_2 * acc[i];
    pos[i] += dt * vel[i];
}

kernel void velocities(global float2* vel, global float2* acc, const float dt) {
    unsigned long i = get_global_id(0);
    const float dt_2 = dt / 2.0f;

    vel[i] += dt_2 * acc[i];
}
