#include <vrj/vrjConfig.h> // for DEG2RAD macro
#include <fstream>        // for ifstream
#include "KeyFramer.h"
#include <vrj/Util/Debug.h>
#include <vrj/Math/Math.h>

namespace kev
{
   class KeyFramerExporter
   {
   public:
      KeyFramerExporter()
      {
      }

      void execute( const char* const filename, const kev::KeyFramer& kf )
      {
         std::ofstream frames_file( filename, std::ios::out );

         if (!frames_file.rdbuf()->is_open())
         {
            vprDEBUG(vprDBG_ALL,0) << clrSetNORM(clrYELLOW) << "WARNING: "
                           << clrRESET
                           << "couldn't open keyframe file: "
                           << filename <<"\n"
                           << vprDEBUG_FLUSH;
            return;
         }

         vprDEBUG(vprDBG_ALL,0) << "Writing keyframe(s) to file: "
                <<filename<<"\n"<<vprDEBUG_FLUSH;

         std::map<float, kev::KeyFramer::Key>::const_iterator it1 = kf.keys().begin();
         for (; it1 != kf.keys().end(); ++it1)
         {
            const kev::KeyFramer::Key& key = (*it1).second;
            const float& time = key.time();
            const vrj::Vec3& pos = key.position();
            const Quat& quat = key.rotation();
            float deg;
            vrj::Vec3 vec;
            quat.setRot( deg, vec[0], vec[1], vec[2] );
            deg = vrj::Math::rad2deg( deg );
            frames_file<<time<<" "<<pos[0]<<" "<<pos[1]<<" "
                       <<pos[2]<<" "<<deg<<" "<<vec[0]<<" "
                       <<vec[1]<<" "<<vec[2]<<"\n";
         }
         frames_file.close();
      }
   };
};
