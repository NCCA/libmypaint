#ifndef PAINT_H_
#define PAINT_H_
#include "mypaint-brush.h"
#include "mypaint-fixed-tiled-surface.h"
#include <vector>
#include <string>
#include <string_view>
class Paint
{
public :
    Paint()=delete;
    Paint(int _w, int _h);
    ~Paint();
    void lockSurface();
    void unlockSurface();
    void loadBrushes(std::string_view _directory);
private :
    std::vector<std::string> m_brushes;
    MyPaintRectangle m_roi;
    MyPaintRectangles m_rois;
    // raw pointer as using C api below!
    MyPaintFixedTiledSurface *m_surface ;
    int m_width=0;
    int m_height=0;
};


#endif