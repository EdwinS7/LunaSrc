// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "clantag.h"

void spammers::clan_tag()
{
	auto apply = [](const char* tag) -> void
	{
		using Fn = int(__fastcall*)(const char*, const char*);
		static auto fn = reinterpret_cast<Fn>(util::FindSignature(ENGINE_DLL, crypt_str("53 56 57 8B DA 8B")));
		fn(tag, tag);
	};

	static auto removed = false;

	if (!cfg::g_cfg.misc.clantag_spammer && !removed)
	{
		removed = true;
		apply(crypt_str(""));
		return;
	}

	if (cfg::g_cfg.misc.clantag_spammer)
	{
		auto nci = m_engine()->GetNetChannelInfo();

		if (!nci)
			return;

		static auto time = -1;

		auto ticks = TIME_TO_TICKS(nci->GetAvgLatency(FLOW_OUTGOING)) + (float)m_globals()->m_tickcount;
		auto intervals = 0.33f / m_globals()->m_intervalpertick;

		auto main_time = (int)(ticks / intervals) % 26;

		if (main_time != time && !m_clientstate()->iChokedCommands)
		{
			auto tag = crypt_str("");

            switch (main_time)
            {
            case 0:
                tag = crypt_str("|");
                break;
            case 1:
                tag = crypt_str("l");
                break;
            case 2:
                tag = crypt_str("l\\");
                break;
            case 3:
                tag = crypt_str("l\\/");
                break;
            case 4:
                tag = crypt_str("lu");
                break;
            case 5:
                tag = crypt_str("lu|");
                break;
            case 6:
                tag = crypt_str("lu|/");
                break;
            case 7:
                tag = crypt_str("lu|/|");
                break;
            case 8:
                tag = crypt_str("lun");
                break;
            case 9:
                tag = crypt_str("lun/");
                break;
            case 10:
                tag = crypt_str("lun/\\");
                break;
            case 11:
                tag = crypt_str("luna");
                break;
            case 12:
                tag = crypt_str("luna");
                break;
            case 13:
                tag = crypt_str("luna");
                break;
            case 14:
                tag = crypt_str("lun/\\");
                break;
            case 15:
                tag = crypt_str("lun/");
                break;
            case 16:
                tag = crypt_str("lun");
                break;
            case 17:
                tag = crypt_str("lu|/|");
                break;
            case 18:
                tag = crypt_str("lu|/");
                break;
            case 19:
                tag = crypt_str("lu|");
                break;
            case 20:
                tag = crypt_str("lu");
                break;
            case 21:
                tag = crypt_str("l\\/");
                break;
            case 22:
                tag = crypt_str("l\\");
                break;
            case 23:
                tag = crypt_str("l\\");
                break;
            case 24:
                tag = crypt_str("l");
                break;
            case 25:
                tag = crypt_str("|");
                break;
            }

			apply(tag);
			time = main_time;
		}

		removed = false;
	}
}