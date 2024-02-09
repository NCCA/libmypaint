#include "NGLScene.h"
#include <iostream>
#include <ngl/NGLInit.h>


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
  glClearColor(0.4f, 0.4f, 0.4f, 1.0f);			   // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
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
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0,0,m_win.width,m_win.height);
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
