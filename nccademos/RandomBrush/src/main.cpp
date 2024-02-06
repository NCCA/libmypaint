#include <stdio.h>
#include "writeppm.h"
#include "mypaint-brush.h"  
#include "mypaint-fixed-tiled-surface.h"
#include <vector>
#include <filesystem>
#include <string>
#include <string_view>
#include <iostream>
#include <fstream>
#include <random>

namespace fs = std::filesystem;

std::vector<std::string> loadbrushesfrompath(std::string_view _path)
{
    std::vector<std::string> brushes;
    for (auto const& dir_entry : fs::directory_iterator{_path}) 
    {
        auto path=dir_entry.path();
        if(path.extension() == ".myb")
        {
          std::ifstream brushFile(path.c_str());        
          auto json = std::string((std::istreambuf_iterator< char >(brushFile)), std::istreambuf_iterator< char >());
          brushes.push_back(json);
        }
    }    
    return brushes;

}


void stroke_to(MyPaintBrush *brush, MyPaintSurface *surf, float x, float y)
{
    float viewzoom = 1.0, viewrotation = 0.0, barrel_rotation = 0.0;
    float pressure = 1.0, ytilt = 0.0, xtilt = 0.0, dtime = 1.0/10;
    gboolean linear = FALSE;
    mypaint_brush_stroke_to
      (brush, surf, x, y, pressure, xtilt, ytilt, dtime, viewzoom, viewrotation, barrel_rotation, linear);
}

int main(int argc, char *argv[]) 
{



    auto brushes=loadbrushesfrompath("../../brushes/classic");
    std::random_device rd;  // Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
   
    MyPaintBrush *brush =    mypaint_brush_new();
    int w = 1024;
    int h = 720;
    std::uniform_real_distribution<> colour(0.0f, 1.0f);
    std::uniform_int_distribution<> width(0, w);
    std::uniform_int_distribution<> height(0, w);
    std::uniform_int_distribution<> rbrush(0, brushes.size()-1);
    

    float wq = w/5;
    float hq= h/5;
    /* Create an instance of the simple and naive fixed_tile_surface to draw on. */
    MyPaintFixedTiledSurface *surface = mypaint_fixed_tiled_surface_new(w, h);
    mypaint_surface_begin_atomic((MyPaintSurface*)surface);
    int result;
    for(int i=0; i<100; ++ i)
    {
    result = mypaint_brush_from_string(brush, brushes[rbrush(gen)].c_str());

    /* Create a brush with default settings for all parameters, then set its color to red. */
    mypaint_brush_set_base_value(brush, MYPAINT_BRUSH_SETTING_COLOR_H, colour(gen));
    mypaint_brush_set_base_value(brush, MYPAINT_BRUSH_SETTING_COLOR_S, colour(gen));
    mypaint_brush_set_base_value(brush, MYPAINT_BRUSH_SETTING_COLOR_V, colour(gen));

    /* Draw a rectangle on the surface using the brush */
    stroke_to(brush, (MyPaintSurface*)surface, width(gen), height(gen));
    // stroke_to(brush, (MyPaintSurface*)surface, 4 * wq, hq);
    // stroke_to(brush, (MyPaintSurface*)surface, 4 * wq, 4 * hq);
    // stroke_to(brush, (MyPaintSurface*)surface, wq, 4 * hq);
    // stroke_to(brush, (MyPaintSurface*)surface, wq, hq);
    std::cout<<"iteration "<<i<<'\n';
    }

    /* Finalize the surface operation, passing one or more invalidation
       rectangles to get information about which areas were affected by
       the operations between ``surface_begin_atomic`` and ``surface_end_atomic.``
    */
    MyPaintRectangle roi;
    MyPaintRectangles rois;
    rois.num_rectangles = 1;
    rois.rectangles = &roi;
    mypaint_surface_end_atomic((MyPaintSurface *)surface, &rois);
    /* Write the surface pixels to a ppm image file */
    fprintf(stdout, "Writing output\n");
    write_ppm(surface, "output.ppm");

    mypaint_brush_unref(brush);
    mypaint_surface_unref((MyPaintSurface *)surface);
}
