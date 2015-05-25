/**Version 1.0
 * Hinweise zur Programmierung und zum Protokoll finden sie in der beiliegenden Dokumentation
 * Beachten Sie auch die beiliegenden Nutzungsbedingungen und den Haftungsausschluss
 *
 * Copyright (c) 2005 by ELV Elektronik AG,
 * Maiburger Str. 32-36 D-26789 Leer, Germany
 * All rights reserved.
 */

// ComPort.cpp: Implementierung der Klasse ComPort.
//
//////////////////////////////////////////////////////////////////////

#include "ComPort.h"
#include <stdio.h>

ComPort::ComPort()
{
	m_hHandle = INVALID_HANDLE_VALUE;
}

ComPort::~ComPort()
{
	if (m_hHandle != INVALID_HANDLE_VALUE)
		CloseHandle (m_hHandle);
}

bool ComPort::open(LPCTSTR port, DWORD baudRate, DWORD timeOut)
{
	// Make sure that channel is closed
	if (m_hHandle != INVALID_HANDLE_VALUE)
		return false;

	// Open com port
	m_hHandle = CreateFile(port, GENERIC_READ | GENERIC_WRITE, 0, NULL,	OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (m_hHandle == INVALID_HANDLE_VALUE)
		return false;

	// Communication buffers
	if (!SetupComm(m_hHandle, 1024, 1024))
	{
		CloseHandle(m_hHandle);
		return false;
	}

	// Set up the serial communications device
	DCB dcb;

	ZeroMemory (&dcb, sizeof(DCB));
	dcb.DCBlength = sizeof(DCB);
	dcb.BaudRate = baudRate;
	dcb.fBinary = 1;
	dcb.fDtrControl = DTR_CONTROL_DISABLE;
	dcb.fRtsControl = RTS_CONTROL_DISABLE;
	dcb.ByteSize = 8;
	dcb.Parity = EVENPARITY;
	dcb.StopBits = ONESTOPBIT;

	if (!::SetCommState(m_hHandle, &dcb))
	{
		CloseHandle(m_hHandle);
		return false;
	}

	if (!setTimeOut(timeOut))
	{
		CloseHandle(m_hHandle);
		return false;
	}

	return true;
}

bool ComPort::setTimeOut(DWORD timeOut)
{
	COMMTIMEOUTS ctmo;
	ZeroMemory (&ctmo, sizeof(COMMTIMEOUTS));
	ctmo.ReadIntervalTimeout = timeOut;
	ctmo.ReadTotalTimeoutMultiplier = timeOut;
	ctmo.ReadTotalTimeoutConstant = timeOut;

	return SetCommTimeouts(m_hHandle, &ctmo);
}

bool ComPort::flush()
{
	if (m_hHandle == INVALID_HANDLE_VALUE)
		return false;

	if (! PurgeComm(m_hHandle, PURGE_TXCLEAR|PURGE_RXCLEAR))
		return false;

	return true;
}

bool ComPort::close()
{
	if (m_hHandle != INVALID_HANDLE_VALUE)
		CloseHandle (m_hHandle);

	m_hHandle = INVALID_HANDLE_VALUE;

	return true;
}

bool ComPort::write(unsigned char *buffer, DWORD size)
{
	// Return if com port handler is not valid
	if (m_hHandle == INVALID_HANDLE_VALUE)
		return false;
	
	DWORD written;

	if (!WriteFile (m_hHandle, buffer, size, &written, NULL))
		return false;

	if (written != size)
		return false;

	return true;
}

bool ComPort::read(unsigned char *buffer, DWORD size)
{
	// Return if com port handler is not valid
	if (m_hHandle == INVALID_HANDLE_VALUE)
		return false;

	DWORD read;
	
	if (!ReadFile(m_hHandle, buffer, size, &read, NULL))
		return false;

	return (read == size);
}

bool ComPort::isPortAvailable(int iComPort)
{
	char strPort[20];
	HANDLE  hPort = NULL;
	int nSize(0), nPort(0);
	DWORD dwErr(0);

	// Abprüfen des Ports auf Verfügbarkeit
	sprintf(strPort,"\\\\.\\COM%d", iComPort);
	hPort = CreateFile (strPort,        // Pointer to the name of the port
					  GENERIC_READ | GENERIC_WRITE,
									  // Access (read-write) mode
					  0,              // Share mode
					  NULL,           // Pointer to the security attribute
					  OPEN_EXISTING,  // How to open the serial port
					  0,              // Port attributes
					  NULL);          // Handle to port with attribute to copy

	if (hPort == INVALID_HANDLE_VALUE)  // Invalid Porthandle, port NOT available
	{
		dwErr = GetLastError();
		if (dwErr == ERROR_ACCESS_DENIED || dwErr == ERROR_GEN_FAILURE)
			return TRUE;   // Port exists but not available
		else
			return FALSE;  //Port not exists
	}
	else // Valid PortHandle
	{
		CloseHandle(hPort); // Port wieder freimachen
		return TRUE;  // Port exists
	}
}

bool ComPort::EscapeCommFunction(DWORD dwFunc)
{
	return ::EscapeCommFunction(m_hHandle,dwFunc);
}

bool ComPort::isConnected()
{
	if(m_hHandle == INVALID_HANDLE_VALUE)
		return false;
	return true;
}

bool ComPort::GetCommState(DCB *dcb)
{
	return ::GetCommState(m_hHandle,dcb);
}

bool ComPort::SetCommState(DCB *dcb)
{
	return ::SetCommState(m_hHandle,dcb);
}
