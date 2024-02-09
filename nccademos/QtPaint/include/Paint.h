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
    void brushDown(float _x, float _y);
    void brushStroke(float _x, float _y);
    void brushUp();
    void writeImage(std::string_view _fname);
    void changeBrush(int index);
    void setRadius(float _r){m_brushRadius=_r;}
private :
    std::vector<guint16> iterate_over_surface();

    std::vector<std::string> m_brushes;
    MyPaintBrush *m_brush;
    MyPaintRectangle m_roi;
    MyPaintRectangles m_rois;
    // raw pointer as using C api below!
    MyPaintFixedTiledSurface *m_surface ;
    int m_width=0;
    int m_height=0;
    int m_activeBrush=0;
    float m_brushRadius=1.0f;
};


#endif