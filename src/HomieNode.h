#pragma once
#include "Arduino.h"

#include <functional>
#include <vector>

#include "Config.h"

class HomieProperty;
class HomieNode;
class HomieDevice;

typedef std::function<void(HomieProperty * pSource)> HomiePropertyCallback;

enum eHomieDataType
{
	homieString,
	homieInt,
	homieInteger=homieInt,
	homieFloat,
	homieBool,
	homieBoolean=homieBool,
	homieEnum,
	homieColor,
	homieColour=homieColor,
};


const char * GetHomieDataTypeText(eHomieDataType datatype);
bool HomieDataTypeAllowsEmpty(eHomieDataType datatype);
const char * GetDefaultForHomieDataType(eHomieDataType datatype);

#if defined(USE_PANGOLIN)
struct PANGO_PROPS;
#elif defined(USE_ASYNCMQTTCLIENT)
struct AsyncMqttClientMessageProperties;
#elif defined(USE_ARDUINOMQTT)
#elif defined(USE_PUBSUBCLIENT)
#elif defined(USE_MPSCG)
#endif

class HomieProperty
{
public:
	HomieProperty();

	void SetStandardMQTT(const String & strMqttTopic);	//call before init to subscribe to a standard MQTT topic. Receive only.

	//int user1;
	//int user2;

	void SetUnit(const char * szUnit);
	String GetUnit();

	String strID;
	String strFriendlyName;

	void SetSettable(bool bEnable);
	void SetRetained(bool bEnable);
	void SetFakeRetained(bool bEnable);
	void SetPublishEmptyString(bool bEnable);
	void SetInitialPublishingDone(bool bDone);
	void SetDebug(bool bEnable);
	void SetClearPayloadAfterCallback(bool bEnable);
	void SetNoPublishOnSet(bool bEnable);

	bool GetSettable();
	bool GetRetained();
	bool GetFakeRetained();
	bool GetPublishEmptyString();
	bool GetInitialPublishingDone();
	bool GetDebug();
	bool GetClearPayloadAfterCallback();
	bool GetNoPublishOnSet();

//	bool bSettable=false;
//	bool bRetained=true;
//	bool bFakeRetained=false;
//	bool bPublishEmptyString=true;
	//String strUnit;
	String strFormat;

	void Init();

	const String & GetValue();
	void SetValue(const String & strNewValue);
	void SetBool(bool bValue);

#if defined(HOMIELIB_VIRTUAL_ONCALLBACK)
	virtual void OnCallback() {};
#endif

	void DoCallback();
	void AddCallback(HomiePropertyCallback cb);

	bool Publish();


#if defined(USE_PANGOLIN)
	void OnMqttMessage(const char* topic, uint8_t * payload, PANGO_PROPS & properties, size_t len, size_t index, size_t total);
#elif defined(USE_ASYNCMQTTCLIENT)
	void OnMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties & properties, size_t len, size_t index, size_t total);
#elif defined(USE_ARDUINOMQTT)
	void OnMqttMessage(char* topic, char* payload, void * properties, size_t len, size_t index, size_t total);
#elif defined(USE_PUBSUBCLIENT)
	void OnMqttMessage(char* topic, byte* payload, void * properties, unsigned int len, int index, int total);
#elif defined(USE_MPSCG)
	void OnMqttMessage(const char* topic, const char* payload, void * properties, size_t len, int index, int total);
#endif

	HomieNode * GetParentNode() { return pParent; }


	String GetTopic();
	String GetSetTopic();

	bool GetReceivedRetained();

protected:
	void SetReceivedRetained(bool bEnable);
	HomieNode * pParent=NULL;

private:

//	String strTopic;
	//String strSetTopic;
	String * pstrUnit=NULL;
	String strValue;
	std::vector<HomiePropertyCallback> * pVecCallback=NULL;



	friend class HomieDevice;
	friend class HomieNode;

	bool SetValueConstrained(const String & strNewValue);

	bool ValidateFormat_Int(int & min, int & max);
	bool ValidateFormat_Double(double & min, double & max);

	void PublishDefault();

	//bool bReceivedRetained=false;

	//bool bStandardMQTT=false;


	void SetInitialized(bool bEnable);
	void SetIsStandardMQTT(bool bEnable);
	bool GetInitialized();
	bool GetIsStandardMQTT();

	void SetNeedsPublish(bool bEnable);
	bool GetNeedsPublish();

public:
	uint8_t datatype=homieString;	/* eHomieDataType */
private:
	uint16_t flags=0;

};



class HomieNode
{
public:
	HomieNode();

	String strID;
	String strFriendlyName;
	String strType;

	void AddProperty(HomieProperty * pProp);
	HomieProperty * NewProperty();
	std::vector<HomieProperty *> vecProperty;
	HomieDevice * GetParentDevice();

	String GetTopic();

private:

	void Init();

	friend class HomieDevice;
	friend class HomieProperty;
	HomieDevice * pParent;
//	String strTopic;

	void PublishDefaults();

};


#define MqttSubscription HomieProperty
#define MqttSubscriptionCallback HomiePropertyCallback

