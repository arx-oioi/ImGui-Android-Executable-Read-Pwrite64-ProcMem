#include "tools.h"


const char*  pkg = OBFUSCATE("pkg");
int ipid = getPID(pkg); //test

int ARX(int mode) {

	int ipid = getPID(pkg);
        if (ipid == -1) {
            printf(OBFUSCATE("ไม่พบ PID สำหรับ %s\n"), pkg);
            return -1;
        }
        
        char lj[64];
        sprintf(lj,OBFUSCATE("/proc/%d/mem"), ipid);
        handle = open(lj, O_RDWR);
        if (handle == -1) {
            perror(OBFUSCATE("ไม่พบ /proc/mem"));
            return -1;
        }
        
        
    long int BaseGames = GetAddr(ipid, OBFUSCATE("libbase.so")); // libname
    if (BaseGames == -1) {
        printf(OBFUSCATE("ยังไม่พบ\n"));
        return -1;
    }


    if (mode >= 0) {
        switch (mode) {
            case 0:
                PHEX(BaseGames + 0x0, OBFUSCATE("000080D2C0035FD6")); // test 8 byte
                break;
            case 1:
                PHEX(BaseGames + 0x0, OBFUSCATE("00008052")); // test 4 byte
                break;

           case 2:
                PHEX(BaseGames + 0x0, OBFUSCATE("00")); //test 1 byte
                break;
                
           case 3:
                PHEX(BaseGames + 0x0, OBFUSCATE("0000")); // test 2 byte
                break;
        }
    }
    return 0;
}

