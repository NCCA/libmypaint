#include "Paint.h"
#include <filesystem>
#include <iostream>
#include <fstream>

namespace fs = std::filesystem;

Paint::Paint(int _w, int _h) : m_width{_w},m_height{_h}
{
m_surface= mypaint_fixed_tiled_surface_new(_w, _h);

}

Paint::~Paint()
{
    mypaint_surface_unref((MyPaintSurface *)m_surface);
}

void Paint::loadBrushes(std::string_view _directory)
{
    for (auto const& dir_entry : fs::directory_iterator{_directory})
    {
        auto path=dir_entry.path();
        if(path.extension() == ".myb")
        {
            std::ifstream brushFile(path.c_str());
            auto json = std::string((std::istreambuf_iterator< char >(brushFile)), std::istreambuf_iterator< char >());
            m_brushes.push_back(json);
        }
    }
}

void Paint::lockSurface()
{
    mypaint_surface_begin_atomic((MyPaintSurface*)m_surface);

}

void Paint::unlockSurface()
{
    m_rois.num_rectangles = 1;
    m_rois.rectangles = &m_roi;
    mypaint_surface_end_atomic((MyPaintSurface *)m_surface, &m_rois);
}