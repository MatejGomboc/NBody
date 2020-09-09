#include <QFile>
#include <QMessageBox>
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_rendering_timer(new QTimer(this))
{
    m_ui->setupUi(this);

    connect(m_ui->central_widget, &OpenGLSceneWidget::errorOccurred,
        this, &MainWindow::openglSceneWidget_errorOccurred,
        Qt::ConnectionType::QueuedConnection);

    connect(m_ui->central_widget, &OpenGLSceneWidget::openGlInitialized,
        this, &MainWindow::openglSceneWidget_openGlInitialized,
        Qt::ConnectionType::QueuedConnection);

    connect(m_ui->central_widget, &OpenGLSceneWidget::openGlDestroyed,
        this, &MainWindow::openglSceneWidget_openGlDestroyed,
        Qt::ConnectionType::QueuedConnection);

    connect(m_rendering_timer, &QTimer::timeout,
        this, &MainWindow::rendering_timer_timeout,
        Qt::ConnectionType::QueuedConnection);
}

MainWindow::~MainWindow()
{
    delete m_rendering_timer;
    delete m_ui;
}

void MainWindow::openglSceneWidget_errorOccurred(const QString& error_message)
{
    QMessageBox error_dialog(this);
    error_dialog.setWindowTitle("OpenGL error");
    error_dialog.setIcon(QMessageBox::Icon::Critical);
    error_dialog.setText(error_message);
    error_dialog.setModal(true);
    error_dialog.setTextInteractionFlags(Qt::TextSelectableByMouse);
    error_dialog.exec();
    QApplication::quit();
}

void MainWindow::openglSceneWidget_openGlInitialized()
{
    QMessageBox error_dialog(this);
    error_dialog.setIcon(QMessageBox::Icon::Critical);
    error_dialog.setModal(true);
    error_dialog.setTextInteractionFlags(Qt::TextSelectableByMouse);

    std::vector<float> vertices_data = NBodySim2D::generateRandomLocations(NUM_POINTS);

    QString error_message_1;
    if (!m_ui->central_widget->initVertices(vertices_data, error_message_1)) {
        error_dialog.setWindowTitle("OpenGL error");
        error_dialog.setText(error_message_1);
        error_dialog.exec();
        QApplication::quit();
        return;
    }

    QFile opencl_source_file_gravity(":/gravity.cl");
    if (!opencl_source_file_gravity.open(QIODevice::ReadOnly)) {
        error_dialog.setWindowTitle("OpenCL error");
        error_dialog.setText("Cannot open OpenCL source file.");
        error_dialog.exec();
        QApplication::quit();
        return;
    }

    std::vector<std::string> opencl_sources;
    opencl_sources.push_back(opencl_source_file_gravity.readAll().toStdString());
    opencl_source_file_gravity.close();

    QFile opencl_source_file_leapfrog(":/leapfrog.cl");
    if (!opencl_source_file_leapfrog.open(QIODevice::ReadOnly)) {
        error_dialog.setWindowTitle("OpenCL error");
        error_dialog.setText("Cannot open OpenCL source file.");
        error_dialog.exec();
        QApplication::quit();
        return;
    }

    opencl_sources.push_back(opencl_source_file_leapfrog.readAll().toStdString());
    opencl_source_file_leapfrog.close();

    std::string error_message_2;
    if (!m_nbodysim.init(opencl_sources, m_ui->central_widget->getVertexBufferId(),
        static_cast<uint32_t>(vertices_data.size()),
        ATTRACTION, RADIUS, TIME_STEP, 1.0f, 1.0f, error_message_2)) {
        error_dialog.setWindowTitle("OpenCL error");
        error_dialog.setText(error_message_2.c_str());
        error_dialog.exec();
        QApplication::quit();
        return;
    }

    m_rendering_timer->setSingleShot(false);
    m_rendering_timer->setInterval(RENDER_UPDATE_TIME_MS);
    m_rendering_timer->start();
}

void MainWindow::openglSceneWidget_openGlDestroyed()
{
    m_rendering_timer->stop();
}

void MainWindow::rendering_timer_timeout()
{
    std::string error_message;
    if (!m_nbodysim.updateLocations(NUM_POINTS, error_message)) {
        QMessageBox error_dialog(this);
        error_dialog.setIcon(QMessageBox::Icon::Critical);
        error_dialog.setModal(true);
        error_dialog.setTextInteractionFlags(Qt::TextSelectableByMouse);
        error_dialog.setWindowTitle("OpenCL error");
        error_dialog.setText(error_message.c_str());
        error_dialog.exec();
        QApplication::quit();
    }

    m_ui->central_widget->update();
}
