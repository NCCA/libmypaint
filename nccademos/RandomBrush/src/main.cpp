#include <stdio.h>
#include "mypaint-brush.h"
#include "mypaint-fixed-tiled-surface.h"
#include <vector>
#include <filesystem>
#include <string>
#include <string_view>
#include <iostream>
#include <fstream>
#include <random>
#include <OpenImageIO/imageio.h>

namespace fs = std::filesystem;
std::random_device rd;  // Will be used to obtain a seed for the random number engine
std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()

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
    std::uniform_real_distribution<> br(0.0f, 360.0f);
    std::uniform_real_distribution<> press(0.0f, 10.0f);
    std::uniform_real_distribution<> tilt(-1.0f, 1.0f);

    float viewzoom = 1.0;
    float viewrotation = 0.0;
    float barrel_rotation = br(gen);
    float pressure = press(gen);
    float ytilt = tilt(gen);
    float xtilt = tilt(gen); 
    float dtime = 1.0/10;
    gboolean linear = FALSE;
    mypaint_brush_stroke_to
      (brush, surf, x, y, pressure, xtilt, ytilt, dtime, viewzoom, viewrotation, barrel_rotation, linear);
}

void move_to(MyPaintBrush *brush, MyPaintSurface *surf, float x, float y)
{
    float viewzoom = 1.0;
    float viewrotation = 0.0;
    float barrel_rotation = 0;
    float pressure = 0;
    float ytilt = 0; //tilt(gen);
    float xtilt = 0;// tilt(gen); 
    float dtime = 1.0/10;
    gboolean linear = TRUE;
    mypaint_brush_stroke_to
      (brush, surf, x, y, pressure, xtilt, ytilt, dtime, viewzoom, viewrotation, barrel_rotation, linear);
    mypaint_brush_stroke_to
      (brush, surf, x, y, 2.0f, xtilt, ytilt, dtime, viewzoom, viewrotation, barrel_rotation, linear);

}


std::vector<guint16> iterate_over_surface(MyPaintTiledSurface * tiled_surface, int height, int width)
{
    std::vector<guint16> data;
    const int tile_size = MYPAINT_TILE_SIZE;
    const int number_of_tile_rows = (height / tile_size) + 1*(height % tile_size != 0);
    const int tiles_per_row = (width / tile_size) + 1*(width % tile_size != 0);

    MyPaintTileRequest *requests = (MyPaintTileRequest *)malloc(tiles_per_row * sizeof(MyPaintTileRequest));

    for (int ty = 0; ty < number_of_tile_rows; ty++)
    {
        // Fetch all horizontal tiles in current tile row
        for (int tx = 0; tx < tiles_per_row; tx++ )
        {
            MyPaintTileRequest *req = &requests[tx];
            mypaint_tile_request_init(req, 0, tx, ty, TRUE);
            mypaint_tiled_surface_tile_request_start(tiled_surface, req);
        }

        // For each pixel line in the current tile row, fire callback
        const int max_y = (ty < number_of_tile_rows - 1 || height % tile_size == 0) ? tile_size : height % tile_size;
        for (int y = 0; y < max_y; y++)
        {
            for (int tx = 0; tx < tiles_per_row; tx++)
            {
                const int y_offset = y * tile_size * 4; // 4 channels
                const int chunk_length = (tx < tiles_per_row - 1 || width % tile_size == 0) ? tile_size : width % tile_size;
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
            mypaint_tiled_surface_tile_request_end(tiled_surface, &requests[tx]);
        }

    }

    free(requests);
    return data;
}

bool writeFile(std::string_view filename, MyPaintFixedTiledSurface *surface)
{
    // write_ppm(surface, filename);
    using namespace OIIO;
    std::unique_ptr<ImageOutput> out = ImageOutput::create (filename.data());
    auto w=mypaint_fixed_tiled_surface_get_width(surface);
    auto h= mypaint_fixed_tiled_surface_get_width(surface);
    ImageSpec spec (w,h,4, TypeDesc::UINT16);
    auto success=out->open(filename.data(),spec);
    auto data=iterate_over_surface((MyPaintTiledSurface *)surface,w,h);
    std::cout<<"data size is "<<data.size()<<'\n';
    success=out->write_image(TypeDesc::UINT16, &data[0]);

    success=out->close();
    return success;
}


int main(int argc, char *argv[]) 
{

    auto brushes=loadbrushesfrompath("../../brushes/Dieterle");
   
    int w = 1024;
    int h = 1024;
    std::uniform_real_distribution<> colour(0.0f, 1.0f);
    std::uniform_real_distribution<> radius(0.1f, 5.0f);
    
    std::uniform_int_distribution<> width(0, w);
    std::uniform_int_distribution<> height(0, w);
    std::uniform_int_distribution<> stroke(0,450);
    std::uniform_int_distribution<> rbrush(0, brushes.size()-1);

    MyPaintRectangle roi;
    MyPaintRectangles rois;
    /* Create an instance of the simple and naive fixed_tile_surface to draw on. */
    MyPaintFixedTiledSurface *surface = mypaint_fixed_tiled_surface_new(w, h);

    mypaint_surface_begin_atomic((MyPaintSurface*)surface);

    int result;

    for(int i=0; i<200 ; ++ i)
    {
    int bi=rbrush(gen);
    MyPaintBrush *brush =    mypaint_brush_new();

    std::cout<<"using brush "<<bi<<'\n';
    result = mypaint_brush_from_string(brush, brushes[bi].c_str());
    std::cout<<"loaded \n";
    /* Create a brush with default settings for all parameters, then set its color to red. */
    // mypaint_brush_new_stroke(brush);

    mypaint_brush_set_base_value(brush, MYPAINT_BRUSH_SETTING_RADIUS_LOGARITHMIC, log(radius(gen)));

    /* Draw a rectangle on the surface using the brush */
    std::cout<<"start stroke\n";
    mypaint_brush_set_base_value(brush, MYPAINT_BRUSH_SETTING_COLOR_H, colour(gen));
    mypaint_brush_set_base_value(brush, MYPAINT_BRUSH_SETTING_COLOR_S, colour(gen));
    mypaint_brush_set_base_value(brush, MYPAINT_BRUSH_SETTING_COLOR_V, colour(gen));
    move_to(brush, (MyPaintSurface*)surface, width(gen), height(gen));
    std::cout<<"move to\n";
    // stroke_to(brush, (MyPaintSurface*)surface, stroke(gen), stroke(gen));
    // stroke_to(brush, (MyPaintSurface*)surface, stroke(gen), stroke(gen));
    stroke_to(brush, (MyPaintSurface*)surface, width(gen), height(gen));

    std::cout<<"end stroke\n";
    std::cout<<"iteration "<<i<<'\n';
        mypaint_brush_unref(brush);
//    char filename[100];
//    snprintf(filename, 100,"test/output_%d.png", i);
//    writeFile(filename,surface);
  }
    writeFile("output.png",surface);
    std::cout<<"done, cleaning up\n";
    rois.num_rectangles = 1;
    rois.rectangles = &roi;
    mypaint_surface_end_atomic((MyPaintSurface *)surface, &rois);

    /* Finalize the surface operation, passing one or more invalidation
       rectangles to get information about which areas were affected by
       the operations between ``surface_begin_atomic`` and ``surface_end_atomic.``
    */
    /* Write the surface pixels to a ppm image file */
   // fprintf(stdout, "Writing output\n");
   // write_ppm(surface, "output.ppm");
    mypaint_surface_unref((MyPaintSurface *)surface);
}
