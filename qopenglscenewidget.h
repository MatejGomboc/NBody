#ifndef QOPENGLSCENEWIDGET_H
#define QOPENGLSCENEWIDGET_H

#include <random>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

class QOpenGLSceneWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit QOpenGLSceneWidget(QWidget* parent = nullptr);
    ~QOpenGLSceneWidget();

signals:
    void glErrorOccurred(QString error_message);

private:
    static constexpr uint32_t NUM_POINTS = 10000;
    static constexpr float POINT_SIZE = 2.0f;
    static constexpr QVector4D POINT_COLOR{ 1.0f, 1.0f, 0.0f, 1.0f };

    bool m_opengl_initialized = false;
    QOpenGLShaderProgram* m_shader_program = nullptr;
    QOpenGLBuffer m_vertex_buffer = QOpenGLBuffer(QOpenGLBuffer::Type::VertexBuffer);

    static float xScale(int w, int h);
    static float yScale(int w, int h);

    void initializeGL() override;
    void destroyGL();
    void resizeGL(int w, int h) override;
    void paintGL() override;
};

#endif // QOPENGLSCENEWIDGET_H
