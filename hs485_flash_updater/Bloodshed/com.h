/**Version 1.0
 * Hinweise zur Programmierung und zum Protokoll finden sie in der beiliegenden Dokumentation
 * Beachten Sie auch die beiliegenden Nutzungsbedingungen und den Haftungsausschluss
 *
 * Copyright (c) 2005 by ELV Elektronik AG,
 * Maiburger Str. 32-36 D-26789 Leer, Germany
 * All rights reserved.
 */

// Com.h: Schnittstelle für die abstrakte Klasse Com.
//
//////////////////////////////////////////////////////////////////////

#if !defined __COM_H_
#define __COM_H_

#include <windows.h>

class Com 
{
public:
	virtual bool EscapeCommFunction(DWORD dwFunc)=0;	
	virtual bool read(unsigned char *buffer, DWORD size)=0;
	virtual bool write(unsigned char *buffer, DWORD size)=0;
	virtual bool close()=0;
	virtual bool flush()=0;
	virtual bool setTimeOut(DWORD timeOut)=0;
	virtual bool open(LPCTSTR port, DWORD baudRate, DWORD timeOut)=0;	
	
	virtual bool isConnected()=0;
	Com();
	virtual ~Com() {};
};

#endif
