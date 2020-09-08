#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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
    Ui::MainWindow* m_ui;
    NBodySim2D m_nbodysim;

private slots:
    void openglSceneWidget_errorOccurred(const QString& error_message);
    void openglSceneWidget_openGlInitialized();
    void openglSceneWidget_openGlDestroyed();
};

#endif // MAINWINDOW_H
