attribute highp vec2 position;
uniform highp float x_scale;
uniform highp float y_scale;
uniform highp float point_size;

void main(void)
{
    gl_Position = vec4(position.x * x_scale, position.y * y_scale, 0.0f, 1.0f);
    gl_PointSize = point_size;
}
