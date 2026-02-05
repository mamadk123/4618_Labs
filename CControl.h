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

	double _press_start = -1.0; ///< Timestamp (seconds) when a button press is first detected
	double _counted_time = -1.0; ///< Timestamp (seconds) of the last registered debounced press

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


	/**
	 * @brief Reads an analog input channel and returns the value as a percentage.
	 *
	 * This method sends a GET command for an analog channel, converts the returned
	 * raw ADC value into a percentage (0.0 to 100.0), and returns the result.
	 * The conversion is performed using the full-scale ADC range.
	 *
	* @param channel Analog input channel to read
	 * @param percent Reference that receives the converted percentage value
	 * @return true if the analog value is read successfully, false on timeout or error
	 */
	bool get_analog_percent(int channel, double& percent);


	/**
	3 * @brief Reads a digital input channel using debounce logic.
	 *
	 * This method implements software debounce for a digital input channel.
	 * A button press is registered only once per physical press after the
	 * debounce time has elapsed. The function returns true exactly once
	 * for each valid button press.
	 *
	 * @param channel Digital input channel to read
	 * @return true if a new debounced button press is detected, false otherwise
	*/
	bool get_button_debounced(int channel);
};
