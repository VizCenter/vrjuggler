#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <unistd.h>

#include <gmtl/Matrix.h>
#include <gmtl/Vec.h>
#include <gmtl/Generate.h>

#include <jccl/RTRC/ConfigManager.h>
#include <gadget/InputManager.h>
#include <gadget/Devices/Sim/SimPosition.h>
#include <gadget/Devices/Sim/SimDigital.h>
#include <gadget/Devices/Sim/SimAnalog.h>
#include <gadget/Devices/Sim/SimInput.h>
#include <gadget/Devices/Keyboard/KeyboardXWin.h>
#include <gadget/Type/AnalogProxy.h>

#include <vrj/Display/DisplayManager.h>
#include <vrj/Util/Debug.h>
#include <reconfigApp.h>

void reconfigApp::init()
{

   mWand.init("VJWand") ;
   mHead.init("VJHead") ;
   mButton0.init("VJButton0") ;
   mButton1.init("VJButton1") ;
   mButton2.init("VJButton2") ;
   mButton3.init("VJButton3") ;
   mButton4.init("VJButton4") ;
   mButton5.init("VJButton5") ;

   mFrameCount = 0;
   mStartTime = 1000;
   mWaitTime = 100;
   mTestState = 0;
   mTesting = false;
   mFinished = false;

   mNewChunkDB = NULL;
}

void reconfigApp::contextInit()
{
   initGLState();
}

//Do some pre draw calculations
void reconfigApp::preFrame()
{
   if (mFinished) return;

   //Don't start tests until a certain frame number
   if (mStartTime > 1) { mStartTime-- ; return; }


   bool status = false;

   //If we are not currently doing any testing, start a new test state
   if (!mTesting) 
   {
      mTesting = true;

      //Switch on the test state
      switch (mTestState)
      {
         case  0:  status =       addGFXWindow_exec();         break;

         case  1:  status =       removeGFXWindow_exec();      break;

         case  2:  status =       readdGFXWindow_exec();       break;

         case  3:  status =       resizeGFXWindow_exec();      break;

         case  4:  status =       moveGFXWindow_exec();        break;

         case  5:  status =       addViewport_exec();          break;

         case  6:  status =       removeViewport_exec();       break;

         case  7:  status =       resizeViewport_exec();       break;

         case  8:  status =       moveViewport_exec();         break;

         case  9:  status =       enableStereoSurface_exec();  break;

         case 10:  status =       disableStereoSurface_exec(); break;

         case 11:  status =       removeKeyboardWin_exec();    break;

         case 12:  status =       readdKeyboardWin_exec();     break;

         case 13:  status =       addSimPos_exec();            break;

         case 14:  status =       removeSimPos_exec();         break;

         case 15:  status =       readdSimPos_exec();          break;

         case 16:  status =       repointProxy_exec();         break;

         case 17:  status =       reconfigSimPos_exec();       break;

         case 18:  status =       reconfigSimDigital_exec();   break;

         case 19:  status =       reconfigSimAnalog_exec();    break;

         case 20:  status =       addSimDigital_exec();        break;

         case 21:  status =       removeSimDigital_exec();     break;

         case 22:  status =       readdSimDigital_exec();      break;

         case 23:  status =       addSimAnalog_exec();         break;

         case 24:  status =       removeSimAnalog_exec();      break;

         case 25:  status =       readdSimAnalog_exec();       break;

         case 26:  status =       addPosProxy_exec();          break;

         case 27:  status =       reconfigPosProxy_exec();     break;

         case 28:  status =       removePosProxy_exec();       break;

         case 29:  status =       addAnalogProxy_exec();       break;

         case 30:  status =       reconfigAnalogProxy_exec();  break;

         case 31:  status =       removeAnalogProxy_exec();    break;

         case 32:  status =       addDigitalProxy_exec();      break;

         case 33:  status =       reconfigDigitalProxy_exec(); break;

         case 34:  status =       removeDigitalProxy_exec();   break;

         case 35:  status =       addKeyboardProxy_exec();     break;

         case 36:  status =       reconfigKeyboardProxy_exec();break;

         case 37:  status =       removeKeyboardProxy_exec();  break;
         
         case 38:  status =       addMachineSpecific_exec();  break;
         
         //case 39:  status =       reconfigMachineSpecific_exec();  break;
         
         case 39:  status =       removeMachineSpecific_exec();  break;
         
         case 40:  status =       addStupifiedAnalogProxy_exec();  break;
         
         case 41:  status =       removeStupifiedAnalogProxy_exec();  break;

         
         case 42:  status = true; 
                   mFinished = true; 
                   std::cout << "\n\n[Test battery completed]\n\n" << std::flush; 
                   mKernel->stop();
                   break;

         default: status = true; break;
      }

      if (!status)
      {
         std::cout << "WARNING: Problem executing test. Subsequent results may be inaccurate\n\n" << std::flush;
      }

   }

   //If we have waited enough time after executing a test...
   //Then we need to run the 
   if (checkTime())
   {

      switch ( mTestState )
      {
         case  0:  status =       addGFXWindow_check();         break;

         case  1:  status =       removeGFXWindow_check();      break;

         case  2:  status =       readdGFXWindow_check();       break;

         case  3:  status =       resizeGFXWindow_check();      break;

         case  4:  status =       moveGFXWindow_check();        break;

         case  5:  status =       addViewport_check();          break;

         case  6:  status =       removeViewport_check();       break;

         case  7:  status =       resizeViewport_check();       break;

         case  8:  status =       moveViewport_check();         break;

         case  9:  status =       enableStereoSurface_check();  break;

         case 10:  status =       disableStereoSurface_check(); break;

         case 11:  status =       removeKeyboardWin_check();    break;

         case 12:  status =       readdKeyboardWin_check();     break;

         case 13:  status =       addSimPos_check();            break;

         case 14:  status =       removeSimPos_check();         break;

         case 15:  status =       readdSimPos_check();          break;

         case 16:  status =       repointProxy_check();         break;

         case 17:  status =       reconfigSimPos_check();       break;

         case 18:  status =       reconfigSimDigital_check();   break;

         case 19:  status =       reconfigSimAnalog_check();    break;

         case 20:  status =       addSimDigital_check();        break;

         case 21:  status =       removeSimDigital_check();     break;

         case 22:  status =       readdSimDigital_check();      break;

         case 23:  status =       addSimAnalog_check();         break;

         case 24:  status =       removeSimAnalog_check();      break;

         case 25:  status =       readdSimAnalog_check();       break;

         case 26:  status =       addPosProxy_check();          break;

         case 27:  status =       reconfigPosProxy_check();     break;

         case 28:  status =       removePosProxy_check();       break;

         case 29:  status =       addAnalogProxy_check();       break;

         case 30:  status =       reconfigAnalogProxy_check();  break;

         case 31:  status =       removeAnalogProxy_check();    break;

         case 32:  status =       addDigitalProxy_check();      break;

         case 33:  status =       reconfigDigitalProxy_check(); break;

         case 34:  status =       removeDigitalProxy_check();   break;

         case 35:  status =       addKeyboardProxy_check();     break;

         case 36:  status =       reconfigKeyboardProxy_check();break;

         case 37:  status =       removeKeyboardProxy_check();  break;
      
         case 38:  status =       addMachineSpecific_check();   break;

         //case 39:  status =       reconfigMachineSpecific_check();break;

         case 39:  status =       removeMachineSpecific_check();break;
         
         case 40:  status =       addStupifiedAnalogProxy_check();break;
         
         case 41:  status =       removeStupifiedAnalogProxy_check();break;
         
         default: status = true; break;

      }

      mTestState++;      //Next test state
      mTesting = false;  //No longer in the middle of a test

      if (status)
         std::cout << "[TEST PASSED]\n\n" << std::flush;
      else
         std::cout << "[ *** TEST FAILED *** ]\n\n" << std::flush;

   }

}

void reconfigApp::postFrame()
{

}

void reconfigApp::bufferPreDraw()
{
   glClearColor(0.0, 0.0, 0.0, 0.0);
   glClear(GL_COLOR_BUFFER_BIT);
}

void reconfigApp::draw()
{
   glClear(GL_DEPTH_BUFFER_BIT);

   initGLState();

   glColor3f( 0.3, 0.05, 0.6 );
   glBegin( GL_POLYGON );
      glVertex3f( 5.0, 0.0, -5.0 );
      glVertex3f( -5.0, 0.0, -5.0 );
      glVertex3f( 0.0, 10.0, -5.0 );
   glEnd();
}

void reconfigApp::initGLState()
{
   GLfloat light0_ambient[] = { .2,  .2,  .2,  1.0};
   GLfloat light0_diffuse[] = { 0.8,  0.8,  0.8,  1.0};
   GLfloat light0_specular[] = { 1.0,  1.0,  1.0,  1.0};
   GLfloat light0_position[] = {0.0, 10,0, 0.0, 1.0};

   glLightfv(GL_LIGHT0, GL_AMBIENT,  light0_ambient);
   glLightfv(GL_LIGHT0, GL_DIFFUSE,  light0_diffuse);
   glLightfv(GL_LIGHT0, GL_SPECULAR,  light0_specular);
   glLightfv(GL_LIGHT0, GL_POSITION,  light0_position);

   glEnable(GL_DEPTH_TEST);
   glEnable(GL_NORMALIZE);
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glEnable(GL_COLOR_MATERIAL);
   glShadeModel(GL_SMOOTH);

   glMatrixMode(GL_MODELVIEW);
}



/********************************************************************
UTILITY FUNCTIONS
********************************************************************/

void reconfigApp::setPath( std::string path )
{
   mPath = path;
}


bool reconfigApp::addChunkFile( std::string filename )
{
   //Attempt to load a new config chunk
   if (mNewChunkDB != NULL) delete mNewChunkDB;

   mNewChunkDB = new jccl::ConfigChunkDB;

   if (mNewChunkDB->load( std::string( filename )))
   {
      jccl::ConfigManager::instance()->addPendingAdds( mNewChunkDB );
      return true;
   }
   else
   {
      std::cout << "\tError: Failed to load config file: " << filename << std::endl << std::flush;
      delete mNewChunkDB;  
      mNewChunkDB = NULL;
      return false;
   }
}

bool reconfigApp::removeChunkFile( std::string filename )
{
   //Attempt to load a new config chunk
   if (mNewChunkDB != NULL) delete mNewChunkDB;

   mNewChunkDB = new jccl::ConfigChunkDB;

   if (mNewChunkDB->load( std::string( filename )))
   {
      jccl::ConfigManager::instance()->addPendingRemoves( mNewChunkDB );
      return true;
   }
   else
   {
      std::cout << "\tError: Failed to load config file: " << filename << std::endl << std::flush;
      delete mNewChunkDB;  
      mNewChunkDB = NULL;
      return false;
   }
}

bool reconfigApp::swapChunkFiles( std::string remove_file, std::string add_file )
{
   if (!removeChunkFile( remove_file ))
   {
      std::cout << "\tError: Could not remove config file " << remove_file << "\n" << std::flush;
      return false;
   }

   if (!addChunkFile( add_file ))
   {
      std::cout << "\tError: Could not add config file " << add_file << "\n" << std::flush;
      return false;
   }

   return true;

}

bool reconfigApp::removeRecentChunkDB()
{
   if (mNewChunkDB != NULL)
   {
      jccl::ConfigManager::instance()->addPendingRemoves( mNewChunkDB );
      return true;
   }
   else
   {
      std::cout << "\tError: Cannot remove a NULL chunk db\n" << std::flush;
      return false;
   }
}

bool reconfigApp::checkTime()
{
   if (mFrameCount >= mWaitTime)
   {
      mFrameCount = 0;
      return true;
   }
   else
   {
      mFrameCount++;
      return false;
   }
}

bool reconfigApp::verifyProxy( std::string proxyName, std::string deviceName )
{
   //Try to get the new proxy and check its values
   gadget::Proxy* proxy = gadget::InputManager::instance()->getProxy( proxyName );

   if (proxy == NULL)
   {
      std::cout << "\tError: Could not find the proxy\n" << std::flush;
      return false;
   }

   if (proxy->isStupified())
   {
      std::cout << "\tError: Proxy is stupified\n" << std::flush;
      return false;
   }

   gadget::Input* inputDevice = proxy->getProxiedInputDevice();

   if (inputDevice == NULL)
   {
      std::cout << "\tError: Proxy does not point at a valid device\n" << std::flush;
      return false;
   }

   if (inputDevice->getInstanceName() != deviceName )
   {
      std::cout << "\tError: Input device has wrong name: " << inputDevice->getInstanceName() << "   It should be " << deviceName << "\n" << std::flush;
      return false;
   }

   return true;

}

vrj::Display* reconfigApp::getDisplay( std::string name )
{
   //Search the display manager for the display by name
   std::vector<vrj::Display*>::iterator iter;
   std::vector<vrj::Display*> allDisplays = vrj::DisplayManager::instance()->getAllDisplays();

   for (iter = allDisplays.begin(); iter != allDisplays.end(); iter++)
   {
      if (name == (*iter)->getName())
      {
         return *iter;
      }
   }

   return NULL;
}

bool reconfigApp::verifyDisplayProps(  vrj::Display* disp,
                                       std::string name,
                                       int x_origin,
                                       int y_origin,
                                       int x_size,
                                       int y_size,
                                       int pipe_num,
                                       bool stereo,
                                       bool border,
                                       bool active  )
{
   bool ok = true;

   if (disp->getName() != name)
   {
      ok = false;
   }

   int xo, yo, xs, ys;   
   disp->getOriginAndSize(xo, yo, xs, ys);

   if (xo != x_origin)
   {
      ok = false;      
   }

   if (yo != y_origin)
   {
      ok = false;      
   }

   if (xs != x_size)
   {
      ok = false;      
   }

   if (ys != y_size)
   {
      ok = false;      
   }

   if (disp->getPipe() != pipe_num)
   {
      ok = false;      
   }

   if (disp->inStereo() != stereo)
   {
      ok = false;        
   }

   if (disp->shouldDrawBorder() != border)
   {
      ok = false;        
   }

   if (disp->isActive() != active)
   {
      ok = false;        
   }

   return ok;
}


bool reconfigApp::verifyAllViewports( vrj::Display* display, jccl::ConfigChunkPtr viewportChunk )
{
   //Check to see if this viewport chunk matches with ANY of the display's viewports
   bool status = false;
   for (int i=0; i < display->getNumViewports(); i++)
   {
      vrj::Viewport* viewport = display->getViewport( i );
      if (verifyViewport( viewport, viewportChunk ))
      {
         status = true;
      }
   }

   return status;
}

bool reconfigApp::verifyViewport( vrj::Viewport* viewport, jccl::ConfigChunkPtr viewportChunk )
{
   //Verify that the display object has the specified viewport

   //First, get the properties from the config chunk
   std::string viewportName = viewportChunk->getName();

   //Get origin values
   if (viewportChunk->getNum( "origin" ) < 2 )
   {
      std::cout << "\tError: viewport chunk named " << viewportName << " doesn't have origin properties\n" << std::flush;
      return false;
   }
   float x_origin = viewportChunk->getProperty<float>("origin", 0);
   float y_origin = viewportChunk->getProperty<float>("origin", 1);

   //Get size values
   if (viewportChunk->getNum( "size" ) < 2 )
   {
      std::cout << "\tError: viewport chunk named " << viewportName << " doesn't have size properties\n" << std::flush;
      return false;
   }
   float x_size = viewportChunk->getProperty<float>("size", 0);
   float y_size = viewportChunk->getProperty<float>("size", 1);

   //TEST size and origin
   float xo, xs, yo, ys;
   viewport->getOriginAndSize( xo, yo, xs, ys );
   if ((x_size != xs) || (y_size != ys) || (x_origin != xo) || (y_origin != yo))
   {
      std::cout << "\tError: viewport has incorrect size and/or origin\n" << std::flush;
      return false;
   }

   //Get view property
   if (viewportChunk->getNum( "view" ) < 1 )
   {
      std::cout << "\tError: viewport chunk named " << viewportName << " doesn't have view properties\n" << std::flush;
      return false;
   }
   int view_num = viewportChunk->getProperty<int>("view", 0);

   //TEST view prop
   if (view_num != viewport->getView())
   {
      std::cout << "\tError: viewport has incorrect view property\n" << std::flush;
      return false;
   }


   //Get cameraPos property
   if (viewportChunk->getNum( "cameraPos" ) < 1 )
   {
      std::cout << "\tError: viewport chunk named " << viewportName << " doesn't have cameraPos properties\n" << std::flush;
      return false;
   }
   std::string cameraPos = viewportChunk->getProperty<std::string>("cameraPos", 0);

   //TEST cameraPos property
   //There is no camera position property for the vrj::Viewport object

   //Get user property
   if (viewportChunk->getNum( "user" ) < 1 )
   {
      std::cout << "\tError: viewport chunk named " << viewportName << " doesn't have user properties\n" << std::flush;
      return false;
   }
   std::string username = viewportChunk->getProperty<std::string>("user", 0);
   if (viewport->getUser()->getName() != username)
   {
      std::cout << "\tError: viewport has incorrect user name\n" << std::flush;
      return false;
   }

   //Get wandPos
   if (viewportChunk->getNum( "wandPos" ) < 1 )
   {
      std::cout << "\tError: viewport chunk named " << viewportName << " doesn't have wandPos properties\n" << std::flush;
      return false;
   }
   std::string wandPos = viewportChunk->getProperty<std::string>("wandPos", 0);
   //TEST wandPos property
   //There is no wand position property for the vrj::Viewport object

   //Get active
   if (viewportChunk->getNum( "active" ) < 1 )
   {
      std::cout << "\tError: viewport chunk named " << viewportName << " doesn't have cameraPos properties\n" << std::flush;
      return false;
   }
   bool active = (viewportChunk->getProperty<int>("active", 0) == 1 ? true : false );

   if (viewport->isActive() != active)
   {
      std::cout << "\tError: viewport has incorrect active property\n" << std::flush;
      return false;
   }

   //drawProjections
   if (viewportChunk->getNum( "drawProjections" ) < 1 )
   {
      std::cout << "\tError: viewport chunk named " << viewportName << " doesn't have drawProjections properties\n" << std::flush;
      return false;
   }
   bool drawProjections = (viewportChunk->getProperty<int>("drawProjections", 0) == 1 ? true : false );
   //TEST drawProjections property
   //There is no test for this property


   //surfaceColor x 3
   if (viewportChunk->getNum( "surfaceColor" ) < 3 )
   {
      std::cout << "\tError: viewport chunk named " << viewportName << " doesn't have surfaceColor properties\n" << std::flush;
      return false;
   }
   float color[3];
   color[0] = viewportChunk->getProperty<float>("surfaceColor", 0);
   color[1] = viewportChunk->getProperty<float>("surfaceColor", 1);
   color[2] = viewportChunk->getProperty<float>("surfaceColor", 2);

   //Also no test for this

   //vert_fov
   if (viewportChunk->getNum( "vert_fov" ) < 1 )
   {
      std::cout << "\tError: viewport chunk named " << viewportName << " doesn't have vert_fov properties\n" << std::flush;
      return false;
   }
   float vert_fov = viewportChunk->getProperty<float>("vert_fov", 0);

   //Also no test for this

   return true;

}

bool reconfigApp::verifyDisplayFile( std::string filename )
{
   //Assume that the file given has the displayWindow chunk in it
   //Load up the given file 
   jccl::ConfigChunkDB fileDB ; fileDB.load( filename );
   std::vector<jccl::ConfigChunkPtr> windowChunks;
   std::vector<jccl::ConfigChunkPtr> machineChunks;
   std::vector<jccl::ConfigChunkPtr> machineSpecificChunks;
   fileDB.getByType( "displayWindow", windowChunks );
   fileDB.getByType( "machineSpecific", machineChunks );
   for (int i=0; i < machineChunks.size(); i++)
   {
      
      machineSpecificChunks = windowChunks[i]->getEmbeddedChunks();
      for (int j=0; i < machineSpecificChunks.size(); i++)
      {
         if (machineSpecificChunks[i]->getDescToken() == "displayWindow")
         {
            std::cout << "Adding an embedded display to the list to test named: " 
               << machineSpecificChunks[i]->getName() << std::endl;
            windowChunks.push_back(machineSpecificChunks[i]);
         }
      }
   }
   
   //Verify EACH display in the file
   for (int i=0; i < windowChunks.size(); i++)
   {
      //Get its attributes
      std::string displayName = windowChunks[i]->getName();

      //Get the display with the same name from the DisplayManager
      //and make sure it exists first
      vrj::Display* display = getDisplay( displayName );
      if (display == NULL)
      {
         std::cout << "\tError: display chunk named " << displayName << " is not in the display manager\n" << std::flush;
         return false;
      }

      //Get origin values
      if (windowChunks[i]->getNum( "origin" ) < 2 )
      {
         std::cout << "\tError: display chunk named " << displayName << " doesn't have origin properties\n" << std::flush;
         return false;
      }
      int x_origin = windowChunks[i]->getProperty<int>("origin", 0);
      int y_origin = windowChunks[i]->getProperty<int>("origin", 1);

      //Get size values
      if (windowChunks[i]->getNum( "size" ) < 2 )
      {
         std::cout << "\tError: display chunk named " << displayName << " doesn't have size properties\n" << std::flush;
         return false;
      }
      int x_size = windowChunks[i]->getProperty<int>("size", 0);
      int y_size = windowChunks[i]->getProperty<int>("size", 1);


      //Get pipe number
      if (windowChunks[i]->getNum( "pipe" ) < 1 )
      {
         std::cout << "\tError: display chunk named " << displayName << " doesn't have pipe property\n" << std::flush;
         return false;
      }
      int pipe_num = windowChunks[i]->getProperty<int>( "pipe" );

      //Get stereo value
      if (windowChunks[i]->getNum( "stereo" ) < 1 )
      {
         std::cout << "\tError: display chunk named " << displayName << " doesn't have stereo property\n" << std::flush;
         return false;
      }
      bool stereo = (windowChunks[i]->getProperty<int>( "stereo" ) == 1 ? true : false );

      //Get border value
      if (windowChunks[i]->getNum( "border" ) < 1 )
      {
         std::cout << "\tError: display chunk named " << displayName << " doesn't have border property\n" << std::flush;
         return false;
      }
      bool border = (windowChunks[i]->getProperty<int>( "border" ) == 1 ? true : false );

      //Get active value
      if (windowChunks[i]->getNum( "active" ) < 1 )
      {
         std::cout << "\tError: display chunk named " << displayName << " doesn't have active property\n" << std::flush;
         return false;
      }
      bool active = (windowChunks[i]->getProperty<int>( "active" ) == 1 ? true : false );

      //Verify the number of viewports
      if (windowChunks[i]->getNum("sim_viewports") != display->getNumViewports())
      {
         std::cout << "\tError: display chunk named " << displayName << " has the wrong number of viewports\n" << std::flush;
         return false;
      }

      //Verify all of the viewports
      for (int j=0; j < windowChunks[i]->getNum( "sim_viewports" ); j++)
      {
         jccl::ConfigChunkPtr viewlist = windowChunks[i]->getProperty<jccl::ConfigChunkPtr>("sim_viewports", j);

         //Get the actual simViewport chunk and check if it really contains any viewports
         if (viewlist->getNum("simViewport") > 0)
         {
            jccl::ConfigChunkPtr viewport = viewlist->getProperty<jccl::ConfigChunkPtr>("simViewport", 0);
            if (!verifyAllViewports( display, viewport ))
            {
               std::cout << "\tError: display chunk named " << displayName << " has an incorrect viewport\n" << std::flush;
               return false;
            }
         }
         
      }

      //Run a verification check (verifyDisplayProps)
      return verifyDisplayProps( display, displayName, x_origin, y_origin, x_size, y_size, pipe_num, stereo, border, active );
   }

   return true;
}

/********************************************************************
TEST SUITE FUNCTIONS
********************************************************************/
         
//  - Adding machine specfic gfx window     
bool reconfigApp::addMachineSpecific_exec()
{
   std::cout << "Beginning test for adding machine specific graphics windows...\n" << std::flush;
   return addChunkFile( mPath + "sim.ms.01.config" );
}

bool reconfigApp::addMachineSpecific_check()
{
   return verifyDisplayFile( mPath + "sim.ms.01.config" );
}


//  - Remove machine specific gfx window     
bool reconfigApp::removeMachineSpecific_exec()
{
   std::cout << "Beginning test for removing machine specific graphics windows...\n" << std::flush;
   return removeChunkFile( mPath + "sim.ms.01.config" );
}

bool reconfigApp::removeMachineSpecific_check()
{

   //Load up the given file 
   jccl::ConfigChunkDB fileDB ; fileDB.load( mPath + "sim.extradisplay.01.config" );
   std::vector<jccl::ConfigChunkPtr> windowChunks;
   fileDB.getByType( "displayWindow", windowChunks );

   if (windowChunks.size() != 1)
   {
      std::cout << "\tError: the display config chunk file contains " << windowChunks.size() << " displays (should be 1)\n" << std::flush;
      return false;
   }   

   vrj::Display* display = getDisplay(windowChunks[0]->getName());
   if (display != NULL)
   {
      std::cout << "\tError: there is still a display in the system named " << windowChunks[0]->getName() << "\n" << std::flush;
      return false;
   }   

   return true;

}

//  - Adding gfx window     
bool reconfigApp::addGFXWindow_exec()
{
   std::cout << "Beginning test for adding a graphics window...\n" << std::flush;
   return addChunkFile( mPath + "sim.extradisplay.01.config" );
}

bool reconfigApp::addGFXWindow_check()
{
   return verifyDisplayFile( mPath + "sim.extradisplay.01.config" );
}


//  - Remove gfx window     
bool reconfigApp::removeGFXWindow_exec()
{
   std::cout << "Beginning test for removing a graphics window...\n" << std::flush;
   return removeChunkFile( mPath + "sim.extradisplay.01.config" );
}

bool reconfigApp::removeGFXWindow_check()
{

   //Load up the given file 
   jccl::ConfigChunkDB fileDB ; fileDB.load( mPath + "sim.extradisplay.01.config" );
   std::vector<jccl::ConfigChunkPtr> windowChunks;
   fileDB.getByType( "displayWindow", windowChunks );

   if (windowChunks.size() != 1)
   {
      std::cout << "\tError: the display config chunk file contains " << windowChunks.size() << " displays (should be 1)\n" << std::flush;
      return false;
   }   

   vrj::Display* display = getDisplay(windowChunks[0]->getName());
   if (display != NULL)
   {
      std::cout << "\tError: there is still a display in the system named " << windowChunks[0]->getName() << "\n" << std::flush;
      return false;
   }   

   return true;

}

bool reconfigApp::readdGFXWindow_exec()
{
   std::cout << "Beginning test for readding a graphics window...\n" << std::flush;

   return (   addChunkFile( mPath + "sim.extradisplay.01.config" )
           && addChunkFile( mPath + "sim.extradisplay.02.config" ));
}

bool reconfigApp::readdGFXWindow_check()
{
   return verifyDisplayFile( mPath + "sim.extradisplay.01.config" ) &&
          verifyDisplayFile( mPath + "sim.extradisplay.02.config" );
}

bool reconfigApp::resizeGFXWindow_exec()
{
   std::cout << "Beginning test for resizing a graphics window...\n" << std::flush;

   return swapChunkFiles( mPath + "sim.extradisplay.02.config",
                          mPath + "sim.extradisplay.02.resize.config" );

}

bool reconfigApp::resizeGFXWindow_check()
{
   return verifyDisplayFile( mPath + "sim.extradisplay.02.resize.config" );
}

bool reconfigApp::moveGFXWindow_exec()
{
   std::cout << "Beginning test for moving a graphics window...\n" << std::flush;
  
   return swapChunkFiles( mPath + "sim.extradisplay.01.config",
                          mPath + "sim.extradisplay.01.move.config" );
}

bool reconfigApp::moveGFXWindow_check()
{
   return verifyDisplayFile( mPath + "sim.extradisplay.01.move.config" );
}

bool reconfigApp::addViewport_exec()
{
   std::cout << "Beginning test for adding a viewport to a display window...\n" << std::flush;

   return swapChunkFiles( mPath + "sim.extradisplay.02.resize.config",
                          mPath + "sim.extradisplay.02.twoviews.config" );
}

bool reconfigApp::addViewport_check()
{
   return verifyDisplayFile( mPath + "sim.extradisplay.02.twoviews.config" );
}

bool reconfigApp::removeViewport_exec()
{
   std::cout << "Beginning test for removing a viewport from a graphics window...\n" << std::flush;

   return swapChunkFiles( mPath + "sim.extradisplay.02.twoviews.config",
                          mPath + "sim.extradisplay.02.oneview.config" );
}

bool reconfigApp::removeViewport_check()
{
   return verifyDisplayFile( mPath + "sim.extradisplay.02.oneview.config" );
}

bool reconfigApp::resizeViewport_exec()
{
   std::cout << "Beginning test for resizing a viewport...\n" << std::flush;

   return swapChunkFiles( mPath + "sim.extradisplay.02.oneview.config",
                          mPath + "sim.extradisplay.02.largerview.config" );
}

bool reconfigApp::resizeViewport_check()
{
   return verifyDisplayFile( mPath + "sim.extradisplay.02.largerview.config" );;
}

bool reconfigApp::moveViewport_exec()
{
   std::cout << "Beginning test for moving a viewport...\n" << std::flush;

   return swapChunkFiles( mPath + "sim.extradisplay.02.largerview.config",
                          mPath + "sim.extradisplay.02.moveview.config" );
}

bool reconfigApp::moveViewport_check()
{
   return verifyDisplayFile( mPath + "sim.extradisplay.02.moveview.config" );
}

bool reconfigApp::enableStereoSurface_exec()
{
   std::cout << "Beginning test for enabling stereo on a surface display...\n" << std::flush;

   return        removeChunkFile( mPath + "sim.extradisplay.02.moveview.config" )
              && removeChunkFile( mPath + "sim.extradisplay.01.move.config" )
              && addChunkFile( mPath + "sim.surfacedisplay.01.stereo.config" );

}

bool reconfigApp::enableStereoSurface_check()
{
   //Load up the given file 
   jccl::ConfigChunkDB fileDB ; fileDB.load( mPath + "sim.surfacedisplay.01.stereo.config" );
   std::vector<jccl::ConfigChunkPtr> windowChunks;
   fileDB.getByType( "displayWindow", windowChunks );

   if (windowChunks.size() < 1)
   {
      std::cout << "\tError: There are no display window config chunks in the file\n" << std::flush;
      return false;
   }

   //Get the SimSurfaceX01
   vrj::Display* surfaceDisplay = getDisplay( windowChunks[0]->getName() );

   //If we could not find the window...we have a problem to start with
   if (surfaceDisplay == NULL)
   {
      std::cout << "\tError: Could not find the display to check its viewport\n" << std::flush;
      return false;
   }

   //Make sure we have one viewport
   if (surfaceDisplay->getNumViewports() != 1)
   {
      std::cout << "\tError: " << windowChunks[0]->getName() << " has an incorrect number of viewports\n" << std::flush;
      return false;   
   }

   vrj::Viewport* surfaceViewport = surfaceDisplay->getViewport(0);

   if (!surfaceViewport->inStereo())
   {
      std::cout << "\tError: the surface viewport is not in stereo\n" << std::flush;
      return false;   
   }

   return true;

}

bool reconfigApp::disableStereoSurface_exec()
{
   std::cout << "Beginning test for disabling stereo on a surface display...\n" << std::flush;

   return swapChunkFiles( mPath + "sim.surfacedisplay.01.stereo.config",
                          mPath + "sim.surfacedisplay.01.mono.config" );
}

bool reconfigApp::disableStereoSurface_check()
{
   //Load up the given file 
   jccl::ConfigChunkDB fileDB ; fileDB.load( mPath + "sim.surfacedisplay.01.mono.config" );
   std::vector<jccl::ConfigChunkPtr> windowChunks;
   fileDB.getByType( "displayWindow", windowChunks );

   if (windowChunks.size() < 1)
   {
      std::cout << "\tError: There are no display window config chunks in the file\n" << std::flush;
      return false;
   }

   //Get the SimSurfaceX01
   vrj::Display* surfaceDisplay = getDisplay( windowChunks[0]->getName() );

   //If we could not find the window...we have a problem to start with
   if (surfaceDisplay == NULL)
   {
      std::cout << "\tError: Could not find " << windowChunks[0]->getName() << " to check its viewport\n" << std::flush;
      return false;
   }

   //Make sure we have one viewport
   if (surfaceDisplay->getNumViewports() != 1)
   {
      std::cout << "\tError: " << windowChunks[0]->getName() << " has an incorrect number of viewports\n" << std::flush;
      return false;   
   }

   vrj::Viewport* surfaceViewport = surfaceDisplay->getViewport(0);

   if (surfaceViewport->inStereo())
   {
      std::cout << "\tError: " << windowChunks[0]->getName() << "'s viewport is in stereo\n" << std::flush;
      return false;   
   }

   return true;

}


bool reconfigApp::removeKeyboardWin_exec()
{
   std::cout << "Beginning test for removing a keyboard window...\n" << std::flush;

   //Note that we are assuming ./Chunks/startup/sim.wandkeyboardproxy.config has 
   //already been loaded

   return removeChunkFile( mPath + "startup/sim.wandkeyboard.config" );
}

bool reconfigApp::removeKeyboardWin_check()
{
   //First check that the keyboard device no longer exists
   jccl::ConfigChunkDB fileDB ; fileDB.load( mPath + "startup/sim.wandkeyboard.config" );
   std::vector<jccl::ConfigChunkPtr> fileChunks;
   fileDB.getByType( "Keyboard", fileChunks );

   std::string keyboardName = fileChunks[0]->getName();

   gadget::Keyboard* keyboard = (gadget::Keyboard*)gadget::InputManager::instance()->getDevice( keyboardName );

   if (keyboard != NULL)
   {
      std::cout << "\tError: the keyboard device named " << keyboardName << " still exists\n" << std::flush;
      return false;
   }

   //Check the wand keyboard proxy that is supposed to point to it is stupified
   jccl::ConfigChunkDB fileDB2 ; fileDB2.load( mPath + "startup/sim.wandkeyboardproxy.config" );
   fileChunks.clear();
   fileDB2.getByType( "KeyboardProxy", fileChunks );

   if (fileChunks.size() != 1)
   {
      std::cout << "\tError: there are " << fileChunks.size() << " keyboard proxies in the file (should be 1)\n" << std::flush;
      return false;
   }

   gadget::KeyboardProxy* keyboard_proxy = (gadget::KeyboardProxy*)gadget::InputManager::instance()->getProxy( fileChunks[0]->getName() );

   if (keyboard_proxy == NULL)
   {
      std::cout << "\tError: there is no keyboard proxy named " << fileChunks[0]->getName() << "\n" << std::flush;
      return false;
   }
 
   if (!keyboard_proxy->isStupified())
   {
      std::cout << "\tError: keyboard proxy named " << fileChunks[0]->getName() << " is not stupified\n" << std::flush;
      return false;

   }

   if (keyboard_proxy->getDeviceName() != keyboardName)
   {
      std::cout << "\tError: this keyboard proxy points at a keyboard named " << keyboard_proxy->getDeviceName() << "\n" << std::flush;
      return false;
   }

   return true;
}


bool reconfigApp::readdKeyboardWin_exec()
{
   std::cout << "Beginning test for readding a keyboard window...\n" << std::flush;
   return addChunkFile( mPath + "startup/sim.wandkeyboard.config" );
}

bool reconfigApp::readdKeyboardWin_check()
{
   //Check the wand keyboard proxy that is supposed to point to it

   //First check that the keyboard device exists again
   jccl::ConfigChunkDB fileDB ; fileDB.load( mPath + "startup/sim.wandkeyboard.config" );
   std::vector<jccl::ConfigChunkPtr> fileChunks;
   fileDB.getByType( "Keyboard", fileChunks );

   std::string keyboardName = fileChunks[0]->getName();

   gadget::Keyboard* keyboard = (gadget::Keyboard*)gadget::InputManager::instance()->getDevice( keyboardName );

   if (keyboard == NULL)
   {
      std::cout << "\tError: the keyboard device named " << keyboardName << " doesn't exist\n" << std::flush;
      return false;
   }

   //Check the wand keyboard proxy that is supposed to point to it is stupified
   jccl::ConfigChunkDB fileDB2 ; fileDB2.load( mPath + "startup/sim.wandkeyboardproxy.config" );
   fileChunks.clear();
   fileDB2.getByType( "KeyboardProxy", fileChunks );

   if (fileChunks.size() != 1)
   {
      std::cout << "\tError: there are " << fileChunks.size() << " keyboard proxies in the file (should be 1)\n" << std::flush;
      return false;
   }

   gadget::KeyboardProxy* keyboard_proxy = (gadget::KeyboardProxy*)gadget::InputManager::instance()->getProxy( fileChunks[0]->getName() );

   if (keyboard_proxy == NULL)
   {
      std::cout << "\tError: there is no keyboard proxy named " << fileChunks[0]->getName() << "\n" << std::flush;
      return false;
   }
 
   if (keyboard_proxy->isStupified())
   {
      std::cout << "\tError: keyboard proxy named " << fileChunks[0]->getName() << " is stupified\n" << std::flush;
      return false;

   }

   if (keyboard_proxy->getDeviceName() != keyboardName)
   {
      std::cout << "\tError: this keyboard proxy points at a keyboard named " << keyboard_proxy->getDeviceName() << "\n" << std::flush;
      return false;
   }

   //Make sure the pointers match up
   if (((gadget::Keyboard*)(keyboard_proxy->getProxiedInputDevice())) != keyboard)
   {
      std::cout << "\tError: pointers don't match up\n" << std::flush;
      return false;
   }

   return true;
}


bool reconfigApp::addSimPos_exec()
{
   std::cout << "Beginning test for adding a sim position device and pointing proxies at it...\n" << std::flush;
   return (  addChunkFile( mPath + "sim.positiondevice.config" ) 
          && addChunkFile( mPath + "sim.positiondeviceproxy.config" ));
}

bool reconfigApp::addSimPos_check()
{
   //Check that the sim pos device exists and that the proxy points at it
   return verifyProxy( "ExtraPositionProxy", "ExtraPositionDevice" );
}


bool reconfigApp::removeSimPos_exec()
{
   std::cout << "Beginning test for removing a sim position device and checking its proxies...\n" << std::flush;
   return removeChunkFile( mPath + "sim.positiondevice.config" );
}

bool reconfigApp::removeSimPos_check()
{

   //Look at the button proxy and see what it points at
   gadget::Proxy* proxy = gadget::InputManager::instance()->getProxy( "ExtraPositionProxy" );

   if (proxy == NULL)
   {
      std::cout << "\tError: Could not find the proxy\n" << std::flush;
      return false;
   }

   if (!proxy->isStupified())
   {
      std::cout << "\tError: Proxy is not stupified\n" << std::flush;
      return false;
   }
 
   return true;

}


bool reconfigApp::readdSimPos_exec()
{
   std::cout << "Beginning test for readding a sim position device and checking its proxies...\n" << std::flush;
   return addChunkFile( mPath + "sim.positiondevice.config" );
}

bool reconfigApp::readdSimPos_check()
{
   //Check that the sim pos device exists and that the proxy points at it
   return verifyProxy( "ExtraPositionProxy", "ExtraPositionDevice" );
}


bool reconfigApp::repointProxy_exec()
{
   std::cout << "Beginning test for repointing a proxy to something else...\n" << std::flush;

   return swapChunkFiles( mPath + "sim.positiondeviceproxy.config",
                          mPath + "sim.positiondeviceproxy.repoint.config" );
}

bool reconfigApp::repointProxy_check()
{
   //Load the repoint config file, get its device property, match that name
   //with the gadget::Proxy's instance name
   jccl::ConfigChunkDB fileDB ; fileDB.load( mPath + "sim.positiondeviceproxy.repoint.config" );
   std::vector<jccl::ConfigChunkPtr> fileChunks;
   fileDB.getByType( "PosProxy", fileChunks );


   if (fileChunks.size() != 1)
   {
      std::cout << "\tError: there are " << fileChunks.size() << " proxies in the config file (should be 1)\n" << std::flush;
      return false;
   }

   if (fileChunks[0]->getNum( "device" ) != 1)
   {
      std::cout << "\tError: there are " << fileChunks[0]->getNum("device") << " device properties in the config file (should be 1)\n" << std::flush;
      return false;
   }

   //Get the proxy's name from the file
   std::string proxyname = fileChunks[0]->getName();

   gadget::PositionProxy* proxy = (gadget::PositionProxy*)gadget::InputManager::instance()->getProxy( proxyname );

   if (proxy == NULL)
   {
      std::cout << "\tError: Could not find proxy named " << proxyname << " in the Input Manager\n" << std::flush;
      return false;
   }

   if (fileChunks[0]->getProperty<std::string>("device", 0) != proxy->getDeviceName())
   {
      std::cout << "\tError: the proxy points at device named " << proxy->getDeviceName() 
                << ", but should point at " << fileChunks[0]->getProperty<std::string>("device", 0) << "\n" << std::flush;
      return false;
   }

   return true;
}


bool reconfigApp::reconfigSimPos_exec()
{
   std::cout << "Beginning test for reconfiguring a sim position device...\n" << std::flush;

   return swapChunkFiles( mPath + "startup/sim.simheadpos.config", mPath + "sim.simheadpos.reconfig.config" );
}

bool reconfigApp::reconfigSimPos_check()
{
   //Get the sim position pointer
   gadget::SimPosition* device = (gadget::SimPosition*)gadget::InputManager::instance()->getDevice( "SimHeadPos" );

   if ( device == NULL )
   {
      std::cout << "\tError: Could not find the sim position device\n" << std::flush;
      return false;      
   }

   gmtl::Matrix44f mat = *(device->getPositionData().getPosition());

   gmtl::Vec3f pos = gmtl::makeTrans<gmtl::Vec3f>(mat);

   if ((pos[0] != 1.0) || (pos[1] != 2.0) || (pos[2] != 3.0))
   {
      std::cout << "\tError: Sim position device has incorrect initial position: " << pos[0] << "  " << pos[1] << "  " << pos[2] << "\n" << std::flush;
   }

   return true;
}


bool reconfigApp::reconfigSimDigital_exec()
{
   std::cout << "Beginning test for reconfiguring a sim digital device...\n" << std::flush;

   return swapChunkFiles( mPath + "startup/sim.wandbuttonsdigital02.config", 
                          mPath + "sim.wandbuttonsdigital02.reconfig.config" );
}

bool reconfigApp::reconfigSimDigital_check()
{

   //Get the sim digital pointer
   gadget::SimDigital* device = (gadget::SimDigital*)gadget::InputManager::instance()->getDevice( "SimWandButtons02" );

   if ( device == NULL )
   {
      std::cout << "\tError: Could not find the sim digital device\n" << std::flush;
      return false;      
   }

   //Get the vector of keymodpairs
   std::vector<gadget::SimInput::KeyModPair> keypairs = device->getKeys();

   //Load up the config file.
   jccl::ConfigChunkDB fileDB ; fileDB.load( mPath + "sim.wandbuttonsdigital02.reconfig.config" );
   std::vector<jccl::ConfigChunkPtr> fileChunks;
   fileDB.getByType( "SimDigital", fileChunks );

   bool key_status;
   //For each keymodpair that was defined in the device...
   for (int i=0; i < keypairs.size(); i++)
   {
      key_status = false;

      //Scan the sim digital config chunk to see if was defined there
      for ( int j=0; j < fileChunks[0]->getNum("keyPairs"); j++ )
      {
         jccl::ConfigChunkPtr key_mod_pair = fileChunks[0]->getProperty<jccl::ConfigChunkPtr>("keyPairs", j);

         //If they are the same..
         if (( keypairs[i].mModifier == key_mod_pair->getProperty<int>("modKey", 0))
            && keypairs[i].mKey == key_mod_pair->getProperty<int>("key", 0))
         {
            key_status = true;
            continue;
         }
      }  

      //If we have not found a key that matches...
      if (key_status == false)
      {
         std::cout << "\tError: One or more of the key pair definitions are incorrect\n" << std::flush;
         return false;
      }

   }

   return true;

}


bool reconfigApp::reconfigSimAnalog_exec()
{
   std::cout << "Beginning test for reconfiguring a sim analog device...\n" << std::flush;

   return swapChunkFiles( mPath + "startup/sim.analogdevice1.config",
                          mPath + "sim.analogdevice1.reconfig.config" );
}

bool reconfigApp::reconfigSimAnalog_check()
{

   //Get the sim analog pointer
   gadget::SimAnalog* device = (gadget::SimAnalog*)gadget::InputManager::instance()->getDevice( "AnalogDevice1" );

   if ( device == NULL )
   {
      std::cout << "\tError: Could not find the sim analog device\n" << std::flush;
      return false;      
   }

   //Get the vector of keymodpairs
   std::vector<gadget::SimInput::KeyModPair> keypairsup = device->getUpKeys();
   std::vector<gadget::SimInput::KeyModPair> keypairsdown = device->getDownKeys();

   //Load up the config file.
   jccl::ConfigChunkDB fileDB ; fileDB.load( mPath + "sim.analogdevice1.reconfig.config" );
   std::vector<jccl::ConfigChunkPtr> fileChunks;
   fileDB.getByType( "SimAnalog", fileChunks );

   bool key_status;

   //Scan the sim digital config chunk to see if was defined there
   for ( int i=0; i < fileChunks[0]->getNum("keyPairs"); i++ )
   {
      key_status = false;

      jccl::ConfigChunkPtr key_mod_pair = fileChunks[0]->getProperty<jccl::ConfigChunkPtr>("keyPairs", i);

      //Scan the up keys
      for (int j=0; j < keypairsup.size(); j++)
      {
         //If they are the same..
         if (( keypairsup[j].mModifier == key_mod_pair->getProperty<int>("modKey", 0))
            && keypairsup[j].mKey == key_mod_pair->getProperty<int>("key", 0))
         {
            key_status = true;
            continue;
         }
      }  

      //Scan the down keys
      for (int j=0; j < keypairsdown.size(); j++)
      {
         //If they are the same..
         if (( keypairsdown[j].mModifier == key_mod_pair->getProperty<int>("modKey", 0))
            && keypairsdown[j].mKey == key_mod_pair->getProperty<int>("key", 0))
         {
            key_status = true;
            continue;
         }
      }  

      //If we have not found a key that matches...
      if (key_status == false)
      {
         std::cout << "\tError: One or more of the key pair definitions are incorrect\n" << std::flush;
         return false;
      }
   }

   return true;
}


bool reconfigApp::addSimDigital_exec()
{
   std::cout << "Beginning test for adding a sim digital device and pointing proxies at it...\n" << std::flush;   

   return (  addChunkFile( mPath + "sim.digitaldevice.config" )
          && addChunkFile( mPath + "sim.digitalproxy.config"  ));
}

bool reconfigApp::addSimDigital_check()
{
   return verifyProxy( "DigitalProxy", "ExtraDigitalDevice" );
}


bool reconfigApp::removeSimDigital_exec()
{
   std::cout << "Beginning test for removing a sim digital device...\n" << std::flush;
   return removeChunkFile( mPath + "startup/sim.wandbuttonsdigital.config" );
}

bool reconfigApp::removeSimDigital_check()
{
   //Look at the button proxy and see what it points at
   gadget::Proxy* proxy = gadget::InputManager::instance()->getProxy( "Button0Proxy" );

   if (proxy == NULL)
   {
      std::cout << "\tError: Could not find the proxy\n" << std::flush;
      return false;
   }

   if (!proxy->isStupified())
   {
      std::cout << "\tError: Proxy is not stupified\n" << std::flush;
      return false;
   }
   
   return true;
}


bool reconfigApp::readdSimDigital_exec()
{
   std::cout << "Beginning test for readding a sim digital device and checking the proxies pointing at it...\n" << std::flush;
   
   return addChunkFile( mPath + "startup/sim.wandbuttonsdigital.config" );
}

bool reconfigApp::readdSimDigital_check()
{
   return (   verifyProxy( "Button0Proxy", "SimWandButtons" )
           && verifyProxy( "Button1Proxy", "SimWandButtons" )
           && verifyProxy( "Button2Proxy", "SimWandButtons" )
           && verifyProxy( "Button3Proxy", "SimWandButtons" )
           && verifyProxy( "Button4Proxy", "SimWandButtons" )
           && verifyProxy( "Button5Proxy", "SimWandButtons" ));
}

bool reconfigApp::addSimAnalog_exec()
{
   std::cout << "Beginning test for adding a sim analog device and pointing proxies at it...\n" << std::flush;   

   return (  addChunkFile( mPath + "sim.extraanalogdevice.config" )
          && addChunkFile( mPath + "sim.extraanalogproxy.config"  ));
}

bool reconfigApp::addSimAnalog_check()
{
   return verifyProxy( "ExtraAnalogProxy", "ExtraAnalogDevice" );
}


bool reconfigApp::removeSimAnalog_exec()
{
   std::cout << "Beginning test for removing a sim analog device and checking the proxies that point at it...\n" << std::flush;
   return removeChunkFile( mPath + "sim.extraanalogdevice.config" );
}

bool reconfigApp::removeSimAnalog_check()
{
   //Look at the button proxy and see what it points at
   gadget::Proxy* proxy = gadget::InputManager::instance()->getProxy( "ExtraAnalogProxy" );

   if (proxy == NULL)
   {
      std::cout << "\tError: Could not find the proxy\n" << std::flush;
      return false;
   }

   if (!proxy->isStupified())
   {
      std::cout << "\tError: Proxy is not stupified\n" << std::flush;
      return false;
   }
   
   return true;
}


bool reconfigApp::readdSimAnalog_exec()
{
   std::cout << "Beginning test for readding a sim analog device and checking the old proxies...\n" << std::flush;
   return addChunkFile( mPath + "sim.extraanalogdevice.config" ) ;
}

bool reconfigApp::readdSimAnalog_check()
{
   return verifyProxy( "ExtraAnalogProxy", "ExtraAnalogDevice" );
}

bool reconfigApp::addAnalogProxy_exec()
{
   std::cout << "Beginning test for adding an analog proxy...\n" << std::flush;
   return addChunkFile( mPath + "sim.analogproxy01.config" ) ;
}

bool reconfigApp::addAnalogProxy_check()
{
   return verifyProxy( "AnalogProxy01", "AnalogDevice1" );
}


bool reconfigApp::reconfigAnalogProxy_exec()
{
   std::cout << "Beginning test for reconfiguring an analog proxy...\n" << std::flush;
   return swapChunkFiles( mPath + "sim.analogproxy01.config", mPath + "sim.analogproxy01.reconfig.config" );
}

bool reconfigApp::reconfigAnalogProxy_check()
{
   return verifyProxy( "AnalogProxy01", "AnalogDevice2" );
}


bool reconfigApp::removeAnalogProxy_exec()
{
   std::cout << "Beginning test for removing an analog proxy...\n" << std::flush;
   return removeChunkFile( mPath + "sim.analogproxy01.reconfig.config" );
}

bool reconfigApp::removeAnalogProxy_check()
{

  //Try to get the new proxy and check its values

   if (gadget::InputManager::instance()->getProxy( "AnalogProxy01" ) != NULL)
   {
      std::cout << "\tError: AnalogProxy01 still exists\n" << std::flush;
      return false;
   }

   return true;
}


bool reconfigApp::addPosProxy_exec()
{
   std::cout << "Beginning test for adding a sim digital device...\n" << std::flush;
   return addChunkFile( mPath + "sim.simcam1proxy02.config" );
}

bool reconfigApp::addPosProxy_check()
{
   return verifyProxy( "SimCam1 Proxy02", "SimCamera1" );
}


bool reconfigApp::reconfigPosProxy_exec()
{
   std::cout << "Beginning test for reconfiguring a position proxy...\n" << std::flush;
   return swapChunkFiles( mPath + "sim.simcam1proxy02.config", mPath + "sim.simcam1proxy02.reconfig.config");
}

bool reconfigApp::reconfigPosProxy_check()
{
   return verifyProxy( "SimCam1 Proxy02", "SimCamera2") ;
}


bool reconfigApp::removePosProxy_exec()
{
   std::cout << "Beginning test for removing a position proxy...\n" << std::flush;
   return removeChunkFile( mPath + "sim.simcam1proxy02.reconfig.config" );
}

bool reconfigApp::removePosProxy_check()
{
  //Try to get the new proxy and check its values

   if (gadget::InputManager::instance()->getProxy( "SimCam1 Proxy02" ) != NULL)
   {
      std::cout << "\tError: SimCam1 Proxy02 still exists\n" << std::flush;
      return false;
   }

   return true;
}


bool reconfigApp::addDigitalProxy_exec()
{
   std::cout << "Beginning test for adding a digital proxy...\n" << std::flush;
   return addChunkFile( mPath + "sim.button0proxy02.config" );
}

bool reconfigApp::addDigitalProxy_check()
{
   return verifyProxy( "Button0Proxy02", "SimWandButtons" );
}


bool reconfigApp::reconfigDigitalProxy_exec()
{
   std::cout << "Beginning test for reconfiguring a digital proxy...\n" << std::flush;
   return swapChunkFiles( mPath + "sim.button0proxy02.config", mPath + "sim.button0proxy02.reconfig.config");
}

bool reconfigApp::reconfigDigitalProxy_check()
{
   return verifyProxy( "Button0Proxy02", "SimWandButtons02" );
}


bool reconfigApp::removeDigitalProxy_exec()
{
   std::cout << "Beginning test for removing a digital proxy...\n" << std::flush;
   return removeChunkFile( mPath + "sim.button0proxy02.reconfig.config" );
}

bool reconfigApp::removeDigitalProxy_check()
{
  //Try to get the new proxy and check its values

   if (gadget::InputManager::instance()->getProxy( "Button0Proxy02" ) != NULL)
   {
      std::cout << "\tError: Button0 proxy 02 still exists\n" << std::flush;
      return false;
   }

   return true;
}


bool reconfigApp::addKeyboardProxy_exec()
{
   std::cout << "Beginning test for adding a keyboard proxy...\n" << std::flush;
   return addChunkFile( mPath + "sim.wandkeyboardproxy02.config" );
}

bool reconfigApp::addKeyboardProxy_check()
{
   return verifyProxy( "WandKeyboardProxy02", "Wand Keyboard" );
}


bool reconfigApp::reconfigKeyboardProxy_exec()
{
   std::cout << "Beginning test for reconfiguring a keyboard proxy...\n" << std::flush;
   return swapChunkFiles( mPath + "sim.wandkeyboardproxy02.config", mPath + "sim.wandkeyboardproxy02.reconfig.config");
}

bool reconfigApp::reconfigKeyboardProxy_check()
{
   return verifyProxy( "WandKeyboardProxy02", "Head Keyboard" );
}


bool reconfigApp::removeKeyboardProxy_exec()
{
   std::cout << "Beginning test for removing a keyboard proxy...\n" << std::flush;
   return removeChunkFile( mPath + "sim.wandkeyboardproxy02.reconfig.config" );
}

bool reconfigApp::removeKeyboardProxy_check()
{

   //Try to get the new proxy and check its values

   if (gadget::InputManager::instance()->getProxy( "WandKeyboardProxy02" ) != NULL)
   {
      std::cout << "\tError: Keyboard proxy 02 still exists\n" << std::flush;
      return false;
   }

   return true;
}

bool reconfigApp::addStupifiedAnalogProxy_exec()
{
   std::cout << "Beginning test for adding a stupified analog proxy...\n" << std::flush;
   return addChunkFile( mPath + "sim.stupidproxy01.config" );
}

bool reconfigApp::addStupifiedAnalogProxy_check()
{
   //Try to get the new proxy and check its values
   gadget::Proxy* tempProxy = gadget::InputManager::instance()->getProxy( "StupidAnalogProxy01" );
   gadget::AnalogProxy* proxy = dynamic_cast<gadget::AnalogProxy*>(tempProxy);

   if (proxy == NULL)
   {
      std::cout << "\tError: Could not find the proxy\n" << std::flush;
      return false;
   }
   if (!proxy->isStupified())
   {
      std::cout << "\tError: Proxy is not stupified\n" << std::flush;
      return false;
   }
   if (proxy->getData() != 0.0 )
   {
   }
   if (proxy->getProxiedInputDevice() != NULL)
   {
      std::cout << "\tError: Proxy points at a device\n" << std::flush;
      return false;
   }
   if (proxy->getChunkType() != "AnaProxy")
   {
      std::cout << "\tError: Proxy has the wrong chunk type: " << proxy->getChunkType() << "   It should be AnaProxy\n" << std::flush;
      return false;
   }

   return true;
}
bool reconfigApp::removeStupifiedAnalogProxy_exec()
{
   std::cout << "Beginning test for removing a stupified analog proxy...\n" << std::flush;
   return removeChunkFile( mPath + "sim.stupidproxy01.config" );
}

bool reconfigApp::removeStupifiedAnalogProxy_check()
{

   //Try to get the new proxy and check its values

   if (gadget::InputManager::instance()->getProxy( "StupidAnalogProxy01" ) != NULL)
   {
      std::cout << "\tError: Stupid Proxy 01 still exists\n" << std::flush;
      return false;
   }

   return true;
}

