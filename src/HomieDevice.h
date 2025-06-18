#pragma once

#include "LeifHomieLib.h"

#if defined(USE_PANGOLIN)
#include "PangolinMQTT.h"
#elif defined(USE_ASYNCMQTTCLIENT)
#include "AsyncMqttClient.h"
#elif defined(USE_ARDUINOMQTT)
#include "MQTT.h"
#include <list>
#elif defined(USE_PUBSUBCLIENT)
#include "PubSubClient.h"
#include <list>
#elif defined(USE_MPSCG)
	#define _MQTT_PUBSUB_LOGLEVEL_ 0
	#ifndef MQTT_MAX_PACKET_SIZE
		#define MQTT_MAX_PACKET_SIZE 256
	#endif
	#include "MQTTPubSubClient_Generic.h"
	#include <list>
	using MyPubSubClient = arduino::mqtt::PubSubClient<MQTT_MAX_PACKET_SIZE>;
#endif

#include "HomieNode.h"
#include <map>

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#else
#include "WiFi.h"
#endif

#if defined(ARDUINO_ARCH_ESP32) && (defined(USE_PUBSUBCLIENT) | defined(USE_MPSCG))
#define HOMIELIB_CONNECT_ASYNC
#endif


/*#if defined(ARDUINO_ARCH_ESP32)
#include <mutex>
#endif*/

typedef std::function<bool(const char* topic, uint8_t * payload, size_t length)> MqttMessageCallback;	//return true if handled

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
	int iMainLoopInterval_ms=100;
	int iInitialPublishingThrottle_ms=200;

	String strFirmwareName;
	String strFirmwareVersion;

	String strMqttServerIP;
	String strMqttUserName;
	String strMqttPassword;

	String strFriendlyName;
	String strID;

	MqttMessageCallback fnMessageCallback;

	bool bRapidUpdateRSSI=false;

	bool bTelemetrySent=false;

	bool IsInitialized() { return bInitialized; }

	void Init();
	void Quit();

	void Loop();

	HomieNode * NewNode();

	std::vector<HomieNode *> vecNode;


	HomieProperty * NewSubscription(const String & strTopic);	//to create a LeifSimpleMQTT-compatible mqtt subscription object


	bool IsConnected();
	bool IsConnecting() { return bConnecting; };

	bool IsReady();

	uint16_t PublishDirect(const String & topic, uint8_t qos, bool retain, const String & payload);
	uint16_t PublishDirectUint8(const char * topic, uint8_t qos, bool retain, const uint8_t * payload, uint32_t length);

#if defined(USE_PANGOLIN)
	PangolinMQTT mqtt;
#elif defined(USE_ASYNCMQTTCLIENT)
	AsyncMqttClient mqtt;
#elif defined(USE_ARDUINOMQTT)
	MQTTClient * pMQTT=NULL;
	WiFiClient net;
#elif defined(USE_PUBSUBCLIENT)
	PubSubClient * pMQTT=NULL;
	WiFiClient net;
#elif defined(USE_MPSCG)
	WiFiClient client;
	MyPubSubClient mqttClient;
#endif

#ifdef HOMIELIB_CONNECT_ASYNC
	bool bDoConnect=false;
	TaskHandle_t hTaskConnect;
	void TaskConnect();
#endif

/*#if defined(ARDUINO_ARCH_ESP32)
	std::mutex mutexPublish;
#endif*/

	uint32_t GetUptimeSeconds_WiFi();
	uint32_t GetUptimeSeconds_MQTT();

	const uint32_t * GetUptimeSecondsPtr_WiFi() { return &ulSecondCounter_WiFi; }
	const uint32_t * GetUptimeSecondsPtr_MQTT() { return &ulSecondCounter_MQTT; }

#if defined(USE_ETHERNET) & defined(ARDUINO_ARCH_ESP32)
	uint32_t GetUptimeSeconds_Ethernet();
	const uint32_t * GetUptimeSecondsPtr_Ethernet() { return &ulSecondCounter_Ethernet; }
#endif

	bool GetEnableMQTT();
	void SetEnableMQTT(bool bEnable) { this->bEnableMQTT=bEnable; }

#if defined(USE_PANGOLIN)
	const char * GetMqttLibraryID() { return "LeifHomieLib/PangolinMQTT"; }
#elif defined(USE_ASYNCMQTTCLIENT)
	const char * GetMqttLibraryID() { return "LeifHomieLib/AsyncMqttClient"; }
#elif defined(USE_ARDUINOMQTT)
	const char * GetMqttLibraryID() { return "LeifHomieLib/ArduinoMQTT"; }
#elif defined(USE_PUBSUBCLIENT)
	const char * GetMqttLibraryID() { return "LeifHomieLib/PubSubClient"; }
#elif defined(USE_MPSCG)
	const char * GetMqttLibraryID() { return "LeifHomieLib/MQTTPubSubClient_Generic"; }
#endif

	void InitialUnsubscribe(HomieProperty * pProp);

private:


	bool bEnableMQTT=true;
	bool bWasConnected=false;

	String strClientID;

	uint16_t Publish(const char* topic, uint8_t qos, bool retain, const char* payload = nullptr, size_t length = 0, bool dup = false, uint16_t message_id = 0);

	friend class HomieNode;
	friend class HomieProperty;

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
#elif defined(USE_PUBSUBCLIENT)
	void onConnect(bool sessionPresent);
	void onDisconnect(int8_t reason);
	void onMqttMessage(char* topic, byte* payload, unsigned int len);
#elif defined(USE_MPSCG)
	void onConnect(bool sessionPresent);
	void onDisconnect(int8_t reason);
	void onMqttMessage(const char* topic, const char* payload, const size_t len);
#endif

	void DoDisconnect();

	bool bConnecting=false;

	bool bDoInitialPublishing=false;

	bool bInitialPublishingDone=false;

	int iInitialPublishing=0;
	int iInitialPublishing_Node=0;
	int iInitialPublishing_Prop=0;

	int iRePublishReady=0;

	int iPubCount_Props=0;

	unsigned long ulInitialPublishing=0;

	void DoLazyPublishing();
	unsigned long ulLazyPublishing=0;
	int iLazyPublishingNodeIdx=0;
	int iLazyPublishingPropIdx=0;

	unsigned long ulConnectTimestamp=0;

	bool bInitialized=false;

	String strTopic;
	char szWillTopic[128];

	_map_incoming mapIncoming;

	_map_incoming mapPlainSubscriptions;

	uint32_t ulSecondCounter_Uptime=0;
	uint32_t ulSecondCounter_WiFi=0;
	uint32_t ulSecondCounter_MQTT=0;

#if defined(USE_ETHERNET) & defined(ARDUINO_ARCH_ESP32)
	uint32_t ulSecondCounter_Ethernet=0;
#endif

	unsigned long ulLastLoopSecondCounterTimestamp=0;
	unsigned long ulLastLoopTimestamp=0;


	bool bDoPublishDefaults=false;	//publish default retained values that did not yet exist in the controller
	unsigned long ulPublishDefaultsTimestamp=0;

	void HandleInitialPublishingError();

	bool bSendError=false;
	unsigned long ulSendErrorTimestamp;

	int GetErrorRetryFrequency();

	unsigned long GetReconnectInterval();

	bool subscribe(const char * topic, uint8_t sub_qos);

#if defined(USE_ARDUINOMQTT) | defined(USE_PUBSUBCLIENT) | defined(USE_MPSCG)
	std::list<HomieProperty *> listUnsubQueue;
#endif

};
