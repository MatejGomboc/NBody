kernel void accelerations(global float2* pos, global float2* acc, const float attr, const float rad) {
    unsigned long n = get_global_size(0);
    unsigned long i = get_global_id(0);

    acc[i] = (float2)(0.0f, 0.0f);

    for (unsigned long j = 0; j < n; j++) {
        if (i != j) {
            float dist = distance(pos[j], pos[i]);
            if (dist > rad) {
                acc[i] += (attr / dist / dist / dist) * (pos[j] - pos[i]);
            }
        }
    }
}
