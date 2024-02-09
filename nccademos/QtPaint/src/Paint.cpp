#include "Paint.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <OpenImageIO/imageio.h>

namespace fs = std::filesystem;

Paint::Paint(int _w, int _h) : m_width{_w},m_height{_h}
{
m_surface= mypaint_fixed_tiled_surface_new(_w, _h);
m_brush =    mypaint_brush_new();
loadBrushes("../../brushes/classic");
std::cout<<"loaded "<<m_brushes.size()<<" brushes \n";
mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_RADIUS_LOGARITHMIC, logf(m_brushRadius));
mypaint_brush_from_string(m_brush, m_brushes[m_activeBrush].c_str());
//mypaint_brush_from_defaults(m_brush);

mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_COLOR_H, 0.37f);
mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_COLOR_S, 0.56f);
mypaint_brush_set_base_value(m_brush, MYPAINT_BRUSH_SETTING_COLOR_V, 0.97f);

}
void Paint::changeBrush(int index)
{
    m_activeBrush=index;
    std::cout<<"new brush" <<m_activeBrush<<" "<< m_brushes[m_activeBrush]<<"\n";
    mypaint_brush_from_string(m_brush, m_brushes[m_activeBrush].c_str());

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

void Paint::brushDown(float _x, float _y)
{
    lockSurface();
    float viewzoom = 1.0;
    float viewrotation = 0.0;
    float barrel_rotation = 0;
    float pressure = 0;
    float ytilt = 0; //tilt(gen);
    float xtilt = 0;// tilt(gen);
    float dtime = 1.0/10;
    gboolean linear = TRUE;
    mypaint_brush_stroke_to
            (m_brush,(MyPaintSurface *) m_surface, _x, _y, pressure, xtilt, ytilt, dtime, viewzoom, viewrotation, barrel_rotation, linear);
    mypaint_brush_stroke_to
            (m_brush, (MyPaintSurface *) m_surface, _x, _y, 2.0f, xtilt, ytilt, dtime, viewzoom, viewrotation, barrel_rotation, linear);

}
void Paint::brushStroke(float _x, float _y)
{
    float br=0.0f;
    float press=4.0f;
    float tilt=0.0f;
    float viewzoom = 1.0;
    float viewrotation = 0.0;
    float barrel_rotation = 0;
    float pressure = 4.0f;
    float ytilt = 0;
    float xtilt = 0;
    float dtime = 1.0/10;
    gboolean linear = FALSE;
    mypaint_brush_stroke_to
            (m_brush,(MyPaintSurface *) m_surface, _x, _y, pressure, xtilt, ytilt, dtime, viewzoom, viewrotation, barrel_rotation, linear);
}
void Paint::brushUp()
{
    unlockSurface();
}


std::vector<guint16> Paint::iterate_over_surface()
{
    std::vector<guint16> data;
    const int tile_size = MYPAINT_TILE_SIZE;
    const int number_of_tile_rows = (m_height / tile_size) + 1*(m_height % tile_size != 0);
    const int tiles_per_row = (m_width / tile_size) + 1*(m_width % tile_size != 0);

    MyPaintTileRequest *requests = (MyPaintTileRequest *)malloc(tiles_per_row * sizeof(MyPaintTileRequest));

    for (int ty = 0; ty < number_of_tile_rows; ty++)
    {
        // Fetch all horizontal tiles in current tile row
        for (int tx = 0; tx < tiles_per_row; tx++ )
        {
            MyPaintTileRequest *req = &requests[tx];
            mypaint_tile_request_init(req, 0, tx, ty, TRUE);
            mypaint_tiled_surface_tile_request_start((MyPaintTiledSurface *)m_surface, req);
        }

        // For each pixel line in the current tile row, fire callback
        const int max_y = (ty < number_of_tile_rows - 1 || m_height % tile_size == 0) ? tile_size : m_height % tile_size;
        for (int y = 0; y < max_y; y++)
        {
            for (int tx = 0; tx < tiles_per_row; tx++)
            {
                const int y_offset = y * tile_size * 4; // 4 channels
                const int chunk_length = (tx < tiles_per_row - 1 || m_width % tile_size == 0) ? tile_size : m_width % tile_size;
                //callback(requests[tx].buffer + y_offset, chunk_length, user_data);
                auto chunk = requests[tx].buffer + y_offset;
                for (int px = 0; px < chunk_length; px++)
                {
                    data.push_back(chunk[px*4]);
                    data.push_back(chunk[px*4+1]);
                    data.push_back(chunk[px*4+2]);
                    data.push_back(chunk[px*4+3]);
                }

            }
        }

        // Complete tile requests on current tile row
        for (int tx = 0; tx > tiles_per_row; tx++ )
        {
            mypaint_tiled_surface_tile_request_end((MyPaintTiledSurface *)m_surface, &requests[tx]);
        }

    }

    free(requests);
    return data;
}


void Paint::writeImage(std::string_view _fname)
{
    using namespace OIIO;
    std::unique_ptr<ImageOutput> out = ImageOutput::create (_fname.data());
    auto w=mypaint_fixed_tiled_surface_get_width(m_surface);
    auto h= mypaint_fixed_tiled_surface_get_width(m_surface);
    ImageSpec spec (m_width,m_height,4, TypeDesc::UINT16);
    auto success=out->open(_fname.data(),spec);
    auto data=iterate_over_surface();
    std::cout<<"data size is "<<data.size()<<'\n';
    success=out->write_image(TypeDesc::UINT16, &data[0]);

    success=out->close();

}
