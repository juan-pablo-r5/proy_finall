#ifndef PROYECTIL_H
#define PROYECTIL_H

#include "entidad.h"
#include <QGraphicsPixmapItem>

class Proyectil : public QGraphicsPixmapItem, public Entidad {
public:
    Proyectil(const QPixmap &pix)
        : QGraphicsPixmapItem(pix), Entidad(TipoEntidad::Proyectil)
    {}

    // Movimiento simple vertical
    void moverBase() {
        setY(y() + velY);
    }

    // Método polimórfico que Qt llama cada frame
    void actualizar() override {
        moverBase();
    }

    ~Proyectil() override {}
};

#endif
