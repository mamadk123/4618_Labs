#pragma once
#include "Serial.h"

/**
 * @file CControl.h
 * @brief CControl class for communicating with the TM4C123G using the ELEX4618 serial protocol.
 *
 * Protocol format (ASCII):
 *  GET: "G <type> <channel>\n"
 *  SET: "S <type> <channel> <value>\n"
 *
 * Expected reply (ASCII):
 *  "A <type> <channel> <value>\n"
 *
 * type:
 *  0 = DIGITAL
 *  1 = ANALOG
 *  2 = SERVO
 */

 /**
  * @enum ControlType
  * @brief Defines the I/O types supported by the embedded system.
  */
enum ControlType
{
	DIGITAL = 0, /**< Digital input or output */
	ANALOG = 1, /**< Analog input */
	SERVO = 2  /**< Servo output */
};

/**
 * @class CControl
 * @brief Implements GET/SET communication with the embedded system over a serial COM port.
 *
 * CControl wraps the provided Serial class and sends commands using the ELEX4618 protocol.
 * Each command waits for an acknowledgement line beginning with 'A'.
 * The call returns false if a valid acknowledgement is not received before the timeout.
 */
class CControl
{
private:
	Serial _com; ///< Serial port object used to communicate with the embedded system

public:
	/**
	 * @brief Constructs a CControl object.
	 *
	 * The object is not connected until init_com is called.
	 */
	CControl();

	/**
	 * @brief Destroys the CControl object.
	 */
	~CControl();

	/**
	 * @brief Opens the serial COM port used to communicate with the microcontroller.
	 *
	 * This method opens COM<comport> and flushes any startup text from the embedded system
	 * so that subsequent get_data and set_data calls receive clean protocol replies.
	 *
	 * @param comport COM port number (example: 5 means "COM5")
	 */
	void init_com(int comport);

	/**
	 * @brief Sends a GET command and returns the value from the embedded system.
	 *
	 * Command format:
	 *  "G <type> <channel>\n"
	 *
	 * Reply format:
	 *  "A <type> <channel> <value>\n"
	 *
	 * @param type I/O type (DIGITAL, ANALOG, SERVO)
	 * @param channel Channel index to read (meaning depends on type)
	 * @param result Reference that receives the returned value
	 * @return true if a valid reply is received before timeout, false otherwise
	 */
	bool get_data(int type, int channel, int& result);

	/**
	 * @brief Sends a SET command to write a value to the embedded system.
	 *
	 * Command format:
	 *  "S <type> <channel> <value>\n"
	 *
	 * Reply format:
	 *  "A <type> <channel> <value>\n"
	 *
	 * @param type I/O type (DIGITAL or SERVO)
	 * @param channel Channel index to write (meaning depends on type)
	 * @param val Value to write
	 * @return true if a valid reply is received before timeout, false otherwise
	 */
	bool set_data(int type, int channel, int val);
};
