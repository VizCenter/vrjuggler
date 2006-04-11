#include <vjConfig.h>
#include <Input/vjKeyboard/vjXWinKeyboard.h>
#include <Kernel/vjDebug.h>

//: Constructor
vjXWinKeyboard::vjXWinKeyboard(vjConfigChunk *c) : vjInput(c), vjKeyboard(c)
{
    cout << "     vjKeyboard::vjKeyboard(vjConfigChunk*c) " << endl;
    myThread = NULL;

    oldMouseX = 0; oldMouseY = 0;

    int i;
    for(i =0; i < 256; i++)
         m_realkeys[i] = m_keys[i] = 0;
    m_realkeys[0] = m_keys[0] = 1;

    // Get size and position
    m_width = (int)c->getProperty("width");
    m_height = (int)c->getProperty("height");
    if (m_width == 0) m_width = 400;
    if (m_height == 0) m_height = 400;

    m_x = c->getProperty("origin", 0);
    m_y = c->getProperty("origin", 1);

    m_mouse_sensitivity = c->getProperty("msens");
    if (0 == m_mouse_sensitivity)
       m_mouse_sensitivity = 0.5;

    vjDEBUG(1) << "Mouse Sensititivty: " << m_mouse_sensitivity << endl << vjDEBUG_FLUSH;
}

int vjXWinKeyboard::StartSampling()
{

   if (myThread == NULL)
   {
      int i;

      resetIndexes();

      m_display = XOpenDisplay(NULL);
      if (m_display == NULL)
      {
         cerr << "vjKeyboard::StartSampling() : failed to open display" << endl;
         return 0;
      }
      m_screen = DefaultScreen(m_display);

      XVisualInfo vTemplate;
      long vMask = VisualScreenMask;
      vTemplate.screen = m_screen;
      int nVisuals;

      m_visual = XGetVisualInfo( m_display, vMask, &vTemplate, &nVisuals);
      XVisualInfo* p_visinfo;
      for (i = 0, p_visinfo = m_visual; i < nVisuals; i++, p_visinfo++)
      {
         if (p_visinfo->depth > 8)
         {
            m_visual = p_visinfo;
            break;
         }
      }

      if (i == nVisuals)
      {
         cerr << "vjKeyboard::StartSampling() : find visual failed" << endl;
         return 0;
      }

      m_swa.colormap = XCreateColormap(m_display,
                                       RootWindow(m_display,m_visual->screen),
                                       m_visual->visual, AllocNone);
      m_swa.border_pixel = WhitePixel(m_display,m_screen);
      m_swa.event_mask = ExposureMask | StructureNotifyMask | KeyPressMask;
      m_swa.background_pixel = BlackPixel(m_display,m_screen);
      CheckGeometry();

      m_window = CreateWindow ( DefaultRootWindow(m_display) ,
                                1, BlackPixel(m_display,m_screen),
                                WhitePixel(m_display,m_screen), ExposureMask |
                                StructureNotifyMask |
                                KeyPressMask | KeyReleaseMask | ButtonPressMask |
                                ButtonReleaseMask | ButtonMotionMask | PointerMotionMask);
      SetHints(m_window, instName , "VJm_keys" , "VJKeyboard2", "VJInputD" );
      XSelectInput(m_display,m_window, KeyPressMask | KeyReleaseMask | ButtonPressMask |
                   ButtonReleaseMask);
      XMapWindow(m_display, m_window);
      XFlush(m_display);
      XRaiseWindow(m_display,m_window);
      XClearWindow(m_display,m_window);    // Try to clear the background


      vjDEBUG(0) << "vjKeyboard::StartSampling() : ready to go.." << endl << vjDEBUG_FLUSH;


      myThread = (vjThread *) 1;
      return 1;

   }
   else
      return 0; // already sampling
}

int vjXWinKeyboard::OnlyModifier(int mod)
{
  switch (mod) {
     case VJKEY_NONE:
        return (!m_keys[VJKEY_SHIFT] && !m_keys[VJKEY_CTRL] && !m_keys[VJKEY_ALT]);
     case VJKEY_SHIFT:
        return (m_keys[VJKEY_SHIFT] && !m_keys[VJKEY_CTRL] && !m_keys[VJKEY_ALT]);
     case VJKEY_CTRL:
        return (!m_keys[VJKEY_SHIFT] && m_keys[VJKEY_CTRL] && !m_keys[VJKEY_ALT]);
     case VJKEY_ALT:
        return (!m_keys[VJKEY_SHIFT] && !m_keys[VJKEY_CTRL] && m_keys[VJKEY_ALT]);
     default:
       vjASSERT(false);
       return 0;
  }
}

void vjXWinKeyboard::UpdateData()
{
  int i;
  UpdKeys();
}


void vjXWinKeyboard::UpdKeys()
{
   XEvent event;
   KeySym key;
   Window win1, win2;
   int wX, wY, rootX, rootY;
   unsigned int mask;

   for(int i = 0; i < 256; i++)
      m_keys[i] = m_realkeys[i];

   while(XCheckWindowEvent(m_display,m_window,KeyPressMask |
		KeyReleaseMask | ButtonPressMask | ButtonReleaseMask, &event))
   {
      switch (event.type)
      {
      case KeyPress:
         key = XLookupKeysym((XKeyEvent*)&event,0);
         m_realkeys[XKeyTovjKey(key)] = 1;
         m_keys[XKeyTovjKey(key)] += 1;
         vjDEBUG(4) << "KeyPress:  " << hex << key
                    << " state:" << ((XKeyEvent*)&event)->state << " ==> " << XKeyTovjKey(key) << endl << vjDEBUG_FLUSH;
         break;
      case KeyRelease:
         key = XLookupKeysym((XKeyEvent*)&event,0);
         m_realkeys[XKeyTovjKey(key)] = 0;
         vjDEBUG(4) << "KeyRelease:" << hex << key
                    << " state:" << ((XKeyEvent*)&event)->state << " ==> " << XKeyTovjKey(key) << endl << vjDEBUG_FLUSH;
         break;
      }
   }


   XQueryPointer(m_display, m_window, &win1, &win2, &rootX, &rootY,
		   &wX, &wY, &mask);
	

   int delta_x = (rootX - oldMouseX);
   int delta_y = (rootY - oldMouseY);

   if (delta_x > 0)
      m_keys[VJMOUSE_POSX] = delta_x * m_mouse_sensitivity;
   else
      m_keys[VJMOUSE_POSX] = 0;

   if (delta_x < 0)
      m_keys[VJMOUSE_NEGX] = -delta_x * m_mouse_sensitivity;
   else
      m_keys[VJMOUSE_NEGX] = 0;

   if (delta_y > 0)     // Mouse moving down
      m_keys[VJMOUSE_POSY] = delta_y * m_mouse_sensitivity;
   else
      m_keys[VJMOUSE_POSY] = 0;

   if (delta_y < 0)     // Mouse moving up
      m_keys[VJMOUSE_NEGY] = -delta_y * m_mouse_sensitivity;
   else
      m_keys[VJMOUSE_NEGY] = 0;

  oldMouseX = rootX; oldMouseY = rootY;

   m_keys[VJMBUTTON1] = ((mask | Button1Mask) == mask) ? 1 : 0;
   m_keys[VJMBUTTON2] = ((mask | Button2Mask) == mask) ? 1 : 0;
   m_keys[VJMBUTTON3] = ((mask | Button3Mask) == mask) ? 1 : 0;

}

int vjXWinKeyboard::StopSampling()
{
  if (myThread != NULL)
  {
    myThread = NULL;
    cout << "Stoppping Keyboard.." << endl;
    sleep(1);


    XDestroyWindow(m_display,m_window);
    XFree(m_visual);
    XCloseDisplay((Display*) m_display);
  }

  return 1;
}

int vjXWinKeyboard::XKeyTovjKey(KeySym xKey)
{
   switch (xKey)
   {
   case XK_Up        : return VJKEY_UP;
   case XK_Down      : return VJKEY_DOWN;
   case XK_Left      : return VJKEY_LEFT;
   case XK_Right     : return VJKEY_RIGHT;
   case XK_Control_L : return VJKEY_CTRL;
   case XK_Control_R : return VJKEY_CTRL;
   case XK_Shift_L   : return VJKEY_SHIFT;
   case XK_Shift_R   : return VJKEY_SHIFT;
   case XK_Alt_L     : return VJKEY_ALT;
   case XK_Alt_R     : return VJKEY_ALT;

      // Map all number keys
      // Note we map keypad and normal keys making no distinction
   case XK_1            : return VJKEY_1;
   case XK_KP_End       : return VJKEY_1;
   case XK_2            : return VJKEY_2;
   case XK_KP_Down      : return VJKEY_2;
   case XK_3            : return VJKEY_3;
   case XK_KP_Page_Down : return VJKEY_3;
   case XK_4            : return VJKEY_4;
   case XK_KP_Left      : return VJKEY_4;
   case XK_5            : return VJKEY_5;
   case XK_KP_Begin     : return VJKEY_5;
   case XK_6            : return VJKEY_6;
   case XK_KP_Right     : return VJKEY_6;
   case XK_7            : return VJKEY_7;
   case XK_KP_Home      : return VJKEY_7;
   case XK_8            : return VJKEY_8;
   case XK_KP_Up        : return VJKEY_8;
   case XK_9            : return VJKEY_9;
   case XK_KP_Page_Up   : return VJKEY_9;
   case XK_0            : return VJKEY_0;
   case XK_KP_Insert    : return VJKEY_0;

   case XK_A         : return VJKEY_A;
   case XK_B         : return VJKEY_B;
   case XK_C         : return VJKEY_C;
   case XK_D         : return VJKEY_D;
   case XK_E         : return VJKEY_E;
   case XK_F         : return VJKEY_F;
   case XK_G         : return VJKEY_G;
   case XK_H         : return VJKEY_H;
   case XK_I         : return VJKEY_I;
   case XK_J         : return VJKEY_J;
   case XK_K         : return VJKEY_K;
   case XK_L         : return VJKEY_L;
   case XK_M         : return VJKEY_M;
   case XK_N         : return VJKEY_N;
   case XK_O         : return VJKEY_O;
   case XK_P         : return VJKEY_P;
   case XK_Q         : return VJKEY_Q;
   case XK_R         : return VJKEY_R;
   case XK_S         : return VJKEY_S;
   case XK_T         : return VJKEY_T;
   case XK_U         : return VJKEY_U;
   case XK_V         : return VJKEY_V;
   case XK_W         : return VJKEY_W;
   case XK_X         : return VJKEY_X;
   case XK_Y         : return VJKEY_Y;
   case XK_Z         : return VJKEY_Z;
   case XK_a         : return VJKEY_A;
   case XK_b         : return VJKEY_B;
   case XK_c         : return VJKEY_C;
   case XK_d         : return VJKEY_D;
   case XK_e         : return VJKEY_E;
   case XK_f         : return VJKEY_F;
   case XK_g         : return VJKEY_G;
   case XK_h         : return VJKEY_H;
   case XK_i         : return VJKEY_I;
   case XK_j         : return VJKEY_J;
   case XK_k         : return VJKEY_K;
   case XK_l         : return VJKEY_L;
   case XK_m         : return VJKEY_M;
   case XK_n         : return VJKEY_N;
   case XK_o         : return VJKEY_O;
   case XK_p         : return VJKEY_P;
   case XK_q         : return VJKEY_Q;
   case XK_r         : return VJKEY_R;
   case XK_s         : return VJKEY_S;
   case XK_t         : return VJKEY_T;
   case XK_u         : return VJKEY_U;
   case XK_v         : return VJKEY_V;
   case XK_w         : return VJKEY_W;
   case XK_x         : return VJKEY_X;
   case XK_y         : return VJKEY_Y;
   case XK_z         : return VJKEY_Z;
   case XK_Escape    : return VJKEY_ESC;
   default: return 255;
   }

}

/*****************************************************************/
/*****************************************************************/
/***********************X WINDOW STUFF****************************/
/*****************************************************************/
/*****************************************************************/
/* Sets basic window manager hints for a window. */
void vjXWinKeyboard::SetHints(Window window,
    char*  window_name,
    char*  icon_name,
    char*  class_name,
    char*  class_type)

{
    XTextProperty  w_name;
    XTextProperty  i_name;
    XSizeHints     sizehints;
    XWMHints       wmhints;
    XClassHint     classhints;
    int            status;

    /*
     * Generate window and icon names.
     */
	status = XStringListToTextProperty(&window_name,
			1,	/* 1 string to convert */
			&w_name);

	status = XStringListToTextProperty(&icon_name,
			1,	/* 1 string to convert */
			&i_name);

    sizehints.x           = m_x;        /* -- Obsolete in R4 */
    sizehints.y           = m_y;        /* -- Obsolete in R4 */
    sizehints.width       = m_width;    /* -- Obsolete in R4 */
    sizehints.height      = m_height;   /* -- Obsolete in R4 */
    sizehints.base_width  = m_width;    /* -- New in R4 */
    sizehints.base_height = m_height;   /* -- New in R4 */


    /* Set up flags for all the fields we've filled in. */
    sizehints.flags = USPosition | USSize | PBaseSize;


    /*   assume the window starts in "normal" (rather than
     *    iconic) state and wants keyboard input.
     */
    wmhints.initial_state = NormalState;
    wmhints.input         = True;
    wmhints.flags         = StateHint | InputHint;


    /* Fill in class name. */
    classhints.res_name  = class_name;
    classhints.res_class = class_type;

    XSetWMProperties(m_display, window,
        &w_name,
        &i_name,
        //argv, argc, /* Note reversed order. */
        NULL,0,
	&sizehints,
        &wmhints,
        &classhints);

	XFree(w_name.value);
	XFree(i_name.value);

}

Window vjXWinKeyboard::CreateWindow (Window parent, unsigned int border, unsigned long
     fore, unsigned long back, unsigned long event_mask)
{
  Window window;
  //UnUsed// XSetWindowAttributes attributes;
  //UnUsed// unsigned long attribute_mask;

  // set up attributes
  //UnUsed// attributes.background_pixel = back;
  //UnUsed// attributes.border_pixel = fore;
  //UnUsed// attributes.event_mask = event_mask;
  //UnUsed// attribute_mask = CWBackPixel | CWBorderPixel | CWEventMask;

  // create it
  window = XCreateWindow( m_display, parent,
           m_x,m_y,m_width,m_height, border,
           m_visual->depth, /*depth*/
           InputOutput, /*window class*/
           m_visual->visual,CWBorderPixel|CWColormap|CWEventMask, &m_swa);

  return window;
} /*CreateWindow*/

void vjXWinKeyboard::CheckGeometry()
{
  char* geo_string;
  int status;
  int screen_width, screen_height;

  geo_string = CheckArgs( "-geom" );
  if (geo_string != (char*) NULL) {
    status = XParseGeometry(geo_string,&m_x,&m_y,&m_width,&m_height);
    if (status & XNegative) {
       screen_width = DisplayWidth(m_display,m_visual->screen);
       m_x = screen_width - m_width + m_x;
    }
    if (status & YNegative) {
       screen_height = DisplayHeight(m_display, m_visual->screen);
       m_y = screen_height - m_height + m_y;
    }
  }
} /*CheckGeometry*/

char* vjXWinKeyboard::CheckArgs(char* look_for)
{
  //Unused//int i, l;

 /* l = strlen(look_for);
  i = 1;
  while (i < argc)
  { if (strncmp(argv[i], look_for, l) == 0)
   {i++;

    if (i < argc) {
       return argv[i];
    } else {
      cerr << "ERROR: Usage is:\n" << look_for << " value\n";
    }
   }
   i++;
  }*/
  return ((char*) NULL);
} /*CheckArgs*/

int vjXWinKeyboard::FilterEvent( XEvent* event, int want_exposes,
		  int width, int height)
{
    int status = 1;
    if (XFilterEvent( event, (Window)NULL ) )
    {
      return 0;
    }
    switch (event->type) {
       case Expose:
         if (!want_exposes ) {
	   if (event->xexpose.count != 0) {
	     status = 0;
	     }
	 }
	 break;
        case ConfigureNotify:
	  if ((width == event->xconfigure.width ) && (height == event->xconfigure.height))
	  {  status = 0; }
	  break;
	case MappingNotify:
	  XRefreshKeyboardMapping ( (XMappingEvent*) event );
	  status = 0;
	  break;
	default: ;
     }
     return status;
}
