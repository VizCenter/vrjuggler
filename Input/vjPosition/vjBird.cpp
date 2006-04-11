#include <vjConfig.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include <Input/vjPosition/vjBird.h>


inline int getReading(vjMatrix* data, int port);
inline float rawToFloat(char& r1, char& r2);
inline void  pickBird(int sensor, int port_id);
static int open_port(char* serialPort, int baud);
static void set_blocking(int port, int blocking);
static void set_sync(int port, int sync);
static void set_hemisphere(int port, BIRD_HEMI hem);//, int transmitter);
static void set_rep_and_stream(int port, char repRate);
static void set_pos_angles(int port);//, int transmitter);
static void set_filter(int port, BIRD_FILT filter);
static void set_transmitter(int port, int transmitter);
static void set_autoconfig(int port, int transmitter);
static void set_group(int port);

vjBird::vjBird(vjConfigChunk *c) : vjPosition(c), vjInput(c)
{
  port_id = -1;
  active = 0;
  syncStyle = 1;
  blocking = 0;
  //theTransmitter = 3;
  hemisphere = LOWER_HEM;
  repRate = 'Q';
  baudRate = 38400;
  strncpy(sPort,"/dev/ttyd3", 30);
  InitCorrectionTable();
  deviceAbilities = deviceAbilities & DEVICE_POSITION;
  theData = (vjMatrix*)allocate(3*sizeof(vjMatrix));
}

vjBird::~vjBird()
{
  StopSampling();
    if (theData != NULL)
       getMyMemPool()->deallocate((void*)theData);
}

static void SampleBirds(void* pointer)
{

  struct timeval tv;
  double start_time, stop_time;

   vjBird* devPointer = (vjBird*) pointer;
   for (;;) {

     gettimeofday(&tv,0);
     start_time = (double)tv.tv_sec+ (double)tv.tv_usec / 1000000.0;

     for(int i = 0; i < 60; i++)
	devPointer->Sample();

     gettimeofday(&tv,0);
     stop_time = (double)tv.tv_sec+ (double)tv.tv_usec / 1000000.0;
        cout << 1/((stop_time-start_time) / 60)
           << "  " << endl;


   }

}

int vjBird::StartSampling()
{
   if (myThread == NULL) {
      //int processID;
      //int i;

 // if (theData != NULL)
 //      getMyMemPool()->deallocate((void*)theData);
 // theData = (vjPOS_DATA*)allocate((theTransmitter-1)*3*sizeof(vjPOS_DATA));


  current = 0;
  valid = 1;
  progress = 2;

  port_id = open_port(sPort, baudRate);
  set_blocking(port_id, blocking);
  set_sync(port_id,syncStyle);
  set_group(port_id);
  //set_autoconfig(port_id, theTransmitter);
  //set_transmitter(port_id, theTransmitter);
  set_filter(port_id, filter);
  set_hemisphere(port_id, hemisphere); //, theTransmitter);
  set_pos_angles(port_id); //, theTransmitter);
  //pickBird(theTransmitter,port_id);
  set_rep_and_stream(port_id, repRate);

//  cout << "port_id:" << port_id << endl;


  cout << "vjBird ready to go.." << endl;

  vjBird* devicePtr = this;

  myThread = new vjThread(SampleBirds,(void*)devicePtr,0);

  if ( !myThread->valid() ) {
     return -1; //fail
  }
  else {
    //samplePID = processID;
    return 1;
    }
  }else return -2; // already sampling

}

int vjBird::Sample()
{
     //int i;
     int tmp;

     //for(i=1; i < theTransmitter; i++)
     {
	//int subNum;
        getReading(&theData[progress], port_id);	
	/* XXX:
   Position_Correct(theData[progress].pos[0],
			 theData[progress].pos[1],
			 theData[progress].pos[2]);
   */
     }
	
     lock.acquire();
     tmp = valid;
     valid = progress;
     progress = tmp;
     lock.release();

     return 1;
}

int vjBird::StopSampling()
{
   if (myThread != NULL)
   {
      myThread->kill();
      delete(myThread);
      myThread = NULL;

      sginap(1);
      char   bird_command[4];

      cout << "stopping the flock.." << endl;

      bird_command[0] = 'B';
      write(port_id, bird_command, 1);
      ioctl(port_id, TCFLSH, (char *) 0);
      sginap(5);
      bird_command[0] = 'G';
      write(port_id, bird_command, 1);
      ioctl(port_id, TCFLSH, (char *) 0);
      sleep(2);
      close(port_id);
      port_id = -1;

   }
   return 1;
}

// XXX: Bad Bad Bad
vjMatrix* vjBird::GetPosData( int d) // d is 0 based
{
  return (vjMatrix*)&theData[current];
}

void vjBird::UpdateData()
{
  // swap the indicies for the pointers

  lock.acquire();
  int tmp = current;
  current = valid;
  valid = tmp;
  lock.release();

  return;
}

void vjBird::SetHemisphere(BIRD_HEMI h)
{
  if (active) {
      cerr << "Cannot change the hemisphere\n";
      return;
   }
   hemisphere = h;
}

void vjBird::SetFilters(BIRD_FILT f)
{
  if (active) {
      cerr << "Cannot change filters while active\n";
      return;
  }
  filter = f;
}

void vjBird::SetReportRate(char rRate)
{
  if (active) {
      cerr << "Cannot change report rate while active\n";
      return;
  }
  repRate = rRate;
}

void vjBird::SetSync(int sync)
{
  if (active) {
      cerr << "Cannot change report rate while active\n";
      return;
  }
  syncStyle = sync;
}

void vjBird::SetBlocking(int blVal)
{
  if (active) {
      cerr << "Cannot change report rate while active\n";
      return;
  }
  blocking = blVal;
}

void vjBird::Position_Correct(float&x,float&y,float&z) {
  int xlo,ylo,zlo,xhi,yhi,zhi;
  float a,b,c,a1,b1,c1;

 /*   if(x<caltable.xmin || y<caltable.ymin || z<caltable.zmin ||
        x>=caltable.xmax || y>=caltable.ymax || z>=caltable.zmax){
        cerr << "Point out of range " << x << " " <<  y << " " << z << endl;
        cerr <<"                   "
             <<caltable.xmin<<" "<< caltable.xmax << " " <<  caltable.ymin
             <<caltable.ymax<<" "<< caltable.zmin << " " << caltable.zmax
             << endl;
        return;
    }
*/
    // Find corners
    xlo = (int)(x-caltable.xmin);
    ylo = (int)(y-caltable.ymin);
    zlo = (int)(z-caltable.zmin);
    xhi = xlo+1;
    yhi = ylo+1;
    zhi = zlo+1;
    a1 = 1 - (a = caltable.xmin + xhi - x);
    b1 = 1 - (b = caltable.ymin + yhi - y);
    c1 = 1 - (c = caltable.zmin + zhi - z);

    x = a * b* c*caltable.xloc[xlo][ylo][zlo] +
        a * b*c1*caltable.xloc[xlo][ylo][zhi] +
        a *b1* c*caltable.xloc[xlo][yhi][zlo]+
        a *b1*c1*caltable.xloc[xlo][yhi][zhi]+
        a1* b* c*caltable.xloc[xhi][ylo][zlo]+
        a1* b*c1*caltable.xloc[xhi][ylo][zhi]+
        a1*b1* c*caltable.xloc[xhi][yhi][zlo]+
        a1*b1*c1*caltable.xloc[xhi][yhi][zhi];
    y = a * b* c*caltable.yloc[xlo][ylo][zlo] +
        a * b*c1*caltable.yloc[xlo][ylo][zhi] +
        a *b1* c*caltable.yloc[xlo][yhi][zlo]+
        a *b1*c1*caltable.yloc[xlo][yhi][zhi]+
        a1* b* c*caltable.yloc[xhi][ylo][zlo]+
        a1* b*c1*caltable.yloc[xhi][ylo][zhi]+
        a1*b1* c*caltable.yloc[xhi][yhi][zlo]+
        a1*b1*c1*caltable.yloc[xhi][yhi][zhi];
    z = a * b* c*caltable.zloc[xlo][ylo][zlo] +
        a * b*c1*caltable.zloc[xlo][ylo][zhi] +
        a *b1* c*caltable.zloc[xlo][yhi][zlo]+
        a *b1*c1*caltable.zloc[xlo][yhi][zhi]+
        a1* b* c*caltable.zloc[xhi][ylo][zlo]+
        a1* b*c1*caltable.zloc[xhi][ylo][zhi]+
        a1*b1* c*caltable.zloc[xhi][yhi][zlo]+
        a1*b1*c1*caltable.zloc[xhi][yhi][zhi];
    return;
}

void vjBird::InitCorrectionTable()
{
  int i,j,k, xsize,ysize,zsize;
  float dump;
  ifstream inFile;

  cout << "Initializing calibration table ... ";
  inFile.open("calibration.table");
  if (!inFile)
  {
	cout << "Unable to open calibration.table\n";
        return;
  }

  inFile >> caltable.xmin >> caltable.xmax
         >> caltable.ymin >> caltable.ymax
         >> caltable.zmin >> caltable.zmax;

  xsize = caltable.xmax - caltable.xmin + 1;
  ysize = caltable.ymax - caltable.ymin + 1;
  zsize = caltable.zmax - caltable.zmin + 1;

  for (i = 0; i < xsize; i++)
    for (j = 0; j < ysize; j++)
      for (k = 0; k < zsize; k++)
         {
	    inFile >> dump >> dump >> dump
		   >> caltable.xloc[i][j][k]
		   >> caltable.yloc[i][j][k]
		   >> caltable.zloc[i][j][k];
	 }

   inFile.close();
}


///////////////////////////////////////////////////////////////////
// Local functions to vjBird.cpp
//////////////////////////////////////////////////////////////////
inline int getReading(vjMatrix *data, int port)
{
  char buff[12];
  //char group;
  int  c,i;
  //int addr;

  {
        c=i = 0;
	while(!i && c < 99999) { c++;
	  if((read(port,buff,1) == 1) && (buff[0] & 0x80))
              i = 1;
         }
	while(i != 12 && c < 99999) {
	  c++;
	  i += read(port,&buff[i], 12-i);
          }
	
	//while((read(port,&group,1) == 0) && c < 99999)
	  //c++;

        if (c >= 5000) cout << "timeout" << endl;
        //addr = group;
  } // while(addr != n);
  //cout << "addr: " << addr << endl;

  //vjPOS_DATA *dataPtr = data + addr - 1;

  vjVec3 pos_data, or_data;

  pos_data[0] = rawToFloat(buff[1],buff[0]) * POSITION_RANGE;       // X
  pos_data[1] = rawToFloat(buff[3],buff[2]) * POSITION_RANGE;       // Y
  pos_data[2] = rawToFloat(buff[5],buff[4]) * POSITION_RANGE;       // Z
  or_data[0] = rawToFloat(buff[7],buff[6]) * ANGLE_RANGE;           // rotZ
//  if (data->or[0] < -180.0f)
//     data->or[0] += 360.0f;
  or_data[1] = rawToFloat(buff[9],buff[8]) * ANGLE_RANGE;           // rotY
  or_data[2] = rawToFloat(buff[11],buff[10]) * ANGLE_RANGE;         // rotX

  data->makeZYXEuler(or_data[0], or_data[1], or_data[2]);
  data->setTrans(pos_data[0], pos_data[1], pos_data[2]);

  return 0;  //addr;
}

inline float rawToFloat(char& r1, char& r2)
{
 // return ((float) (((r1 & 0x7f) << 9) | (r2 & 0x7f) << 2) / 0x7fff);
 short int ival1,ival2,val;
  ival1 = r1 & 0x7f;
  ival2 = r2 & 0x7f;
  val = (ival1 << 9) | ival2<<2;
  return ((float)val) / 0x7fff;
}

inline void  pickBird(int birdID, int port)
{
   char buff = 0xF0 + birdID;
   write(port, &buff, 1);
   ioctl(port, TCFLSH, (char*) 0);
   sginap(1);
}

static int open_port(char* serialPort, int baud)
{
 ///////////////////////////////////////////////////////////////////
 // Open and close the port to reset the tracker, then
 // Open the port
 ///////////////////////////////////////////////////////////////////
  int port_id = open(serialPort, O_RDWR | O_NDELAY);
  if (port_id == -1)
   { cerr << "tracker port open failed\n";
    return port_id;
   }
  sleep(2);
  close(port_id);
  port_id = open(serialPort,O_RDWR | O_NDELAY);
  if (port_id == -1)
   { cerr << "tracker port open failed\n";
    return port_id;
   }

 //////////////////////////////////////////////////////////////////
 // Setup the port, current setting is 38400 baud
 //
 //////////////////////////////////////////////////////////////////
    termio port_a;
    ioctl(port_id,TCGETA,&port_a);

    port_a.c_iflag = port_a.c_oflag = port_a.c_lflag = 0;

    switch(baud){
       case 38400:port_a.c_cflag = (B38400 | CS8 | CLOCAL | CREAD);
  		break;
       case 19200:port_a.c_cflag = (B19200 | CS8 | CLOCAL | CREAD);
		break;
       case 9600:port_a.c_cflag = (B9600 | CS8 | CLOCAL | CREAD);
		break;
       case 4800:port_a.c_cflag = (B4800 | CS8 | CLOCAL | CREAD);
		break;
    }

    port_a.c_cflag = (B38400 | CS8 | CLOCAL | CREAD);
    port_a.c_cc[0] = port_a.c_cc[1] = port_a.c_cc[2] = port_a.c_cc[3] = 0;
    port_a.c_cc[4] = port_a.c_cc[5] = 1;

   // Set the new attributes
    ioctl(port_id,TCSETA,&port_a);
    ioctl(port_id,TIOCNOTTY);
    return port_id;
  }

  void set_blocking(int port, int blocking)
  {
 //////////////////////////////////////////////////////////////////
 // Setup a non/blocked port & Flush port
 //////////////////////////////////////////////////////////////////
  static int blockf,nonblock;
  int flags;

  flags = fcntl(port,F_GETFL,0);
  blockf = flags & ~FNDELAY;  // Turn blocking on
  nonblock = flags | FNDELAY; // Turn blocking off

  fcntl(port,F_SETFL,blocking ? blockf : nonblock); // 0 Non Blocked
                                                    // 1 Blocked
  ioctl(port,TCFLSH,2);
  sginap(10);
  char junk[1024];
  read(port, junk, 1024);
  sleep(1);

  }

static void set_sync(int port, int sync)
{
/////////////////////////////////////////////////////////////////
 // Set CRT sync: (manual page 82)
 //   set crt sync
 //   nosync    -   0
 //   > 72Hz    -   1 (type 1)
 //                 2 (type 2)
 /////////////////////////////////////////////////////////////////
  unsigned char buff[4] = {'A', 1};
  buff[1] = sync;
  write(port,buff,2);
  ioctl(port,TCFLSH,0);
}


static void set_hemisphere(int port, BIRD_HEMI hem)//, int transmitter)
{
 /////////////////////////////////////////////////////////////////
 // Set Hemisphere for birds taking input
 //
 //  buff   [1]   [2]
 // Front: 0x00, 0x00
 // Aft  : 0x00, 0x01
 // Upper: 0x0C, 0x01
 // Lower: 0x0C, 0x00
 // Left : 0x06, 0x01
 // Right: 0x06, 0x00
 /////////////////////////////////////////////////////////////////
  char buff[3];
  buff[0] = 'L';
  //for (int i = 1; i < transmitter; i++)
  {
    //pickBird(i,port);
    switch(hem) {
      case FRONT_HEM:
        buff[1] = 0x00;
	buff[2] = 0x00;
        break;
      case AFT_HEM:
        buff[1] = 0x00;
	buff[2] = 0x01;
	break;
      case UPPER_HEM:
        buff[1] = 0x0C;
	buff[2] = 0x01;
	break;
      case LOWER_HEM:
        buff[1] = 0x0C;
	buff[2] = 0x00;
	break;		
      case LEFT_HEM:
        buff[1] = 0x06;
	buff[2] = 0x01;
	break;
      case RIGHT_HEM:
        buff[1] = 0x06;
	buff[2] = 0x00;
	break;
   }
    write(port, buff, 3);
    ioctl(port, TCFLSH, (char *) 0);
    sginap(5);
  }
}

static void set_rep_and_stream(int port, char repRate)
{
 char buff[1];
 /////////////////////////////////////////////////////////////////
 // Set report rate
 //             Q  Every cycle
 //  buff[0] - 'R' Every other bird cycle
 //             S  every 8 cycles
 //             T  every 32 cycles
 /////////////////////////////////////////////////////////////////
  buff[0] = repRate;
  write(port, buff, 1);
  ioctl(port, TCFLSH, (char *) 0);
  sginap(20);

 ////////////////////////////////////////////////////////////////
 // set stream mode
 ////////////////////////////////////////////////////////////////
  buff[0] = '@';
  write(port, buff, 1);
  ioctl(port, TCFLSH, (char *) 0);
  sginap(5);

}

static void set_pos_angles(int port)//, int transmitter)
{
 //////////////////////////////////////////////////////////////////
 // Set Position Angles
 /////////////////////////////////////////////////////////////////
  char buff[1];
  //for (int i = 1; i < transmitter; i++)
  {
    //pickBird(i,port);
    buff[0] = 'Y';
    write(port, buff, 1);
    ioctl(port, TCFLSH, (char *) 0);
    sginap(5);
  }

}

static void set_filter(int port, BIRD_FILT filter)
{
 ///////////////////////////////////////////////////////////////
 // Turn filters on (manual page 48)
 // 0s turn AC NARROW notch filter ON
 //         AC WIDE notch filter ON
 //         DC filter ON
 ///////////////////////////////////////////////////////////////
  char buff[4];
  buff[0] = 'P';
  buff[1] = 0x04;
  buff[2] = 0x00;
  buff[3] = 0;
  write(port,buff,4);
  ioctl(port,TCFLSH,(char*)0);
 // Do I need to sleep here?

}

static void set_transmitter(int port, int transmitter)
{
///////////////////////////////////////////////////////////////
 // Sets up the device for Transmitting (manual page 67)
 // Command (0x30) for Next Transmitter
 ///////////////////////////////////////////////////////////////
  char buff[2];
  buff[0] = (unsigned char) (0x30);
  buff[1] = (unsigned char) transmitter  << 4;
  write(port, buff, 2);
  ioctl(port, TCFLSH,(char*) 0);
  sginap(120);
 }


static void set_autoconfig(int port, int transmitter)
{
 ///////////////////////////////////////////////////////////////
 // FBB AUTO-CONFIGURATION (manual page 60)
 //
 // Must wait 300 milliseconds before and after this command
 ///////////////////////////////////////////////////////////////
  char buff[3];
  buff[0] = 'P';
  buff[1] = 0x32;
  buff[2] = transmitter;  //number of input devices + 1 for transmitter
  write(port, buff,3);
  ioctl(port, TCFLSH, (char*) 0);
  sleep(2);
}

static void set_group(int port)
{
 ////////////////////////////////////////////////////////////////
 // Setup group mode: (manual page 59)
 // 'P' Change Parameter
 // Number 35 (hex 23),
 // Set flag to 1 enabling group mode.
 ////////////////////////////////////////////////////////////////
  char buff[3];
  buff[0] = 'P';
  buff[1] = 0x23;
  buff[2] = 0x00;  // set group mode off
  write(port, buff, 3);
  ioctl(port, TCFLSH, (char *) 0);
  sleep(2);

}
