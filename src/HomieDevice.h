#pragma once

#include "AsyncMqttClient.h"
#include "HomieNode.h"
#include <map>

#define HOMIELIB_VERBOSE

typedef std::map<String, HomieProperty *> _map_incoming;

typedef std::function<void(const char * szText)> HomieDebugPrintCallback;

void HomieLibRegisterDebugPrintCallback(HomieDebugPrintCallback cb);

String HomieDeviceName(const char * in);

bool HomieParseRGB(const char * in, uint32_t & rgb);
bool HomieParseHSV(const char * in, uint32_t & rgb);


class HomieDevice
{
public:

	HomieDevice();
	
	int iInitialPublishingThrottle_ms=500;

	String strMqttServerIP;
	String strMqttUserName;
	String strMqttPassword;

	String strFriendlyName;
	String strID;

	bool bRapidUpdateRSSI=false;


	void Init();
	void Quit();

	void Loop();

	HomieNode * NewNode();

	bool IsConnected();

	void PublishDirect(const String & topic, uint8_t qos, bool retain, const String & payload);

	AsyncMqttClient mqtt;

private:

	friend class HomieNode;
	friend class HomieProperty;

	void DoInitialPublishing();

	unsigned long ulMqttReconnectCount=0;
	unsigned long ulHomieStatsTimestamp=0;
	unsigned long ulLastReconnect=0;

	void onConnect(bool sessionPresent);
	void onDisconnect(AsyncMqttClientDisconnectReason reason);
	void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);

	bool bConnecting=false;

	bool bDoInitialPublishing=false;

	bool bInitialPublishingDone=false;

	int iInitialPublishing=0;
	int iInitialPublishing_Node=0;
	int iInitialPublishing_Prop=0;

	int iPubCount_Props=0;

	unsigned long ulInitialPublishing=0;


	bool bDebug=false;

	bool bInitialized=false;


	String strTopic;
	char szWillTopic[128];

	std::vector<HomieNode *> vecNode;

	_map_incoming mapIncoming;

	unsigned long ulSecondCounter=0;
	unsigned long ulLastLoopSeconds=0;



	bool bDoPublishDefaults=false;	//publish default retained values that did not yet exist in the controller
	unsigned long ulPublishDefaultsTimestamp=0;

};
