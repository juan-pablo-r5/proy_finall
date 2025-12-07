#include "personaje.h"
#include <QTransform>
#include <QDebug>
#include <QBrush>
#include <QPen>
#include <QString>
#include "colisiones.h"


personaje::personaje() : Entidad(TipoEntidad::Jugador) {
    sonidoAtaque = new QMediaPlayer(this);
    audioAtaque = new QAudioOutput(this);
    sonidoAtaque->setAudioOutput(audioAtaque);

    sonidoAtaque->setSource(QUrl("qrc:/sounds/ataque.mp3"));
    audioAtaque->setVolume(0.8);

    try {
        if (!spriteSheet.load(":/sprites/Viking-Sheet.png"))
            throw std::runtime_error("No se pudo cargar el sprite del personaje");
    } catch (const std::exception &e) {
        qDebug() << "ERROR:" << e.what();
    }

    vidas = 3;       // contador visual del jugador
    vidasMax = 3;
    vida = 3;        // vida real de la Entidad heredada


    hitbox = new QGraphicsRectItem(100, 130, 80, 50, this);
    hitbox->setBrush(QBrush(QColor(255, 0, 0, 0)));  // rojo semitransparente
    hitbox->setPen(Qt::NoPen);

    hitboxAtaque = new QGraphicsRectItem(0, 0, 80, 60, this);
    hitboxAtaque->setBrush(QColor(255, 0, 0, 0));
    hitboxAtaque->setPen(Qt::NoPen);
    hitboxAtaque->hide();

    velocidad = 3.0f;    // 🔹 movimiento lateral
    gravedad = 0.6f;     // 🔹 fuerza de caída
    mirandoDerecha = true;

    int frameWidth  = 115;   // 1380 / 12
    int frameHeight = 84;    // 420 / 5

    // ---- Cargar animaciones ----
    framesIdle = extraerFrames(0 * frameHeight, frameWidth, frameHeight, 8);
    framesRun  = extraerFrames(2 * frameHeight, frameWidth, frameHeight, 8);
    framesJump = extraerFrames(6 * frameHeight, frameWidth, frameHeight, 3);
    framesSlide = extraerFrames(15 * frameHeight, frameWidth, frameHeight, 7);
    framesAttack = extraerFrames(8 * frameHeight, frameWidth, frameHeight, 4);;


    animacionActual = &framesIdle;
    frameActual = 0;

    setPixmap(animacionActual->at(frameActual)
                  .scaled(290, 290, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    setPos(300, 80);

    animTimer = new QTimer(this);
    connect(animTimer, &QTimer::timeout, this, &personaje::actualizarFrame);
    animTimer->start(70);
}

QVector<QPixmap> personaje::extraerFrames(int y, int frameWidth, int frameHeight, int numFrames) {
    QVector<QPixmap> frames;
    for (int i = 0; i < numFrames; ++i) {
        frames.append(spriteSheet.copy(i * frameWidth, y, frameWidth, frameHeight));
    }
    return frames;
}


QRectF personaje::posHitbox(){
    return hitbox->sceneBoundingRect();

}

void personaje::actualizar() {
    actualizarFisica();
}

void personaje::moverBase() {
    setX(x() + velX);
    setY(y() + velY);
}



void personaje::perderVida() {

    if (invulnerable) return;

    invulnerable = true;

    vida -= 1;
    if (vida < 0) vida = 0;
    vidas = vida;

    qDebug() << "Vidas restantes:" << vidas;

    // volver vulnerable en el SIGUIENTE frame
    QTimer::singleShot(30, [this]() { invulnerable = false; });

    if (vidas == 0)
        morir();
}

void personaje::morir() {
    qDebug() << "Jugador murió";

}



void personaje::actualizarFrame() {
    if (!animacionActual || animacionActual->isEmpty())
        return;

    frameActual = (frameActual + 1) % animacionActual->size();
    QPixmap frame = animacionActual->at(frameActual);

    if (!mirandoDerecha)
        frame = frame.transformed(QTransform().scale(-1, 1));

    setPixmap(frame.scaled(290, 290, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}


void personaje::moverIzquierda() {
    velocidadX = -velocidad;   // ← establece movimiento continuo
    if (enSuelo) {
        if (animacionActual != &framesRun)
            animacionActual = &framesRun;
    } else {
        if (animacionActual != &framesJump)
            animacionActual = &framesJump;
    }
    mirandoDerecha = false;
}

void personaje::moverDerecha() {
    velocidadX = velocidad;

    if (enSuelo) {
        if (animacionActual != &framesRun)
            animacionActual = &framesRun;
    } else {
        if (animacionActual != &framesJump)
            animacionActual = &framesJump;
    }

    mirandoDerecha = true;
}

void personaje::parar() {
    velocidadX = 0;            // ← detiene el movimiento lateral
    if (enSuelo)
        animacionActual = &framesIdle;
}


void personaje::saltar() {
    if (coyoteCounter <= 0) return;   // salto permitido si aún queda “coyote”

    velocidadY = -15;
    animacionActual = &framesJump;
    frameActual = 0;

    // Consumimos el coyote para evitar saltar varias veces sin tocar suelo
    coyoteCounter = 0;
    enSuelo = false;
}




void personaje::deslizar() {
    if (!enSuelo || accionEspecialActiva) return;

    accionEspecialActiva = true;

    // --- Reducir hitbox manteniendo la base ---
    QRectF r = hitbox->rect();
    qreal bottom = r.top() + r.height();   // borde inferior actual

    qreal nuevaAltura = 25;                // altura más pequeña para deslizar
    qreal nuevaY = bottom - nuevaAltura;   // ajustar para no flotar

    hitbox->setRect(r.left(), nuevaY, r.width(), nuevaAltura);

    // --- Animación slide ---
    animacionActual = &framesSlide;
    frameActual = 0;

    // --- Aumentar velocidad según dirección ---
    velocidadX = mirandoDerecha ? velocidad + 2 : -velocidad - 2;

    // --- Recuperar estado normal ---
    QTimer::singleShot(700, [this]() {
        // restaurar hitbox grande
        hitbox->setRect(100, 130, 80, 50);

        velocidadX = 0;
        accionEspecialActiva = false;

        animacionActual = &framesIdle;
        frameActual = 0;
    });
}


void personaje::atacar() {
    if (accionEspecialActiva) return;

    sonidoAtaque->stop();
    sonidoAtaque->play();

    accionEspecialActiva = true;
    atacando = true;

    animacionActual = &framesAttack;
    frameActual = 0;

    // --- ACTIVAR hitbox de ataque ---
    if (mirandoDerecha) {
        hitboxAtaque->setRect(180, 90, 70, 60);  // Y subió de 130 → 90
    } else {
        hitboxAtaque->setRect(30, 90, 70, 60);
    }

    hitboxAtaque->show();

    // El ataque dura 300ms
    QTimer::singleShot(300, [this]() {
        hitboxAtaque->hide();
        atacando = false;
    });

    // Volver a idle después de terminar animación
    QTimer::singleShot(800, [this]() {
        accionEspecialActiva = false;
        animacionActual = &framesIdle;
    });
}




void personaje::actualizarFisica()
{
    // Movimiento horizontal
    setX(x() + velocidadX);

    // Aplicar gravedad
    if (!enSuelo) {
        velocidadY += gravedad;
        setY(y() + velocidadY);
    }

    bool tocandoSuelo = false;

    // --- Colisiones usando la HITBOX ---
    QList<QGraphicsItem*> colisiones = hitbox->collidingItems();
    for (QGraphicsItem *item : colisiones) {

        if (item->data(0).toString() == "plataforma" || item->data(0).toString() == "suelo") {

            qreal topPlataforma = item->sceneBoundingRect().top();
            qreal bottomHitbox = hitbox->sceneBoundingRect().bottom();

            // Solo si cae desde arriba
            if (velocidadY > 0 && bottomHitbox >= topPlataforma - 5) {

                // Reposicionar al personaje encima de la plataforma
                // Información de la hitbox respecto al sprite
                const QRectF rectoHitbox = hitbox->rect();
                const qreal alturaHitbox = rectoHitbox.height();
                const qreal desplazamientoHitbox = rectoHitbox.top();

                // Mover al personaje para que la parte inferior de la hitbox
                // quede alineada con la plataforma en coordenadas de escena
                const qreal nuevaY = topPlataforma - (desplazamientoHitbox + alturaHitbox);

                // Ajustar al personaje (NO a la hitbox)
                setY(nuevaY);
                velocidadY = 0;
                enSuelo = true;
                tocandoSuelo = true;

                // Animación correcta
                if (!accionEspecialActiva) {
                    if (enSuelo) {
                        animacionActual = (velocidadX == 0) ? &framesIdle : &framesRun;
                    }
                }
                break;
            }
        }

        else if (item->data(0).toString() == "plataforma_inelastica") {

            Cuerpo jugador;
            jugador.masa = 2.0f;
            jugador.vel  = { velocidadX, velocidadY };

            Cuerpo piso;
            piso.masa = 1000000.0f;
            piso.vel  = { 0.0f, 0.0f };

            resolverColision(jugador, piso, INELASTICA, 0.2f);

            velocidadX = jugador.vel.x;

            float rebote = -velocidadY * 0.3f;
            if (rebote < -2) rebote = -2; // límite

            velocidadY = rebote;

            QRectF box = item->sceneBoundingRect();
            setY(box.top() - hitbox->rect().height() - 130);

            enSuelo = true;
            coyoteCounter = COYOTE_FRAMES;

            animacionActual = &framesIdle;
        }

    }

    // --- Si NO tocó piso ni plataforma ---
    if (tocandoSuelo) {
        enSuelo = true;
        coyoteCounter = COYOTE_FRAMES;   // recuperar tiempo para permitir salto
    }
    else {
        enSuelo = false;

        if (coyoteCounter > 0)
            coyoteCounter--;             // cuenta regresiva mientras estás en el aire
    }

}
