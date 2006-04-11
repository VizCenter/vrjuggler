#include <Input/vjSim/vjSimPosition.h>

vjSimPosition::vjSimPosition(vjConfigChunk* chunk)
   : vjPosition(chunk), vjSimInput(chunk), vjInput(chunk)
{
   mDTrans = chunk->getProperty("dtrans");
   mDRot   = chunk->getProperty("drot");

   // Create keypairs
   for(int i=0;i<NUM_POS_CONTROLS;i++)
   {
      vjKeyModPair key_pair;
      key_pair.mKey = (int)chunk->getProperty("keys",i);
      key_pair.mModifier = (int)chunk->getProperty("modKeys",i);
      mSimKeys[i] = key_pair;
   }

      // Set initial position
   float x_pos = chunk->getProperty("initialPos",0);
   float y_pos = chunk->getProperty("initialPos",1);
   float z_pos = chunk->getProperty("initialPos",2);
   float x_rot = chunk->getProperty("initialRot",0);
   float y_rot = chunk->getProperty("initialRot",1);
   float z_rot = chunk->getProperty("initialRot",2);

   if((x_pos != 0.0f) || (y_pos != 0.0f) || (z_pos != 0.0f))
      mPos.makeTrans(x_pos, y_pos, z_pos);
   if((x_rot != 0.0f) || (y_rot != 0.0f) || (z_rot != 0.0f))
      mPos.postXYZEuler(mPos, x_rot, y_rot, z_rot);
}


void vjSimPosition::UpdateData()
{
   int amt = 0;      // Number of times key pressed
                     // Used to keep from calling checkKey twice on success
      // NOTE: Could have implemented using side effects of assignment
      //       and used less lines, but this is more explicit
   amt = checkKeyPair(mSimKeys[FORWARD]);
   if(amt)
      MoveFor( 1 * amt);
   amt = checkKeyPair(mSimKeys[BACK]);
   if(amt)
      MoveFor( -1 * amt);
   amt = checkKeyPair(mSimKeys[LEFT]);
   if(amt)
      MoveLeft( 1 * amt);
   amt = checkKeyPair(mSimKeys[RIGHT]);
   if(amt)
      MoveLeft( -1 * amt);
   amt = checkKeyPair(mSimKeys[UP]);
   if(amt)
      MoveUp ( 1 * amt);
   amt = checkKeyPair(mSimKeys[DOWN]);
   if(amt)
      MoveUp (-1 * amt);

   amt = checkKeyPair(mSimKeys[ROTR]);
   if(amt)
      RotLeft( -1 *amt);
   amt = checkKeyPair(mSimKeys[ROTL]);
   if(amt)
      RotLeft( 1  * amt);
   amt = checkKeyPair(mSimKeys[ROTU]);
   if(amt)
      RotUp( 1 * amt);
   amt = checkKeyPair(mSimKeys[ROTD]);
   if(amt)
      RotUp( -1 * amt);
   amt = checkKeyPair(mSimKeys[ROT_ROLL_CCW]);
   if(amt)
      RotRollCCW( 1 * amt);
   amt = checkKeyPair(mSimKeys[ROT_ROLL_CW]);
   if(amt)
      RotRollCCW( -1 * amt);
}


// Move forward the given amount on position data n
// Forward is in th -Z direction
void vjSimPosition::MoveFor(const float amt)
{
   mPos.postTrans(mPos, 0.0, 0.0, -amt*mDTrans);
}

// Move left the given amount on position data n
// Left is -X dir
void vjSimPosition::MoveLeft(const float amt)
{
    mPos.postTrans(mPos, -amt*mDTrans, 0.0, 0.0);
}

// Move up the given amount on position data n
// Up is in th +Y dir
void vjSimPosition::MoveUp(const float amt)
{
  mPos.postTrans(mPos, 0.0, amt*mDTrans, 0.0);
}

// Pitch up - rot +x axis
void vjSimPosition::RotUp(const float amt)
{
   static vjVec3 x_axis(1.0,0.0,0.0);
   mPos.postRot(mPos, amt*mDRot, x_axis);
}

// Yaw left - rot +Y axis
void vjSimPosition::RotLeft(const float amt)
{
   static vjVec3 y_axis(0.0, 1.0, 0.0);
   mPos.postRot(mPos, amt*mDRot, y_axis);
}

// Roll Left - rot -z axis
void vjSimPosition::RotRollCCW(const float amt)
{
   static vjVec3 neg_z_axis(0.0, 0.0, -1.0);
   mPos.postRot(mPos, amt*mDRot, neg_z_axis);
}

