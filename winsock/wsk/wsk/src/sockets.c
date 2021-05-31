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

/* initwsk()
 *
 * Summary
 *
 * This function registers with Wsk and also captures the
 * provider NPI. After this function, Wsk is ready to use.
 *
 * Params:
 *
 * PWSK_REGISTRATION clireg - Caller allocated structure (output)
 * PWSK_PROVIDER_NPI pronpi - Called allocated structure (output)
 *
 * Returns:
 *
 * Follows standard NTSTATUS codes for success/errors.
 */
NTSTATUS
initwsk(PWSK_REGISTRATION clireg, PWSK_PROVIDER_NPI pronpi)
{
	NTSTATUS            status;
	WSK_CLIENT_NPI      clinpi;
	WSK_CLIENT_DISPATCH dispatch = { MAKE_WSK_VERSION(1, 0), 0, NULL };

	clinpi.ClientContext = NULL;
	clinpi.Dispatch      = &dispatch;

	status = WskRegister(&clinpi, clireg);
	if(!NT_SUCCESS(status)){
		DbgPrint("WskRegister() error : 0x%X\n", status);
		return status;
	}

	status = WskCaptureProviderNPI(clireg, WSK_INFINITE_WAIT, pronpi);
	if(!NT_SUCCESS(status)){
		DbgPrint("WskCaptureProviderNPI() error : 0x%X\n", status);
		WskDeregister(clireg);
		return status;
	}

	return STATUS_SUCCESS;
}

/* cleanupwsk()
 *
 * Summary:
 *
 * This function undos everything done in initwsk().
 *
 * Params:
 *
 * PWSK_REGISTRATION clireg - The structure from initwsk()
 */
VOID
cleanupwsk(PWSK_REGISTRATION clireg)
{
	WskReleaseProviderNPI(clireg);
	WskDeregister(clireg);

	return;
}


/* connecttoserver()
 *
 * Summary:
 *
 * This function connects to the server indicated by remoteAddress.
 *
 * Params:
 *
 * PWSK_PROVIDER_NPI providerNpi   - This is the structure from initwsk()
 * PSOCKADDR         remoteAddress - The location we want to connect to
 * PWSK_SOCKET       socket        - The socket we obtained from connection
 *
 * Returns:
 *
 * Follows standard NTSTATUS codes for success/errors.
 */
NTSTATUS
connecttoserver(PWSK_PROVIDER_NPI pronpi, PSOCKADDR remaddr, PWSK_SOCKET *sock)
{
	NTSTATUS status;
	PIRP     irp;
	KEVENT   event;
	SOCKADDR locaddr = { 0 };

	irp = IoAllocateIrp(1, FALSE);
	if(!irp){
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	KeInitializeEvent(&event, NotificationEvent, FALSE);

	IoSetCompletionRoutine(irp, createcomplete, &event, TRUE, TRUE, TRUE);

	locaddr.sa_family = remaddr->sa_family;
	
	status = pronpi->Dispatch->WskSocketConnect(
			pronpi->Client, /* Client             */
			SOCK_STREAM,    /* SocketType         */
			IPPROTO_TCP,    /* Protocol           */
			&locaddr,       /* LocalAddress       */
			remaddr,        /* RemoteAddress      */
			0,              /* Flags              */
			NULL,           /* SocketContext      */
			NULL,           /* Dispatch           */
			NULL,           /* OwningProcess      */
			NULL,           /* OwningThread       */
			NULL,           /* SecurityDescriptor */
			irp);           /* Irp                */
	if(!NT_SUCCESS(status)){
		IoFreeIrp(irp);
		return status;
	}

	if(status == STATUS_PENDING){
		KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);

		status = irp->IoStatus.Status;
		if(!NT_SUCCESS(status)){
			IoFreeIrp(irp);
			return status;
		}
	}

	*sock = (PWSK_SOCKET)irp->IoStatus.Information;

	IoFreeIrp(irp);

	return STATUS_SUCCESS;
}

/* createcomplete()
 *
 * Summary:
 *
 * This function handles the callback from when WskSocketConnect()
 * finishes.
 *
 * Returns:
 *
 * STATUS_MORE_PROCESSING_REQUIRED
 */
NTSTATUS
createcomplete(PDEVICE_OBJECT devobj, PIRP irp, PVOID context)
{
	UNREFERENCED_PARAMETER(devobj);
	UNREFERENCED_PARAMETER(irp);

	KeSetEvent((PKEVENT)context, IO_NO_INCREMENT, FALSE);

	return STATUS_MORE_PROCESSING_REQUIRED;
}

/* senddata()
 *
 * Summary:
 *
 * This function sends data to the specified socket.
 *
 * Params:
 *
 * PWSK_SOCKET sock  - The socket to send the data to
 * PVOID       data  - The data to send
 * ULONG       datal - The length of the data in bytes
 *
 * Returns:
 *
 * Follows standard NTSTATUS codes for success/errors.
 */
NTSTATUS
senddata(PWSK_SOCKET sock, PVOID data, ULONG datal)
{
	WSK_BUF  wskbuf;
	NTSTATUS status;
	KEVENT   event;
	PIRP     irp;

	irp = IoAllocateIrp(1, FALSE);
	if(!irp){
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	KeInitializeEvent(&event, NotificationEvent, FALSE);

	IoSetCompletionRoutine(irp, senddatacomplete, &event, TRUE, TRUE, TRUE);

	wskbuf.Mdl = IoAllocateMdl(data, datal, FALSE, FALSE, NULL);
	if(!wskbuf.Mdl){
		DbgPrint("Failed to allocate MDL!\n");
		return STATUS_MORE_PROCESSING_REQUIRED;
	}
	MmBuildMdlForNonPagedPool(wskbuf.Mdl);
	wskbuf.Offset = 0;
	wskbuf.Length = datal;

	status = ((PWSK_PROVIDER_CONNECTION_DISPATCH)(sock->Dispatch))->
		WskSend(sock, &wskbuf, 0, irp);
	if(status == STATUS_PENDING){
		KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);

		status = irp->IoStatus.Status;
	}

	IoFreeIrp(irp);

	return status;
}

/* senddatacomplete()
 *
 * Summary:
 *
 * This function handles the callback from when WskSend()
 * finishes.
 *
 * Returns:
 *
 * STATUS_MORE_PROCESSING_REQUIRED
 */
NTSTATUS
senddatacomplete(PDEVICE_OBJECT devobj, PIRP irp, PVOID context)
{
	UNREFERENCED_PARAMETER(devobj);
	UNREFERENCED_PARAMETER(irp);

	KeSetEvent((PKEVENT)context, IO_NO_INCREMENT, FALSE);

	return STATUS_MORE_PROCESSING_REQUIRED;
}

/* recvdata()
 *
 * Summary:
 *
 * This function receives data on the specified socket into data.
 *
 * Params:
 *
 * PWSK_SOCKET sock  - The socket to receive from
 * PVOID       data  - The buffer to store the received data
 * ULONG       datal - The length of data in bytes
 * ULONG       flags - Flags to be passed to WskReceive
 * PULONG_PTR  recvd - The length in bytes received
 */
NTSTATUS
recvdata(PWSK_SOCKET sock, PVOID data, ULONG datal, ULONG flags,
		PULONG_PTR recvd)
{
	WSK_BUF  wskbuf;
	NTSTATUS status;
	KEVENT   event;
	PIRP     irp;

	irp = IoAllocateIrp(1, FALSE);
	if(!irp){
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	KeInitializeEvent(&event, NotificationEvent, FALSE);

	IoSetCompletionRoutine(irp, recvdatacomplete, &event, TRUE, TRUE,
			TRUE);

	wskbuf.Mdl = IoAllocateMdl(data, datal, FALSE, FALSE, NULL);
	if(!wskbuf.Mdl){
		DbgPrint("Failed to allocate MDL!\n");
		return STATUS_MORE_PROCESSING_REQUIRED;
	}
	MmBuildMdlForNonPagedPool(wskbuf.Mdl);
	wskbuf.Offset = 0;
	wskbuf.Length = datal;

	status = ((PWSK_PROVIDER_CONNECTION_DISPATCH)(sock->Dispatch))->
		WskReceive(sock, &wskbuf, flags, irp);
	if(status == STATUS_PENDING){
		KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);

		status = irp->IoStatus.Status;
	}

	*recvd = irp->IoStatus.Information;

	IoFreeIrp(irp);

	return status;
}

/* recvdatacomplete()
 *
 * Summary:
 *
 * This function handles the callback from when WskSend()
 * finishes.
 *
 * Returns:
 *
 * STATUS_MORE_PROCESSING_REQUIRED
 */
NTSTATUS
recvdatacomplete(PDEVICE_OBJECT devobj, PIRP irp, PVOID context)
{
	UNREFERENCED_PARAMETER(devobj);
	UNREFERENCED_PARAMETER(irp);

	KeSetEvent((PKEVENT)context, IO_NO_INCREMENT, FALSE);

	return STATUS_MORE_PROCESSING_REQUIRED;
}

/* disconnectfromserver()
 *
 * Summary:
 *
 * Disconnects from a server.
 *
 * Params:
 *
 * PWSK_PROVIDER_NPI providerNpi - This is the structure from initwsk()
 *
 * Returns:
 *
 * Follows standard NTSTATUS codes for success/errors.
 */
NTSTATUS
disconnectfromserver(PWSK_SOCKET sock)
{
	NTSTATUS status;
	KEVENT   event;
	PIRP     irp;

	irp = IoAllocateIrp(1, FALSE);
	if(!irp){
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	KeInitializeEvent(&event, NotificationEvent, FALSE);

	IoSetCompletionRoutine(irp, disconnectcomplete, &event, TRUE, TRUE,
			TRUE);

	status = ((PWSK_PROVIDER_CONNECTION_DISPATCH)(sock->Dispatch))->
		WskCloseSocket(sock, irp);
	if(status == STATUS_PENDING){
		KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);

		status = irp->IoStatus.Status;
	}

	IoFreeIrp(irp);

	return status;
}

/* disconnectcomplete()
 *
 * Summary:
 *
 * This function handles the callback from when WskCloseSocket()
 * finishes.
 *
 * Returns:
 *
 * STATUS_MORE_PROCESSING_REQUIRED
 */
NTSTATUS
disconnectcomplete(PDEVICE_OBJECT devobj, PIRP irp, PVOID context)
{
	UNREFERENCED_PARAMETER(devobj);
	UNREFERENCED_PARAMETER(irp);

	KeSetEvent((PKEVENT)context, IO_NO_INCREMENT, FALSE);

	return STATUS_MORE_PROCESSING_REQUIRED;
}

