#ifndef _SIMPLE_APP
#define _SIMPLE_APP

#include <vjConfig.h>

#include <iostream.h>
#include <iomanip.h>

#include <Kernel/GL/vjGlApp.h>
#include <Kernel/GL/vjGlContextData.h>
#include <Math/vjMatrix.h>

#include <Input/InputManager/vjPosInterface.h>
#include <Input/InputManager/vjAnalogInterface.h>
#include <Input/InputManager/vjDigitalInterface.h>

// Utility function to draw a cube
void drawbox( GLdouble x0, GLdouble x1, GLdouble y0, GLdouble y1,
              GLdouble z0, GLdouble z1, GLenum type );

//----------------------------------------------------
//: Simple OGL Demonstration application
//
// This application intialized and recieves positional
// and digital intput from the wand.
//
// It also has basic code that draws a box centered on
// the origin
//----------------------------------------------------
class contextApp : public vjGlApp
{
public:
   contextApp()
   {;}

   virtual ~contextApp (void) {;}

public: // ---- INITIALIZATION FUNCITONS ---- //
   // Execute any initialization needed before the API is started
   //
   // POST: Device interfaces are initialized with the device names
   //       we want to use
   //! NOTE: This is called once before OpenGL is initialized
   virtual void init()
   {
      cout << "---------- App:init() ---------------" << endl;
      // Initialize devices
      mWand.init("VJWand");
      mHead.init("VJHead");
      mButton0.init("VJButton0");
      mButton1.init("VJButton1");
   }

   // Execute any initialization needed after API is started
   //  but before the drawManager starts the drawing loops.<BR><BR>
   //
   // This is called once after OGL is initialized
   virtual void apiInit()
   {;}

public:
   // ----- Drawing Loop Functions ------
   //
   //  The drawing loop will look similar to this:
   //
   //  while (drawing)
   //  {
   //        preFrame();
   //       draw();
   //        intraFrame();      // Drawing is happening while here
   //       sync();
   //        postFrame();      // Drawing is now done
   //
   //       UpdateDevices();
   //  }
   //------------------------------------

   //: Called before start of frame
   //!NOTE: Function called after device updates but before start of drawing
   virtual void preFrame()
   {
      if(mButton0->getData())
         { cout << "Button 0 pressed" << endl; }
      if(mButton1->getData())
         { cout << "Button 1 pressed" << endl; }
   }

   //: Called during the Frame
   //!NOTE: Function called after drawing has been triggered but BEFORE it completes
   virtual void intraFrame() {;}

   //: Called at end of frame
   //!NOTE: Function called before updating trackers but after the frame is drawn
   virtual void postFrame() {;}

public: // ----- OpenGL FUNCTIONS ---- //
   //: Function that is called immediately after a new OGL context is created
   // Initialize GL state here. Also used to create context specific information
   //
   // This is called once for each context
   virtual void contextInit();

   //: Function to draw the scene
   //! PRE: OpenGL state has correct transformation and buffer selected
   //! POST: The current scene has been drawn
   //
   //!NOTE: Called 1 or more times per frame
   virtual void draw();

private:
   void initGLState();


   // Draw a cube
   void drawCube()
   {
      drawbox(-0.5, 0.5, -0.5, 0.5, -0.5, 0.5, GL_QUADS);
   }

public:
   vjGlContextData<GLuint>  mCubeDlId;       // Id of the cube display list

   vjPosInterface    mWand;         // Positional interface for Wand position
   vjPosInterface    mHead;         // Positional interface for Head position
   vjDigitalInterface   mButton0;   // Digital interface for button 0
   vjDigitalInterface   mButton1;   // Digital interface for button 1v
};


#endif
