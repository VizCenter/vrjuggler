/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998, 1999, 2000, 2001, 2002
 *   by Iowa State University
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
 *
 *************** <auto-copyright.pl END do not edit this line> ***************/
#define CLOCKTICKSPERSEC 18.2   /* DOS tick (IRQ0) frequency */

/*
    Serial (8250) Port Base Addresses on PC compatibles
*/
#define COM1BASE 0x3f8          /* base address of COM1 on PC/PCAT */
#define COM2BASE 0x2f8          /* base address of COM2 on PC/PCAT */
#define COM3BASE 0x3e8          /* base address of COM3 on PC/PCAT */


/*
    RX and TX buffers
*/
#define RXBUFSIZE 0x100             /* size of RX circular buffer */
#define TXBUFSIZE 50                /* size of TX buffer */

/*
    Labels for the Com Ports
*/
#define COM1 0                      /* com port 1 */
#define COM2 1                      /* com port 2 */
#define COM3 2                      /* com port 3 */

/*
    Serial Port Control Register offsets from com_base address
*/
#define MODEMSTATUS     6           /* modem status register */
#define LINESTATUS      5           /* line status register */
#define MODEMCONT       4           /* modem control register */
#define LINECONT        3           /* line control register */
#define INTERIDENT      2           /* Interupt Identification register */
#define INTERENABLE     1           /* interrupt register */
#define DIVISORHIGH     1           /* baud rate divisor reg high */
#define DIVISORLOW      0           /* baud rate divisor reg low */
#define TXBUF           0           /* transmit register */
#define RXBUF           0           /* receive register */

/*
    Serial Port Bit Specifics
*/
#define DLAB        0x80            /* bit 7 of Line Control Reg, DLAB  */
#define DATARDY     0x01            /* bit 0 of Line Status Reg, DR */
#define TXHOLDEMPTY 0x20            /* bit 5 of Line Status Reg, THRE */
#define DTRON       0x01            /* bit 0 of Modem Control Reg, DTR */
#define RTSON       0x02            /* bit 1 of Modem Control Reg, RTS */
#define OUT2        0x08            /* bit 3 of Modem Control Reg, OUT2 */
#define RXERRORMSK  0x0e            /* Rx Error Mask, bits 1,2,3 on */

/*
    UART Interrupt Enable Register
*/
#define RXDATAAVAILINTENABLE    0x01
#define TXHOLDINTENABLE         0x02
#define RXLINESTATUSINTENABLE   0x04
#define MODEMSTATUSINTENABLE    0x08

/*
    Serial Port Return Values
*/
#define NODATAAVAIL     -1
#define RXERRORS        -2
#define RXTIMEOUT       -3
#define RXBUFOVERRUN    -4
#define TXBUFOVERRUN    -5
#define TXNOTEMPTY      -6
#define RXPHASEERROR    -7
#define TXBUFFERFULL    -8

/*
    UART Identification Register definition
    ....lsb = 0 implies interrupt pending = TRUE
*/
#define RXLINESTATUS    6
#define RXDATAAVAIL     4
#define TXEMPTY         2
#define MODEMSTATUSCHG  0

/*
    Serial Port Configuration Constants
*/
#define STOP_WORDLEN_PARITY 0x3     /* 1 start, 8 data, 1 stop, no parity */

/*
    Interrupt Definitions
*/
#define COM1INTERRUPT   IRQ4_VEC    /* vector # for COM1 */
#define COM2INTERRUPT   IRQ3_VEC    /* vector # for COM2 */
#define COM3INTERRUPT   IRQ5_VEC    /* vector # for COM3 */
#define COM4INTERRUPT   IRQ2_VEC    /* vector # for COM4 */

#define DISABLESERIALMSK IRQ3 + IRQ4  /* MSK to disable serial Interrupts on 8259 */

/*
    External Global Definition
*/
extern long baud;                    /* holds the current baud rate */
extern short baudspeedbits;          /* holds the baud speed in bits */
extern short baudspeedbittable[];    /* the baud speed in bit table */
extern short comport;                /* holds the comport # */
extern short phaseerror_count;       /* holds the phase errors */
extern short rxerrors;               /* holds the rx line errors */
extern long baudratetable[];         /* holds the baud rate selections */
extern short serialconfigsaved;          /* indicates config saved */
extern short rs232tofbbaddr;         /* RS232 to FBB address */
extern char * sys_com_port[];        /* comport names */

/*
    Prototypes
*/
#ifdef KNR

void clear_rx();
int configserialport();
int get_serial_char();
int get_serial_record();
void restoreserialconfig();
int saveserialconfig();
int send_serial_cmd();
int send_serial_char();
int waitforchar();
int waitforphase();

#else

void clear_rx(void);
int configserialport(void);
int get_serial_char(void);
int get_serial_record(unsigned char * rxbuf, short recsize, short outputmode);
void restoreserialconfig(void);
int saveserialconfig(void);
int send_serial_cmd(unsigned char * cmd, short cmdsize);
int send_serial_char(unsigned char chr);
int waitforchar(void);
int waitforphase(void);


#endif


#endif /* SERIAL */



