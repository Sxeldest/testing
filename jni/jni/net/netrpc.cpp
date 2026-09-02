#include "../main.h"
#include "../game/game.h"
#include "netgame.h"
#include "../gui/gui.h"
#include "../vendor/encoding/encoding.h"
#include "../settings.h"

extern UI* pUI;
extern CGame *pGame;
extern CNetGame *pNetGame;
extern CSettings *pSettings;

extern int iNetModeNormalOnFootSendRate;
extern int iNetModeNormalInCarSendRate;
extern int iNetModeFiringSendRate;
extern int iNetModeSendMultiplier;

int g_iLagCompensationMode = 0;

#define REJECT_REASON_BAD_VERSION   1
#define REJECT_REASON_BAD_NICKNAME  2
#define REJECT_REASON_BAD_MOD		3
#define REJECT_REASON_BAD_PLAYERID	4

void ProcessIncommingEvent(PLAYERID playerID, int iEventType,
	uint32_t dwParam1, uint32_t dwParam2, uint32_t dwParam3);

void InitGame(RPCParameters *rpcParams)
{
	Log::traceLastFunc("[RPC-IN] InitGame");

	if (!pNetGame || !pNetGame->m_pNetSet) return;

	unsigned char * Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	RakNet::BitStream bsInitGame(Data,(iBitLength/8)+1,false);

	PLAYERID MyPlayerID;
	bool bLanMode, bStuntBonus;

	bsInitGame.ReadCompressed(pNetGame->m_pNetSet->bZoneNames);
	bsInitGame.ReadCompressed(pNetGame->m_pNetSet->bUseCJWalk);
	bsInitGame.ReadCompressed(pNetGame->m_pNetSet->bAllowWeapons);
	bsInitGame.ReadCompressed(pNetGame->m_pNetSet->bLimitGlobalChatRadius);
	bsInitGame.Read(pNetGame->m_pNetSet->fGlobalChatRadius);

	bsInitGame.ReadCompressed(bStuntBonus);

	bsInitGame.Read(pNetGame->m_pNetSet->fNameTagDrawDistance);
	bsInitGame.ReadCompressed(pNetGame->m_pNetSet->bDisableInteriorEnterExits);
	bsInitGame.ReadCompressed(pNetGame->m_pNetSet->bNameTagLOS);
	bsInitGame.ReadCompressed(pNetGame->m_pNetSet->bManualVehicleEngineAndLight);
	bsInitGame.Read(pNetGame->m_pNetSet->iSpawnsAvailable);

	bsInitGame.Read(MyPlayerID);
	bsInitGame.ReadCompressed(pNetGame->m_pNetSet->bShowNameTags);
	bsInitGame.Read(pNetGame->m_pNetSet->iShowPlayerMarkers);
	bsInitGame.Read(pNetGame->m_pNetSet->byteWorldTime_Hour);
	bsInitGame.Read(pNetGame->m_pNetSet->byteWeather);
	bsInitGame.Read(pNetGame->m_pNetSet->fGravity);
	bsInitGame.ReadCompressed(bLanMode);
	bsInitGame.Read(pNetGame->m_pNetSet->iDeathDropMoney);
	bsInitGame.ReadCompressed(pNetGame->m_pNetSet->bInstagib);

	bsInitGame.Read(iNetModeNormalOnFootSendRate);
	bsInitGame.Read(iNetModeNormalInCarSendRate);
	bsInitGame.Read(iNetModeFiringSendRate);
	bsInitGame.Read(iNetModeSendMultiplier);

	int iLagCompensation;
	bsInitGame.Read(iLagCompensation);
	pNetGame->m_pNetSet->bFriendlyFire = (iLagCompensation != 0);

	uint8_t byteStrLen;
	bsInitGame.Read(byteStrLen);
	if(byteStrLen)
	{
		memset(pNetGame->m_szHostName, 0, sizeof(pNetGame->m_szHostName));
		bsInitGame.Read(pNetGame->m_szHostName, byteStrLen);
	}
	pNetGame->m_szHostName[byteStrLen] = '\0';

	uint8_t byteVehicleModels[212];
	bsInitGame.Read((char*)&byteVehicleModels[0], 212);
	bsInitGame.Read(pNetGame->m_pNetSet->iVehicleFriendlyFire);

	if (pGame) {
		pGame->DisableAutoAim();
	}

	pNetGame->GetPlayerPool()->SetLocalPlayerID(MyPlayerID);
	pGame->EnableStuntBonus(bStuntBonus);

	pNetGame->SetGameState(GAMESTATE_CONNECTED);
	pNetGame->GetPlayerPool()->GetLocalPlayer()->HandleClassSelection();

	pGame->SetGravity(pNetGame->m_pNetSet->fGravity);
	pGame->SetWorldWeather(pNetGame->m_pNetSet->byteWeather);
	pGame->ToggleCJWalk(pNetGame->m_pNetSet->bUseCJWalk);

	if (pUI) {
		pUI->chat()->addDebugMessage("Connected to {B9C9BF}%.64s", Encoding::cp2utf(pNetGame->m_szHostName).c_str());
		pUI->chat()->setVisible(true);
		extern bool Mchat;
		Mchat = true;
		pUI->voicebutton()->setVisible(true);
	}
}

void DialogBox(RPCParameters *rpcParams)
{
	Log::traceLastFunc("[RPC-IN] DialogBox");

	unsigned char * Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);

	uint16_t wDialogID;
	uint8_t byteDialogStyle;
	uint8_t len;
	char szBuff[8192 + 1];

	char szTitle[512];
	char szButton1[512];
	char szButton2[512];

	if (!bsData.Read(wDialogID)) return;
	if (!bsData.Read(byteDialogStyle)) return;

	// title
	if (!bsData.Read(len)) return;
	if (len > 0) {
		if (len > 511) len = 511;
		bsData.Read(szTitle, len);
		szTitle[len] = '\0';
	} else {
		szTitle[0] = '\0';
	}

	// button1
	if (!bsData.Read(len)) return;
	if (len > 0) {
		if (len > 511) len = 511;
		bsData.Read(szButton1, len);
		szButton1[len] = '\0';
	} else {
		szButton1[0] = '\0';
	}

	// button2
	if (!bsData.Read(len)) return;
	if (len > 0) {
		if (len > 511) len = 511;
		bsData.Read(szButton2, len);
		szButton2[len] = '\0';
	} else {
		szButton2[0] = '\0';
	}

	// info (message)
	if (!stringCompressor->DecodeString(szBuff, 8192, &bsData)) {
		szBuff[0] = '\0';
	}

	if (wDialogID == 0xFFFF) {
		if (pUI) pUI->dialog()->Hide();
		return;
	}

	DialogStyle style = (DialogStyle)byteDialogStyle;

	if (pUI) 
	{
		pUI->dialog()->Show(wDialogID, style,
			Encoding::cp2utf(szTitle),
			Encoding::cp2utf(szBuff),
			Encoding::cp2utf(szButton1),
			Encoding::cp2utf(szButton2));
	}

	return;
}

// 0.3.7
void Chat(RPCParameters *rpcParams)
{
	Log::traceLastFunc("[RPC-IN] Chat");

	unsigned char * Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
	PLAYERID playerId;
	uint8_t byteMsgLen;
	char szMessage[256];
	memset(szMessage, 0, sizeof(szMessage));

	if (pNetGame->GetGameState() != GAMESTATE_CONNECTED) return;

	bsData.Read(playerId);
	bsData.Read(byteMsgLen);
	bsData.Read(szMessage, byteMsgLen);
	szMessage[byteMsgLen] = '\0';

	Log::addParameter("playerId", playerId);
	Log::addParameter("szMessage", szMessage);

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	if (playerId == pPlayerPool->GetLocalPlayerID()) 
	{
		if (pUI) pUI->chat()->addChatMessage(Encoding::cp2utf(szMessage),
			pPlayerPool->GetLocalPlayerName(), UI::fixcolor(pPlayerPool->GetLocalPlayer()->GetPlayerColorAsRGBA()));
	}
	else {
		CRemotePlayer *pRemotePlayer = pPlayerPool->GetAt(playerId);
		if (pRemotePlayer) {
			pRemotePlayer->Say(szMessage);
		}
	}
}

// 0.3.7
void ClientMessage(RPCParameters *rpcParams)
{
	Log::traceLastFunc("[RPC-IN] Client message ");

	unsigned char * Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	uint32_t dwColor, dwStrLen;
	char szMsg[256];
	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
	
	memset(szMsg, 0, 256);
	bsData.Read(dwColor);
	bsData.Read(dwStrLen);

	Log::addParameter("dwColor", dwColor);
	Log::addParameter("dwStrLen", dwStrLen);

	if (dwStrLen <= 255)
	{
		bsData.Read(szMsg, dwStrLen);
		szMsg[dwStrLen] = '\0';
		Log::addParameter("szMsg", szMsg);
		if (pUI) pUI->chat()->addClientMessage(Encoding::cp2utf(szMsg), UI::fixcolor(dwColor));
	}
}

// 0.3.7
void RequestClass(RPCParameters *rpcParams)
{
	Log::traceLastFunc("[RPC-IN] Request class");

	unsigned char * Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
	uint8_t byteRequestOutcome = 0;
	PLAYER_SPAWN_INFO SpawnInfo;
	CLocalPlayer *pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();

	bsData.Read(byteRequestOutcome);
	bsData.Read((char*)&SpawnInfo, sizeof(PLAYER_SPAWN_INFO));

	Log::addParameter("byteRequestOutcome", byteRequestOutcome);
	Log::addParameter("SpawnInfo.byteTeam", SpawnInfo.byteTeam);
	Log::addParameter("SpawnInfo.fRotation", SpawnInfo.fRotation);
	Log::addParameter("SpawnInfo.iSkin", SpawnInfo.iSkin);
	Log::addParameter("SpawnInfo.iSpawnWeapons", SpawnInfo.iSpawnWeapons);
	Log::addParameter("SpawnInfo.iSpawnWeaponsAmmo", SpawnInfo.iSpawnWeaponsAmmo);

	if (byteRequestOutcome) {
		pLocalPlayer->SetSpawnInfo(&SpawnInfo);
		pLocalPlayer->HandleClassSelectionOutcome(true);
	}
	else {
		pLocalPlayer->HandleClassSelectionOutcome(false);
	}
}

// 0.3.7
void RequestSpawn(RPCParameters *rpcParams)
{
	Log::traceLastFunc("[RPC-IN] Request spawn");

	unsigned char * Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
	uint8_t byteRequestOutcome = false;
	CLocalPlayer *pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();

	bsData.Read(byteRequestOutcome);
	Log::addParameter("byteRequestOutcome", byteRequestOutcome);

	if (byteRequestOutcome == 2 || (byteRequestOutcome && pLocalPlayer->m_bWaitingForSpawnRequestReply)) {
		pLocalPlayer->Spawn();
	}
	else {
		pLocalPlayer->m_bWaitingForSpawnRequestReply = false;
	}
}

// 0.3.7
void ToggleClock(RPCParameters *rpcParams)
{
	Log::traceLastFunc("[RPC-IN] Toggle clock");

	unsigned char * Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
	uint8_t byteClock;

	bsData.Read(byteClock);
	Log::addParameter("byteClock", byteClock);

	if (byteClock) {
		pNetGame->m_pNetSet->byteHoldTime = false;
	}
	else {
		pNetGame->m_pNetSet->byteHoldTime = true;
		pGame->GetWorldTime((int*)&pNetGame->m_pNetSet->byteWorldTime_Hour, 
			(int*)&pNetGame->m_pNetSet->byteWorldTime_Minute);
	}
}

// 0.3.7
void SetTimeEx(RPCParameters *rpcParams)
{
	Log::traceLastFunc("[RPC-IN] SetTimeEx");

	unsigned char * Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
	uint8_t byteHour;
	uint8_t byteMinute;

	bsData.Read(byteHour);
	bsData.Read(byteMinute);

	Log::addParameter("byteHour", byteHour);
	Log::addParameter("byteMinute", byteMinute);

	pGame->SetWorldTime(byteHour, byteMinute);
	pNetGame->m_pNetSet->byteWorldTime_Hour = byteHour;
	pNetGame->m_pNetSet->byteWorldTime_Minute = byteMinute;
}

// 0.3.7
void Weather(RPCParameters *rpcParams)
{
	Log::traceLastFunc("[RPC-IN] Weather");

	unsigned char * Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
	uint8_t byteWeather;
	
	bsData.Read(byteWeather);
	
	Log::addParameter("byteWeather", byteWeather);

	pNetGame->m_pNetSet->byteWeather = byteWeather;
	pGame->SetWorldWeather(byteWeather);
}

// 0.3.7
void WorldTime(RPCParameters *rpcParams)
{
	Log::traceLastFunc("[RPC-IN] World time");

	unsigned char * Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
	uint8_t byteWorldTIme;
	bsData.Read(byteWorldTIme);

	Log::addParameter("byteWorldTIme", byteWorldTIme);

	pNetGame->m_pNetSet->byteWorldTime_Hour = byteWorldTIme;
}

// 0.3.7
void ConnectionRejected(RPCParameters *rpcParams)
{
	Log::traceLastFunc("[RPC-IN] Connection rejected");

	unsigned char * Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	uint8_t byteRejectReason;
	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
	bsData.Read(byteRejectReason);

	Log::addParameter("byteRejectReason", byteRejectReason);

	switch (byteRejectReason)
	{
	case REJECT_REASON_BAD_VERSION:
		if (pUI) pUI->chat()->addInfoMessage("CONNECTION REJECTED: Incorrect Version.");
		break;

	case REJECT_REASON_BAD_NICKNAME:
		if (pUI) pUI->chat()->addInfoMessage("CONNECTION REJECTED: Unacceptable NickName");
		if (pUI) pUI->chat()->addInfoMessage("Please choose another nick between and 3-20 characters");
		if (pUI) pUI->chat()->addInfoMessage("Please use only a-z, A-Z, 0-9");
		if (pUI) pUI->chat()->addInfoMessage("Use /quit to exit or press ESC and select Quit Game");
		pNetGame->SetGameState(GAMESTATE_WAIT_CONNECT);
		break;

	case REJECT_REASON_BAD_MOD:
		if (pUI) pUI->chat()->addInfoMessage("CONNECTION REJECTED: Bad mod version.");
		break;
	case REJECT_REASON_BAD_PLAYERID:
		if (pUI) pUI->chat()->addInfoMessage("CONNECTION REJECTED: Unable to allocate a player slot.");
		break;
	}

	pNetGame->GetRakClient()->Disconnect(500);
}

// 0.3.7
void GameModeRestart(RPCParameters *rpcParams)
{
	Log::traceLastFunc("[RPC-IN] Gamemode restart");

	pNetGame->ShutdownForGameModeRestart();
}

// 0.3.7
void ServerJoin(RPCParameters *rpcParams)
{
	Log::traceLastFunc("[RPC-IN] Server join");

	unsigned char * Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	CRemotePlayer *pRemotePlayer = nullptr;
	uint8_t byteNameLen;
	uint8_t byteIsNPC;
	PLAYERID PlayerID;
	uint32_t dwMarkerColor;
	char szPlayerName[256];
	memset(szPlayerName, 0, sizeof(szPlayerName));

	bsData.Read(PlayerID);
	bsData.Read(dwMarkerColor);
	bsData.Read(byteIsNPC);
	bsData.Read(byteNameLen);
	bsData.Read(szPlayerName, byteNameLen);
	szPlayerName[byteNameLen] = '\0';

	Log::addParameter("PlayerID", PlayerID);
	Log::addParameter("dwMarkerColor", dwMarkerColor);
	Log::addParameter("byteIsNPC", byteIsNPC);
	Log::addParameter("szPlayerName", szPlayerName);

	if (strlen(szPlayerName) > MAX_PLAYER_NAME) {
		return;
	}

	pPlayerPool->New(PlayerID, szPlayerName, byteIsNPC);

	pRemotePlayer = pPlayerPool->GetAt(PlayerID);

	if (dwMarkerColor && pRemotePlayer) {
		pRemotePlayer->SetPlayerColor(dwMarkerColor);
	}
}

// 0.3.7
void ServerQuit(RPCParameters *rpcParams)
{
	Log::traceLastFunc("[RPC-IN] Server quit");

	unsigned char * Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
	PLAYERID playerId;
	uint8_t byteReason;
	bsData.Read(playerId);
	bsData.Read(byteReason);

	Log::addParameter("playerId", playerId);
	Log::addParameter("byteReason", byteReason);

	pNetGame->GetPlayerPool()->Delete(playerId, byteReason);
}

// 0.3.7
void WorldPlayerAdd(RPCParameters *rpcParams)
{
	Log::traceLastFunc("[RPC-IN] World player add");

	unsigned char * Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	CRemotePlayer *pRemotePlayer = nullptr;
	CPlayerPed *pPlayerPed = nullptr;
	PLAYERID playerId;
	uint8_t byteTeam;
	int iSkin;
	VECTOR vecPos;
	float fRotation;
	uint32_t dwColor;
	uint8_t byteFightingStyle;

	bsData.Read(playerId);
	bsData.Read(byteTeam);
	bsData.Read(iSkin);
	bsData.Read(vecPos.X);
	bsData.Read(vecPos.Y);
	bsData.Read(vecPos.Z);
	bsData.Read(fRotation);
	bsData.Read(dwColor);
	bsData.Read(byteFightingStyle);

	if (pPlayerPool)
	{
		pRemotePlayer = pPlayerPool->GetAt(playerId);
		if (pRemotePlayer) 
		{
			if (pRemotePlayer->Spawn(byteTeam, iSkin, &vecPos, fRotation, dwColor, byteFightingStyle))
			{
				if (pPlayerPool->IsPlayerNPC(playerId)) {
					pPlayerPed = pRemotePlayer->GetPlayerPed();
					if (pPlayerPed) {
						pPlayerPed->SetImmunities(0, 1, 1, 0, 0);
					}
				}
			}
		}
	}
}

// 0.3.7
void WorldPlayerRemove(RPCParameters *rpcParams)
{
	Log::traceLastFunc("[RPC-IN] World player remove");

	unsigned char * Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	CRemotePlayer *pRemotePlayer = nullptr;
	PLAYERID playerId;

	bsData.Read(playerId);
	Log::addParameter("playerId", playerId);

	if (pPlayerPool) {
		pRemotePlayer = pPlayerPool->GetAt(playerId);
		if (pRemotePlayer) {
			pRemotePlayer->Remove();
		}
	}
}

// 0.3.7
void WorldPlayerDeath(RPCParameters *rpcParams)
{
	Log::traceLastFunc("[RPC-IN] World player death");

	unsigned char * Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	CRemotePlayer *pRemotePlayer = nullptr;
	PLAYERID playerId;

	bsData.Read(playerId);
	Log::addParameter("playerId", playerId);

	if (pPlayerPool) {
		pRemotePlayer = pPlayerPool->GetAt(playerId);
		if (pRemotePlayer) {
			pRemotePlayer->HandleDeath();
		}
	}
}

// 0.3.7
void WorldVehicleAdd(RPCParameters* rpcParams)
{
	Log::traceLastFunc("[RPC-IN] World vehicle add");

	unsigned char* Data = reinterpret_cast<unsigned char*>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	NEW_VEHICLE VehicleInfo;
	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);

	CVehiclePool* pVehiclePool = pNetGame->GetVehiclePool();
	if (!pVehiclePool) return;

	memset(&VehicleInfo, 0, sizeof(NEW_VEHICLE));

	bsData.Read((char*)& VehicleInfo, sizeof(NEW_VEHICLE));

	if (VehicleInfo.iVehicleType < 400 || VehicleInfo.iVehicleType > 611) return;

	pVehiclePool->New(&VehicleInfo);
}

// 0.3.7
void WorldVehicleRemove(RPCParameters* rpcParams)
{
	Log::traceLastFunc("[RPC-IN] World vehicle remove");

	unsigned char* Data = reinterpret_cast<unsigned char*>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);

	VEHICLEID VehicleID;
	CVehiclePool* pVehiclePool = pNetGame->GetVehiclePool();
	if (!pVehiclePool) return;

	bsData.Read(VehicleID);

	if(pVehiclePool->GetAt(VehicleID))
		pVehiclePool->Delete(VehicleID);
}

// 0.3.7
void TimerUpdate(RPCParameters *rpcParams)
{
	Log::traceLastFunc("[RPC-IN] Timer update");

	unsigned char * Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
	uint32_t dwTime = 0;

	bsData.Read(dwTime);

	pGame->UpdateGlobalTimer(dwTime);
}

// 0.3.7
void ScmEvent(RPCParameters *rpcParams)
{
	Log::traceLastFunc("[RPC-IN] SCM event");

	unsigned char * Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
	PLAYERID playerId;
	int iEvent;
	uint32_t dwParam1, dwParam2, dwParam3;

	bsData.Read(playerId);
	bsData.Read(iEvent);
	bsData.Read(dwParam1);
	bsData.Read(dwParam2);
	bsData.Read(dwParam3);

	ProcessIncommingEvent(playerId, iEvent, dwParam1, dwParam2, dwParam3);
}

// 0.3.7
void UpdateScoresPingsIPs(RPCParameters *rpcParams)
{
	Log::traceLastFunc("[RPC-IN] Update scores and pings");

	unsigned char * Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	PLAYERID playerId;
	int iPlayerScore;
	int iPlayerPing;

	for (int i = 0; i < (iBitLength / 8) / 10; i++) {
		bsData.Read(playerId);
		bsData.Read(iPlayerScore);
		bsData.Read(iPlayerPing);

		pPlayerPool->UpdatePlayerScore(playerId, iPlayerScore);
		pPlayerPool->UpdatePlayerPing(playerId, iPlayerPing);
	}
}

// 0.3.7
void Pickup(RPCParameters *rpcParams)
{
	Log::traceLastFunc("[RPC-IN] Pickup");

	unsigned char * Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	int iIndex;
	PICKUP Pickup;
	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
	bsData.Read(iIndex);
	bsData.Read((char*)&Pickup, sizeof(PICKUP));

	CPickupPool *pPickupPool = pNetGame->GetPickupPool();
	if (pPickupPool) {
		pPickupPool->New(&Pickup, iIndex);
	}
}

// 0.3.7
void DestroyPickup(RPCParameters *rpcParams)
{
	Log::traceLastFunc("[RPC-IN] Destroy pickup");

	unsigned char * Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	int iIndex;
	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
	bsData.Read(iIndex);

	CPickupPool *pPickupPool = pNetGame->GetPickupPool();
	if (pPickupPool) {
		pPickupPool->Destroy(iIndex);
	}
}

// 0.3.7
void Create3DTextLabel(RPCParameters* rpcParams)
{
	Log::traceLastFunc("[RPC-IN] Create 3D text label");

	unsigned char* Data = reinterpret_cast<unsigned char*>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	C3DTextLabelPool* pTextLabelPool = pNetGame->GetTextLabelPool();
	if (pTextLabelPool == nullptr) return;

	uint16_t wLabelId;
	TEXT_LABEL label;
	char szBuffer[2048 + 1];
	memset(szBuffer, 0, sizeof(szBuffer));

	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
	bsData.Read(wLabelId);
	bsData.Read(label.dwColor);
	bsData.Read(label.vecPos.X);
	bsData.Read(label.vecPos.Y);
	bsData.Read(label.vecPos.Z);
	bsData.Read(label.fDistance);
	bsData.Read(label.bTestLOS);
	bsData.Read(label.playerId);
	bsData.Read(label.vehicleId);

	stringCompressor->DecodeString(szBuffer, sizeof szBuffer, &bsData);
	label.text = szBuffer;

	if (wLabelId < MAX_TEXT_LABELS) {
		pTextLabelPool->NewLabel(wLabelId, &label);
	}
}

// 0.3.7
void Update3DTextLabel(RPCParameters* rpcParams)
{
	Log::traceLastFunc("[RPC-IN] Update 3D text label");

	unsigned char* Data = reinterpret_cast<unsigned char*>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	C3DTextLabelPool* pTextLabelPool = pNetGame->GetTextLabelPool();
	if (pTextLabelPool == nullptr)  return;

	uint16_t wLabelId;
	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
	bsData.Read(wLabelId);
	
	if (pTextLabelPool->GetSlotState(wLabelId)) {
			pTextLabelPool->ClearLabel(wLabelId);
	}
}

// 0.3.7
void SetCheckpoint(RPCParameters* rpcParams)
{
	Log::traceLastFunc("[RPC-IN] Set checkpoint");

	unsigned char* Data = reinterpret_cast<unsigned char*>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	VECTOR vecPos;
	float fSize;
	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
	bsData.Read(vecPos.X);
	bsData.Read(vecPos.Y);
	bsData.Read(vecPos.Z);
	bsData.Read(fSize);

	VECTOR vecSize;
	vecSize.X = fSize;
	vecSize.Y = fSize;
	vecSize.Z = fSize;

	pGame->SetCheckpointInformation(&vecPos, &vecSize);
	pGame->m_bCheckpointsEnabled = true;
}

// 0.3.7
void DisableCheckpoint(RPCParameters* rpcParams)
{
	Log::traceLastFunc("[RPC-IN] Disable checkpoint");

	pGame->m_bCheckpointsEnabled = false;
}

// 0.3.7
void SetRaceCheckpoint(RPCParameters* rpcParams)
{
	Log::traceLastFunc("[RPC-IN] Set race checkpoint");

	unsigned char* Data = reinterpret_cast<unsigned char*>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	uint8_t byteType;
	VECTOR vecPos;
	VECTOR vecNextPos;
	float fRadius;
	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
	bsData.Read(byteType);
	bsData.Read(vecPos.X);
	bsData.Read(vecPos.Y);
	bsData.Read(vecPos.Z);
	bsData.Read(vecNextPos.X);
	bsData.Read(vecNextPos.Y);
	bsData.Read(vecNextPos.Z);
	bsData.Read(fRadius);

	pGame->SetRaceCheckpointInformation(byteType, &vecPos, &vecNextPos, fRadius);
	pGame->m_bRaceCheckpointsEnabled = true;
}

// 0.3.7
void DisableRaceCheckpoint(RPCParameters* rpcParams)
{
	Log::traceLastFunc("[RPC-IN] Disable race checkpoint");

	pGame->m_bRaceCheckpointsEnabled = false;
}

// 0.3.7
void DamageVehicle(RPCParameters *rpcParams)
{
	Log::traceLastFunc("[RPC-IN] Damage vehicle");

	RakNet::BitStream bsData(rpcParams->input, (rpcParams->numberOfBitsOfData / 8) + 1, false);

	VEHICLEID vehId;
	bsData.Read(vehId);
	if (pNetGame->GetVehiclePool()->GetSlotState(vehId))
	{
		uint32_t dwPanelStatus, dwDoorStatus;
		uint8_t byteLightStatus, byteTireStatus;

		bsData.Read(dwPanelStatus);
		bsData.Read(dwDoorStatus);
		bsData.Read(byteLightStatus);
		bsData.Read(byteTireStatus);

		CVehicle* pVehicle = pNetGame->GetVehiclePool()->GetAt(vehId);
		if (pVehicle)
		{
			pVehicle->SetWheelPoppedStatus(byteTireStatus);
			pVehicle->UpdateDamageStatus(dwPanelStatus, dwDoorStatus, byteLightStatus);
		}
	}
}

void SetVehicleTireStatus(RPCParameters* rpcParams)
{
	unsigned char* Data = reinterpret_cast<unsigned char *>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;
	RakNet::BitStream bsData((unsigned char*)Data, (iBitLength / 8) + 1, false);

	VEHICLEID vehicleId;
	uint8_t byteTire;

	bsData.Read(vehicleId);
	if(vehicleId < 0 || vehicleId >= MAX_VEHICLES)
		return;

	bsData.Read(byteTire);

	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
	if(pVehiclePool)
	{
		CVehicle *pVehicle = pVehiclePool->GetAt(vehicleId);
		if(pVehicle)
			pVehicle->SetWheelPoppedStatus(byteTire);
	}
}

// 0.3.7
void EnterVehicle(RPCParameters* rpcParams)
{
	Log::traceLastFunc("[RPC-IN] Enter vehicle");

	unsigned char* Data = reinterpret_cast<unsigned char*>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	PLAYERID PlayerID;
	VEHICLEID VehicleID;
	uint8_t bytePassenger;
	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
	bsData.Read(PlayerID);
    if(PlayerID < 0 || PlayerID >= MAX_PLAYERS)
        return;

	bsData.Read(VehicleID);
    if(VehicleID < 0 || VehicleID >= MAX_VEHICLES)
        return;

	bsData.Read(bytePassenger);

	CRemotePlayer* pRemotePlayer = pNetGame->GetPlayerPool()->GetAt(PlayerID);
	if (pRemotePlayer) {
		pRemotePlayer->EnterVehicle(VehicleID, bytePassenger);
	}
}

// 0.3.7
void ExitVehicle(RPCParameters* rpcParams)
{
	Log::traceLastFunc("[RPC-IN] Exit vehicle");

	unsigned char* Data = reinterpret_cast<unsigned char*>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	PLAYERID PlayerID;
	VEHICLEID VehicleID;
	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
	bsData.Read(PlayerID);
	bsData.Read(VehicleID);

	CRemotePlayer* pRemotePlayer = pNetGame->GetPlayerPool()->GetAt(PlayerID);
	if (pRemotePlayer) {
		pRemotePlayer->ExitVehicle();
	}
}

// 0.3.7
void VehicleParamsEx(RPCParameters* rpcParams)
{
	Log::traceLastFunc("[RPC-IN] VehicleParamsEx");

	unsigned char* Data = reinterpret_cast<unsigned char*>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	CVehiclePool* pVehiclePool = pNetGame->GetVehiclePool();
	if (!pVehiclePool) return;

	VEHICLEID VehicleID;
	VEHICLE_PARAMS_EX vehParamsEx;
	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
	bsData.Read(VehicleID);
	bsData.Read((char*)& vehParamsEx, sizeof(VEHICLE_PARAMS_EX));

	CVehicle* pVehicle = pVehiclePool->GetAt(VehicleID);
	if (!pVehicle) return;

	pVehicle->ApplyEngineState(vehParamsEx.byteEngine);
	pVehicle->ApplyLightState(vehParamsEx.byteLight);
	pVehicle->SetDoorState(vehParamsEx.byteDoors == 1);
}

// 0.3.7
void ShowActor(RPCParameters* rpcParams)
{
	Log::traceLastFunc("[RPC-IN] Show actor");

	unsigned char* Data = reinterpret_cast<unsigned char*>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	CActorPool* pActorPool = pNetGame->GetActorPool();
	if (!pActorPool) return;

	NEW_ACTOR newActor;
	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
	bsData.Read((char*)& newActor, sizeof(NEW_ACTOR));

	pActorPool->New(&newActor);
}

// 0.3.7
void HideActor(RPCParameters* rpcParams)
{
	Log::traceLastFunc("[RPC-IN] Hide actor");

	unsigned char* Data = reinterpret_cast<unsigned char*>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	CActorPool* pActorPool = pNetGame->GetActorPool();
	if (!pActorPool) return;

	PLAYERID ActorID;
	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);
	bsData.Read(ActorID);

	pActorPool->Delete(ActorID);
}

void ScrChatBubble(RPCParameters* rpcParams)
{
	Log::traceLastFunc("RPC: ChatBubble");

	unsigned char* Data = reinterpret_cast<unsigned char*>(rpcParams->input);
	int iBitLength = rpcParams->numberOfBitsOfData;

	ChatBubble* pPlayerBubblePool = pNetGame->GetPlayerBubblePool();
	if (pPlayerBubblePool == nullptr) return;

	RakNet::BitStream bsData(Data, (iBitLength / 8) + 1, false);

	PLAYERID playerId;
	uint32_t color;
	float drawDistance;
	uint32_t duration;
	uint8_t byteTextLen;

	char szText[256];
	memset(szText, 0, 256);

	bsData.Read(playerId);

	if (playerId == pNetGame->GetPlayerPool()->GetLocalPlayerID()) return;
	if (playerId >= MAX_PLAYERS) return;

	bsData.Read(color);
	bsData.Read(drawDistance);
	bsData.Read(duration);
	bsData.Read(byteTextLen);
	bsData.Read((char*)szText, byteTextLen);
	szText[byteTextLen] = '\0';

	pPlayerBubblePool->New(playerId, szText, color, drawDistance, duration);
}

void RegisterRPCs(RakClientInterface *pRakClient)
{
	Log::traceLastFunc("Registering RPC's..");
    pRakClient->RegisterAsRemoteProcedureCall(&RPC_InitGame, InitGame);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrCreate3DTextLabel, Create3DTextLabel);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrUpdate3DTextLabel, Update3DTextLabel);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_DialogBox, DialogBox);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_SetCheckpoint, SetCheckpoint);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_DisableCheckpoint, DisableCheckpoint);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_SetRaceCheckpoint, SetRaceCheckpoint);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_DisableRaceCheckpoint, DisableRaceCheckpoint);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_UpdateScoresPingsIPs, UpdateScoresPingsIPs);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_GameModeRestart, GameModeRestart);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ConnectionRejected, ConnectionRejected);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ClientMessage, ClientMessage);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_WorldTime, WorldTime);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_Pickup, Pickup);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_DestroyPickup, DestroyPickup);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScmEvent, ScmEvent);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_Weather, Weather);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_SetTimeEx, SetTimeEx);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ToggleClock, ToggleClock);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_TimerUpdate, TimerUpdate);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_WorldPlayerAdd, WorldPlayerAdd);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_WorldPlayerDeath, WorldPlayerDeath);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_WorldPlayerRemove, WorldPlayerRemove);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_WorldVehicleAdd, WorldVehicleAdd);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_WorldVehicleRemove, WorldVehicleRemove);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_DamageVehicle, DamageVehicle);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ScrSetVehicleTireStatus, SetVehicleTireStatus);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_VehicleParamsEx, VehicleParamsEx);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_EnterVehicle, EnterVehicle);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ExitVehicle, ExitVehicle);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ServerJoin, ServerJoin);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ServerQuit, ServerQuit);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_Chat, Chat);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_RequestClass, RequestClass);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_RequestSpawn, RequestSpawn);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ShowActor, ShowActor);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_HideActor, HideActor);
	pRakClient->RegisterAsRemoteProcedureCall(&RPC_ChatBubble, ScrChatBubble);
}

void UnregisterRPCs(RakClientInterface *pRakClient)
{
	Log::traceLastFunc("Unregistering RPC's..");
    pRakClient->UnregisterAsRemoteProcedureCall(&RPC_InitGame);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrCreate3DTextLabel);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrUpdate3DTextLabel);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_DialogBox);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_SetCheckpoint);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_DisableCheckpoint);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_UpdateScoresPingsIPs);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_GameModeRestart);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ConnectionRejected);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ClientMessage);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_WorldTime);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_Pickup);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_DestroyPickup);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScmEvent);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_Weather);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_SetTimeEx);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ToggleClock);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_TimerUpdate);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_WorldPlayerAdd);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_WorldPlayerDeath);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_WorldPlayerRemove);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_WorldVehicleAdd);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_WorldVehicleRemove);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_DamageVehicle);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ScrSetVehicleTireStatus);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_VehicleParamsEx);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_EnterVehicle);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ExitVehicle);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ServerJoin);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ServerQuit);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_Chat);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_RequestClass);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_RequestSpawn);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ShowActor);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_HideActor);
	pRakClient->UnregisterAsRemoteProcedureCall(&RPC_ChatBubble);
}
