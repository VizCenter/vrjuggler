//----------------------------------------
// Simple OpenGL sample application
//
// main.cpp
//----------------------------------------
#include <simpleApp.h>

// --- Lib Stuff --- //
#include <Kernel/vjKernel.h>

int main(int argc, char* argv[])
{
   // Allocate the kernel object and the application object
   vjKernel* kernel = vjKernel::instance();           // Get the kernel
   simpleApp* application = new simpleApp();          // Instantiate an instance of the app

   // IF not args passed to the program
   //    Display usage information and exit
   if (argc <= 1)
   {
      std::cout << "\n\n";
      std::cout << "Usage: " << argv[0] << " vjconfigfile[0] vjconfigfile[1] ... vjconfigfile[n]" << std::endl;
      exit(1);
   }

   // Load any config files specified on the command line
   for(int i=1;i<argc;i++)
      kernel->loadConfigFile(argv[i]);

   // Start the kernel running
   kernel->start();

   // Give the kernel an application to execute
   kernel->setApplication(application);

   // Keep thread alive and waiting
   // If the developer makes a way to request that the application exits,
   // then this loop can check for that
   while(1)
   {
      usleep(250000);
   }
}
