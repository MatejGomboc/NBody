kernel void positions(global float2* pos, global float2* vel, global float2* acc, const float dt, const float max_pos, const float max_vel) {
    unsigned long i = get_global_id(0);
    const float dt_2 = dt / 2.0f;

    vel[i] += dt_2 * acc[i];

    if (length(vel[i]) > max_vel) {
        vel[i] = max_vel * normalize(vel[i]);
    }

    pos[i] += dt * vel[i];

    if (pos[i].x > max_pos) {
        pos[i].x = max_pos;
        vel[i].x *= -1.0f;
    }

    if (pos[i].x < -max_pos) {
        pos[i].x = -max_pos;
        vel[i].x *= -1.0f;
    }

    if (pos[i].y > max_pos) {
        pos[i].y = max_pos;
        vel[i].y *= -1.0f;
    }

    if (pos[i].y < -max_pos) {
        pos[i].y = -max_pos;
        vel[i].y *= -1.0f;
    }
}

kernel void velocities(global float2* vel, global float2* acc, const float dt, const float max_vel) {
    unsigned long i = get_global_id(0);
    const float dt_2 = dt / 2.0f;

    vel[i] += dt_2 * acc[i];

    if (length(vel[i]) > max_vel) {
        vel[i] = max_vel * normalize(vel[i]);
    }
}
