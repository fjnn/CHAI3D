#include "devices/CGenericHapticDevice.h"
#include "math/CMaths.h"
#include "UsartDevice.h"
#include "libraries\Serial.h"
#include <iostream>

namespace chai3d {

	/* Constructor */
	UsartDevice::UsartDevice(int device_port)
		: cGenericHapticDevice(0),
		port{ device_port },
		serial{ Serial(device_port) },
		origin(0.0065, 0.0, 0.0)
	{
		this->rotation.identity();
	}

	/* Destructor */
	UsartDevice::~UsartDevice() {

	}

	/**-----------------------------------
	Our custom defined functions 
	-----------------------------------**/
	/* Smooth the given X, Y, Z values to a finite floating point precision */
	static cVector3d filter_xyz(Eigen::Vector4d &xyz) {
		const double resolution = 10000.0;
		int x = xyz.x() * resolution;
		int y = xyz.y() * resolution;
		int z = xyz.z() * resolution;

		cVector3d output;
		output.x((double)x / resolution);
		output.y((double)y / resolution);
		output.z((double)z / resolution);
		return output;
	}

	/* Updates device's position and orientation */
	void UsartDevice::updateDevice() {
		this->origin.x(0); //s3 = s* + s; this is s.
		this->origin.y(0.0);
		this->origin.z(0.0);
		
		
		double theta1, theta2, s3;
		theta1 = this->angle.x();
		theta2 = -this->angle.z();
		//s3 = this->origin.x() + this->pivotOffset;
		s3 = 0.01;

		/* Create Homogenous Transformation Matrix */
		//Update Rotation Matrix
		this->rotation.setExtrinsicEulerRotationDeg(0.0, theta2, theta1, C_EULER_ORDER_XZY);

		Eigen::Matrix4d R(Eigen::Matrix4d::Identity());
		R.block<3, 3>(0, 0) << this->rotation.eigen();
		//HTM.block<4, 1>(0, 3) << s3*cos(theta2)*sin(theta1), s3*sin(theta1)*sin(theta2), s3*cos(theta2), 1.0;

		Eigen::Matrix4d T(Eigen::Matrix4d::Identity());
		T.block<4, 1>(0, 3) << s3, 0.0, 0.0, 1.0;

		Eigen::Matrix4d invT(Eigen::Matrix4d::Identity());
		invT.block<4, 1>(0, 3) << -s3, 0.0, 0.0, 1.0;
		
		Eigen::Matrix4d TRinvT(T * R * invT);
	
		//Update Position
		Eigen::Vector4d originHomogenous(this->origin.x(), this->origin.y(), this->origin.z(), 1.0);
		Eigen::Vector4d resultV4d(TRinvT * originHomogenous);
		this->origin = filter_xyz(resultV4d);
		std::cout << this->origin << std::endl;
	}

	/* Read raw data via USART-USB interface and extracts the values from it and saves them so they can be used by other functions */
	void UsartDevice::getData() {
		if (m_deviceReady) {
			/* read preamble (header) which is 0xAA 0xAA 0xAA 0xAA 0xAA 0xAA */
			int count = 0;
			while (count < 6) {
				UINT8 byte = (UINT8)this->serial.readByte();
				// printf("0x%X\n", byte);
				if (byte == 0xAA) {
					count++;
				}
			}

			/* read 24 bytes (6 bytes per axis); Each 6 bytes represents one double; three axises X, Y, Z */
			const int bytes_per_packet = 24;
			UINT8 buffer[bytes_per_packet];
			this->serial.read(bytes_per_packet, (char*)buffer);
			/* extract X, Y, Z angles, and convert them to double */
			double angle_x, angle_y, angle_z;
			memcpy(&angle_x, buffer, 8);
			memcpy(&angle_y, buffer + 8, 8);
			memcpy(&angle_z, buffer + 16, 8);
			/* Save them to this object's member variables */
			this->angle.set(angle_x, angle_y, angle_z);

			//printf("%.2f, %.2f, %.2f\n", angle_x, angle_y, angle_z);
		}
	}



	/** cGenericHapticDevice implemented functions
	These are the virtual functions defined in the cGenericHapticDevice that we need to implement so the cGenericTool class
	can work with our device without having to modify it's code 
	------------------------------------------------------------ **/
	/* Open USART Connection */
	bool UsartDevice::open() {
		this->m_deviceReady = this->serial.open();
		if (this->m_deviceReady) {
			std::cout << std::endl << "Successfully opened device on COM" << this->port << std::endl;
		}
		else {
			std::cout << std::endl << "Failed to open device on COM" << this->port << "!" << std::endl;
		}
		return this->m_deviceReady;
	}



	/* Close USART Connection */
	bool UsartDevice::close() {
		if (this->serial.close()) {
			this->m_deviceReady = false;  // reset status to closed
			return true;
		}
		else {  // failed to close
			return false;
		}
	}



	/* Return Orientation of Device */
	bool UsartDevice::getRotation(cMatrix3d& a_rotation) {
		a_rotation = this->rotation;
		return m_deviceReady;
	}



	/* Return Position of Device */
	bool UsartDevice::getPosition(cVector3d& a_position) {
		/* We call this function only here because cGenericTool::updateFromDevice calls getPosition first
		so we read data from our USART device once, save the read values, and the other functions like getRotation just use those values */

		this->getData();
		this->updateDevice();
		
		a_position.x(this->origin.x());
		a_position.y(this->origin.y());
		a_position.z(this->origin.z());

		return m_deviceReady;
	}



	/* Returns a structure containing information about our device and its capabilities */
	cHapticDeviceInfo UsartDevice::getSpecifications() {
		/* todo */
		return cHapticDeviceInfo();
	}
}