#pragma once
#include "arduino.h"

#include <functional>

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

struct AsyncMqttClientMessageProperties;

class HomieProperty
{
public:
	HomieProperty();

	void SetStandardMQTT(const String & strMqttTopic);	//call before init to subscribe to a standard MQTT topic. Receive only.

	String strID;
	String strFriendlyName;
	bool bSettable=false;
	bool bRetained=true;
	bool bPublishEmptyString=true;
	String strUnit;
	eHomieDataType datatype=homieString;
	String strFormat;

	void Init();

	void AddCallback(HomiePropertyCallback cb);

	const String & GetValue();
	void SetValue(const String & strNewValue);
	void SetBool(bool bValue);

	bool Publish();

	void OnMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties & properties, size_t len, size_t index, size_t total);

private:
	String strTopic;
	String strSetTopic;
	HomieNode * pParent;
	String strValue;
	std::vector<HomiePropertyCallback> vecCallback;

	bool bInitialized=false;

	friend class HomieDevice;
	friend class HomieNode;
	void DoCallback();

	bool SetValueConstrained(const String & strNewValue);

	bool ValidateFormat_Int(int & min, int & max);
	bool ValidateFormat_Double(double & min, double & max);

	void PublishDefault();

	bool bReceivedRetained=false;

	bool bStandardMQTT=false;

};



class HomieNode
{
public:
	HomieNode();

	String strID;
	String strFriendlyName;
	String strType;

	HomieProperty * NewProperty();


private:

	void Init();
	std::vector<HomieProperty *> vecProperty;

	friend class HomieDevice;
	friend class HomieProperty;
	HomieDevice * pParent;
	String strTopic;

	void PublishDefaults();

};

