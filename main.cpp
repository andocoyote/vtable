#include <windows.h>
#include <dbghelp.h>
#include <stdlib.h>
#include <queue>
 
using namespace std;

// Defining our base class from which we will derive other classes
// in the exploration of the implementation of vtables
class P
{
public:
    P() : x(0) {}
    ~P() {}

    virtual void SetAll() {x = 5;}
    virtual void Display();
    int GetX() {return x;}

protected:
    int x;
};

// Display the integer value of x
void P::Display()
{
    printf("x is %d\n", x);
}

// Derive a new class from class P and override P's virtual methods
class Q : public P
{
public:
    Q() : P(), y(0) {}
    ~Q() {}

    void SetAll() {x = 10, y = 20;}
    void Display();
    int GetY() {return y;}

private:
    int y;
};

// Display the integer values of x and y
void Q::Display()
{
    printf("x is %d and y is %d\n", x, y);
}

typedef struct _VTABLE_ENTRY
{
    DWORD FunctionAddress;
    char* FunctionName;
} VTABLE_ENTRY, *PVTABLE_ENTRY;
 
queue<VTABLE_ENTRY> List;

// Initialize Symbol engine.
BOOL InitializeSymbols()
{
    BOOL status = FALSE;
    DWORD Options = SymGetOptions();

    Options |= SYMOPT_DEBUG; // Display symbol info to the debugger
    Options |= SYMOPT_UNDNAME; // Present symbols undecorated
 
    SymSetOptions(Options); 
 
    status = SymInitialize(GetCurrentProcess(), NULL, TRUE);

    return status;
}
 
// Get symbol name from address.
char* GetSymbolNameFromAddr(DWORD SymbolAddress)
{
    BOOL status = FALSE;
    DWORD64 Displacement = 0;
    DWORD error = 0;
    char* symbolName = 0;
    
    // Allocate a buffer to hold the SYMBOL_INFO structure including the symbol name
    char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME];

    PSYMBOL_INFO SymbolInfo = (PSYMBOL_INFO)buffer;

    SymbolInfo->SizeOfStruct = sizeof(SYMBOL_INFO);
    SymbolInfo->MaxNameLen = MAX_SYM_NAME;

    // Get the symbol from the function address from the vtable
    status = SymFromAddr(GetCurrentProcess(), SymbolAddress, &Displacement, SymbolInfo);

    if(!status)
    {
        error = GetLastError();
        printf("SymFromAddr returned error : %d\n", error);
        return NULL;
    }

    symbolName = new char[SymbolInfo->NameLen+1];
    strcpy_s(symbolName, SymbolInfo->NameLen+1, SymbolInfo->Name);

    return symbolName;
}
 
// Get list of virtual functions.
void WalkVTable(void* pClass)
{
    BOOL status = FALSE;
    char* symbolName = 0;

    // Initialize symbols.
    status = InitializeSymbols();

    if(!status)
    {
        printf("Call to InitializeSymbols failed: %d\n", GetLastError());
        return;
    }
 
    // Obtain the base pointer for the vtable
    //		DWORD* pVptr = (DWORD*)*(DWORD*)pClass;
    DWORD* pBase = (DWORD*)pClass;
    DWORD* pVptr = (DWORD*)*pBase;

	// Display a graphical represention of the C++ object and associated vtable:
	//	The first memory location of the object contains the address of the vtable.
	//	The first memory location in the vtable contains the assress of the first virtual function.
    printf("        Object          v-table\n");
	printf("Addr:   0x%X        0x%X\n", (UINT)pBase, (UINT)pVptr);
    printf("      ------------    ------------\n");
    printf("Data: | 0x%X | -> | 0x%X |\n", *pBase, *pVptr);
    printf("      ------------    ------------\n\n");
 
    // Iterate through VirtualTable and create a list of VTABLE_ENTRY objects
    DWORD index = 0;
    DWORD functionAddress = pVptr[index];

    while(functionAddress)
    {
        // Translate VFunctionAddress to FunctionName.
        symbolName = GetSymbolNameFromAddr(functionAddress);

        if(!symbolName)
        {
            printf("Failed to intialize symbols: %d\n", GetLastError());
            break;
        }

        VTABLE_ENTRY vtableEntry;
        vtableEntry.FunctionAddress = functionAddress;
        vtableEntry.FunctionName = symbolName;		

        List.push(vtableEntry);
 
        // Next function pointer.
        functionAddress = pVptr[++index];
    }
	
	status = SymCleanup(GetCurrentProcess());
	
	if(!status)
    {
        printf("Call to SymCleanup failed: %d\n", GetLastError());
        return;
    }
}
 
void main()
{
    Q q1;

    q1.SetAll();
 
    WalkVTable(&q1);

    while(!List.empty())
    {
        VTABLE_ENTRY vtableEntry = List.front();

        printf("Virtual Function Address: 0x%X. Virtual Function Name: %s.\n",
               vtableEntry.FunctionAddress,
               vtableEntry.FunctionName);

        List.pop();
        
        delete vtableEntry.FunctionName;
    }
}
