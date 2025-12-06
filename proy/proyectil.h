#ifndef PROYECTIL_H
#define PROYECTIL_H

#include <QGraphicsPixmapItem>

class Proyectil : public QGraphicsPixmapItem
{
public:
    float velY = 10;

    Proyectil(QPixmap sprite) {
        setPixmap(sprite);
    }

    void actualizar() {
        setY(y() + velY);
    }
};

#endif
