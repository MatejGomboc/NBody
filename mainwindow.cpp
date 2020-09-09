#include <QFile>
#include <QMessageBox>
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow)
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
}

MainWindow::~MainWindow()
{
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

    std::vector<float> vertices_data = NBodySim2D::generateRandomLocations(1000);

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
    if (!m_nbodysim.init(opencl_sources, m_ui->central_widget->getVertexBufferId(), vertices_data.size(), 1.0f, 0.1f, 0.001f, error_message_2)) {
        error_dialog.setWindowTitle("OpenCL error");
        error_dialog.setText(error_message_2.c_str());
        error_dialog.exec();
        QApplication::quit();
        return;
    }
}

void MainWindow::openglSceneWidget_openGlDestroyed()
{
}
