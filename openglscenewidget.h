#ifndef OPENGLSCENEWIDGET_H
#define OPENGLSCENEWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

class OpenGLSceneWidget : public QOpenGLWidget, protected QOpenGLFunctions {

    Q_OBJECT

public:
    explicit OpenGLSceneWidget(QWidget* parent = nullptr);
    ~OpenGLSceneWidget();
    bool initVertices(const std::vector<float>& vertices_data, QString& error_message);
    GLuint getVertexBufferId() const;
    void setZoom(float zoom);
    float getZoom() const;

signals:
    void errorOccurred(const QString& error_message);
    void openGlInitialized();
    void openGlDestroyed();

private:
    static constexpr float POINT_SIZE = 2.0f;
    static constexpr QVector4D POINT_COLOR{ 1.0f, 1.0f, 0.0f, 1.0f };

    bool m_opengl_initialized = false;
    QOpenGLShaderProgram* m_shader_program = nullptr;
    QOpenGLBuffer m_vertex_buffer = QOpenGLBuffer(QOpenGLBuffer::Type::VertexBuffer);
    float m_zoom = 1.0f;

    float xScale(int w, int h);
    float yScale(int w, int h);

    void initializeGL() override;
    void destroyGL();
    void resizeGL(int w, int h) override;
    void paintGL() override;
};

#endif // OPENGLSCENEWIDGET_H
