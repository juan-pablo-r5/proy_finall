#ifndef ENTIDAD_H
#define ENTIDAD_H

enum class TipoEntidad {
    Ninguno,
    Jugador,
    Enemigo,
    Proyectil
};

class Entidad {
public:
    int vida;
    float velX;
    float velY;
    TipoEntidad tipo;

    explicit Entidad(TipoEntidad t = TipoEntidad::Ninguno)
        : vida(1), velX(0), velY(0), tipo(t)
    {}

    virtual void recibirDaño(int dmg) {
        vida -= dmg;
        if (vida < 0) vida = 0;
    }

    virtual bool estaVivo() const {
        return vida > 0;
    }

    // --- Movimiento genérico ---
    virtual void moverBase() {}

    // --- Método polimórfico obligatorio ---
    virtual void actualizar() {}

    virtual ~Entidad() {}
};

#endif
