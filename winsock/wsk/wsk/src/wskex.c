/*
 * Copyright (c) 2011, Brandon Falk <bfalk@brandonfa.lk>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 	1. Redistributions of source code must retain the above copyright notice,
 * 	this list of conditions and the following disclaimer.
 *
 * 	2. Redistributions in binary form must reproduce the above copyright
 * 	notice, this list of conditions and the following disclaimer in the
 * 	documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "wskex.h"

/* Main device object */
PDEVICE_OBJECT devobj;

/* DriverEntry()
 *
 * Summary:
 *
 * This is the main entry and init routine.
 */
NTSTATUS
DriverEntry(PDRIVER_OBJECT driverobj, PUNICODE_STRING regpath)
{
	NTSTATUS        status;
	UNICODE_STRING  devname = RTL_CONSTANT_STRING(L"\\Device\\wskex");

	UNREFERENCED_PARAMETER(regpath);

	/* Set up the unload routine */
	driverobj->DriverUnload = unload;

	/* Set up the control device object */
	status = IoCreateDevice(
			driverobj,               /* DriverObject          */
			0,                       /* DeviceExtensionSize   */
			&devname,                /* DeviceName            */
			FILE_DEVICE_CONTROLLER,  /* DeviceType            */
			FILE_DEVICE_SECURE_OPEN, /* DeviceCharacteristics */
			FALSE,                   /* Exclusive             */
			&devobj);                /* DeviceObject          */
	if(!NT_SUCCESS(status)){
		DbgPrint("IoCreateDevice() error : 0x%X\n", status);
		return status;
	}

	sendthenrecv();

	return STATUS_SUCCESS;
}

/* sendthenrecv()
 *
 * Summary:
 *
 * This is an example usage of the wsk api provided here. This function
 * connects, sends data, recieves data, then cleans up and closes.
 */
NTSTATUS
sendthenrecv(void)
{
	NTSTATUS         status;
	WSK_REGISTRATION clireg;
	WSK_PROVIDER_NPI pronpi;
	SOCKADDR_IN      srvaddr;
	PWSK_SOCKET      sock;
	char             sendstr[] = "testing\n";
	ULONG_PTR        recvd;

	/* 64 byte recv buffer */
	char             recvbuf[64];

	srvaddr.sin_family                 = AF_INET;
	srvaddr.sin_port                   = RtlUshortByteSwap(1337);
	srvaddr.sin_addr.S_un.S_un_b.s_b1  = 192;
	srvaddr.sin_addr.S_un.S_un_b.s_b2  = 168;
	srvaddr.sin_addr.S_un.S_un_b.s_b3  = 42;
	srvaddr.sin_addr.S_un.S_un_b.s_b4  = 64;

	status = initwsk(&clireg, &pronpi);
	if(!NT_SUCCESS(status)){
		DbgPrint("initwsk() failed: 0x%x\n", status);
		return status;
	}

	DbgPrint("Initialized!\n");

	status = connecttoserver(&pronpi, (PSOCKADDR)&srvaddr, &sock);
	if(!NT_SUCCESS(status)){
		cleanupwsk(&clireg);
		DbgPrint("connecttoserver() failed: 0x%x\n", status);
		return status;
	}

	DbgPrint("Connected!\n");

	status = senddata(sock, sendstr, sizeof(sendstr));
	if(!NT_SUCCESS(status)){
		disconnectfromserver(sock);
		cleanupwsk(&clireg);
		DbgPrint("senddata() failed: 0x%x\n", status);
		return status;
	}

	DbgPrint("Sent data!\n");

	status = recvdata(sock, recvbuf, sizeof(recvbuf), 0, &recvd);
	if(!NT_SUCCESS(status)){
		disconnectfromserver(sock);
		cleanupwsk(&clireg);
		DbgPrint("recvdata() failed: 0x%x\n", status);
		return status;
	}

	DbgPrint("Recv: %.*s\n", recvd, recvbuf);

	disconnectfromserver(sock);
	cleanupwsk(&clireg);

	return STATUS_SUCCESS;
}

/* unload()
 *
 * Summary:
 *
 * This function unloads and frees the driver.
 */
VOID
unload(PDRIVER_OBJECT driverObject)
{
	UNREFERENCED_PARAMETER(driverObject);

	IoDeleteDevice(devobj);

	return;
}

