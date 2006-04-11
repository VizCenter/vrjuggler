/*
 * VRJuggler
 *   Copyright (C) 1997,1998,1999,2000
 *   Iowa State University Research Foundation, Inc.
 *   All Rights Reserved
 *
 * Original Authors:
 *   Allen Bierbaum, Christopher Just,
 *   Patrick Hartling, Kevin Meinert,
 *   Carolina Cruz-Neira, Albert Baker
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * -----------------------------------------------------------------
 * File:          $RCSfile$
 * Date modified: $Date$
 * Version:       $Revision$
 * -----------------------------------------------------------------
 */


#ifndef _CYBER_GLOVE_BASIC_H_
#define _CYBER_GLOVE_BASIC_H_

#include <vjConfig.h>

#ifdef VJ_OS_HPUX
#   include <float.h>
#   include <limits>
#endif

#include <Input/vjGlove/vt_types.h>
#include <Input/vjGlove/vt_glove-types.h>
#include <Input/vjGlove/vt_virtual_hand-types.h>
#include <stdio.h>

struct CYBER_GLOVE_DATA {
  float joints[MAX_SENSOR_GROUPS][MAX_SENSOR_VALUES];
 // matrix4x4 xforms[5][3];
};

/* a note about fingers...
 * the GetJoints and GetDigitXForms both return 2d arrays.  The
 * first index of each of these is a finger, enumerated in vt_glove-types.h
 *    THUMB  = 0
 *    INDEX  = 1
 *    MIDDLE = 2
 *    RING   = 3
 *    PINKY  = 4
 *    CARPUS = 5    (refers to the wrist joints)
 * note that the joints array is 6 items, the digit xforms are 5 items.
 * The Fingers defined above can also be used as the first argument to
 * GetFingerState.
 * Each finger has 4 positions, which are the 2nd index to the array:
 *    MCP    = 0    (first knuckle)
 *    PIP    = 1
 *    DIP    = 2    (2nd knuckle - closest to fingertip
 *    ABDUCT = 3    (lateral spread of each finger)
 * The CARPUS is a little different - you have WRIST_PITCH and WRIST_YAW
 * to work with instead of finger knuckles.
 *
 * Note to those who like pointer math:  returning 2d arrays is troublesome,
 * so the GetJoints and GetDigitXForms actually return float** / matrix4x4**
 * -- that is, pointers to an array of pointers to arrays of objects.
 * But all you have to do is look at the results like this:
 *    float ** joints = glove.GetJoints();
 *    cout << "this joint is the middle finger, 3rd knuckle:" <<
 *         joints[MIDDLE][DIP];
 */
class CyberGloveBasic
{
public:	

     //: Construct a cyber glove
     // initializes hand & opens hand on serport
     //! ARGS:  calFileDir - directory with glove data files,
     //+           e.g. "/home/vr/CAVE/glove"
     //! ARGS: serport - e.g. "/dev/ttyd2".  the named port must
     //+   actually exist, but it's ok if there's no glove attached
     //+   to it - initialization will give a warning, and all values
     //+   will be 0.0 unless set by hand
     //! ARGS: baud - baud rate, set on the back of the glove box.
     //+   Usually 38400.
    CyberGloveBasic (char* calFileDir, char *serport, int baud);


     /** destroys virtualhand
      */
    ~CyberGloveBasic ();


    //: Open the glove
    //! RETURNS: 0 - Already open
    int open();

    int close();

    int sample();

    CYBER_GLOVE_DATA* getData();


     /** An the digit transforms are a [5][3] array of matrix4x4's
       * used when drawing the glove - they represent an unflexed
       * transform to successive joints.  Also used when calulating
       * the fingertip positions.
       */
    matrix4x4** getDigitXForms ();


      /** returns the thumb roll (in radians).
       */
    float getThumbRoll ();

     /** reads the actual surface object file (eg "hires-hand.surf")
      * Better be called before calling GetSurfaceModel...
      * Not necessary if you don't want to actually draw the hand.
      * resolution is 0 (lowres) or 1 (hires)
      * dir is the glove files directory
      * name is the filename of the surface file - usually
      * "lowres-hand.surf" or "hires-hand.surf"
      */
    void readSurfaceModel (int resolution, char *dir, char *name);

      /** returns a pointer to the surface model struct, defined in
       * vt_read_hand_model-types.h, used to draw a glove.
       * This struct basically contains a bunch of GL lists to
       * draw the hand.
       */
    SurfaceModel getSurfaceModel();



      /** the state vec is used in the CAVE code which calculates
       * fingertip positions.
       * indices are TR_AZ, TR_EL, TR_ROLL, in that order.
       */
      /* Note: state_vec & digit_xform are used in the Get[finger]TipPosition
       * functions of cave.glove.c.  That functionality could be stuck
       * into this class, with a function that takes the
       * wand x y z az el roll as args... with a cleaner interface to
       * boot...
       */
    volatile float *getStateVec();

      /** gets unflexed abduction value of the thumb; used when
       * calculating thumbtip position.
       */
    float getThumbUnflexedAbduction();

      /** quick function to determine position of a finger
       * return values:
       * 0 finger open
       * 1 finger closed
       * 2 finger inbetween/relaxed (whatever isn't zero or one)
       */
    int getFingerState (int finger);


  private:
    CYBER_GLOVE_DATA theData;    //[MAX_SENSOR_GROUPS][MAX_GROUP_VALUES];

    /* we're gonna start out with all the old glove stuff still global...*/
    VirtualHand hand;
    CbGlove current_glove;
    CbGlovePrivate current_glove_private;
    volatile float *joints[MAX_SENSOR_GROUPS];
    matrix4x4 *xforms[5];

    int     needToClosePorts;

    // Config params
    // XXX: We should allow the user to specify a config file
    char*   mCalFileDir;          // The directory with the calibration file
    char*   mSerialPort;         // The serial port to use
    int     mBaudRate;           // The baud rate to use

    bool    mItsOpen;	         //: Is the glove open


private:
  /* private member functions */
  /** @name the VirtualHand utility functions
   *  Utility functions provided by Virtual Technologies
   */
   //@{

   // vt_init.cpp
    void vt_init(char *dir, AppDataStruct &app);

    // vt_virtual_hand.cpp
    VirtualHand vt_create_VirtualHand(char *port, int baud,
					 AppDataStruct &app );
    void vt_destroy_VirtualHand(VirtualHand hand);
    void *vt_hand_receiver_map_address(VirtualHand hand);
    VirtualHand vt_virtual_hand_from_glove_id(int glove_id);
    void vt_build_hand_matrices(VirtualHand hand);
    void vt_build_flexed_hand_matrices(VirtualHand hand,
					  matrix4x4 palm_matrix,
				   matrix4x4 finger_matrix[][MAX_SENSOR_GROUPS]);
    void vt_build_hand_pos_orient_matrix(VirtualHand hand,
					    matrix4x4 hand_matrix);
    void vt_request_new_hand_data(VirtualHand hand);
    void vt_update_hand_state(VirtualHand hand);
    float vt_calc_thumb_roll(VirtualHand hand);
    void init_VirtualHand(VirtualHand hand, int glove_index,
		 char calfile[],
		 Boolean visible);

    // vt_read_user_cal.cpp
    int vt_geom_in(FILE *inputfp, UserGeometry user);
    void vt_geom_out( FILE *outputfp, UserGeometry user);
    int vt_read_glove_calibration(FILE *infile, CbGlove glove);
    void vt_write_glove_calibration(FILE *outputfp, CbGlove glove,
				       char *version_string);
    int vt_hand_read(VirtualHand hand, char *filename);
    int vt_hand_write(VirtualHand hand, char *filename, char *version_string);

    // vt_glove.h
    int vt_open_glove_port(char *dev_name, int baudrate);
    void vt_close_glove_port(CbGlove);
    CbGlove vt_create_CbGlove(Boolean connect, ...);
    void vt_destroy_CbGlove(CbGlove glove);
    void vt_glove_set(CbGlove glove);
    CbGlove vt_glove_get(void);
    float vt_get_glove_gain(int finger, int joint);
    void vt_set_glove_gain(int finger, int joint, float gain);
    int vt_get_glove_offset(int finger, int joint);
    void vt_set_glove_offset(int finger, int joint, int offset);
    void vt_unprocess_glove_angles(void);
    unsigned long vt_get_glove_param_flags(void);
    void vt_glove_private_fieldoffset(int finger, int joint,
					 int *offset, int *gain);
    int vt_read_glove_data(void);
    int vt_read_processed_glove_data(void);
    int vt_send_glove_command(int id, ...);
    int vt_send_glove_query(int id, ...);
    void vt_process_glove_data(void);

    int vt_serial_read_bytes(int, unsigned char *, int);
    int vt_serial_read_byte(int portfd);
    int vt_serial_clear_to_terminator(int portfd, int terminator);
    int vt_serial_read_longs(int portfd, void *buffer, int num_longs);
    int vt_serial_write_byte(int portfd, char ch);
    int vt_serial_write_bytes(int portfd, unsigned char buffer[], int numbytes);
    int vt_serial_write_shorts(int portfd, void *buffer, int num_shorts);
    int vt_serial_read_shorts(int portfd, void *buffer, int num_shorts);
    int vt_serial_read_string(int portfd, char *buffer, int length);
    int vt_serial_read_ushort(int portfd);
    char * vt_serial_read_line(int portfd, char *buffer, int length);
    int query_reply_head_check(int portfd, char *query_string);
    int reply_tail_check(CbGlove glove);
    int command_reply_check(CbGlove glove, char command_char);
    int read_and_decode_timestamp(int portfd, unsigned long *timestamp);
    int get_max_num_sensors(CbGlove glove, void *address_arg1);
    CbGlove allocate_CbGlove(void);
    int glove_boot(CbGlove glove);
    int set_baud_rate(CbGlove glove, void *address_arg1);
    int enable_timestamp(CbGlove glove, void *address_arg1);
    int enable_filter(CbGlove glove, void *address_arg1);
    int request_single_sample(CbGlove glove, void *address_arg1);
    int request_sample_stream(CbGlove glove, void *address_arg1);
    int enable_switch_controls_light(CbGlove glove, void *address_arg1);
    int set_sensor_mask(CbGlove glove, void *address_arg1);
    int set_num_values_returned(CbGlove glove, void *address_arg1);
    int set_param_flags(CbGlove glove, void *address_arg1);
    int reboot_CGIU(CbGlove glove, void *address_arg1);

    int glove_ok_query(CbGlove glove, void *address_arg1);
    int get_glove_info(CbGlove glove, void *address_arg1);
    int get_avail_sensor_mask(CbGlove glove, void *address_arg1);
    int get_sensor_mask(CbGlove glove, void *address_arg1);
    int get_num_values_returned(CbGlove glove, void *address_arg1);
    int read_param_flags(CbGlove glove, void *address_arg1);
    int right_hand_glove_query(CbGlove glove, void *address_arg1);
    void abduct_18 (void);

    // vt_serial_io.h
    int vt_serial_open(char *devname, int baudrate);
    int vt_serial_write_string(int portfd, char *str);
    char vt_serial_find_byte(int portfd, char delimiter);
    int vt_serial_flush_in(int portfd);
    void vt_serial_read_all(int portfd);
    int vt_serial_read_int(int portfd);
    int vt_serial_close(int portfd);
    void vt_serial_close_ports(void);
    int vt_serial_match_string(int portfd, char *str);

    // vt_read_hand_model
    void vt_read_hand_model(char infilename[], VirtualHand hand, char dir[]);
    void vt_read_lowres_hand_model(char infilename[], VirtualHand hand);

    // vt_read_object
    loopsegptr vt_loopseg_twin(loopsegptr theloopseg);
    void vt_read_object(FILE *infile, objptr thisobj);
    void vt_calculate_vertex_normals(objptr thisobj);
    void vt_calculate_dihedral_angles(objptr thisobj);
    void vt_calculate_face_normals(objptr thisobj, Boolean clockwise);

};

#endif	/* _CYBERGLOVE_H_ */
