
#include "upgrade.h"

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
        }

        ++gameboy->settings.version;
    }
}