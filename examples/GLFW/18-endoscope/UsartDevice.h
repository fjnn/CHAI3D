#pragma once
#include "devices/CGenericHapticDevice.h"
#include "math/CMaths.h"
#include "libraries\Serial.h"

namespace chai3d {
	/*
	X is Zoom
	Y is Horizontal Axis
	Z is Vertical Axis
	We store the coordinates as a 3D homogeneous vector (x, y, z, w)
	// this->x is zoom
	// this->rotY is pitch
	// this->rotZ is yaw

	/* Axis information
			Z
			|
			|
			|
			|
			|______________Y
		   /
	      /
	     /
	    /
	   X
	*/

	class UsartDevice;
	typedef std::shared_ptr<UsartDevice> UsartDevicePtr;

	class UsartDevice : public cGenericHapticDevice {
	private:
		/* Calculation Parameters */
		const double pivotOffset = 0.0;
		const double angle_limit = 45.0;
		const double zoom_limit = 0.01;
		const double angle_scale = 75.0;
		const double zoom_scale = 1000.0;
		const double filter_resolution = 10000.0;
		/* Variables related to our Serial communication over USART */
		Serial serial;  // This class provides our USART-USB functionality
		int port;  // The port of our USART-USB device
		/* Position and Rotation variables */
		cVector3d angle;  // Gyroscope Rotation values. These are received from our USART device
		cVector3d origin;  // position of the endoscope 3D model in the simulation (these values are very sensitive to small changes)
		cMatrix3d rotation;  // Rotation Matrix

		/* Our own custom defined functions */
		void getData();
		void updateDevice();
	
	public:
		UsartDevice(int port);
		~UsartDevice();

		/* cGenericHapticDevice implemented virtual functions */
		bool open();
		bool close();
		bool getRotation(cMatrix3d& a_rotation);
		bool getPosition(cVector3d& a_position);
		cHapticDeviceInfo getSpecifications();
		// this functions is used to create an instance of this class and return a shared pointer to that instance
		static UsartDevicePtr create(int port = 0) { return (std::make_shared<UsartDevice>(port)); }

	};
}
