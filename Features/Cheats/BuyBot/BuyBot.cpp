#include "BuyBot.h"

void BuyBot::Buy()
{
	if (!cfg::g_cfg.misc.buybot_enable)
		return;

	if (g_ctx.globals.should_buy)
	{
		--g_ctx.globals.should_buy;

		if (!g_ctx.globals.should_buy)
		{
			std::string buy;

			//Primary
			switch (cfg::g_cfg.misc.buybot1)
			{
			case 1:
				buy += crypt_str("buy awp; ");
				break;
			case 2:
				if (g_ctx.local()->m_iTeamNum() == 2)
					buy += crypt_str("buy g3sg1; ");
				else if (g_ctx.local()->m_iTeamNum() == 3)
					buy += crypt_str("buy scar20; ");
				break;
			case 3:
				buy += crypt_str("buy ssg08; ");
				break;
			}

			//Secondary
			switch (cfg::g_cfg.misc.buybot2)
			{
			case 1:
				buy += crypt_str("buy deagle; ");
				break;
			case 2:
				if (g_ctx.local()->m_iTeamNum() == 2)
					buy += crypt_str("buy tec9; ");
				else if (g_ctx.local()->m_iTeamNum() == 3)
					buy += crypt_str("buy fn57; ");
				break;
			case 3:
				buy += crypt_str("buy elite; ");
				break;
			}

			//Utility's
			if (cfg::g_cfg.misc.buybot3[BUY_ARMOR])
				buy += crypt_str("buy vest; buy vesthelm; ");

			if (cfg::g_cfg.misc.buybot3[BUY_GRENADES])
				buy += crypt_str("buy smokegrenade; buy hegrenade; buy molotov; ");

			if (cfg::g_cfg.misc.buybot3[BUY_TASER])
				buy += crypt_str("buy taser; ");

			if (cfg::g_cfg.misc.buybot3[BUY_DEFUSER] && g_ctx.local()->m_iTeamNum() == 3)
				buy += crypt_str("buy defuser; ");


			//Execute the command in the console.
			m_engine()->ExecuteClientCmd(buy.c_str());
		}
	}
}