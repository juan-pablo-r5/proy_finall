#ifndef PERSONAJE_H
#define PERSONAJE_H

#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QObject>
#include <QTimer>
#include <QVector>
#include <QMediaPlayer>
#include <QAudioOutput>
#include "entidad.h"

class personaje : public QObject, public QGraphicsPixmapItem, public Entidad {
    Q_OBJECT

public:
    personaje();

    // GETTERS / SETTERS
    float getVelocidadX() const { return velocidadX; }
    float getVelocidadY() const { return velocidadY; }
    void setVelocidadX(float v) { velocidadX = v; }
    void setVelocidadY(float v) { velocidadY = v; }

    // VIDA
    int vidas = 3;
    int vidasMax = 3;

    void perderVida();
    void morir();

    // MOVIMIENTO / ACCIONES
    void moverIzquierda();
    void moverDerecha();
    void parar();
    void saltar();
    void deslizar();
    void atacar();

    // FÍSICA
    void actualizarFisica();
    QRectF posHitbox();

    // ATAQUE
    QGraphicsRectItem *hitboxAtaque;
    bool atacando = false;
    void actualizar() override;
    void moverBase();

private slots:
    void actualizarFrame();

private:

    // AUDIO
    QMediaPlayer *sonidoAtaque;
    QAudioOutput *audioAtaque;

    // ESTADO
    bool invulnerable = false;
    bool accionEspecialActiva = false;   // <--- ESTE ES EL VERDADERO ESTADO ESPECIAL
    bool enSuelo = true;
    bool mirandoDerecha = true;

    // VELOCIDADES
    float velocidad = 3.0f;  // <--- AHORA EXISTE Y ES PÚBLICA PARA EL .CPP
    float velocidadX = 0;
    float velocidadY = 0;
    float gravedad = 0.8f;

    // COYOTE TIME
    int coyoteCounter = 0;
    const int COYOTE_FRAMES = 6;

    // ANIMACIÓN
    QPixmap spriteSheet;
    QVector<QPixmap> framesIdle;
    QVector<QPixmap> framesRun;
    QVector<QPixmap> framesJump;
    QVector<QPixmap> framesSlide;
    QVector<QPixmap> framesAttack;

    QVector<QPixmap> *animacionActual;
    int frameActual = 0;

    // HITBOX
    QGraphicsRectItem *hitbox;

    // TIMER
    QTimer *animTimer;

    // INTERNAL METHODS
    QVector<QPixmap> extraerFrames(int y, int frameWidth, int frameHeight, int numFrames);
    void cambiarAnimacion(int estado, bool reiniciarFrame = false);
    void actualizarDireccion(bool derecha);
};

#endif // PERSONAJE_H
