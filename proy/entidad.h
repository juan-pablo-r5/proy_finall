#ifndef ENTIDAD_H
#define ENTIDAD_H

class Entidad {
public:
    int vida = 1;

    virtual void recibirDaño(int dmg) {
        vida -= dmg;
        if (vida < 0) vida = 0;
    }

    virtual bool estaVivo() const {
        return vida > 0;
    }

    virtual ~Entidad() {}
};

#endif
