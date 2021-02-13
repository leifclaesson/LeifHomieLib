#pragma once

#include "LeifHomieLib.h"

#if defined(USE_PANGOLIN)
#include "PangolinMQTT.h"
#elif defined(USE_ASYNCMQTTCLIENT)
#include "AsyncMqttClient.h"
#elif defined(USE_ARDUINOMQTT)
#include "MQTT.h"
#include <list>
#endif
#include "HomieNode.h"
#include <map>

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#else
#include "WiFi.h"
#endif



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
	virtual ~HomieDevice();
	
	bool bDebug=false;
	int iInitialPublishingThrottle_ms=200;

	String strMqttServerIP;
	String strMqttUserName;
	String strMqttPassword;

	String strFriendlyName;
	String strID;

	bool bRapidUpdateRSSI=false;

	bool IsInitialized() { return bInitialized; }

	void Init();
	void Quit();

	void Loop();

	HomieNode * NewNode();

	bool IsConnected();
	bool IsConnecting() { return bConnecting; };

	uint16_t PublishDirect(const String & topic, uint8_t qos, bool retain, const String & payload);

#if defined(USE_PANGOLIN)
	PangolinMQTT mqtt;
#elif defined(USE_ASYNCMQTTCLIENT)
	AsyncMqttClient mqtt;
#elif defined(USE_ARDUINOMQTT)
	MQTTClient * pMQTT=NULL;
	WiFiClient net;
#endif

	uint32_t GetUptimeSeconds_WiFi();
	uint32_t GetUptimeSeconds_MQTT();

	const uint32_t * GetUptimeSecondsPtr_WiFi() { return &ulSecondCounter_WiFi; }
	const uint32_t * GetUptimeSecondsPtr_MQTT() { return &ulSecondCounter_MQTT; }

	void SetEnableMQTT(bool bEnable) { this->bEnableMQTT=bEnable; }

private:

	bool bEnableMQTT=true;
	bool bWasConnected=false;

	uint16_t Publish(const char* topic, uint8_t qos, bool retain, const char* payload = nullptr, size_t length = 0, bool dup = false, uint16_t message_id = 0);

	friend class HomieNode;
	friend class HomieProperty;

	void InitialUnsubscribe(HomieProperty * pProp);

	void DoInitialPublishing();

	unsigned long ulMqttReconnectCount=0;
	unsigned long ulHomieStatsTimestamp=0;
	unsigned long ulLastReconnect=0;

#if defined(USE_PANGOLIN)
	void onConnect(bool sessionPresent);
	void onDisconnect(int8_t reason);
	void onMqttMessage(const char* topic,uint8_t* payload, PANGO_PROPS properties,size_t len,size_t index,size_t total);
#elif defined(USE_ASYNCMQTTCLIENT)
	void onConnect(bool sessionPresent);
	void onDisconnect(AsyncMqttClientDisconnectReason reason);
	void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
#elif defined(USE_ARDUINOMQTT)
	void onConnect(bool sessionPresent);
	void onDisconnect(int8_t reason);
	void onClientCallbackAdvanced(MQTTClient *client, char topic[], char payload[], int len);
#endif

	bool bConnecting=false;

	bool bDoInitialPublishing=false;

	bool bInitialPublishingDone=false;

	int iInitialPublishing=0;
	int iInitialPublishing_Node=0;
	int iInitialPublishing_Prop=0;

	int iPubCount_Props=0;

	unsigned long ulInitialPublishing=0;

	unsigned long ulConnectTimestamp=0;

	bool bInitialized=false;


	String strTopic;
	char szWillTopic[128];

	std::vector<HomieNode *> vecNode;

	_map_incoming mapIncoming;

	uint32_t ulSecondCounter_Uptime=0;
	uint32_t ulSecondCounter_WiFi=0;
	uint32_t ulSecondCounter_MQTT=0;

	unsigned long ulLastLoopSecondCounterTimestamp=0;
	unsigned long ulLastLoopDeciSecondCounterTimestamp=0;


	bool bDoPublishDefaults=false;	//publish default retained values that did not yet exist in the controller
	unsigned long ulPublishDefaultsTimestamp=0;

	void HandleInitialPublishingError();

	bool bSendError=false;
	unsigned long ulSendErrorTimestamp;

	int GetErrorRetryFrequency();

	unsigned long GetReconnectInterval();

#if defined(USE_ARDUINOMQTT)
	std::list<HomieProperty *> listUnsubQueue;
#endif

};
