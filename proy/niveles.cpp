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
