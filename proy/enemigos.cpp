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
    Entidad(TipoEntidad::Enemigo),
    velX(0.0f),
    velY(0.0f),
    radioVision(100.0),
    objetivoEnVision(false),
    areaVision(nullptr),
    modoCampo(false),
    masa(1.0f),
    velocidadCampo(0,0)
{
    vida = 2;
    setData(0, QVariant(QStringLiteral("enemigo_centinela")));
    spriteSheet.load(":/sprites/enemigo1.png");

    zonaPatrullaIzq = x() - 60;
    zonaPatrullaDer = x() + 60;

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


void enemigos::moverBase() {
    setX(x() + velX);
    setY(y() + velY);
}

void enemigos::actualizar() {
    mover();
}


void enemigos::mover()
{

    if (modoCampo && objetivo) {

        QPointF dir = objetivo->posHitbox().center()
        - sceneBoundingRect().center();

        qreal dist = std::hypot(dir.x(), dir.y());
        if (dist > 1.0) {

            dir /= dist;

            float intensidad = 0.6f;
            if (dist < 150) intensidad = 1.2f;
            if (dist < 60)  intensidad = 2.5f;

            QPointF fuerza = dir * intensidad;
            QPointF aceleracion = fuerza / masa;
            velocidadCampo += aceleracion;

            velocidadCampo *= 0.83f;
            setPos(pos() + velocidadCampo);
        }

        return;
    }


    if (enPersecucion) {

        // Si aún lo ve → usar posición fresca
        if (objetivoEnVision) {
            ultimaPosJugador = jugadorPosActual;
        }

        // Perseguir dirección del jugador
        if (ultimaPosJugador.x() > x()) {
            velX = 1.8f;
            setDireccion(true);
        } else {
            velX = -1.8f;
            setDireccion(false);
        }

        setX(x() + velX);

        if (!objetivoEnVision && fabs(x() - ultimaPosicionVisto) < 25) {
            enPersecucion = false;
            velX = velPatrulla;
        }

        return;
    }



    // ============================
    // 3) PATRULLA NORMAL (NIVEL 1)
    // ============================
    if (patrullaMin != patrullaMax) {

        velY = 0;
        moverBase();

        // Límite izquierdo
        if (x() < zonaPatrullaIzq) {
            velX = velPatrulla;

            if (!mirandoDerecha) {
                mirandoDerecha = true;
                setPixmap(pixmap().transformed(QTransform().scale(-1, 1)));
                areaVision->setRotation(0);
            }
        }

        // Límite derecho
        else if (x() > zonaPatrullaDer) {
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

    ultimaPosJugador = jugadorPos;
    jugadorPosActual = jugadorPos;

    QPointF dirJugador = jugadorPos - enemigoPos;
    qreal dist = std::hypot(dirJugador.x(), dirJugador.y());

    if (dist > 0.1)
        dirJugador /= dist;

    QPointF dirVision = mirandoDerecha ? QPointF(1,0) : QPointF(-1,0);

    // Fuera de rango
    if (dist > radioVision)
    {
        if (objetivoEnVision) {
            objetivoEnVision = false;
            animacionActual = &framesIdle;
            frameActual = 0;
            enPersecucion = true;
        }
        return;
    }

    float dot = dirVision.x()*dirJugador.x() + dirVision.y()*dirJugador.y();
    bool detectado = (dot > 0.4f);

    if (detectado != objetivoEnVision) {

        objetivoEnVision = detectado;

        animacionActual = detectado ? &framesAlerta : &framesIdle;
        frameActual = 0;

        if (detectado) {

            //Centro de patrullaje es donde encontró al jugador
            float centro = jugadorPos.x();
            float rango  = 90.0f;

            zonaPatrullaIzq = centro - rango;
            zonaPatrullaDer = centro + rango;

            // Actualizar límites reales
            patrullaMin = zonaPatrullaIzq;
            patrullaMax = zonaPatrullaDer;

            ultimaPosicionVisto = jugadorPos.x();
            enPersecucion = true;
        }
        else {
            enPersecucion = true;
        }
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

    zonaPatrullaIzq = patrullaMin;
    zonaPatrullaDer = patrullaMax;

}

qreal enemigos::rangoVision() const {
    return radioVision;
}


void enemigos::aplicarSprite(const QPixmap &sprite) {
    setPixmap(sprite);
    setOffset(-sprite.width() / 2.0, -sprite.height() / 2.0);
}
