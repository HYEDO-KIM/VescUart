#include "VescUart.h"
#include "main.h"

void reverse(uint8_t arr[], int start, int end) {
	int temp;
	end = end - 1;
	while (start < end) {
		temp = arr[start];
		arr[start] = arr[end];
		arr[end] = temp;
		start++;
		end--;
	}
}

void shiftRight(uint8_t arr[], int d, int n) {
	reverse(arr, 0, n - d);
	reverse(arr, n - d, n);
	reverse(arr, 0, n);
}

void initVesc(VescUart *vesc, uint8_t id, uint8_t* dataStructurePtr, size_t dataStructureSize, USART_TypeDef* usart)
{
	(vesc)->id = id;
	(vesc)->dataStructurePtr = dataStructurePtr;
	(vesc)->dataStructSize = dataStructureSize;
	(vesc)->usart = usart;
	(vesc)->nominalPacketCnt = 0;
	(vesc)->state = HEADER_1;
	(vesc)->trialCnt = 0;
	(vesc)->nominalTransmitCnt = 0;
}

/**
 * @brief      Set the current to drive the motor
 * @param      current  - The current to apply
 */
void setCurrent(VescUart *vesc, float current) {
	int32_t index = 0;
	uint8_t payload[5];

	payload[index++] = COMM_SET_CURRENT;
	buffer_append_int32(payload, (int32_t)(current * 1000), &index);

	packSendPayload(vesc, payload, 5);
}

/**
 * @brief      Set the current to brake the motor
 * @param      brakeCurrent  - The current to apply
 */
void setBrakeCurrent(VescUart *vesc, float brakeCurrent) {
	int32_t index = 0;
	uint8_t payload[5];

	payload[index++] = COMM_SET_CURRENT_BRAKE;
	buffer_append_int32(payload, (int32_t)(brakeCurrent * 1000), &index);

	packSendPayload(vesc, payload, 5);
}

/**
 * @brief      Set the rpm of the motor
 * @param      rpm  - The desired RPM (actually eRPM = RPM * poles)
 */
void setRPM(VescUart *vesc, float rpm) {
	int32_t index = 0;
	uint8_t payload[5];

	payload[index++] = COMM_SET_RPM ;
	buffer_append_int32(payload, (int32_t)(rpm), &index);

	packSendPayload(vesc, payload, 5);
}

/**
 * @brief      Set the duty of the motor
 * @param      duty  - The desired duty (0.0-1.0)
 */
void setDuty(VescUart *vesc, float duty) {
	int32_t index = 0;
	uint8_t payload[5];

	payload[index++] = COMM_SET_DUTY;
	buffer_append_int32(payload, (int32_t)(duty * 100000), &index);

	packSendPayload(vesc, payload, 5);
}

/**
 * @brief      Transmit the packet data to Vesc
 */

void writeVescPacket(VescUart *vesc, uint8_t packet[], int length)
{
	for(int i=0; i<length; i++)
	{
		while(!LL_USART_IsActiveFlag_TXE(vesc->usart));
		LL_USART_TransmitData8(vesc->usart, packet[i]);
	}
}

/**
 * @brief      Packs the payload and sends it over Serial
 *
 * @param      vesc
 * @param      payload  - The payload as a unit8_t Array with length of int lenPayload
 * @param      lenPay   - Length of payload
 * @return     The number of bytes send
 */
int packSendPayload(VescUart *vesc, uint8_t * payload, int lenPay)
{
	uint16_t crcPayload = crc16(payload, lenPay);
	int count = 0;
	uint8_t messageSend[256];

	if (lenPay <= 256)
	{
		messageSend[count++] = 2;
		messageSend[count++] = lenPay;
	}
	else
	{
		messageSend[count++] = 3;
		messageSend[count++] = (uint8_t)(lenPay >> 8);
		messageSend[count++] = (uint8_t)(lenPay & 0xFF);
	}

	memcpy(&messageSend[count], payload, lenPay);

	count += lenPay;
	messageSend[count++] = (uint8_t)(crcPayload >> 8);
	messageSend[count++] = (uint8_t)(crcPayload & 0xFF);
	messageSend[count++] = 3;
	messageSend[count] = '\0';

	// Sending package
	writeVescPacket(vesc, messageSend, count);

	// Returns number of send bytes
	return count;
}



/**
 * @brief      Receives the message over Serial
 *
 * @param      payloadReceived  - The received payload as a unit8_t Array
 * @return     The number of bytes receeived within the payload
 */
int receiveUartMessage(VescUart *vesc, uint8_t * payloadReceived) {

	// Messages <= 255 starts with "2", 2nd byte is length
	// Messages > 255 starts with "3" 2nd and 3rd byte is length combined with 1st >>8 and then &0xFF

	uint16_t counter = 0;
	uint16_t endMessage = 256;
	bool messageRead = false;
	uint8_t messageReceived[256];
	uint16_t lenPayload = 0;

	while (messageRead == false && counter<endMessage) {

		memcpy(messageReceived,vesc->packet,256);

		if(messageReceived[0]==2)
		{
			endMessage = messageReceived[1] + 5; //Payload size + 2 for sice + 3 for SRC and End.
			lenPayload = messageReceived[1];

			if (messageReceived[endMessage - 1] == 3) {
				messageReceived[endMessage] = 0;
				messageRead = true;
			}
		}

		if(messageRead == false)
		{
			shiftRight(messageReceived, 1, 256);
		}

		counter++;

	}

	bool unpacked = false;

	if (messageRead) {
		unpacked = unpackPayload(messageReceived, endMessage, payloadReceived);
	}

	if (unpacked) {
		// Message was read
		return lenPayload;
	}
	else {
		// No Message Read
		return 0;
	}
}


/**
 * @brief      Verifies the message (CRC-16) and extracts the payload
 *
 * @param      message  - The received UART message
 * @param      lenMes   - The lenght of the message
 * @param      payload  - The final payload ready to extract data from
 * @return     True if the process was a success
 */
bool unpackPayload(uint8_t * message, int lenMes, uint8_t * payload) {
	uint16_t crcMessage = 0;
	uint16_t crcPayload = 0;
	// Rebuild crc:
	crcMessage = message[lenMes - 3] << 8;
	crcMessage &= 0xFF00;
	crcMessage += message[lenMes - 2];

	// Extract payload:
	memcpy(payload, &message[2], message[1]);
	crcPayload = crc16(payload, message[1]);

	if (crcPayload == crcMessage) {
		return true;
	}
	else{
		return false;
	}
}

/**
 * @brief      Extracts the data from the received payload
 *
 * @param      message  - The payload to extract data from
 * @return     True if the process was a success
 */
bool processReadPacket(VescUart *vesc, uint8_t * message) {
	COMM_PACKET_ID packetId;
	int32_t ind = 0;
	packetId = (COMM_PACKET_ID)message[0];
	message++; // Removes the packetId from the actual message (payload)

	switch (packetId){
	case COMM_GET_VALUES: // Structure defined here: https://github.com/vedderb/bldc/blob/43c3bbaf91f5052a35b75c2ff17b5fe99fad94d1/commands.c#L164

		vesc->data.tempFET            = buffer_get_float16(message, 10.0, &ind);
		vesc->data.tempMotor          = buffer_get_float16(message, 10.0, &ind);
		vesc->data.avgMotorCurrent 	= buffer_get_float32(message, 100.0, &ind);
		vesc->data.avgInputCurrent 	= buffer_get_float32(message, 100.0, &ind);
		ind += 8; // Skip the next 8 bytes
		vesc->data.dutyCycleNow 		= buffer_get_float16(message, 1000.0, &ind);
		vesc->data.rpm 				= buffer_get_int32(message, &ind);
		vesc->data.inpVoltage 		= buffer_get_float16(message, 10.0, &ind);
		vesc->data.ampHours 			= buffer_get_float32(message, 10000.0, &ind);
		vesc->data.ampHoursCharged 	= buffer_get_float32(message, 10000.0, &ind);
		ind += 8; // Skip the next 8 bytes
		vesc->data.tachometer 		= buffer_get_int32(message, &ind);
		vesc->data.tachometerAbs 		= buffer_get_int32(message, &ind);
		return true;

		break;

	default:
		return false;
		break;
	}
}

bool getVescValues(VescUart *vesc)
{
	uint8_t command[1] = { COMM_GET_VALUES };
	uint8_t payload[256];

	packSendPayload(vesc, command, 1);
	// delay(1); //needed, otherwise data is not read

	int lenPayload = receiveUartMessage(vesc, payload);

	if (lenPayload > 55) {
		bool read = processReadPacket(vesc, payload); //returns true if sucessful
		return read;
	}
	else
	{
		return false;
	}
}

int pushVescPacket(VescUart *vesc, uint8_t packet[], int length)
{
	for(int i=0; i<length; i++)
	{
		vesc->test = i;
		vesc->packet[i] = packet[i];
		//		if(pushByte(vesc, packet[i]))
		//		{
		//			return 1;
		//		}
	}

	return 0;
}

