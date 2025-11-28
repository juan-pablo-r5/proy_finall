#ifndef MAINWINDOW_H
#define MAINWINDOW_H

extern bool nivel1Completado;
extern bool nivel2Completado;

#include <QMainWindow>
#include "niveles.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnNivel1_clicked();
    void on_btnNivel2_clicked();
    void on_btnNivel3_clicked();

private:
    Ui::MainWindow *ui;
    niveles *game;
};




#endif // MAINWINDOW_H
