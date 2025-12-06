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
//#include "enemigos.h"
//#include "proyectil.h"



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

private slots:
    void actualizarEscena();   // si luego quieres mover enemigos, etc.

signals:
    void gameOver(QString motivo);

private:
    int enemigosEliminados = 0;
    int enemigosMetaNivel3 = 15;
    float velocidadBombas = 6.0f;
    QList<Proyectil*> proyectiles;
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
    QVector<enemigos*> centinelas;
    QVector<QGraphicsPixmapItem*> monedasEscena;


    void configurarEscenaBase();
    void crearPlataformas();
    void generarCentinelas();
    void actualizarProyectilesNivel2();
};

#endif // NIVELES_H
