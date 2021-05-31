#include <windows.h>
#include <winuser.h>
#include <cstdio>

#define UNICODE 1

int main(){

    int msgboxID = MessageBoxExW(
        NULL,
        (LPCWSTR)L"Hello! This is a message box!",
        (LPCWSTR)L"Lil' Message ",
        MB_ICONQUESTION | MB_OK,
        0
    );

    switch(msgboxID){
        case IDOK:
            printf("OK button!\n");
        default:
            return 0;
    };

    return 0;


}