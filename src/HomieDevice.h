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
	
	int iInitialPublishingThrottle_ms=200;

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

	uint16_t PublishDirect(const String & topic, uint8_t qos, bool retain, const String & payload);

	AsyncMqttClient mqtt;

	unsigned long GetUptimeSeconds_WiFi();
	unsigned long GetUptimeSeconds_MQTT();

private:

	uint16_t Publish(const char* topic, uint8_t qos, bool retain, const char* payload = nullptr, size_t length = 0, bool dup = false, uint16_t message_id = 0);

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

	unsigned long ulConnectTimestamp=0;

	bool bDebug=false;

	bool bInitialized=false;


	String strTopic;
	char szWillTopic[128];

	std::vector<HomieNode *> vecNode;

	_map_incoming mapIncoming;

	unsigned long ulSecondCounter_Uptime=0;
	unsigned long ulSecondCounter_WiFi=0;
	unsigned long ulSecondCounter_MQTT=0;

	unsigned long ulLastLoopSecondCounterTimestamp=0;
	unsigned long ulLastLoopDeciSecondCounterTimestamp=0;


	bool bDoPublishDefaults=false;	//publish default retained values that did not yet exist in the controller
	unsigned long ulPublishDefaultsTimestamp=0;

	void HandleInitialPublishingError();

	bool bSendError=false;
	unsigned long ulSendErrorTimestamp;

	int GetErrorRetryFrequency();

	unsigned long GetReconnectInterval();


};
