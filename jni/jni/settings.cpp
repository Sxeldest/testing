#include "main.h"
#include "settings.h"
#include "vendor/inih/cpp/INIReader.h"

#include "vendor/SimpleIni/SimpleIni.h"
#include "game/game.h"

extern CGame *pGame;

CSettings::CSettings()
{
	FLog("Loading settings..");	

	char buff[0x7F];
	sprintf(buff, "%sSAMP/settings.ini", pGame->GetDataDirectory());

	INIReader reader(buff);

	if(reader.ParseError() < 0)
	{
		FLog("Error: can't load %s", buff);
		std::terminate();
		return;
	}

	// client
	size_t length = 0;
	sprintf(buff, "__android_%d%d", rand() % 1000, rand() % 1000);
	length = reader.Get("client", "name", buff).copy(m_Settings.szNickName, 24);
	m_Settings.szNickName[length] = '\0';
	length = reader.Get("client", "host", "127.0.0.1").copy(m_Settings.szHost, MAX_SETTINGS_STRING);
	m_Settings.szHost[length] = '\0';
	length = reader.Get("client", "password", "").copy(m_Settings.szPassword, MAX_SETTINGS_STRING);
	m_Settings.szPassword[length] = '\0';
	m_Settings.iPort = reader.GetInteger("client", "port", 7777);

	// debug
	m_Settings.bDebug = reader.GetBoolean("debug", "debug", false);

	// gui
	length = reader.Get("gui", "Font", "arial.ttf").copy(m_Settings.szFont, 40);
	m_Settings.szFont[length] = '\0';
	m_Settings.fFontSize = reader.GetReal("gui", "FontSize", 30.0f);
	m_Settings.iFontOutline = reader.GetInteger("gui", "FontOutline", 2);

	// chat
	m_Settings.fChatPosX = reader.GetReal("gui", "ChatPosX", 170.0f);
	m_Settings.fChatPosY = reader.GetReal("gui", "ChatPosY", 235.0f);
	m_Settings.fChatSizeX = reader.GetReal("gui", "ChatSizeX", 1150.0f);
	m_Settings.fChatSizeY = reader.GetReal("gui", "ChatSizeY", 220.0f);
	m_Settings.iChatMaxMessages = reader.GetInteger("gui", "ChatMaxMessages", 6);

	// nametags
	m_Settings.fHealthBarWidth = reader.GetReal("gui", "HealthBarWidth", 100.0f);
	m_Settings.fHealthBarHeight = reader.GetReal("gui", "HealthBarHeight", 10.0f);

	// system
	m_Settings.iAndroidKeyboard = reader.GetBoolean("gui", "androidkeyboard", true);
	m_Settings.iFPSCounter = reader.GetBoolean("gui", "fps", true);
	m_Settings.iFPSCount = reader.GetInteger("gui", "FPSLimit", 120);
	m_Settings.bVoiceChatEnable = reader.GetBoolean("gui", "VoiceChatEnable", true);
	m_Settings.iByteMod = reader.GetInteger("client", "mod", 10);

	length = reader.Get("client", "version", SAMP_VERSION).copy(m_Settings.szSampVersion, 20);
	m_Settings.szSampVersion[length] = '\0';

	FLog("Settings loaded.");
}
