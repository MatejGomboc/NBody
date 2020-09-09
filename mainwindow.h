#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "nbodysim2d.h"

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    static constexpr uint32_t NUM_POINTS = 1000;
    static constexpr float ATTRACTION = 100.0f;
    static constexpr float RADIUS = 0.01f;
    static constexpr float TIME_STEP = 0.1f;
    static constexpr int RENDER_UPDATE_TIME_MS = 500;

    Ui::MainWindow* m_ui;
    NBodySim2D m_nbodysim;
    QTimer* m_rendering_timer;

private slots:
    void openglSceneWidget_errorOccurred(const QString& error_message);
    void openglSceneWidget_openGlInitialized();
    void openglSceneWidget_openGlDestroyed();
    void rendering_timer_timeout();
};

#endif // MAINWINDOW_H
