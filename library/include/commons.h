/*
    MIDI Sequencer C++ library 
    Copyright (C) 2006-2008, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along 
    with this program; if not, write to the Free Software Foundation, Inc., 
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.    
*/

#ifndef COMMONS_H_
#define COMMONS_H_

#include <string>
#include <stdexcept>
#include <qglobal.h>
#include <qstring.h>
#include <qapplication.h>

extern "C" {
#include <alsa/asoundlib.h>
}

namespace ALSA 
{
namespace Sequencer 
{

typedef Q_UINT8 MidiByte;  

class FatalError : public std::runtime_error
{
public:	
	FatalError(std::string const& s, int rc) :
		runtime_error(s), errCode(rc) 
	{}

	const QString qstrError() 
	{
		return QString(snd_strerror(errCode));
	}
	
	const std::string strError()
	{
		return std::string(snd_strerror(errCode));
	}
	
	int code() { return errCode; }
	
private:
	int errCode;
};

inline int checkErrorAndThrow(int rc, const char *where)
{
	if (rc < 0) {
		//qDebug("Error=%d (%s)\nlocation: %s", rc, snd_strerror(rc), where);
		throw new FatalError(std::string(where), rc);
	}
	return rc;
}

#define CHECK_ERROR(x) (checkErrorAndThrow((x),__PRETTY_FUNCTION__))

}
}

#endif /*COMMONS_H_*/
