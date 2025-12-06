#ifndef PERSONAJE_H
#define PERSONAJE_H

#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QObject>
#include <QTimer>
#include <QVector>
#include <QMap>
#include <QMediaPlayer>
#include <QAudioOutput>
#include "entidad.h"


class personaje : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT

public:
    float getVelocidadX() const;
    Entidad stats;
    int vidas = 3;        // número total de vidas
    int vidasMax = 3;     // si quieres limitarlo
    personaje();
    void moverIzquierda();
    void moverDerecha();
    void parar();
    void saltar();
    void actualizarFisica();
    void deslizar();
    void atacar();
    QRectF posHitbox();
    void perderVida();
    QGraphicsRectItem *hitboxAtaque;
    bool atacando = false;

    void morir();
private slots:
    void actualizarFrame();

private:
    QMediaPlayer *sonidoAtaque;
    QAudioOutput *audioAtaque;
    bool invulnerable = false;
    enum class EstadoAnimacion { Idle, Run, Jump, Slide, Attack };
    QPixmap spriteSheet;
    QVector<QPixmap> framesIdle;
    QVector<QPixmap> framesRun;
    QVector<QPixmap> framesSlide;
    QVector<QPixmap> *animacionActual;
    QVector<QPixmap> framesJump;
    QVector<QPixmap> framesAttack;
    float velocidadY = 0;     // velocidad vertical
    float gravedad = 0.8;     // gravedad constante
    bool enSuelo = true;      // para evitar doble salto
    float velocidadX = 0;  // velocidad horizontal
    int frameActual;
    bool mirandoDerecha;
    bool accionEspecialActiva=false;
    float velocidad;
    int framesDesdeSuelo;
    int coyoteFrames;
    QGraphicsRectItem *hitbox;
    QTimer *animTimer;
    int coyoteCounter = 0;
    const int COYOTE_FRAMES = 6;
    QVector<QPixmap> extraerFrames(int y, int frameWidth, int frameHeight, int numFrames);
    void cambiarAnimacion(EstadoAnimacion estado, bool reiniciarFrame = false);
    void actualizarDireccion(bool aLaDerecha);
    int getVidas() const { return vidas; }
    void setVidas(int v) { vidas = qBound(0, v, vidasMax); }

};

#endif // PERSONAJE_H
