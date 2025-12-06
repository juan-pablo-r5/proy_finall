#ifndef NIVELES_H
#define NIVELES_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTimer>
#include <QKeyEvent>
#include <QVector>
#include <QLabel>
#include <QMediaPlayer>
#include <QAudioOutput>


#include "personaje.h"



class niveles : public QGraphicsView
{
    Q_OBJECT

public:
    explicit niveles(int numNivel = 1, QWidget *parent = nullptr);
    \
        void mostrarMensajeBloqueo();
protected:
    // Para capturar el teclado en este nivel
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;


signals:
    void gameOver(QString motivo);

private:
    int enemigosEliminados = 0;
    int enemigosMetaNivel3 = 15;
    float velocidadBombas = 6.0f;

    int tiempoNivel2 = 0;
    QMediaPlayer *musicaFondo;
    QAudioOutput *audioSalida;
    QLabel *mensajeBloqueoAtaque = nullptr;
    bool jugadorRecibiendoDaño = false;
    int monedas = 0;
    QLabel *textoMonedas;
    QLabel * textoVidas;
    QGraphicsScene *scene;
    personaje *player;
    QTimer *timerUpdate;
    int nivelActual;

};

#endif // NIVELES_H
