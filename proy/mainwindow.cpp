#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>
#include <QString>

bool nivel1Completado = false;
bool nivel2Completado = false;
bool nivel3Completado = false;



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::on_btnNivel1_clicked()
{
    game = new niveles(1, nullptr);   // <-- IMPORTANTE: SIN parent

    connect(game, &niveles::gameOver, this, [this](QString motivo){

        this->show();
        game->close();
        game->deleteLater();
        game = nullptr;

        if (motivo == "ganar") {
            QMessageBox::information(this, "Nivel completado", "¡Felicidades!");
        }
        else if (motivo == "muerte") {
            QMessageBox::information(this, "Fin del juego", "Te quedaste sin vidas.");
        }
    });

    this->hide();   // oculta el menú
    game->show();   // muestra el nivel
}





void MainWindow::on_btnNivel2_clicked()
{

    if (!nivel1Completado) {
        QMessageBox::warning(this, "Bloqueado",
                             "Debe completar primero el Nivel 1.");
        return;
    }
    game = new niveles(2, nullptr);

    connect(game, &niveles::gameOver, this, [this](QString motivo){

        this->show();
        game->close();
        game->deleteLater();
        game = nullptr;

        if (motivo == "ganar") {
            QMessageBox::information(this, "Nivel completado", "¡Has superado el nivel 2!");
        }
        else if (motivo == "muerte") {
            QMessageBox::information(this, "Fin del juego", "Te quedaste sin vidas.");
        }
    });

    this->hide();
    game->show();
}

void MainWindow::on_btnNivel3_clicked() {

    if (!nivel2Completado) {
        QMessageBox::warning(this, "Bloqueado",
                             "Debe completar primero el Nivel 2.");
        return;
    }


    game = new niveles(3, nullptr);  // importante: nullptr para que no se cierre toda la app

    connect(game, &niveles::gameOver, this, [this](QString motivo){

        this->show();
        game->close();
        game->deleteLater();
        game = nullptr;

        if (motivo == "ganar") {
            QMessageBox::information(this, "Nivel completado", "¡Felicidades!");
        }
        else if (motivo == "muerte") {
            QMessageBox::information(this, "Fin del juego", "Te quedaste sin vidas.");
        }
    });

    this->hide();
    game->show();
}
