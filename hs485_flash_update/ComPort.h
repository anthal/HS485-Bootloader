/**Version 1.0
 * Hinweise zur Programmierung und zum Protokoll finden sie in der beiliegenden Dokumentation
 * Beachten Sie auch die beiliegenden Nutzungsbedingungen und den Haftungsausschluss
 *
 * Copyright (c) 2005 by ELV Elektronik AG,
 * Maiburger Str. 32-36 D-26789 Leer, Germany
 * All rights reserved.
 */

// ComPort.h: Schnittstelle für die Klasse ComPort.
//
//////////////////////////////////////////////////////////////////////

#if !defined __COMPORT_H_
#define __COMPORT_H_

#include <windows.h>
#include "com.h"


class ComPort : public Com
{
public:
	bool EscapeCommFunction(DWORD dwFunc);	
	bool read(unsigned char *buffer, DWORD size);
	bool write(unsigned char *buffer, DWORD size);
	bool close();
	bool flush();
	bool setTimeOut(DWORD timeOut);
	bool open(LPCTSTR port, DWORD baudRate, DWORD timeOut);
	bool GetCommState(DCB *dcb);
	bool SetCommState(DCB *dcb);

	bool isConnected();
	bool isPortAvailable(int iComPort);
	ComPort();
	~ComPort();
protected:
	HANDLE m_hHandle;	
};

#endif 
