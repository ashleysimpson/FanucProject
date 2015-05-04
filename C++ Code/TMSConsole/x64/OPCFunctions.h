#include "extcode.h"
#pragma pack(push)
#pragma pack(1)

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * Given OPC URL and an array of signed longs (with length of array), write 
 * the array to the OPC address 
 */
void __cdecl OPCWrite(char OPCURL[], int32_t DataToWrite[], int32_t len);
/*!
 * Given OPC URL, read the array of size len into OPCData[] located at the  
 * OPC address
 */
void __cdecl OPCRead(char OPCURL[], int32_t OPCData[], int32_t len);
/*!
 * Reads global variable for current position
 */
void __cdecl ReadGlobalVars(double CurrentPosition[], int32_t len);

MgErr __cdecl LVDLLStatus(char *errStr, int errStrLen, void *module);

#ifdef __cplusplus
} // extern "C"
#endif

#pragma pack(pop)

