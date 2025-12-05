#ifndef ENEMIGOS_H
#define ENEMIGOS_H

#include <QObject>
#include <QGraphicsPixmapItem>
#include <QGraphicsEllipseItem>
#include <QPixmap>
#include <QPointF>
#include "personaje.h"


class enemigos : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT

public:
    explicit enemigos(QObject *parent = nullptr);
    void setVelocidadX(float v);
    void mover();
    void habilitarCampo(personaje *p);
    void actualizarVision(const QRectF &objetivo);

    bool jugadorDetectado() const;
    void configurarPatrulla(double xMin, double xMax, double velocidad);
    qreal rangoVision() const;
    QVector<QPixmap> extraerFrames(int fila, int ancho, int alto, int cantidad);

    void actualizarFrame();
    void setDireccion(bool derecha);
private:
    // ---- SPRITES ----
    QPixmap spriteSheet;
    QVector<QPixmap> framesIdle;
    QVector<QPixmap> framesAlerta;
    bool mirandoDerecha = true;


    QTimer *animTimer;
    int frameActual = 0;
    QVector<QPixmap> *animacionActual = nullptr;

    // ---- VISION ----
    QGraphicsPolygonItem *areaVision;
    qreal radioVision;
    bool objetivoEnVision;

    // ---- PATRULLA (nivel 1) ----
    double patrullaMin = 0;
    double patrullaMax = 0;
    double velPatrulla = 0;
    float velX;
    float velY;

    // ---- MODO CAMPO (nivel 3) ----
    personaje *objetivo = nullptr;
    bool modoCampo = false;
    float masa;
    QPointF velocidadCampo;

    // ---- FUNCIONES INTERNAS ----
    void aplicarSprite(const QPixmap &sprite);
};

#endif // ENEMIGOS_H
