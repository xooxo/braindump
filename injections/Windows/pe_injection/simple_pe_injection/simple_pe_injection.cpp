#include <cstdio>
#include <iostream>
#include <windows.h>
#include <memoryapi.h>
#include <winternl.h>
#include <winuser.h>
#include <tlhelp32.h>
#include <winnt.h>
#include <subauth.h>
#include <Psapi.h>
#include <libloaderapi.h>


#define UNICODE 1

using namespace std;

typedef NTSTATUS (NTAPI *pfnNtQueryInformationProcess)(
    IN  HANDLE ProcessHandle,
    IN  PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN  ULONG ProcessInformationLength,
    OUT PULONG ReturnLength    OPTIONAL
    );


// target is hardcoded for now
HANDLE findProcessHandle(char* target){

    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

     if (Process32First(snapshot, &entry) == TRUE)
    {
        while (Process32Next(snapshot, &entry) == TRUE)
        {
            if (stricmp(entry.szExeFile, target) == 0)
            {  
                printf("target pe found!\n");
                HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);
                return hProcess;
            }
        }
    }
    return 0;

}

// try NtQueryInformationProcess Function! 
int demoNtQuery(void){
    /* Get ntdll.dll */
    
    HMODULE hNtDll = GetModuleHandleA("ntdll");
    pfnNtQueryInformationProcess gNtQueryInformationProcess;
    gNtQueryInformationProcess = (pfnNtQueryInformationProcess)GetProcAddress(hNtDll,
                                                        "NtQueryInformationProcess");
    
    /* */
    HANDLE targetHandle = findProcessHandle("target_pe.exe");
    if(targetHandle){
        PVOID processInfoBuffer;
        NTSTATUS processInfo = gNtQueryInformationProcess(targetHandle,ProcessImageFileName,processInfoBuffer,(ULONG)sizeof(processInfoBuffer),(PULONG)sizeof(processInfoBuffer));
        //printf("Process Image Filename: %s",(PWSTR)(processInfoBuffer+2*sizeof(USHORT)));
        printf("Process Image Filename: %s",((USHORT*)(&processInfoBuffer)+2*sizeof(USHORT)));
        //wprintf(processInfoBuffer+2*sizeof(USHORT));
        //wcout << (processInfoBuffer+2*sizeof(USHORT));
        CloseHandle(targetHandle);
        return 0;
    }
    return 1;

}


// Get PE image Path
DWORD getImagePath(HMODULE hModule, LPSTR  lpImageFileName, DWORD  nSize){
    // There must be a LPSTR data type to store name. Need a Pass-by-Reference. I guess LPSTR is already a pointer
    DWORD result = GetModuleFileNameA(hModule, lpImageFileName, nSize);
    return result;

}


int main(void){

    // demoNtQuery();

    HANDLE targetHandle = findProcessHandle("target_pe.exe");
       
    if(targetHandle){
        /* ========== STAGE-1  "Parse the currently running image's PE headers and get its sizeOfImage" ========= */ 
        LPCSTR imagePath;
        if(getImagePath(GetModuleHandle(0),imagePath,1024)){
            printf("Path: %s",imagePath);
        } 
            // Parse Size Stage 1.b
            PVOID imagebase = GetModuleHandle(NULL);
            PIMAGE_DOS_HEADER dosheader = (PIMAGE_DOS_HEADER) imagebase;
            // 00400080
            PIMAGE_NT_HEADERS ntheader = (PIMAGE_NT_HEADERS)( (DWORD_PTR) imagebase + dosheader->e_lfanew);
            //printf("With e_lfanew: %p\n",(void*) ntheader);
        


    }
    else printf("target_pe.exe not found!");

    return 0;


}