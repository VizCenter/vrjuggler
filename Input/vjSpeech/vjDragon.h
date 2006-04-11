/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998, 1999, 2000 by Iowa State University
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


#ifndef _VJ_DRAGON_H_
#define _VJ_DRAGON_H_


class vjDragon : public vjSpeech
{
public:
	//: Construct using chunk
	vjDragon();

	//: Destructor
	~vjDragon();
	
	virtual std::string		getWord();
	virtual int			getId();
	virtual void			setMode( Mode mode );
	virtual std::list<asdfjlk>&	getList();
	virtual void			config();

	enum Mode
	{
		DICTATE,COMMAND,ON,OFF
	};


	
	class asdfjlk
	{
		std::string word;
		int id;
	};
	
	virtual char* getDeviceName() { return "vjDragon";}

       virtual int startSampling();
       virtual int stopSampling();
       virtual int sample();
       virtual void updateData();

protected:
	std::list<asdfjlk>	_wordQueue;
	Mode				_currentMode;

	// Config chunck sets these.
	std::string			_filename;
	std::string			_dragonIP;
	int					_dragonPort;
	std::string			_user;

protected:
	void				startThread();
	static void			controlLoop(vjDragon& currentInstance);
};

inline int vjDragon::startSampling()
{
    //todo: tell dragon to begin.
}

inline int vjDragon::stopSampling()
{
    //todo: tell dragon to stop sending data.
}

inline int vjDragon::sample()
{
    //todo: recieve all words that dragon has to offer this frame.
}

inline void vjDragon::updateData()
{
    //TODO: find out what the difference between this and Sample() is
}

inline void vjDragon::setMode( Mode mode )
{
	vjSpeech::setMode( mode );
	
	// TODO: tell dragon about the mode change.
}

//TODO: take a config chunck here.
inline void vjDragon::config()
{
	_filename = "somefile.vjs";
	_dragonIP = "129.186.232.70";
	_dragonPort = 5600;
	_user = "groucho";
}


#endif	/* _VJ_DRAGON_H_ */
