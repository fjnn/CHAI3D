/*
http://xanthium.in/Serial-Port-Programming-using-Win32-API
https://github.com/xanthium-enterprises/Serial-Programming-Win32API-C/blob/master/USB2SERIAL_Read/Reciever%20(PC%20Side)/USB2SERIAL_Read_W32.c
*/

#include "chai3d.h"
#include <Windows.h>
#include <stdio.h>
#include <string>
#include "Serial.h"

using namespace std;

Serial::Serial(int portNumber, int baudRate, int byteSize, int stopBits, int parity)
{
	this->comPortName = "\\\\.\\COM" + to_string(portNumber); // Name of the Serial port(May Change) to be opened,  "\\\\.\\COM7"
	std::cout << comPortName << std::endl;
	this->baudRate = baudRate;
	this->byteSize = byteSize;
	this->stopBits = stopBits;
	this->parity = parity;
}

Serial::~Serial()
{
	this->close();
}

bool Serial::open()
{
	std::cout << "Opening COM PORT " << this->comPortName << std::endl;
	BOOL Status = FALSE;
	/*---------------------------------- Opening the Serial Port -------------------------------------------*/
	this->hComm = CreateFile(this->comPortName.c_str(),                  // Name of the Port to be Opened
		GENERIC_READ | GENERIC_WRITE, // Read/Write Access
		0,                            // No Sharing, ports cant be shared
		NULL,                         // No Security
		OPEN_EXISTING,                // Open existing port only
		0,                            // Non Overlapped I/O
		NULL);                        // Null for Comm Devices

	if (hComm == INVALID_HANDLE_VALUE) {
		cout << "\n    Error! - Port " << this->comPortName << " can't be opened\n";
		return false;
	}
	else
		printf("\n    Port %s Opened\n ", this->comPortName.c_str());

	/*------------------------------- Setting the Parameters for the SerialPort ------------------------------*/

	DCB dcbSerialParams = { 0 };                         // Initializing DCB structure
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

	Status = GetCommState(this->hComm, &dcbSerialParams);      //retreives  the current settings

	if (Status == FALSE) {
		printf("\n    Error! in GetCommState()");
		return false;
	}

	dcbSerialParams.BaudRate = this->baudRate;      // Setting BaudRate = 9600
	dcbSerialParams.ByteSize = this->byteSize;             // Setting ByteSize = 8
	dcbSerialParams.StopBits = this->stopBits;    // Setting StopBits = 1
	dcbSerialParams.Parity = this->parity;        // Setting Parity = None 

	Status = SetCommState(this->hComm, &dcbSerialParams);  //Configuring the port according to settings in DCB 

	if (Status == FALSE)
	{
		printf("\n    Error! in Setting DCB Structure");
		return false;
	}

	/*------------------------------------ Setting Timeouts --------------------------------------------------*/

	COMMTIMEOUTS timeouts = { 0 };
	timeouts.ReadIntervalTimeout = 50;
	timeouts.ReadTotalTimeoutConstant = 50;
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;

	if (SetCommTimeouts(hComm, &timeouts) == FALSE) {
		printf("\n\n    Error! in Setting Time Outs");
		return false;
	}
	else
		printf("\n\n    Setting Serial Port Timeouts Successfull");

	/*------------------------------------ Setting Receive Mask ----------------------------------------------*/

	Status = SetCommMask(this->hComm, EV_RXCHAR); //Configure Windows to Monitor the serial device for Character Reception

	if (Status == FALSE) {
		printf("\n\n    Error! in Setting CommMask");
		return false;
	}
	else
		printf("\n\n    Setting CommMask successfull");

	/*------------------------------------ Setting WaitComm() Event   ----------------------------------------*/

	printf("\n\n    Waiting for Data Reception");
	DWORD dwEventMask;                     // Event mask to trigger
	Status = WaitCommEvent(hComm, &dwEventMask, NULL); //Wait for the character to be received
	printf("\n Data Received\n");

	/*-------------------------- Program will Wait here till a Character is received ------------------------*/

	if (Status == FALSE)
	{
		printf("\n    Error! in Setting WaitCommEvent()");
		return false;
	}

	return true;
}

bool Serial::close()
{
	CloseHandle(this->hComm);//Closing the Serial Port
	return false;
}

bool Serial::read(int nBytes, char buffer[])
{
	DWORD totalBytesRead = 0;
	BOOL Status = FALSE;
	do
	{
		DWORD bytesRead;
		Status = ReadFile(this->hComm, &(buffer[totalBytesRead]), nBytes - totalBytesRead, &bytesRead, NULL);
		totalBytesRead += bytesRead;
	} while (totalBytesRead < nBytes);
	return false;
}

char Serial::readByte()
{
	const int nBytes = 1;
	char buffer;
	BOOL Status = FALSE;
	DWORD bytesRead;
	do
	{
		Status = ReadFile(this->hComm, &buffer, nBytes, &bytesRead, NULL);
	} while (bytesRead != nBytes);
	return buffer;
}

