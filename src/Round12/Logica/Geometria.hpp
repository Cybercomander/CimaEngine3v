#pragma once
namespace Round12
{
    struct Rect { float x, y, w, h; };
    // punto en coordenadas del escenario (1920x1080); y = profundidad de los pies
    struct Pt
    {
        float x, y;
        bool operator==(const Pt& o) const { return x == o.x && y == o.y; }
    };
}
