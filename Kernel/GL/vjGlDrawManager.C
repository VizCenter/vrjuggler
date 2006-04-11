#include <config.h>
#include <Kernel/GL/vjGlDrawManager.h>
#include <Threads/vjThread.h>
#include <Kernel/vjDisplayManager.h>
#include <Kernel/vjKernel.h>
#include <Kernel/vjSimulator.h>


vjGlDrawManager* vjGlDrawManager::_instance = NULL;

    /**
     * Function to config API specific stuff.
     * Takes a chunkDB and extracts API specific stuff
     */
void vjGlDrawManager::config(vjConfigChunkDB*  chunkDB)
{
    // Setup any config data
}

    // Enable a frame to be drawn
    // Trigger draw
void vjGlDrawManager::draw()
{
   drawTriggerSema.release();
}


//: Blocks until the end of the frame
//! POST:
//+	   The frame has been drawn
void vjGlDrawManager::sync()
{
   drawDoneSema.acquire();
}


//: This is the control loop for the manager
void vjGlDrawManager::main(void* nullParam)
{
   //while(!Exit)
   while (1)
   {
      // Wait for trigger
      drawTriggerSema.acquire();

      // THEN --- Do Rendering --- //
      drawAllPipes();

      // -- Done rendering --- //
      drawDoneSema.release();
   }
}

void vjGlDrawManager::drawAllPipes()
{
   vjDEBUG_BEGIN(3) << "vjGLDrawManager::drawAllPipes: Enter" << endl << flush << vjDEBUG_FLUSH;
   int pipeNum;

      // Start rendering all the pipes
   for(pipeNum=0; pipeNum<pipes.size(); pipeNum++)
      pipes[pipeNum]->triggerRender();

      // Wait for rendering to finish on all the pipes
   for(pipeNum=0; pipeNum<pipes.size(); pipeNum++)
      pipes[pipeNum]->completeRender();

   vjDEBUG_END(3) << "vjGLDrawManager::drawAllPipes: Exit" << endl << flush << vjDEBUG_FLUSH;
}

    /// Initialize the drawing API (if not already running)
void vjGlDrawManager::initAPI()
{
    ; /* Do nothing */
}

    //: Initialize the drawing state for the API based on
    //+ the data in the display manager.
    //
    //! PRE: API is running (initAPI has been called)
    //! POST: API is ready do draw    <br>
    //+	 Process model is configured <br>
    //+	 Multi-pipe data is set      <br>
    //+	 Window list is correct      <br>
void vjGlDrawManager::initDrawing()
{
   vjDEBUG(3) << "vjGlDrawManager::initDrawing: Entering." << endl << vjDEBUG_FLUSH;

   //  For each display:
   //	-- Create a window for it
   //	-- Store the window in the wins vector
   for (vector<vjDisplay*>::iterator dispIter = displayManager->displays.begin();
       dispIter != displayManager->displays.end(); dispIter++)
   {
      vjGlWindow* newWin;
      newWin = vjKernel::instance()->getSysFactory()->getGLWindow();
      newWin->config(*dispIter);
      wins.push_back(newWin);
   }

   // Create Pipes & Add all windows to the correct pipe
   for(int winId=0;winId<wins.size();winId++)   // For each window we created
   {
      int pipeNum = wins[winId]->getDisplay()->pipe();  // Find pipe to add it too
                                                        // ASSERT: pipeNum := [0...n]

      if(pipes.size() < (pipeNum+1))            // ASSERT: Max index of pipes is < our pipe
      {
                                                // XXX: This is not really a good way to do it
         while(pipes.size() < (pipeNum+1))      // While we are out of range
         {
            vjGlPipe* newPipe = new vjGlPipe(pipeNum, this);   // Create a new pipe to use
            pipes.push_back(newPipe);                          // Add the pipe
         }
      }

      vjGlPipe* pipe;                           // The pipe to assign it to
      pipe = pipes[pipeNum];                    // ASSERT: pipeNum is in the valid range
      pipe->addWindow(wins[winId]);             // Window has been added
   }

      // Start all the pipes running
   for(int pipeNum=0;pipeNum<pipes.size();pipeNum++)
   {
      //**//if(pipes[pipeNum]->hasWindows())    // Actually we want all the pipes to run
      pipes[pipeNum]->start();
   }

      // --- Setup Multi-Process stuff --- //
      // Create a new thread to handle the control
	vjThread* control_thread;

   vjThreadMemberFunctor<vjGlDrawManager>* memberFunctor =
      new vjThreadMemberFunctor<vjGlDrawManager>(this, &vjGlDrawManager::main, NULL);

	control_thread = new vjThread(memberFunctor, 0);

   vjDEBUG(0) << "vjGlDrawManager::thread: " << control_thread << endl << vjDEBUG_FLUSH;
   // Dump the state
   debugDump();
}
	

    /// Shutdown the drawing API
void vjGlDrawManager::closeAPI()
{
    // Stop all pipes
   ;
    // Delete all pipes

    // Close and delete all glWindows
}

//: Draw a simulator using OpenGL commands
//! NOTE: This is called internally by the library
void vjGlDrawManager::drawSimulator(vjSimulator* sim)
{
   const float head_radius(0.75);
   const float eye_vertical(0.22);
   const float eye_horizontal(0.7);
   const float interoccular(0.27);
   const float eye_radius(0.08f);

      // Draw a vj-like outline
   glPushMatrix();
      glColor3f(1.0f, 1.0f, 1.0f);
      glTranslatef(0.0f, 5.0f, 0.0f);      // Center it on 0,0,0
      drawWireCube(10.0f);
   glPopMatrix();

      // Draw the user's head
   glPushMatrix();
      glMultMatrixf(sim->getHeadPos().getFloatPtr());
      glColor3f(1.0f, 0.0f, 0.0f);
      drawSphere(head_radius, 10, 10);             // Head
      glPushMatrix();
         glColor3f(0.0f, 0.0f, 1.0f);
         glTranslatef(0.0f, eye_vertical, -eye_horizontal);
         glPushMatrix();                     // Right eye
            glTranslatef((interoccular/2.0f), 0.0f, 0.0f);
            drawSphere(eye_radius, 5, 5);
         glPopMatrix();
         glPushMatrix();                     // Left eye
            glTranslatef(-(interoccular/2.0f), 0.0f, 0.0f);
            drawSphere(eye_radius, 5, 5);
         glPopMatrix();
      glPopMatrix();
   glPopMatrix();

      // Draw the wand
   glPushMatrix();
      glMultMatrixf(sim->getWandPos().getFloatPtr());
      glColor3f(1.0f, 0.0f, 1.0f);
      drawCone(0.2, 0.6f, 6, 1);
   glPopMatrix();
}

    /// dumps the object's internal state
void vjGlDrawManager::debugDump()
{
    vjDEBUG(0) << "-- DEBUG DUMP --------- vjGlDrawManager: " << (void*)this << " ------------" << endl
	         << "\tapp:" << (void*)app << endl << vjDEBUG_FLUSH;
    vjDEBUG(0) << "\tWins:" << wins.size() << endl << vjDEBUG_FLUSH;

    for(vector<vjGlWindow*>::iterator i = wins.begin(); i != wins.end(); i++)
    {
	   vjDEBUG(0) << "\n\t\tvjGlWindow:\n" << *(*i) << endl << vjDEBUG_FLUSH;
    }
    vjDEBUG(0) << flush << vjDEBUG_FLUSH;
}

void vjGlDrawManager::initQuadObj()
{
   if (mQuadObj == NULL)
      mQuadObj = gluNewQuadric();
}

void vjGlDrawManager::drawSphere(float radius, int slices, int stacks)
{
  initQuadObj();
  gluQuadricDrawStyle(mQuadObj, GLU_FILL);
  gluQuadricNormals(mQuadObj, GLU_SMOOTH);
  gluSphere(mQuadObj, radius, slices, stacks);
}


void vjGlDrawManager::drawCone(float base, float height, int slices, int stacks)
{
  initQuadObj();
  gluQuadricDrawStyle(mQuadObj, GLU_FILL);
  gluQuadricNormals(mQuadObj, GLU_SMOOTH);
  gluCylinder(mQuadObj, base, 0.0, height, slices, stacks);
}



void vjGlDrawManager::drawBox(float size, GLenum type)
{
  static GLfloat n[6][3] =
  {
    {-1.0, 0.0, 0.0},
    {0.0, 1.0, 0.0},
    {1.0, 0.0, 0.0},
    {0.0, -1.0, 0.0},
    {0.0, 0.0, 1.0},
    {0.0, 0.0, -1.0}
  };

  static GLint faces[6][4] =
  {
    {0, 1, 2, 3},
    {3, 2, 6, 7},
    {7, 6, 5, 4},
    {4, 5, 1, 0},
    {5, 6, 2, 1},
    {7, 4, 0, 3}
  };
  GLfloat v[8][3];
  GLint i;

  v[0][0] = v[1][0] = v[2][0] = v[3][0] = -size / 2;
  v[4][0] = v[5][0] = v[6][0] = v[7][0] = size / 2;
  v[0][1] = v[1][1] = v[4][1] = v[5][1] = -size / 2;
  v[2][1] = v[3][1] = v[6][1] = v[7][1] = size / 2;
  v[0][2] = v[3][2] = v[4][2] = v[7][2] = -size / 2;
  v[1][2] = v[2][2] = v[5][2] = v[6][2] = size / 2;

  for (i = 5; i >= 0; i--) {
    glBegin(type);
    glNormal3fv(&n[i][0]);
    glVertex3fv(&v[faces[i][0]][0]);
    glVertex3fv(&v[faces[i][1]][0]);
    glVertex3fv(&v[faces[i][2]][0]);
    glVertex3fv(&v[faces[i][3]][0]);
    glEnd();
  }
}


void vjGlDrawManager::drawWireCube(float size)
{
  drawBox(size, GL_LINE_LOOP);
}

void vjGlDrawManager::drawSolidCube(float size)
{
  drawBox(size, GL_QUADS);
}


