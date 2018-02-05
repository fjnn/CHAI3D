#pragma once

#include <Windows.h>
#include <string>

using namespace std;

class Serial
{
private:
	HANDLE hComm = nullptr;                          // Handle to the Serial port
	string comPortName;  // Name of the Serial port(May Change) to be opened,  "\\\\.\\COM7"
	int baudRate;
	int byteSize;
	int stopBits;
	int parity;

public:
	Serial(int portNumber, int baudRate = CBR_9600, int byteSize = 8, int stopBits = ONESTOPBIT, int parity = NOPARITY);
	~Serial();

	bool open();
	bool close();
	bool read(int nBytes, char buffer[]);
	char Serial::readByte();
};