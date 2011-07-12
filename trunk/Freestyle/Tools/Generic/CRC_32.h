#pragma once

class CRC_32
{

public:
    CRC_32(void);
    DWORD CalcCRC(FILE * fp);
    void Calculate (const LPBYTE buffer, UINT size, ULONG &crc);

private:
    ULONG Reflect(ULONG ref, char ch);
    ULONG Table[256];
};
