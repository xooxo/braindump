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

NTSTATUS
initwsk(PWSK_REGISTRATION clireg, PWSK_PROVIDER_NPI pronpi);

VOID
cleanupwsk(PWSK_REGISTRATION clireg);

NTSTATUS
connecttoserver(PWSK_PROVIDER_NPI pronpi, PSOCKADDR remaddr,
		PWSK_SOCKET *sock);

IO_COMPLETION_ROUTINE
createcomplete;

NTSTATUS
senddata(PWSK_SOCKET sock, PVOID data, ULONG datal);

IO_COMPLETION_ROUTINE
senddatacomplete;

NTSTATUS
recvdata(PWSK_SOCKET sock, PVOID data, ULONG datal, ULONG flags,
		PULONG_PTR recvd);

IO_COMPLETION_ROUTINE
recvdatacomplete;

NTSTATUS
disconnectfromserver(PWSK_SOCKET sock);

IO_COMPLETION_ROUTINE
disconnectcomplete;

