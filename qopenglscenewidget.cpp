#include <vector>
#include <algorithm>
#include <functional>
#include "qopenglscenewidget.h"


QOpenGLSceneWidget::QOpenGLSceneWidget(QWidget* parent) :
    QOpenGLWidget(parent)
{
}

QOpenGLSceneWidget::~QOpenGLSceneWidget()
{
    makeCurrent();
    destroyGL();
    doneCurrent();
}

float QOpenGLSceneWidget::xScale(int w, int h)
{
    if (w > h) {
        return static_cast<float>(h) / static_cast<float>(w);
    } else {
        return 1.0f;
    }
}

float QOpenGLSceneWidget::yScale(int w, int h)
{
    if (h > w) {
        return static_cast<float>(w) / static_cast<float>(h);
    } else {
        return 1.0f;
    }
}

void QOpenGLSceneWidget::initializeGL()
{
    if (m_opengl_initialized) {
        return;
    }

    initializeOpenGLFunctions();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);

    m_shader_program = new QOpenGLShaderProgram;
    m_shader_program->addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, ":/qopenglscenevertex.vert");
    m_shader_program->addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, ":/qopenglscenevertex.frag");
    if (!m_shader_program->link()) {
        destroyGL();
        emit glErrorOccurred(m_shader_program->log());
        return;
    }

    m_vertex_buffer.create();
    m_vertex_buffer.bind();

    std::random_device rand_device;
    std::seed_seq rand_seed{rand_device(), rand_device(), rand_device(), rand_device()};
    std::mt19937 rand_gen(rand_seed);
    std::uniform_real_distribution<float> rand_dist(-1.0f, 1.0f);
    std::vector<float> vertices_data(NUM_POINTS * 2);
    std::generate(vertices_data.begin(), vertices_data.end(), std::bind(rand_dist, rand_gen));

    m_vertex_buffer.allocate(vertices_data.data(), NUM_POINTS * 2 * sizeof(float));
    m_vertex_buffer.release();

    m_opengl_initialized = true;
}

void QOpenGLSceneWidget::destroyGL()
{
    delete m_shader_program;
    m_vertex_buffer.destroy();
    m_opengl_initialized = false;
}

void QOpenGLSceneWidget::resizeGL(int w, int h)
{
    if (!m_opengl_initialized) {
        return;
    }

    m_shader_program->bind();
    m_shader_program->setUniformValue("x_scale", xScale(w, h));
    m_shader_program->setUniformValue("y_scale", yScale(w, h));
    m_shader_program->release();
}

void QOpenGLSceneWidget::paintGL()
{
    if (!m_opengl_initialized) {
        return;
    }

    glClear(GL_COLOR_BUFFER_BIT);

    m_shader_program->bind();

    m_shader_program->enableAttributeArray("position");

    m_vertex_buffer.bind();
    m_shader_program->setAttributeBuffer("position", GL_FLOAT, 0, 2, 0);
    m_vertex_buffer.release();

    m_shader_program->setUniformValue("point_color", POINT_COLOR);
    m_shader_program->setUniformValue("point_size", POINT_SIZE);

    glDrawArrays(GL_POINTS, 0, NUM_POINTS);

    m_shader_program->disableAttributeArray("position");

    m_shader_program->release();
}
