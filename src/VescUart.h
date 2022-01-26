#ifndef _VESCUART_h
#define _VESCUART_h

#include "main.h"
#include "datatypes.h"
#include "buffer.h"
#include "crc.h"

#define RX_BUFFER_SIZE 300

struct dataPackage {
	float tempFET;
	float tempMotor;
	float avgMotorCurrent;
	float avgInputCurrent;
	float dutyCycleNow;
	long rpm;
	float inpVoltage;
	float ampHours;
	float ampHoursCharged;
	long tachometer;
	long tachometerAbs;
};

typedef struct {
	uint8_t id;
	size_t dataStructSize;
	uint8_t *dataStructurePtr;
	USART_TypeDef* usart;
	int nominalPacketCnt;
	int state;
	uint8_t packetBuffer[RX_BUFFER_SIZE];
	int length;
	int trialCnt;
	unsigned short expectedCRC, actualCRC;
	uint8_t failByte;
	int packetBufferSize;
	int nominalTransmitCnt;
	uint8_t packet[300];
	int test;

	struct dataPackage data;
}VescUart;

void reverse(uint8_t arr[], int start, int end);

void shiftRight(uint8_t arr[], int d, int n);

void initVesc(VescUart *vesc, uint8_t id, uint8_t* dataStructurePtr, size_t dataStructureSize, USART_TypeDef* usart);

bool getVescValues(VescUart *vesc);

/**
 * @brief      Set the current to drive the motor
 * @param      current  - The current to apply
 */
void setCurrent(VescUart *vesc, float current);

/**
 * @brief      Set the current to brake the motor
 * @param      brakeCurrent  - The current to apply
 */
void setBrakeCurrent(VescUart *vesc, float brakeCurrent);

/**
 * @brief      Set the rpm of the motor
 * @param      rpm  - The desired RPM (actually eRPM = RPM * poles)
 */
void setRPM(VescUart *vesc, float rpm);

/**
 * @brief      Set the duty of the motor
 * @param      duty  - The desired duty (0.0-1.0)
 */
void setDuty(VescUart *vesc, float duty);

/**
 * @brief      Help Function to print struct dataPackage over Serial for Debug
 */
void printVescValues(void);

/**
 * @brief      Packs the payload and sends it over Serial
 *
 * @param      payload  - The payload as a unit8_t Array with length of int lenPayload
 * @param      lenPay   - Length of payload
 * @return     The number of bytes send
 */
int packSendPayload(VescUart *vesc, uint8_t * payload, int lenPay);

void writeVescPacket(VescUart *vesc, uint8_t packet[], int length);
/**
 * @brief      Receives the message over Serial
 *
 * @param      payloadReceived  - The received payload as a unit8_t Array
 * @return     The number of bytes receeived within the payload
 */
int receiveUartMessage(VescUart *vesc, uint8_t * payloadReceived);

/**
 * @brief      Verifies the message (CRC-16) and extracts the payload
 *
 * @param      message  - The received UART message
 * @param      lenMes   - The lenght of the message
 * @param      payload  - The final payload ready to extract data from
 * @return     True if the process was a success
 */
bool unpackPayload(uint8_t * message, int lenMes, uint8_t * payload);

/**
 * @brief      Extracts the data from the received payload
 *
 * @param      message  - The payload to extract data from
 * @return     True if the process was a success
 */
bool processReadPacket(VescUart *vesc, uint8_t * message);

/**
 * @brief      Copy the data you received into the packet arrangement of Vesc.
 *
 * @param      vesc  -
 * @param      packet[]  - Data array you recieved
 * @param      length   - Lenght of the array of packet
 */
int pushVescPacket(VescUart *vesc, uint8_t packet[], int length);


#endif
