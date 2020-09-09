#include <vector>
#include "openglscenewidget.h"


OpenGLSceneWidget::OpenGLSceneWidget(QWidget* parent) :
    QOpenGLWidget(parent)
{
}

OpenGLSceneWidget::~OpenGLSceneWidget()
{
    makeCurrent();
    destroyGL();
    doneCurrent();
}

bool OpenGLSceneWidget::initVertices(const std::vector<float>& vertices_data, QString& error_message)
{
    if (!m_opengl_initialized) {
        error_message = "OpenGL not initialized.";
        return false;
    }

    if (!m_vertex_buffer.bind()) {
        error_message = "Cannot bind OpenGL vertex buffer.";
        return false;
    }

    m_vertex_buffer.allocate(vertices_data.data(), static_cast<int>(vertices_data.size() * sizeof(float)));
    m_vertex_buffer.release();
    return true;
}

GLuint OpenGLSceneWidget::getVertexBufferId() const
{
    return m_vertex_buffer.bufferId();
}

void OpenGLSceneWidget::setZoom(float zoom)
{
    m_zoom = zoom;
}

float OpenGLSceneWidget::getZoom() const
{
    return m_zoom;
}

float OpenGLSceneWidget::xScale(int w, int h)
{
    if (w > h) {
        return static_cast<float>(h) / static_cast<float>(w) / m_zoom;
    } else {
        return 1.0f / m_zoom;
    }
}

float OpenGLSceneWidget::yScale(int w, int h)
{
    if (h > w) {
        return static_cast<float>(w) / static_cast<float>(h) / m_zoom;
    } else {
        return 1.0f / m_zoom;
    }
}

void OpenGLSceneWidget::initializeGL()
{
    if (m_opengl_initialized) {
        return;
    }

    initializeOpenGLFunctions();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);

    m_shader_program = new QOpenGLShaderProgram;

    if (!m_shader_program->addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, ":/openglscenevertex.vert")) {
        emit errorOccurred("OpenGL shader compile error: \"" + m_shader_program->log() + "\".");
        destroyGL();
        return;
    }

    if (!m_shader_program->addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, ":/openglscenevertex.frag")) {
        emit errorOccurred("OpenGL shader compile error: \"" + m_shader_program->log() + "\".");
        destroyGL();
        return;
    }

    if (!m_shader_program->link()) {
        emit errorOccurred("OpenGL shader compile error: \"" + m_shader_program->log() + "\".");
        destroyGL();
        return;
    }

    if (!m_shader_program->bind()) {
        emit errorOccurred("Cannot bind OpenGL shader.");
        destroyGL();
        return;
    }

    m_shader_program->setUniformValue("point_color", POINT_COLOR);
    m_shader_program->setUniformValue("point_size", POINT_SIZE);
    m_shader_program->release();

    if (!m_vertex_buffer.create()) {
        emit errorOccurred("Cannot create OpenGL vertex buffer.");
        destroyGL();
        return;
    }

    m_opengl_initialized = true;
    emit openGlInitialized();
}

void OpenGLSceneWidget::destroyGL()
{
    if (m_shader_program != nullptr) {
        delete m_shader_program;
        m_shader_program = nullptr;
    }

    m_vertex_buffer.destroy();
    m_opengl_initialized = false;
    emit openGlDestroyed();
}

void OpenGLSceneWidget::resizeGL(int w, int h)
{
    if (!m_opengl_initialized) {
        return;
    }

    if (!m_shader_program->bind()) {
        emit errorOccurred("Cannot bind OpenGL shader.");
        destroyGL();
        return;
    }

    m_shader_program->setUniformValue("x_scale", xScale(w, h));
    m_shader_program->setUniformValue("y_scale", yScale(w, h));
    m_shader_program->release();
}

void OpenGLSceneWidget::paintGL()
{
    if (!m_opengl_initialized) {
        return;
    }

    glClear(GL_COLOR_BUFFER_BIT);

    if (!m_shader_program->bind()) {
        emit errorOccurred("Cannot bind OpenGL shader.");
        destroyGL();
        return;
    }

    m_shader_program->enableAttributeArray("position");

    if (!m_vertex_buffer.bind()) {
        emit errorOccurred("Cannot bind OpenGL vertex buffer.");
        destroyGL();
        return;
    }

    m_shader_program->setAttributeBuffer("position", GL_FLOAT, 0, 2, 0);

    glDrawArrays(GL_POINTS, 0, m_vertex_buffer.size() / sizeof(float) / 2);

    m_vertex_buffer.release();
    m_shader_program->disableAttributeArray("position");
    m_shader_program->release();
}
