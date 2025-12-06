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

    }

    else if (nivelActual == 3) {

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
