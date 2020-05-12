
#define ROM_BANK_SIZE 16384
#define VIRTUAL_BANK_COUNT 

struct ROMLayout {
    unsigned char mainBank[16384];
    int virtualBankCount;
    unsigned char* virtualBanks;
};