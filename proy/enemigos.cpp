#include "enemigos.h"

#include <QBrush>
#include <QColor>
#include <QLineF>
#include <QPen>
#include <QPainter>
#include <QPixmap>
#include <QRadialGradient>
#include <QtGlobal>
#include <QVariant>

enemigos::enemigos(QObject *parent)
    : QObject(parent),
    velX(0.0f),
    velY(0.0f),
    radioVision(100.0),
    objetivoEnVision(false),
    areaVision(nullptr),
    modoCampo(false),
    masa(1.0f),
    velocidadCampo(0,0)
{
    setData(0, QVariant(QStringLiteral("enemigo_centinela")));
    spriteSheet.load(":/sprites/enemigo1.png");

    // Calcular tamaño de un frame
    int frameWidth  = spriteSheet.width()  / 10;  // si tu fila más larga tiene 10 frames
    int frameHeight = spriteSheet.height() / 8;   // si tu spritesheet tiene 8 filas

    // ← Aquí defines cuántos frames tiene cada animación real:
    framesIdle   = extraerFrames(1, frameWidth-40, frameHeight-10, 6);  // fila 0, 6 frames
    framesAlerta = extraerFrames(4, frameWidth-35, frameHeight, 7);  // fila 1, 4 frames

    // Establecer primera animación
    animacionActual = &framesIdle;
    frameActual = 0;
    aplicarSprite(animacionActual->at(0));

    QPolygonF cono;
    cono << QPointF(0, 0);
    cono << QPointF(radioVision, -80);
    cono << QPointF(radioVision,  80);

    areaVision = new QGraphicsPolygonItem(cono, this);
    areaVision->setBrush(QColor(255, 255, 255, 25));
    areaVision->setPen(QPen(QColor(255, 255, 255, 160), 2));
    areaVision->setZValue(-1);

    animTimer = new QTimer(this);
    connect(animTimer, &QTimer::timeout, this, &enemigos::actualizarFrame);
    animTimer->start(100);
}

QVector<QPixmap> enemigos::extraerFrames(int fila, int frameWidth, int frameHeight, int cantidad) {
    QVector<QPixmap> frames;
    for (int i = 0; i < cantidad; ++i) {
        frames.append(spriteSheet.copy(i * frameWidth, fila * frameHeight, frameWidth, frameHeight));
    }
    return frames;
}

void enemigos::setVelocidadX(float v) { velX = v; }

bool enemigos::jugadorDetectado() const {
    return objetivoEnVision;
}
