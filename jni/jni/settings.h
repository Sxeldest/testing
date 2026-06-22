#pragma once

#define MAX_SETTINGS_STRING	0x7F

struct stSettings
{
	// client
	char szNickName[24+1];
	char szHost[MAX_SETTINGS_STRING+1];
	int iPort;
	char szPassword[MAX_SETTINGS_STRING+1];

	// debug
	bool bDebug;

	// gui
	char szFont[40];
	float fFontSize;
	int iFontOutline;

	// chat
	float fChatPosX;
	float fChatPosY;
	float fChatSizeX;
	float fChatSizeY;
	int iChatMaxMessages;

	// nametags
	float fHealthBarWidth;
	float fHealthBarHeight;

	// system
	bool iAndroidKeyboard;
	bool iFPSCounter;
	int iFPSCount;
	bool bVoiceChatEnable;
	int iByteMod;

	char szSampVersion[20];
};

class CSettings
{
public:
	CSettings();
	~CSettings();

	stSettings& Get() { return m_Settings; }
	
private:
	struct stSettings m_Settings;
};
