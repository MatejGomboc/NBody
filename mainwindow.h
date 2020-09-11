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
    static constexpr float ATTRACTION = 1.5e-16f; // Newton's gravity constant * Sun's mass [light years^3 / sun mass / year^2]
    static constexpr float RADIUS = 7.0e-8f; // Sun's radius [light years]
    static constexpr float TIME_STEP = 100000.0f; // years
    static constexpr float MAX_VELOCITY = 0.3f; // light speed [light years / years]
    static constexpr float MAX_DISTANCE = 10000.0f; // [light years]
    static constexpr float MAX_START_VELOCITY = 0.0001f; // 100m/s [light years / years]
    static constexpr float MAX_START_DISTANCE = 5000.0f; // [light years]
    static constexpr int RENDER_UPDATE_TIME_MS = 100;

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
