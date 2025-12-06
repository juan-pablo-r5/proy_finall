#include "niveles.h"
#include "mainwindow.h"
#include <QDebug>
#include <QList>
#include <QVariant>
#include <vector>
#include <utility>
#include <QMessageBox>
#include <QRandomGenerator>

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

    musicaFondo = new QMediaPlayer(this);
    audioSalida = new QAudioOutput(this);
    musicaFondo->setAudioOutput(audioSalida);

    if (nivelActual == 1) {
        try {
            musicaFondo->setSource(QUrl("qrc:/sounds/nivel1_music.mp3"));
            if (musicaFondo->source().isEmpty())
                throw std::runtime_error("No se pudo cargar la música del nivel 1");
        } catch (const std::exception &e) {
            qDebug() << "ERROR:" << e.what();
        }
        audioSalida->setVolume(0.5);   // volumen entre 0.0 y 1.0
        musicaFondo->setLoops(QMediaPlayer::Infinite);
        musicaFondo->play();
    }

    else if (nivelActual == 2) {
        try {
            musicaFondo->setSource(QUrl("qrc:/sounds/bomb.mp3"));
            if (musicaFondo->source().isEmpty())
                throw std::runtime_error("No se pudo cargar la música del nivel 1");
        } catch (const std::exception &e) {
            qDebug() << "ERROR:" << e.what();
        }
        audioSalida->setVolume(0.5);   // volumen entre 0.0 y 1.0
        musicaFondo->setLoops(QMediaPlayer::Infinite);
        musicaFondo->play();
    }

    else if (nivelActual == 3) {
        try {
            musicaFondo->setSource(QUrl("qrc:/sounds/epic.mp3"));
            if (musicaFondo->source().isEmpty())
                throw std::runtime_error("No se pudo cargar la música del nivel 1");
        } catch (const std::exception &e) {
            qDebug() << "ERROR:" << e.what();
        }
        audioSalida->setVolume(0.5);   // volumen entre 0.0 y 1.0
        musicaFondo->setLoops(QMediaPlayer::Infinite);
        musicaFondo->play();
    }

    // -- Visor de vidas ---
    textoVidas = new QLabel(this);
    textoVidas->setText("Vidas: 3");
    textoVidas->setStyleSheet("color:white; font-size: 20px;");
    textoVidas->move(10, 10); // esquina superior
    textoVidas->raise();




    // -- Monedas
    textoMonedas = new QLabel(this);
    textoMonedas->setText("Monedas: 0");
    textoMonedas->setStyleSheet("color: yellow; font-size: 20px;");
    textoMonedas->move(10, 40); // debajo del contador de vidas
    textoMonedas->raise();

    if (nivelActual == 3) {
        textoMonedas->setText("Puntos: 0");
        textoMonedas->setStyleSheet("color: red; font-size: 20px;");
        textoMonedas->setFixedWidth(300);
    }



    // Mensaje Sigilo
    mensajeBloqueoAtaque = new QLabel(this);
    mensajeBloqueoAtaque->setText("No puedes atacar en una misión de sigilo");
    mensajeBloqueoAtaque->setStyleSheet("color: red; font-size: 22px; font-weight: bold;");
    mensajeBloqueoAtaque->setGeometry(200, 100, 500, 40);
    mensajeBloqueoAtaque->hide();


    //-- Escena Base ----
    configurarEscenaBase();



    // --- Crear personaje ---
    player = new personaje();
    scene->addItem(player);
    player->setZValue(2);

    if (nivelActual == 3) {
        player->setPos(scene->sceneRect().width() / 2, 500);
    }
    else if (nivelActual == 2) {
        player->setPos(400, 650);  // barco abajo
    }
    else {
        player->setPos(100, 550);
    }


    crearPlataformas();

    generarCentinelas();

    // --- Cámara ---
    centerOn(player);
    setAlignment(Qt::AlignCenter); // evita recortes de borde

    // --- Timer general del nivel ---
    connect(timerUpdate, &QTimer::timeout, this, &niveles::actualizarEscena);
    timerUpdate->start(16); // ~60 FPS
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

        // mantener solo el movimiento horizontal
        player->setPos(player->x() + player->getVelocidadX(), player->y());

        // limitar al rango visible
        if (player->x() < 0) player->setPos(0, player->y());
        if (player->x() > 1500) player->setPos(1500, player->y());

        // PROCESAR PROYECTILES
        actualizarProyectilesNivel2();

        return; // MUY IMPORTANTE: evita usar la física normal
    }

    player->actualizarFisica();

    // --- Cámara sigue al jugador con un pequeño offset vertical ---
    QPointF centro = player->pos();
    centerOn(centro.x(), centro.y() - 100);

    if (centro.x() < 0) {
        player->setPos(0, 600);
    }

    if (centro.x() > 3700) {
        player->setPos(3700, 600);
    }

    // ===========================
    //  PROCESAR ENEMIGOS
    // ===========================
    for (enemigos *centinela : std::as_const(centinelas)) {
        if (!centinela) continue;

        if (nivelActual == 3) {

            QRectF hitboxJugador = player->posHitbox();
            QRectF enemigoRect   = centinela->sceneBoundingRect();

            // Revisión de colisión ANTES de mover
            if (hitboxJugador.intersects(enemigoRect)) {

                scene->removeItem(centinela);
                centinelas.removeOne(centinela);
                delete centinela;

                player->perderVida();
                textoVidas->setText(QString("Vidas: %1").arg(player->vidas));

                if (player->vidas <= 0)
                    emit gameOver("muerte");

                // detener el procesamiento de TODOS los enemigos
                return;
            }
        }

        if (nivelActual == 1) {

            // --- Si ya tomó daño, ignoramos todo ---
            if (jugadorRecibiendoDaño)
                continue;

            centinela->actualizarVision(player->posHitbox());

            if (centinela->jugadorDetectado()) {

                jugadorRecibiendoDaño = true;  // ← activamos el bloqueo
                player->setEnabled(false);     // opcional, para congelar movimiento

                // Mostrar sprite de alerta ya lo hace actualizarVision()

                QTimer::singleShot(400, this, [this]() {

                    // Aplicar daño
                    player->setPos(0, 600);
                    player->perderVida();
                    textoVidas->setText(QString("Vidas: %1").arg(player->vidas));

                    jugadorRecibiendoDaño = false; // ← se desbloquea después del daño
                    player->setEnabled(true);

                    if (player->vidas <= 0)
                        emit gameOver("muerte");
                });

                return; // ← IMPORTANTE: detiene procesamiento de más enemigos
            }
        }

        centinela->mover();
    }

    if (player->atacando) {
        QRectF golpe = player->hitboxAtaque->sceneBoundingRect();

        for (int i = 0; i < centinelas.size(); i++) {
            enemigos *e = centinelas[i];
            if (!e) continue;

            if (golpe.intersects(e->sceneBoundingRect())) {

                scene->removeItem(e);
                centinelas.removeAt(i);
                delete e;

                if (nivelActual == 3) {
                    enemigosEliminados++;
                    textoMonedas->setText(
                        QString("Puntos: %1").arg(enemigosEliminados)
                        );
                    qDebug() << " eliminados:" << enemigosEliminados;

                    // 🔥 Condición de victoria AQUÍ
                    if (enemigosEliminados >= enemigosMetaNivel3) {
                        QMessageBox::information(this, "¡Victoria!",
                                                 "Has eliminado a todos los enemigos.");
                        emit gameOver("ganar");
                        return;
                    }
                }

                return;   // ← RETURN CORRECTO (solo 1)
            }
        }
    }


    for (int i = monedasEscena.size() - 1; i >= 0; i--) {
        QGraphicsPixmapItem *m = monedasEscena[i];

        if (player->posHitbox().intersects(m->sceneBoundingRect())) {

            scene->removeItem(m);
            delete m;
            monedasEscena.removeAt(i);

            monedas++;
            textoMonedas->setText(QString("Cofres: %1").arg(monedas));

            if (monedas >= 3) {
                nivel1Completado = true;
                QMessageBox::information(this, "¡Nivel completado!", "Has recolectado todas las monedas.");
                emit gameOver("ganar");
                return;
            }
        }
    }

    if (nivelActual == 2) {

        QRectF boxJugador = player->posHitbox();

        for (int i = proyectiles.size() - 1; i >= 0; i--) {
            Proyectil *p = proyectiles[i];
            p->actualizar();

            // afuera de pantalla
            if (p->y() > 900) {
                scene->removeItem(p);
                proyectiles.removeAt(i);
                delete p;
                continue;
            }

            // colisión con el barco
            if (boxJugador.intersects(p->sceneBoundingRect())) {
                scene->removeItem(p);
                proyectiles.removeAt(i);
                delete p;

                player->perderVida();
                textoVidas->setText(QString("Vidas: %1").arg(player->vidas));

                if (player->vidas <= 0) {
                    emit gameOver("muerte");
                    return;
                }
            }
        }

        tiempoNivel2++;

        // superar nivel 2 al sobrevivir 15 segundos
        if (tiempoNivel2 >= 16 * 60) {
            QMessageBox::information(this, "¡Nivel Completado!", "Has esquivado todos los disparos.");
            emit gameOver("ganar");
            return;
        }
    }

}

void niveles::keyPressEvent(QKeyEvent *event)
{

    if (nivelActual == 2) {
        if (event->key() == Qt::Key_A) player->moverIzquierda();
        else if (event->key() == Qt::Key_D) player->moverDerecha();
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
            break;
        }
        player->atacar();
        break;
    default:
        QGraphicsView::keyPressEvent(event);
        break;
    }
}

void niveles::keyReleaseEvent(QKeyEvent *event)
{
    if (nivelActual == 2 && (event->key() == Qt::Key_A || event->key() == Qt::Key_D)) {
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


    if(nivelActual ==1){



        bg.load(":/backgrounds/background.jpg");
        QList<QPointF> posicionesMonedas = {
            {1000, 100},
            {1500, 450},
            {2500, 600}
        };

        QGraphicsPixmapItem *ctrlW = new QGraphicsPixmapItem(
            QPixmap(":/sprites/w.png").scaled(50, 50, Qt::KeepAspectRatio)
            );
        ctrlW->setPos(65, 240); // Posición en el MAPA, no en la ventana
        ctrlW->setZValue(3);
        scene->addItem(ctrlW);

        QGraphicsPixmapItem *ctrlA = new QGraphicsPixmapItem(
            QPixmap(":/sprites/a.png").scaled(50, 50, Qt::KeepAspectRatio)
            );
        ctrlA->setPos(10, 300); // Posición en el MAPA, no en la ventana
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
        ctrlD->setPos(120, 300); // Posición en el MAPA, no en la ventana
        ctrlD->setZValue(3);
        scene->addItem(ctrlD);


        QGraphicsPixmapItem *ctrlSpace = new QGraphicsPixmapItem(
            QPixmap(":/sprites/space.png").scaled(120, 1200, Qt::KeepAspectRatio)
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
    }

    else if (nivelActual == 2) {

        QPixmap bg(":/backgrounds/mar.jpg");

        // Escalar al tamaño de la escena
        bg = bg.scaled(1600, 800, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        scene->setSceneRect(0, 0, 1600, 800);

        QGraphicsPixmapItem *background = new QGraphicsPixmapItem(bg);
        background->setPos(0, 0);
        background->setZValue(0);

        scene->addItem(background);

        resetTransform();
        scale(0.9, 0.9);
        return;
    }

    else if (nivelActual == 3) {
        scene->setSceneRect(0, 0, 1600, 800);
        QPixmap bg(":/backgrounds/background3.jpg");
        QGraphicsPixmapItem *background = new QGraphicsPixmapItem(bg);
        background->setPos(0, 0);
        background->setZValue(0);
        scene->addItem(background);

        resetTransform();
        scale(0.9, 0.9);
        return;
    }
    else {
        bg.load(":/backgrounds/background.jpg");         // el que ya usas en nivel 1
    }

    const int backgroundTiles = 2;
    const int levelWidth = bg.width() * backgroundTiles;
    scene->setSceneRect(0, 0, levelWidth, 1080);

    for (int i = 0; i < backgroundTiles; ++i) {
        QGraphicsPixmapItem *background = new QGraphicsPixmapItem(bg);
        background->setPos(i * bg.width(), 0);
        background->setZValue(0);
        scene->addItem(background);
    }

    resetTransform();  // limpia transformaciones anteriores
    scale(0.8, 0.8);
}

void niveles::crearPlataformas()
{

    if (nivelActual == 3) {
        // --- SOLO SUELO, SIN PLATAFORMAS ---
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

        for (const auto &plataforma : plataformas) {
            crearPlataforma(scene, plataforma.x, plataforma.y, plataforma.ancho, plataforma.alto, plataforma.color);
        }

        return; // importante: NO crear las plataformas del nivel 1 aquí
    }

    else if(nivelActual == 1){

        const std::vector<PlataformaInfo> plataformas = {
            {470, 520, 220, 20, QColor(0,0,0,255)},
            {700, 290, 180, 20, QColor(0,0,200,255)},
            {800, 600, 160, 20, QColor(0,0,0,255)},
            {1160, 660, 200, 20, QColor(0,0,0,255)},
            {1400, 700, 180, 20, QColor(0,0,0,255)}
        };

        for (const auto &plataforma : plataformas) {
            crearPlataforma(scene, plataforma.x, plataforma.y, plataforma.ancho, plataforma.alto, plataforma.color);
        }

        const int levelWidth = static_cast<int>(scene->sceneRect().width());
        QGraphicsRectItem *suelo = new QGraphicsRectItem(0, 780, levelWidth, 40);
        suelo->setBrush(QColor(0,0,0,255));   // marrón
        suelo->setPen(Qt::NoPen);
        suelo->setZValue(1);
        suelo->setData(0, QVariant(QStringLiteral("suelo")));
        scene->addItem(suelo);
    }
}

void niveles::generarCentinelas()
{
    if (nivelActual == 3) {

        // Timer para oleadas automáticas
        QTimer *timerOleadas = new QTimer(this);

        connect(timerOleadas, &QTimer::timeout, this, [this]() {

            // Crear 2 enemigos a izquierda y derecha
            for (int i = 0; i < 2; i++) {

                // ENEMIGO IZQUIERDA  (entra por -100 → debe mirar hacia la derecha)
                enemigos *eL = new enemigos(this);
                eL->setPos(-100, 500);
                eL->setZValue(2);
                eL->habilitarCampo(player);
                eL->setDireccion(true);      // → mirando a la derecha
                centinelas.append(eL);
                scene->addItem(eL);

                // ENEMIGO DERECHA  (entra por width+100 → debe mirar hacia la izquierda)
                enemigos *eR = new enemigos(this);
                eR->setPos(scene->sceneRect().width() + 100, 500);
                eR->setZValue(2);
                eR->habilitarCampo(player);
                eR->setDireccion(false);     // ← mirando a la izquierda
                centinelas.append(eR);
                scene->addItem(eR);

            }
        });

        timerOleadas->start(2000); // cada 4 segundos
        return;
    }

    else if (nivelActual == 2) {
        QTimer *timerProyectiles = new QTimer(this);

        connect(timerProyectiles, &QTimer::timeout, this, [this]() {

            int xRand = QRandomGenerator::global()->bounded(50, 1500);

            Proyectil *p = new Proyectil(
                QPixmap(":/sprites/proyectil.png").scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation)
                );

            p->setPos(xRand, -50);
            p->setZValue(3);

            proyectiles.append(p);
            scene->addItem(p);
        });

        timerProyectiles->start(220); // 1 proyectil cada 0.7s
        return;
    }

    else if (nivelActual ==1){
        const QList<QPointF> posiciones = {
            {520, 750},
            {1220, 620},
            {2100, 640},
            {3000, 650}
        };

        for (const QPointF &pos : posiciones) {
            enemigos *centinela = new enemigos(this);
            centinela->setZValue(2);
            centinela->setPos(pos);
            centinela->configurarPatrulla(pos.x() - 80, pos.x() + 80, 1.2);

            // Alternar dirección inicial
            static bool alternar = false;

            if (alternar) {
                centinela->setDireccion(false);  // empezar mirando a la izquierda
                centinela->setVelocidadX(-1.2f);
            } else {
                centinela->setDireccion(true);   // empezar mirando a la derecha
                centinela->setVelocidadX(1.2f);
            }

            alternar = !alternar;
            centinelas.append(centinela);
            scene->addItem(centinela);
        }
    }
}

void niveles::actualizarProyectilesNivel2()
{
    QRectF boxJugador = player->posHitbox();

    if (tiempoNivel2 % 120 == 0) {     // cada 2 segundos
        velocidadBombas += 0.4;        // aumenta velocidad gradualmente
    }

    for (int i = proyectiles.size() - 1; i >= 0; i--) {
        Proyectil *p = proyectiles[i];
        p->setY(p->y() + velocidadBombas);

        // Si sale de la pantalla
        if (p->y() > 900) {
            scene->removeItem(p);
            proyectiles.removeAt(i);
            delete p;
            continue;
        }

        // Colisión con el jugador (barco)
        if (boxJugador.intersects(p->sceneBoundingRect())) {
            scene->removeItem(p);
            proyectiles.removeAt(i);
            delete p;

            player->perderVida();
            textoVidas->setText(QString("Vidas: %1").arg(player->vidas));

            if (player->vidas <= 0) {
                emit gameOver("muerte");
                return;
            }
        }
    }

    tiempoNivel2++;

    // 15 segundos → nivel superado
    if (tiempoNivel2 >= 20 * 60) {
        QMessageBox::information(this, "¡Nivel Completado!", "Has esquivado todos los disparos.");
        nivel2Completado = true;
        emit gameOver("ganar");
        return;
    }
}
