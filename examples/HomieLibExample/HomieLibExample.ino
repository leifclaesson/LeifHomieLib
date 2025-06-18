/*
    This sketch establishes an MQTT connection to your broker and publishes a device with several dummy nodes and properties.
*/

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#else
#include "WiFi.h"
#endif
#include <LeifHomieLib.h>

#ifndef STASSID
#define STASSID "your-ssid"
#define STAPSK  "your-password"
#endif


#ifndef MQTT_IP
#define MQTT_IP "192.168.1.2"
#define MQTT_USER "your-mqttuser"
#define MQTT_PASS "your-mqttpassword"
#endif


HomieDevice homie;

const char* ssid     = STASSID;
const char* password = STAPSK;




// We'll need a place to save pointers to our created properties so that we can access them again later.
HomieProperty * pPropDoorbell=NULL;
HomieProperty * pPropDoorSensor=NULL;
HomieProperty * pPropTemp=NULL;
HomieProperty * pPropHumidity=NULL;
HomieProperty * pPropPrecip=NULL;
HomieProperty * pPropWind=NULL;



void setup() {
  Serial.begin(115200);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());




	HomieLibRegisterDebugPrintCallback([](const char * szText){
		Serial.printf("%s",szText);
		});



	{


		HomieNode * pNode=homie.NewNode();

		pNode->strID="properties";
		pNode->strFriendlyName="Properties";
//		pNode->strType="customtype";

		HomieProperty * pProp;


		pPropDoorbell=pProp=pNode->NewProperty();
		pProp->strFriendlyName="Doorbell";
		pProp->strID="doorbell";
		pProp->SetRetained(false);
		pProp->datatype=homieEnum;
		pProp->strFormat="PRESSED,RELEASED";

		pPropDoorSensor=pProp=pNode->NewProperty();
		pProp->strFriendlyName="Door Open";
		pProp->strID="doorsensor";
		pProp->SetRetained(true);
		pProp->SetBool(false);
		pProp->datatype=homieBool;



		pProp=pNode->NewProperty();
		pProp->strID="dimmer";
//		pProp->strUnit="%";
		pProp->datatype=homieFloat;
		pProp->strFormat="-64:64";
		pProp->SetSettable(true);
		pProp->SetRetained(true);
		pProp->SetValue("26.3");
		pProp->strFriendlyName="Dimmer";
		pProp->AddCallback([](HomieProperty * pSource) {
			//this property is settable. We'll print it into the console whenever it's updated.
			//you can set it from MQTT Explorer by publishing a number between 0-100 to homie/examplehomiedev/nodeid1/dimmer
			//but remember to check the *retain* box.
			Serial.printf("%s is now %s\n",pSource->strFriendlyName.c_str(),pSource->GetValue().c_str());
		});





		pProp=pNode->NewProperty();
		pProp->strID="dimmer2";
//		pProp->strUnit="%";
		pProp->datatype=homieInt;
		pProp->strFormat="0:100";
		pProp->SetSettable(true);
		pProp->SetRetained(true);
		pProp->strFriendlyName="Dimmer2";
		pProp->SetValue("50");
		pProp->AddCallback([](HomieProperty * pSource) {
			//this property is settable. We'll print it into the console whenever it's updated.
			//you can set it from MQTT Explorer by publishing a number between 0-100 to homie/examplehomiedev/nodeid1/dimmer
			//but remember to check the *retain* box.
			Serial.printf("%s is now %s\n",pSource->strFriendlyName.c_str(),pSource->GetValue().c_str());
		});



		pProp=pNode->NewProperty();
		pProp->strFriendlyName="Test Boolean";
		pProp->strID="testboolean";
		pProp->datatype=homieBool;
		pProp->SetSettable(true);
		pProp->SetRetained(true);
		pProp->strFormat="";
		pProp->AddCallback([](HomieProperty * pSource) {
			//this property is settable. We'll print it into the console whenever it's updated.
			//you can set it from MQTT Explorer by publishing a number between 0-100 to homie/examplehomiedev/nodeid1/dimmer
			//but remember to check the *retain* box.
			Serial.printf("%s is now %s\n",pSource->strFriendlyName.c_str(),pSource->GetValue().c_str());
		});


/*
		pPropTemp=pProp=pNode->NewProperty();
		pProp->strID="temp";
		pProp->strUnit="C";
		pProp->datatype=homieInt;
		pProp->strFriendlyName="Temperature";

		pPropHumidity=pProp=pNode->NewProperty();
		pProp->strID="hum";
		pProp->strUnit="%";
		pProp->datatype=homieInt;
		pProp->strFriendlyName="Humidity";

		pPropPrecip=pProp=pNode->NewProperty();
		pProp->strID="precip";
		pProp->strUnit="%";
		pProp->datatype=homieInt;
		pProp->strFriendlyName="Precipitation";



		pPropWind=pProp=pNode->NewProperty();
		pProp->strID="wind";
		pProp->strUnit="m/s";
		pProp->datatype=homieFloat;
		pProp->strFriendlyName="Wind speed";
*/

	}

	if(0)
	{
		//a few more just for good measure. change the if-statement above to enable. we'll leave them unset for now.

		HomieNode * pNode=homie.NewNode();

		pNode->strID="nodeid2";
		pNode->strFriendlyName="Node Two";
		pNode->strType="customtype";

		HomieProperty * pProp;

		for(int a=0;a<10;a++)
		{

			pProp=pNode->NewProperty();
			pProp->strFriendlyName="Light "+String(a+1);
			pProp->strID="light"+String(a+1);

		}

	}


	homie.strFriendlyName="Example Homie Device";
	homie.strID="ExampleHomieDev";
	homie.strID.toLowerCase();

	homie.strMqttServerIP=MQTT_IP;
	homie.strMqttUserName=MQTT_USER;
	homie.strMqttPassword=MQTT_PASS;

	homie.Init();




}


unsigned long ulDummyValueTimestamp=0;

int iDoorState=0;
unsigned long ulDoorTimestamp=5000;


void loop()
{
	homie.Loop();

	//a little state machine to ring doorbell once a minute, then open the door for a few seconds (to let the guests in, of course!)

	if(homie.IsConnected())
	{


		unsigned long ulDoorDiff=millis()-ulDoorTimestamp;

		if(ulDoorDiff>=7000)
		{
			if(iDoorState==0)
			{
				iDoorState++;
				//pPropDoorbell->SetBool(true);	//ding
				if(pPropDoorbell) pPropDoorbell->SetValue("PRESSED");
			}

			if(ulDoorDiff>=12000)
			{
				if(iDoorState==1)
				{
					iDoorState++;
					//pPropDoorbell->SetBool(false);	//dong
					if(pPropDoorbell) pPropDoorbell->SetValue("RELEASED");
				}
			}

			if(ulDoorDiff>=18000)
			{
				if(iDoorState==2)
				{
					if(pPropDoorSensor) pPropDoorSensor->SetBool(true);		//door open
					iDoorState++;
				}
			}

			if(ulDoorDiff>=30000)
			{
				if(iDoorState==3)
				{
					if(pPropDoorSensor) pPropDoorSensor->SetBool(false);	//door closed

					ulDoorTimestamp=millis();		//reset timestamp
					iDoorState=0;					//reset state machine
				}
			}
		}




		// set some properties to some dummy values

		if(millis()-ulDummyValueTimestamp>=2000)
		{
			ulDummyValueTimestamp=millis();

			static int iWhichValue=0;


			const double pi=3.14159265359;

			switch(iWhichValue)
			{
				case 0:
				{
					if(pPropTemp) pPropTemp->SetValue(String((rand()%90)-45));	//siberia
					break;
				}
				case 1:
				{
					static double fAccumulator=0;
					fAccumulator+=0.3;
					if(fAccumulator>pi*2) fAccumulator-=pi*2;

					//make a peaky waveform centered around 50%
					double sine=sin(fAccumulator);
					sine=sine*sine*sine*sine*sine;
					int value=50+(50*sine);

					if(pPropHumidity) pPropHumidity->SetValue(String(value));
					break;
				}
				case 2:
				{
					static bool bFlipFlop_Rain=false;
					if(pPropPrecip) pPropPrecip->SetValue(String(bFlipFlop_Rain?100:0));	//when it rains, it pours
					bFlipFlop_Rain^=true;
					break;
				}
				case 3:
				{
					static double fAccumulator=0;
					fAccumulator+=0.5;

					float value=30+(20*sin(fAccumulator));
					if(fAccumulator>pi*2) fAccumulator-=pi*2;

					char szTemp[32];
					sprintf(szTemp,"%.01f",value);

					if(pPropWind) pPropWind->SetValue(szTemp);
					break;
				}
				default:
				{
					iWhichValue=-1;
					break;
				}
			}

			iWhichValue++;



		}

	}

}
