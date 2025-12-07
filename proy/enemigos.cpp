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
    cono << QPointF(0, 0);                     // en la nariz del enemigo
    cono << QPointF(radioVision, -80);         // arriba
    cono << QPointF(radioVision,  80);         // abajo

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

void enemigos::actualizarFrame()
{
    if (!animacionActual || animacionActual->isEmpty())
        return;

    frameActual = (frameActual + 1) % animacionActual->size();

    QPixmap frame = animacionActual->at(frameActual);

    // Voltear según dirección
    if (mirandoDerecha)
        frame = frame.transformed(QTransform().scale(-1, 1));

    aplicarSprite(frame);
}



void enemigos::mover()
{
    // Si estamos en modo campo (nivel 3), ignoramos patrulla
    if (modoCampo && objetivo) {
        if (modoCampo && objetivo) {

            QPointF dir = objetivo->posHitbox().center()
            - sceneBoundingRect().center();

            qreal dist = std::hypot(dir.x(), dir.y());
            if (dist < 1)
                return;

            dir /= dist;

            float intensidad = 0.6f;
            if (dist < 150) intensidad = 1.2f;
            if (dist < 60)  intensidad = 2.5f;

            QPointF fuerza = dir * intensidad;

            QPointF aceleracion = fuerza / masa;
            velocidadCampo += aceleracion;

            velocidadCampo *= 0.83f;

            setPos(pos() + velocidadCampo);
            return;
        }

    }

    if (patrullaMin != patrullaMax) {

        setPos(x() + velX, y());
        if (x() < patrullaMin) {
            velX = velPatrulla;

            if (!mirandoDerecha) {
                mirandoDerecha = true;
                setPixmap(pixmap().transformed(QTransform().scale(-1, 1)));
                areaVision->setRotation(0);
            }
        }
        else if (x() > patrullaMax) {
            velX = -velPatrulla;

            if (mirandoDerecha) {
                mirandoDerecha = false;
                setPixmap(pixmap().transformed(QTransform().scale(-1, 1)));
                areaVision->setRotation(180);
            }
        }
    }
}

void enemigos::setDireccion(bool derecha)
{
    mirandoDerecha = derecha;

    // Voltear sprite
    QPixmap frame = animacionActual->at(frameActual);

    if (derecha)
        frame = frame.transformed(QTransform().scale(-1, 1));  // mirar → si tu sprite base mira ←
    else
        frame = frame.transformed(QTransform().scale(1, 1));   // mirar ←

    aplicarSprite(frame);

    // Rotar cono de visión si lo usas
    if (areaVision) {
        areaVision->setRotation(derecha ? 0 : 180);
    }
}



void enemigos::habilitarCampo(personaje *p)
{
    objetivo = p;
    modoCampo = true;
    velocidadCampo = QPointF(0, 0);
}

void enemigos::actualizarVision(const QRectF &objetivoRect)
{
    if (!areaVision)
        return;

    QPointF enemigoPos = mapToScene(0,0);
    QPointF jugadorPos = objetivoRect.center();

    QPointF dirJugador = jugadorPos - enemigoPos;
    qreal dist = std::hypot(dirJugador.x(), dirJugador.y());

    if (dist > 0.1)
        dirJugador /= dist;

    QPointF dirVision = mirandoDerecha ? QPointF(1,0) : QPointF(-1,0);

    if (dist > radioVision) {
        // Apagar detección
        if (objetivoEnVision) {
            objetivoEnVision = false;
            animacionActual = &framesIdle;
            frameActual = 0;
        }
        return;
    }

    float dot = dirVision.x()*dirJugador.x() +
                dirVision.y()*dirJugador.y();

    bool dentroCono = (dot > 0.4f); 

    bool detectado = dentroCono;

    if (detectado != objetivoEnVision) {

        objetivoEnVision = detectado;

        // Cambiar animación
        animacionActual = detectado ? &framesAlerta : &framesIdle;
        frameActual = 0;
    }
}



bool enemigos::jugadorDetectado() const {
    return objetivoEnVision;
}

void enemigos::configurarPatrulla(double xMin, double xMax, double velocidad)
{
    patrullaMin = xMin;
    patrullaMax = xMax;
    velPatrulla = velocidad;

    velX = velocidad; // empieza moviéndose a la derecha
}

qreal enemigos::rangoVision() const {
    return radioVision;
}


void enemigos::aplicarSprite(const QPixmap &sprite) {
    setPixmap(sprite);
    setOffset(-sprite.width() / 2.0, -sprite.height() / 2.0);
}
