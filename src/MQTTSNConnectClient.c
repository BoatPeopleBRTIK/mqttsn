/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *******************************************************************************/

#include "MQTTSNPacket.h"
#include "StackTrace.h"

#include <string.h>

/**
  * Determines the length of the MQTT connect packet that would be produced using the supplied connect options.
  * @param options the options to be used to build the connect packet
  * @return the length of buffer needed to contain the serialized version of the packet
  */
int MQTTSNSerialize_connectLength(MQTTSNPacket_connectData* options)
{
	int len = 0;

	FUNC_ENTRY;
	len = 5 + MQTTstrlen(options->clientID);
	FUNC_EXIT_RC(len);
	return len;
}


/**
  * Serializes the connect options into the buffer.
  * @param buf the buffer into which the packet will be serialized
  * @param len the length in bytes of the supplied buffer
  * @param options the options to be used to build the connect packet
  * @return serialized length, or error if 0
  */
int MQTTSNSerialize_connect(unsigned char* buf, int buflen, MQTTSNPacket_connectData* options)
{
	unsigned char *ptr = buf;
	MQTTSNFlags flags;
	int len = 0;
	int rc = -1;

	FUNC_ENTRY;
	if ((len = MQTTSNPacket_len(MQTTSNSerialize_connectLength(options))) > buflen)
	{
		rc = MQTTSNPACKET_BUFFER_TOO_SHORT;
		goto exit;
	}
	ptr += MQTTSNPacket_encode(ptr, len); /* write length */
	writeChar(&ptr, MQTTSN_CONNECT);      /* write message type */

	flags.all = 0;
	flags.bits.cleanSession = options->cleansession;
	flags.bits.will = options->willFlag;
	writeChar(&ptr, flags.all);
	writeChar(&ptr, 0x01);                 /* protocol ID */
	writeInt(&ptr, options->duration);
	writeMQTTSNString(&ptr, options->clientID);

	rc = ptr - buf;

	exit: FUNC_EXIT_RC(rc);
	return rc;
}


/**
  * Deserializes the supplied (wire) buffer into connack data - return code
  * @param connack_rc returned integer value of the connack return code
  * @param buf the raw buffer data, of the correct length determined by the remaining length field
  * @param len the length in bytes of the data in the supplied buffer
  * @return error code.  1 is success, 0 is failure
  */
int MQTTSNDeserialize_connack(int* connack_rc, unsigned char* buf, int buflen)
{
	unsigned char* curdata = buf;
	unsigned char* enddata = NULL;
	int rc = 0;
	int mylen;

	FUNC_ENTRY;
	curdata += (rc = MQTTSNPacket_decodeBuf(curdata, &mylen)); /* read length */
	enddata = buf + mylen;
	if (enddata - curdata < 2)
		goto exit;

	if (readChar(&curdata) != MQTTSN_CONNACK)
		goto exit;

	*connack_rc = readInt(&curdata);

	rc = 1;
exit:
	FUNC_EXIT_RC(rc);
	return rc;
}


/**
  * Determines the length of the MQTT disconnect packet (without length field)
  * @param duration the parameter used for the disconnect
  * @return the length of buffer needed to contain the serialized version of the packet
  */
int MQTTSNSerialize_disconnectLength(int duration)
{
	int len = 0;

	FUNC_ENTRY;
	len = (duration >= 0) ? 3 : 1;
	FUNC_EXIT_RC(len);
	return len;
}


/**
  * Serializes a disconnect packet into the supplied buffer, ready for writing to a socket
  * @param buf the buffer into which the packet will be serialized
  * @param buflen the length in bytes of the supplied buffer, to avoid overruns
  * @param duration optional duration, not added to packet if < 0
  * @return serialized length, or error if 0
  */
int MQTTSNSerialize_disconnect(unsigned char* buf, int buflen, int duration)
{
	int rc = -1;
	unsigned char *ptr = buf;
	int len = 0;

	FUNC_ENTRY;
	if ((len = MQTTSNPacket_len(MQTTSNSerialize_disconnectLength(duration))) > buflen)
	{
		rc = MQTTSNPACKET_BUFFER_TOO_SHORT;
		goto exit;
	}
	ptr += MQTTSNPacket_encode(ptr, len); /* write length */
	writeChar(&ptr, MQTTSN_DISCONNECT);      /* write message type */

	if (duration >= 0)
		writeInt(&ptr, duration);

	rc = ptr - buf;
exit:
	FUNC_EXIT_RC(rc);
	return rc;
}