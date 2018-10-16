#undef __STRICT_ANSI__
#include "main.h"
#include <iostream>
#include <fstream>


UINT_PTR base;

class Color{
    public:
    float red, green, blue, alpha;
    Color(float r, float g, float b, float a){
        red = r;
        green = g;
        blue = b;
        alpha = a;
    }

};
Color defaultColor = Color(1.0, 1.0, 1.0, 1.0);
DWORD defaultColorPtr = (DWORD)&defaultColor;

wchar_t defaultMessage[255];
DWORD defaultMessagePtr = (DWORD)&defaultMessage;

char msgObject[255];
DWORD msgObjectPtr = (DWORD)&msgObject;

_declspec(naked) void ASMHandleNameChange(){

    asm("add esp, 0x8");
    asm("cmp dword ptr [ebp - 0x18], 0x10");
    asm("jb 0f");


    asm("1:"); //Name is too long
    asm("call [_NameTooLongPtr]");
    asm("mov ecx, [_base]"); //jump to end
    asm("add ecx, 0x7E428");
    asm("jmp ecx");


    asm("0:"); //Name is not too long
    asm("mov eax, [_base]"); //jump back
    asm("add eax, 0x7E3EA");
    asm("cmp dword ptr [ebp - 0x18], 0"); //original comparison
    asm("jmp eax");
}

void ASMPrintMessage(){

    asm("push [_defaultMessagePtr]");
    asm("mov ecx, [_msgObjectPtr]");

    asm("mov eax, [_base]");
    asm("add eax, 0x0EB60");
    asm("call eax"); //call some message constructing function

    asm("mov ecx, [_base]");
    asm("add ecx, 0x36B1C8");
    asm("mov ecx, [ecx]"); //ecx points to gamecontroller
    asm("mov ecx, [ecx + 0x800A14]"); //ecx points to chatwidget

    asm("push [_defaultColorPtr]");
    asm("push [_msgObjectPtr]");
    asm("mov edx, [_base]");
    asm("add edx, 0x3AB30");
    asm("call edx"); //prints message


    asm("mov ecx, [_msgObjectPtr]");

    asm("mov eax, [_base]");
    asm("add eax, 0x193E50");
    asm("call eax"); //destructor for that message object

}

void PrintMessage(wchar_t message[]){

    wcsncpy(defaultMessage, message, 255);
    defaultColor.red = 1.0;
    defaultColor.blue = 1.0;
    defaultColor.green = 1.0;
    defaultColor.alpha = 1.0;
    ASMPrintMessage();
}
void PrintMessage(wchar_t message[], int r, int g, int b){
    wcsncpy(defaultMessage, message, 255);
    defaultColor.red = r / 255.0;
    defaultColor.green = g / 255.0;
    defaultColor.blue = b / 255.0;
    ASMPrintMessage();
}

void NameTooLong(){
    PrintMessage(L"Character name too long\n", 255, 51, 51);
}
DWORD NameTooLongPtr = (DWORD)&NameTooLong;


void WriteJMP(BYTE* location, BYTE* newFunction){
	DWORD dwOldProtection;
	VirtualProtect(location, 5, PAGE_EXECUTE_READWRITE, &dwOldProtection);
    location[0] = 0xE9; //jmp
    *((DWORD*)(location + 1)) = (DWORD)(( (unsigned INT32)newFunction - (unsigned INT32)location ) - 5);
	VirtualProtect(location, 5, dwOldProtection, &dwOldProtection);
}


extern "C" DLL_EXPORT BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    base = (UINT_PTR)GetModuleHandle(NULL);
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            WriteJMP((BYTE*)(base + 0x7E3E3), (BYTE*)&ASMHandleNameChange);
            break;
;
    }
    return TRUE;
}
