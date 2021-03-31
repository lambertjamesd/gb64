
#include "upgrade.h"
#include "save.h"

void updateToLatestVersion(struct GameBoy* gameboy)
{
    while (gameboy->settings.version < GB_SETTINGS_CURRENT_VERSION) 
    {
        switch (gameboy->settings.version)
        {
            case 0:
                if (gameboy->memory.mbc->bankSwitch == handleMBC3Write)
                {
                    gameboy->memory.misc.ramRomSelect = gameboy->memory.misc.romBankUpper;
                    gameboy->memory.misc.romBankUpper = 0;
                }
                break;
            case 1:
                gameboy->settings.compressedSize = 0;
                gameboy->settings.storedType = getDeprecatedStoredInfoType(gameboy);
                break;
        }

        ++gameboy->settings.version;
    }
}