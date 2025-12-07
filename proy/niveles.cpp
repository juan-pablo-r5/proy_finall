#include "niveles.h"
#include "mainwindow.h"

#include <QDebug>
#include <QList>
#include <QVariant>
#include <vector>
#include <utility>
#include <QMessageBox>
#include <QRandomGenerator>

#include "colisiones.h"

extern bool nivel1Completado;
extern bool nivel2Completado;
extern bool nivel3Completado;


namespace {

struct PlataformaInfo {
    int x;
    int y;
    int ancho;
    int alto;
    QColor color;
};

QGraphicsRectItem* crearPlataforma(QGraphicsScene *scene, int x, int y, int ancho, int alto,
                                   const QColor &color = QColor(0,0,0,255))
{
    QGraphicsRectItem *plataforma = new QGraphicsRectItem(x, y, ancho, alto);
    plataforma->setBrush(color);
    plataforma->setPen(Qt::NoPen);
    plataforma->setZValue(1);
    plataforma->setData(0, QVariant(QStringLiteral("plataforma")));
    scene->addItem(plataforma);
    return plataforma;
}

}
niveles::niveles(int numNivel, QWidget *parent)
    : QGraphicsView(parent),
    scene(new QGraphicsScene(this)),
    player(nullptr),
    timerUpdate(new QTimer(this)),
    nivelActual(numNivel)
{
    // --- Música de fondo ---
    musicaFondo = new QMediaPlayer(this);
    audioSalida = new QAudioOutput(this);
    musicaFondo->setAudioOutput(audioSalida);

    auto cargarMusica = [&](const QString &ruta){
        try {
            musicaFondo->setSource(QUrl(ruta));
            if (musicaFondo->source().isEmpty())
                throw std::runtime_error("No se pudo cargar la música del nivel");
        } catch (const std::exception &e) {
            qDebug() << "ERROR:" << e.what();
        }
        audioSalida->setVolume(0.5);
        musicaFondo->setLoops(QMediaPlayer::Infinite);
        musicaFondo->play();
    };

    if (nivelActual == 1)
        cargarMusica("qrc:/sounds/nivel1_music.mp3");
    else if (nivelActual == 2)
        cargarMusica("qrc:/sounds/bomb.mp3");
    else if (nivelActual == 3)
        cargarMusica("qrc:/sounds/epic.mp3");

    // --- HUD Vidas ---
    textoVidas = new QLabel(this);
    textoVidas->setText("Vidas: 3");
    textoVidas->setStyleSheet("color:white; font-size: 20px;");
    textoVidas->move(10, 10);
    textoVidas->raise();

    // --- HUD Monedas / Puntos ---
    textoMonedas = new QLabel(this);
    if (nivelActual == 3) {
        textoMonedas->setText("Puntos: 0");
        textoMonedas->setStyleSheet("color: red; font-size: 20px;");
        textoMonedas->setFixedWidth(300);
    } else {
        textoMonedas->setText("Monedas: 0");
        textoMonedas->setStyleSheet("color: yellow; font-size: 20px;");
    }
    textoMonedas->move(10, 40);
    textoMonedas->raise();

    // --- Mensaje bloqueo de ataque (nivel 1 sigilo) ---
    mensajeBloqueoAtaque = new QLabel(this);
    mensajeBloqueoAtaque->setText("No puedes atacar en una misión de sigilo");
    mensajeBloqueoAtaque->setStyleSheet("color: red; font-size: 22px; font-weight: bold;");
    mensajeBloqueoAtaque->setGeometry(200, 100, 500, 40);
    mensajeBloqueoAtaque->hide();

    // --- Escena base (fondos, tamaño, monedas, etc.) ---
    configurarEscenaBase();

    // --- Crear personaje ---
    player = new personaje();
    scene->addItem(player);
    player->setZValue(2);
    entidades.append(player);

    if (nivelActual == 3) {
        player->setPos(scene->sceneRect().width() / 2, 500);
    } else if (nivelActual == 2) {
        player->setPos(400, 650);
    } else {
        player->setPos(100, 550);
    }

    // --- Plataformas y enemigos por nivel ---
    crearPlataformas();
    generarCentinelas();

    // --- Cámara inicial ---
    centerOn(player);
    setAlignment(Qt::AlignCenter);

    // --- Timer general del nivel ---
    connect(timerUpdate, &QTimer::timeout, this, &niveles::actualizarEscena);
    timerUpdate->start(16);   // ~60 FPS
}


void niveles::mostrarMensajeBloqueo()
{
    mensajeBloqueoAtaque->show();
    mensajeBloqueoAtaque->raise();

    QTimer::singleShot(1500, this, [this]() {
        mensajeBloqueoAtaque->hide();
    });
}

void niveles::actualizarEscena()
{
    if (!player) return;

    if (nivelActual == 2) {

        // Movimiento horizontal del jugador (barco)
        player->setPos(player->x() + player->getVelocidadX(), player->y());

        // Limitar al rango visible
        if (player->x() < 0)    player->setPos(0, player->y());
        if (player->x() > 1500) player->setPos(1500, player->y());

        // Proyectiles (bombas) y colisiones
        actualizarProyectilesNivel2();
        return;
    }

    for (Entidad *e : entidades) {
        if (e)
            e->actualizar();   // personaje, enemigos, proyectiles (si hubiera)
    }

    QPointF centro = player->pos();
    centerOn(centro.x(), centro.y() - 100);

    if (centro.x() < 0)
        player->setPos(0, 600);
    if (centro.x() > 3700)
        player->setPos(3700, 600);

    if (nivelActual == 3) {

        QRectF hitboxJugador = player->posHitbox();

        for (enemigos *c : std::as_const(centinelas)) {
            if (!c) continue;

            QRectF r = c->sceneBoundingRect();

            if (hitboxJugador.intersects(r)) {

                Cuerpo J, E;

                J.masa = 2.0f;
                J.vel  = { player->getVelocidadX(), player->getVelocidadY() };

                QPointF v = c->getVelocidadCampo();
                E.masa = 1.0f;
                E.vel  = { float(v.x()), float(v.y()) };

                resolverColision(J, E, ELASTICA);

                player->setVelocidadX(J.vel.x);
                player->setVelocidadY(J.vel.y);
                c->setVelocidadCampo(QPointF(E.vel.x, E.vel.y));

                // Pequeño "knockback"
                player->setPos(player->x() + J.vel.x * 4, player->y());
                c->setPos(c->x() + E.vel.x * 4, c->y());

                // Daño al jugador
                player->perderVida();
                textoVidas->setText(QString("Vidas: %1").arg(player->vidas));

                if (player->vidas <= 0) {
                    emit gameOver("muerte");
                    return;
                }
            }
        }
    }


    if (nivelActual == 1) {

        if (!jugadorRecibiendoDaño) {

            for (enemigos *c : std::as_const(centinelas)) {
                if (!c) continue;

                c->actualizarVision(player->posHitbox());

                if (c->jugadorDetectado()) {

                    jugadorRecibiendoDaño = true;
                    player->setEnabled(false);

                    Cuerpo J, E;

                    J.masa = 2.0f;
                    J.vel = { player->getVelocidadX(), player->getVelocidadY() };

                    E.masa = 1.0f;
                    E.vel = { c->getVelocidadX(), 0.0f };

                    resolverColision(J, E, PERFECTAMENTE_INELASTICA);

                    float retroceso = -J.vel.x * 10.0f;
                    player->setX(player->x() + retroceso);

                    player->setVelocidadX(J.vel.x);
                    player->setVelocidadY(J.vel.y);

                    QTimer::singleShot(400, this, [this]() {

                        player->perderVida();
                        textoVidas->setText(QString("Vidas: %1").arg(player->vidas));

                        jugadorRecibiendoDaño = false;
                        player->setEnabled(true);

                        if (player->vidas <= 0)
                            emit gameOver("muerte");
                    });

                    return;
                }
            }
        }
    }

    if (player->atacando) {
        QRectF golpe = player->hitboxAtaque->sceneBoundingRect();

        for (int i = 0; i < centinelas.size(); i++) {
            enemigos *e = centinelas[i];
            if (!e) continue;

            if (golpe.intersects(e->sceneBoundingRect())) {

                e->recibirDaño(1);

                if (!e->estaVivo()) {
                    scene->removeItem(e);
                    centinelas.removeAt(i);
                    entidades.removeOne(e);
                    delete e;

                    if (nivelActual == 3) {
                        enemigosEliminados++;
                        textoMonedas->setText(
                            QString("Puntos: %1").arg(enemigosEliminados)
                            );

                        if (enemigosEliminados >= enemigosMetaNivel3) {
                            nivel3Completado = true;
                            QMessageBox::information(this, "¡Victoria!",
                                                     "Has eliminado a todos los enemigos.");
                            emit gameOver("ganar");
                            return;
                        }
                    }
                }

                return;
            }
        }
    }

    if (nivelActual == 1) {

        QRectF box = player->posHitbox();

        for (int i = monedasEscena.size() - 1; i >= 0; i--) {
            QGraphicsPixmapItem *m = monedasEscena[i];

            if (box.intersects(m->sceneBoundingRect())) {

                scene->removeItem(m);
                delete m;
                monedasEscena.removeAt(i);

                monedas++;
                textoMonedas->setText(QString("Cofres: %1").arg(monedas));

                if (monedas >= 3) {
                    nivel1Completado = true;
                    QMessageBox::information(this, "¡Nivel Completado!",
                                             "Has recolectado todas las monedas.");
                    emit gameOver("ganar");
                    return;
                }
            }
        }
    }
}


void niveles::keyPressEvent(QKeyEvent *event)
{
    if (!player) {
        QGraphicsView::keyPressEvent(event);
        return;
    }

    if (nivelActual == 2) {
        if (event->key() == Qt::Key_A)
            player->moverIzquierda();
        else if (event->key() == Qt::Key_D)
            player->moverDerecha();
        return;
    }

    switch (event->key()) {
    case Qt::Key_A:
        player->moverIzquierda();
        break;
    case Qt::Key_D:
        player->moverDerecha();
        break;
    case Qt::Key_Space:
        player->saltar();
        break;
    case Qt::Key_S:
        player->deslizar();
        break;
    case Qt::Key_J:
        if (nivelActual == 1) {
            mostrarMensajeBloqueo();
        } else {
            player->atacar();
        }
        break;
    default:
        QGraphicsView::keyPressEvent(event);
        break;
    }
}

void niveles::keyReleaseEvent(QKeyEvent *event)
{
    if (!player) {
        QGraphicsView::keyReleaseEvent(event);
        return;
    }

    if (nivelActual == 2 &&
        (event->key() == Qt::Key_A || event->key() == Qt::Key_D)) {
        player->parar();
        return;
    }

    if (event->key() == Qt::Key_A || event->key() == Qt::Key_D) {
        player->parar();
    } else {
        QGraphicsView::keyReleaseEvent(event);
    }
}


void niveles::configurarEscenaBase()
{
    setScene(scene);
    setFixedSize(800, 600);
    setFocusPolicy(Qt::StrongFocus);
    setRenderHint(QPainter::Antialiasing, false);
    setRenderHint(QPainter::SmoothPixmapTransform, true);

    QPixmap bg;

    // ----------------- NIVEL 1 (fondo + monedas + controles) -----------------
    if (nivelActual == 1) {

        bg.load(":/backgrounds/background.jpg");

        // Posiciones de monedas/cofres
        QList<QPointF> posicionesMonedas = {
            {1000, 100}, // primer cofre
            {1780, 310},  // segundo cofre
            {3000, 650}   // tercer cofre
        };

        // Iconos W A S D SPACE J
        QGraphicsPixmapItem *ctrlW = new QGraphicsPixmapItem(
            QPixmap(":/sprites/w.png").scaled(50, 50, Qt::KeepAspectRatio)
            );
        ctrlW->setPos(65, 240);
        ctrlW->setZValue(3);
        scene->addItem(ctrlW);

        QGraphicsPixmapItem *ctrlA = new QGraphicsPixmapItem(
            QPixmap(":/sprites/a.png").scaled(50, 50, Qt::KeepAspectRatio)
            );
        ctrlA->setPos(10, 300);
        ctrlA->setZValue(3);
        scene->addItem(ctrlA);

        QGraphicsPixmapItem *ctrlS = new QGraphicsPixmapItem(
            QPixmap(":/sprites/s.png").scaled(50, 50, Qt::KeepAspectRatio)
            );
        ctrlS->setPos(65, 300);
        ctrlS->setZValue(3);
        scene->addItem(ctrlS);

        QGraphicsPixmapItem *ctrlD = new QGraphicsPixmapItem(
            QPixmap(":/sprites/d.png").scaled(50, 50, Qt::KeepAspectRatio)
            );
        ctrlD->setPos(120, 300);
        ctrlD->setZValue(3);
        scene->addItem(ctrlD);

        QGraphicsPixmapItem *ctrlSpace = new QGraphicsPixmapItem(
            QPixmap(":/sprites/space.png").scaled(120, 120, Qt::KeepAspectRatio)
            );
        ctrlSpace->setPos(80, 400);
        ctrlSpace->setZValue(3);
        scene->addItem(ctrlSpace);

        QGraphicsPixmapItem *ctrlJ = new QGraphicsPixmapItem(
            QPixmap(":/sprites/j.png").scaled(50, 50, Qt::KeepAspectRatio)
            );
        ctrlJ->setPos(200, 300);
        ctrlJ->setZValue(3);
        scene->addItem(ctrlJ);

        // Monedas / cofres
        for (const QPointF &p : posicionesMonedas) {
            QGraphicsPixmapItem *m = new QGraphicsPixmapItem(
                QPixmap(":/sprites/tesoro.png").scaled(90, 90)
                );
            m->setPos(p);
            m->setData(0, "moneda");
            m->setZValue(3);
            scene->addItem(m);
            monedasEscena.append(m);
        }

        // Fondo repetido horizontal
        const int backgroundTiles = 2;
        const int levelWidth = bg.width() * backgroundTiles;
        scene->setSceneRect(0, 0, levelWidth, 1080);

        for (int i = 0; i < backgroundTiles; ++i) {
            QGraphicsPixmapItem *background = new QGraphicsPixmapItem(bg);
            background->setPos(i * bg.width(), 0);
            background->setZValue(0);
            scene->addItem(background);
        }

        resetTransform();
        scale(0.8, 0.8);
        return;
    }

    // ----------------- NIVEL 2 (mar) -----------------
    if (nivelActual == 2) {
        QPixmap bg2(":/backgrounds/mar.jpg");
        bg2 = bg2.scaled(1600, 800, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        scene->setSceneRect(0, 0, 1600, 800);

        QGraphicsPixmapItem *background = new QGraphicsPixmapItem(bg2);
        background->setPos(0, 0);
        background->setZValue(0);
        scene->addItem(background);

        resetTransform();
        scale(0.9, 0.9);
        return;
    }

    // ----------------- NIVEL 3 (arena cerrada) -----------------
    if (nivelActual == 3) {
        scene->setSceneRect(0, 0, 1600, 800);
        QPixmap bg3(":/backgrounds/background3.jpg");
        QGraphicsPixmapItem *background = new QGraphicsPixmapItem(bg3);
        background->setPos(0, 0);
        background->setZValue(0);
        scene->addItem(background);

        resetTransform();
        scale(0.9, 0.9);
        return;
    }
}

// -------------------------------------------------------------------
//  Plataformas según nivel
// -------------------------------------------------------------------
void niveles::crearPlataformas()
{
    // NIVEL 3: suelo + algunas plataformas internas
    if (nivelActual == 3) {
        const int levelWidth = static_cast<int>(scene->sceneRect().width());

        QGraphicsRectItem *suelo = new QGraphicsRectItem(0, 780, levelWidth, 40);
        suelo->setBrush(QColor("#5B3A29"));   // marrón
        suelo->setPen(Qt::NoPen);
        suelo->setZValue(1);
        suelo->setData(0, QVariant(QStringLiteral("suelo")));
        scene->addItem(suelo);

        const std::vector<PlataformaInfo> plataformas = {
            {420, 620, 220, 20, QColor(0,0,0,255)},
            {700, 500, 180, 20, QColor(0,0,0,255)},
            {950, 700, 160, 20, QColor(0,0,0,255)}
        };

        for (const auto &p : plataformas) {
            crearPlataforma(scene, p.x, p.y, p.ancho, p.alto, p.color);
        }

        return;
    }

    // NIVEL 1: plataformas + una inelástica + suelo largo
    if (nivelActual == 1) {

        const std::vector<PlataformaInfo> plataformas = {
            {470, 520, 220, 20, QColor(0,0,0,255)},
            {700, 290, 180, 20, QColor(0,0,200,255)},
            {800, 600, 160, 20, QColor(0,0,0,255)},
            {1160, 660, 200, 20, QColor(0,0,0,255)},
            {1400, 500, 180, 20, QColor(0,0,0,255)},
            {1920, 620, 180, 20, QColor(0,0,0,255)}
        };

        for (const auto &p : plataformas) {
            crearPlataforma(scene, p.x, p.y, p.ancho, p.alto, p.color);
        }

        QGraphicsRectItem* plataformaInelastica =
            crearPlataforma(scene, 900, 500, 200, 20, QColor(200, 0, 0));

        plataformaInelastica->setData(0, QVariant("plataforma_inelastica"));

        const int levelWidth = static_cast<int>(scene->sceneRect().width());
        QGraphicsRectItem *suelo = new QGraphicsRectItem(0, 780, levelWidth, 40);
        suelo->setBrush(QColor(0,0,0,255));
        suelo->setPen(Qt::NoPen);
        suelo->setZValue(1);
        suelo->setData(0, QVariant(QStringLiteral("suelo")));
        scene->addItem(suelo);
    }
}

// -------------------------------------------------------------------
//  Enemigos / proyectiles por nivel
// -------------------------------------------------------------------
void niveles::generarCentinelas()
{
    // NIVEL 3: oleadas de enemigos que entran por izquierda/derecha
    if (nivelActual == 3) {

        QTimer *timerOleadas = new QTimer(this);

        connect(timerOleadas, &QTimer::timeout, this, [this]() {

            for (int i = 0; i < 2; i++) {

                // Enemigo izquierda
                enemigos *eL = new enemigos(this);
                eL->setPos(-100, 500);
                eL->setZValue(2);
                eL->habilitarCampo(player);
                eL->setDireccion(true);      // mira hacia la derecha
                centinelas.append(eL);
                scene->addItem(eL);
                entidades.append(eL);

                // Enemigo derecha
                enemigos *eR = new enemigos(this);
                eR->setPos(scene->sceneRect().width() + 100, 500);
                eR->setZValue(2);
                eR->habilitarCampo(player);
                eR->setDireccion(false);     // mira hacia la izquierda
                centinelas.append(eR);
                scene->addItem(eR);
                entidades.append(eR);
            }
        });

        timerOleadas->start(2000); // cada 2s
        return;
    }

    // NIVEL 2: proyectiles que caen del cielo (bombas)
    if (nivelActual == 2) {

        QTimer *timerProyectiles = new QTimer(this);

        connect(timerProyectiles, &QTimer::timeout, this, [this]() {

            int xRand = QRandomGenerator::global()->bounded(50, 1500);

            Proyectil *p = new Proyectil(
                QPixmap(":/sprites/proyectil.png")
                    .scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation)
                );

            p->setPos(xRand, -50);
            p->setZValue(3);

            proyectiles.append(p);
            entidades.append(p);
            scene->addItem(p);
        });

        timerProyectiles->start(220); // ~4.5 bombas por segundo
        return;
    }

    // NIVEL 1: centinelas de patrulla
    if (nivelActual == 1) {

        const QList<QPointF> posiciones = {
            {520, 750},
            {1220, 620},
            {2100, 640},
            {3000, 680}
        };

        static bool alternar = false;

        for (const QPointF &pos : posiciones) {
            enemigos *centinela = new enemigos(this);
            centinela->setZValue(2);
            centinela->setPos(pos);
            centinela->configurarPatrulla(pos.x() - 80, pos.x() + 80, 1.2);

            if (alternar) {
                centinela->setDireccion(false);
                centinela->setVelocidadX(-1.2f);
            } else {
                centinela->setDireccion(true);
                centinela->setVelocidadX(1.2f);
            }

            alternar = !alternar;
            centinelas.append(centinela);
            entidades.append(centinela);
            scene->addItem(centinela);
        }
    }
}

// -------------------------------------------------------------------
//  Lógica de proyectiles en el nivel 2
// -------------------------------------------------------------------
void niveles::actualizarProyectilesNivel2()
{
    QRectF boxJugador = player->posHitbox();

    // Aumentar dificultad con el tiempo
    if (tiempoNivel2 % 120 == 0) {
        velocidadBombas += 0.4f;
    }

    for (int i = proyectiles.size() - 1; i >= 0; i--) {
        Proyectil *p = proyectiles[i];

        p->velY = velocidadBombas;
        p->actualizar();

        if (p->y() > 900) {
            scene->removeItem(p);
            proyectiles.removeAt(i);
            entidades.removeOne(p);
            delete p;
            continue;
        }

        if (boxJugador.intersects(p->sceneBoundingRect())) {

            Cuerpo J, B;
            J.masa = 4.0f;
            J.vel = { player->getVelocidadX(), 0.0f };

            B.masa = 1.0f;
            B.vel  = { 0.0f, float(velocidadBombas) };

            resolverColision(J, B, INELASTICA, 0.3f);

            float knockback = (player->getVelocidadX() >= 0 ? -1 : 1) * 60.0f;
            player->setX(player->x() + knockback);

            // pequeño "salto" de impacto
            player->setY(player->y() - 15);
            QTimer::singleShot(90, this, [this]() {
                player->setY(player->y() + 15);
            });

            player->setVelocidadX(player->getVelocidadX() * 0.4f);

            scene->removeItem(p);
            proyectiles.removeAt(i);
            entidades.removeOne(p);
            delete p;

            // Vidas
            player->perderVida();
            textoVidas->setText(QString("Vidas: %1").arg(player->vidas));

            if (player->vidas <= 0) {
                emit gameOver("muerte");
                return;
            }
        }
    }

    tiempoNivel2++;

    // Superar nivel 2 al sobrevivir ~20 segundos
    if (tiempoNivel2 >= 20 * 60) {
        QMessageBox::information(this, "¡Nivel Completado!", "Has esquivado todos los disparos.");
        emit gameOver("ganar");
        nivel2Completado = true;
        return;
    }
}
