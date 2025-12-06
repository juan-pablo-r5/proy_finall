#pragma once
#include <QtMath>

enum TipoColision {
    ELASTICA,
    INELASTICA,
    PERFECTAMENTE_INELASTICA
};

struct Vector2D {
    float x, y;
};

struct Cuerpo {
    float masa;
    Vector2D vel;
};

inline void resolverColision(Cuerpo &A, Cuerpo &B, TipoColision tipo, float e = 0.6f)
{
    float nX = 1, nY = 0;

    float vA = A.vel.x * nX + A.vel.y * nY;
    float vB = B.vel.x * nX + B.vel.y * nY;

    float vA2, vB2;

    if (tipo == PERFECTAMENTE_INELASTICA) {
        float vf = (A.masa*vA + B.masa*vB) / (A.masa + B.masa);
        vA2 = vB2 = vf;
    }
    else if (tipo == INELASTICA) {
        vA2 = (A.masa*vA + B.masa*vB + B.masa*e*(vB - vA)) / (A.masa + B.masa);
        vB2 = (A.masa*vA + B.masa*vB + A.masa*e*(vA - vB)) / (A.masa + B.masa);
    }
    else { // ELASTICA
        vA2 = ((A.masa - B.masa) * vA + 2*B.masa*vB) / (A.masa + B.masa);
        vB2 = ((B.masa - A.masa) * vB + 2*A.masa*vA) / (A.masa + B.masa);
    }

    A.vel.x = vA2 * nX;
    B.vel.x = vB2 * nX;
}
