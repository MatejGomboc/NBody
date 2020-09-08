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
    std::vector<float> vertices_data = NBodySim2D::generateRandomLocations(100000);

    QString error_message_1;
    if (!m_ui->central_widget->initVertices(vertices_data, error_message_1)) {
        QMessageBox error_dialog(this);
        error_dialog.setWindowTitle("OpenGL error");
        error_dialog.setIcon(QMessageBox::Icon::Critical);
        error_dialog.setText(error_message_1);
        error_dialog.setModal(true);
        error_dialog.setTextInteractionFlags(Qt::TextSelectableByMouse);
        error_dialog.exec();
        QApplication::quit();
    }

    std::string error_message_2;
    if (!m_nbodysim.init(m_ui->central_widget->getVertexBufferId(), error_message_2)) {
        QMessageBox error_dialog(this);
        error_dialog.setWindowTitle("OpenCL error");
        error_dialog.setIcon(QMessageBox::Icon::Critical);
        error_dialog.setText(error_message_1);
        error_dialog.setModal(true);
        error_dialog.setTextInteractionFlags(Qt::TextSelectableByMouse);
        error_dialog.exec();
        QApplication::quit();
    }
}

void MainWindow::openglSceneWidget_openGlDestroyed()
{
}
