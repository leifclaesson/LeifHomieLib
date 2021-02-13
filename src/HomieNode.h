#pragma once
#include "Arduino.h"

#include <functional>

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

	bool GetSettable();
	bool GetRetained();
	bool GetFakeRetained();
	bool GetPublishEmptyString();

//	bool bSettable=false;
//	bool bRetained=true;
//	bool bFakeRetained=false;
//	bool bPublishEmptyString=true;
	//String strUnit;
	String strFormat;

	void Init();

	void AddCallback(HomiePropertyCallback cb);

	const String & GetValue();
	void SetValue(const String & strNewValue);
	void SetBool(bool bValue);

	void DoCallback();

	bool Publish();


#if defined(USE_PANGOLIN)
	void OnMqttMessage(const char* topic, uint8_t * payload, PANGO_PROPS & properties, size_t len, size_t index, size_t total);
#elif defined(USE_ASYNCMQTTCLIENT)
	void OnMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties & properties, size_t len, size_t index, size_t total);
#elif defined(USE_ARDUINOMQTT)
	void OnMqttMessage(char* topic, char* payload, void * properties, size_t len, size_t index, size_t total);
#endif



	String GetTopic();
	String GetSetTopic();

private:
//	String strTopic;
	//String strSetTopic;
	String * pstrUnit=NULL;
	HomieNode * pParent;
	String strValue;
	std::vector<HomiePropertyCallback> * pVecCallback=NULL;

	bool bInitialized=false;

	friend class HomieDevice;
	friend class HomieNode;

	bool SetValueConstrained(const String & strNewValue);

	bool ValidateFormat_Int(int & min, int & max);
	bool ValidateFormat_Double(double & min, double & max);

	void PublishDefault();

	//bool bReceivedRetained=false;

	//bool bStandardMQTT=false;


	void SetInitialized(bool bEnable);
	void SetReceivedRetained(bool bEnable);
	void SetIsStandardMQTT(bool bEnable);
	bool GetInitialized();
	bool GetReceivedRetained();
	bool GetIsStandardMQTT();

public:
	uint8_t datatype=homieString;	/* eHomieDataType */
private:
	uint8_t flags=0;

};



class HomieNode
{
public:
	HomieNode();

	String strID;
	String strFriendlyName;
	String strType;

	HomieProperty * NewProperty();

	String GetTopic();

private:

	void Init();
	std::vector<HomieProperty *> vecProperty;

	friend class HomieDevice;
	friend class HomieProperty;
	HomieDevice * pParent;
//	String strTopic;

	void PublishDefaults();

};

