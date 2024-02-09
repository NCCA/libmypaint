#include "NGLScene.h"
#include <iostream>
#include <ngl/NGLInit.h>
#include <ngl/ShaderLib.h>

constexpr auto ScreenTri = "ScreenTri";
constexpr size_t TextureWidth=1024;
constexpr size_t TextureHeight=720;


//----------------------------------------------------------------------------------------------------------------------
NGLScene::NGLScene( QWidget *_parent ) : QOpenGLWidget( _parent )
{

  // set this widget to have the initial keyboard focus
  setFocus();
  // re-size the widget to that of the parent (in this case the GLFrame passed in on construction)
  resize(_parent->size());
  m_paint = std::make_unique<Paint>(1024,720);
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::initializeGL()
{

  ngl::NGLInit::initialize();
  glClearColor(0.4f, 0.4f, 0.4f, 1.0f); // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  // enable multisampling for smoother drawing
  glEnable(GL_MULTISAMPLE);
  ngl::ShaderLib::loadShader(ScreenTri,"shaders/ScreenTriVertex.glsl","shaders/ScreenTriFragment.glsl");
  // Need a vertex array to call draw arrays
  // this will have no buffers
  glGenVertexArrays(1,&m_vao);
  // Now generate a texture
  glGenTextures(1, &m_textureID);
  // Generate our buffer for the texture data
  m_buffer= std::make_unique<unsigned short []>(TextureWidth*TextureHeight);
  clearBuffer();
  updateTextureBuffer();
  startTimer(10);
}

//----------------------------------------------------------------------------------------------------------------------
//This virtual function is called whenever the widget has been resized.
// The new size is passed in width and height.
void NGLScene::resizeGL( int _w, int _h )
{
  m_win.width  = static_cast<int>( _w * devicePixelRatio() );
  m_win.height = static_cast<int>( _h * devicePixelRatio() );
}



void NGLScene::paintGL()
{
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // grab an instance of the shader manager
  ngl::ShaderLib::use(ScreenTri);
  // Draw screen Tri with bound Texture
  glBindVertexArray(m_vao);
  glBindTexture(GL_TEXTURE_2D,m_textureID);
  glDrawArrays(GL_TRIANGLES,0,3);
  glBindVertexArray(0);

}


NGLScene::~NGLScene()
{
}

void NGLScene::writeImage()
{
    m_paint->writeImage("test.png");
    
}

void NGLScene::brushIndexUp()
{
    m_brushIndex+=1;
    std::cout<<"Changing brush up "<<m_brushIndex<<'\n';
    m_paint->changeBrush(m_brushIndex);
}
void NGLScene::brushIndexDown()
{
    m_brushIndex-=1;
    std::cout<<"Changing brush down "<<m_brushIndex<<'\n';
    m_paint->changeBrush(m_brushIndex);

}


void NGLScene::timerEvent(QTimerEvent *)
{
updateTextureBuffer();
update();
}

void NGLScene::clearBuffer()
{
  // clear screen to white
  for(size_t i=0; i<TextureWidth*TextureHeight; ++i)
  {
    m_buffer[i]=0; //encodePixel(255,255,255);
  }
}

void NGLScene::updateTextureBuffer()
{
  auto buffer=m_paint->iterate_over_surface();
  glBindTexture(GL_TEXTURE_2D, m_textureID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TextureWidth, TextureHeight, 0, GL_RGBA, GL_UNSIGNED_SHORT, &buffer[0]);
  glGenerateMipmap(GL_TEXTURE_2D); //  Allocate the mipmaps
}
