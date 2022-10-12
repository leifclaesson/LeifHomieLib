#include "HomieNode.h"
#include "HomieDevice.h"
#include <string>

void HomieLibDebugPrint(const char * szText);

#define csprintf(...) { char szTemp[256]; snprintf(szTemp,255,__VA_ARGS__); szTemp[255]=0; HomieLibDebugPrint(szTemp); }

const char * GetHomieDataTypeText(eHomieDataType datatype)
{
	switch((eHomieDataType) datatype)
	{
	default:
		return "INVALID";
	case homieString:
		return "string";
	case homieInt:
		return "integer";
	case homieFloat:
		return "float";
	case homieBool:
		return "boolean";
	case homieEnum:
		return "enum";
	case homieColor:
		return "color";
	}
};

const char * GetDefaultForHomieDataType(eHomieDataType datatype)
{
	switch((eHomieDataType) datatype)
	{
	default:
		return "uninitialized";
	case homieString:
		return "";
	case homieInt:
		return "0";
	case homieFloat:
		return "0.0";
	case homieBool:
		return "false";
	case homieColor:
		return "100,100,100";
	}
}

bool HomieDataTypeAllowsEmpty(eHomieDataType datatype)
{
	switch((eHomieDataType) datatype)
	{
	case homieString:
		return true;
	default:
		return false;
	}
};


HomieProperty::HomieProperty()
{
//	user1=0;
//	user2=0;
	SetRetained(true);
	SetPublishEmptyString(true);
}

void HomieProperty::SetStandardMQTT(const String & strMqttTopic)
{
	if(GetInitialized()) return;

	SetIsStandardMQTT(true);
	SetRetained(false);
	SetSettable(true);
	strID=strMqttTopic;
	//strSetTopic="";

}

void HomieProperty::Init()
{
/*	if(!GetIsStandardMQTT())
	{
		strTopic=pParent->strTopic+"/"+strID;
		strSetTopic=strTopic+"/set";
	}*/
	SetInitialized(true);

}

void HomieProperty::DoCallback()
{
#if defined(HOMIELIB_VIRTUAL_ONCALLBACK)
	OnCallback();
#endif

	if(pVecCallback)
	{
		for(size_t i=0;i<pVecCallback->size();i++)
		{
			(*pVecCallback)[i](this);
		}
	}
}

void HomieProperty::AddCallback(HomiePropertyCallback cb)
{
	if(!pVecCallback)
	{
		pVecCallback=new std::vector<HomiePropertyCallback>;
	}
	pVecCallback->push_back(cb);
}

void HomieProperty::SetUnit(const char * szUnit)
{
	if(szUnit && strlen(szUnit))
	{
		if(!pstrUnit)
		{
			pstrUnit=new String;
		}
		*pstrUnit=szUnit;
	}
	else
	{
		if(pstrUnit)
		{
			delete pstrUnit;
			pstrUnit=NULL;
		}
	}
}

String HomieProperty::GetUnit()
{
	if(pstrUnit) return *pstrUnit;
	return String();
}


const String & HomieProperty::GetValue()
{
	return strValue;
}

void HomieProperty::PublishDefault()
{
	if(GetSettable() && GetRetained() && !GetReceivedRetained() && !GetIsStandardMQTT())
	{
		SetReceivedRetained(true);
		if(strValue.length())
		{
#ifdef HOMIELIB_VERBOSE
			csprintf("%s didn't receive initial value for base topic %s so unsubscribe and publish default.\n",strFriendlyName.c_str(),GetTopic().c_str());
#endif
#if defined(USE_PANGOLIN) | defined(USE_ASYNCMQTTCLIENT)
			pParent->pParent->mqtt.unsubscribe(GetTopic().c_str());
#elif defined(USE_ARDUINOMQTT) | defined(USE_PUBSUBCLIENT)
			pParent->pParent->pMQTT->unsubscribe(GetTopic().c_str());
#endif
			Publish();
		}
	}

}

bool HomieProperty::Publish()
{
	if(!GetInitialized()) return false;
	if(!pParent->pParent->bEnableMQTT) return false;
	if(GetIsStandardMQTT()) return false;

	bool bRet=false;
	String strPublish=strValue;

	if(!strPublish.length() && !GetPublishEmptyString()) return true;

	if(!strPublish.length() && !HomieDataTypeAllowsEmpty((eHomieDataType) datatype))
	{
		strPublish=GetDefaultForHomieDataType((eHomieDataType) datatype);
#ifdef HOMIELIB_VERBOSE
		csprintf("Empty value for %s/%s encountered, substituting default. ",pParent->strID.c_str(),strID.c_str());
#endif
	}

	if(!pParent->pParent->IsConnected())
	{
#ifdef HOMIELIB_VERBOSE
		csprintf("%s can't publish \"%s\" = no conn. heap=%u\n",strFriendlyName.c_str(),strPublish.c_str(),ESP.getFreeHeap());
#endif
	}
	else
	{
#ifdef HOMIELIB_VERBOSE
		csprintf("%s publishing \"%s\"... heap=%u...",strFriendlyName.c_str(),strPublish.c_str(),ESP.getFreeHeap());
#endif

#ifdef HOMIELIB_VERBOSE
		uint32_t free_before=ESP.getFreeHeap();
#endif
#if defined(USE_PANGOLIN)
		pParent->pParent->mqtt.publish(GetTopic().c_str(), 2, GetRetained(), (uint8_t *) strPublish.c_str(), strPublish.length(), 0);
#elif defined(USE_ASYNCMQTTCLIENT)
		pParent->pParent->mqtt.publish( GetTopic().c_str(), 0, GetRetained(), strPublish.c_str(), strPublish.length());
#elif defined(USE_ARDUINOMQTT)
		pParent->pParent->pMQTT->publish(GetTopic(), strPublish, GetRetained(), 0);
		//csprintf("publish A %s=%s ret=%i\n",GetTopic().c_str(),strValue.c_str(),ret);
#elif defined(USE_PUBSUBCLIENT)
		pParent->pParent->pMQTT->publish(GetTopic().c_str(), (const uint8_t *) strPublish.c_str(), strPublish.length(), GetRetained());

#endif

#ifdef HOMIELIB_VERBOSE
		uint32_t free_after=ESP.getFreeHeap();
		csprintf("done. heap used: %i\n",(int32_t) (free_before-free_after));
#endif
		//bRet=0!=
		bRet=true;
	}
	return bRet;
}


void HomieProperty::SetValue(const String & strNewValue)
{
#ifdef HOMIELIB_VERBOSE
	csprintf("%s setvalue \"%s\"...\n",strFriendlyName.c_str(),strNewValue.c_str());
#endif
	if(SetValueConstrained(strNewValue))
	{
		if(GetInitialPublishingDone())
		{
			Publish();
		}
		else
		{
		}
	}
}

void HomieProperty::SetBool(bool bValue)
{
	String strTemp=bValue?"true":"false";
	SetValue(strTemp);
}


bool HomieProperty::ValidateFormat_Int(int & min, int & max)
{
	int colon=strFormat.indexOf(':');

	if(colon>0)
	{
		min=atoi(strFormat.substring(0, colon).c_str());
		max=atoi(strFormat.substring(colon+1).c_str());
		return true;
	}

	return false;
}

bool HomieProperty::ValidateFormat_Double(double & min, double & max)
{
	int colon=strFormat.indexOf(':');

	if(colon>0)
	{
		min=atof(strFormat.substring(0, colon).c_str());
		max=atof(strFormat.substring(colon+1).c_str());
		return true;
	}

	return false;
}


bool HomieProperty::SetValueConstrained(const String & strNewValue)
{
	switch((eHomieDataType) datatype)
	{
	default:
		strValue=strNewValue;
		return true;
	case homieInt:
		{

			int newvalue=atoi(strNewValue.c_str());

			int min,max;

			if(ValidateFormat_Int(min,max))
			{
				if(newvalue<min || newvalue>max)
				{
#ifdef HOMIELIB_VERBOSE
					csprintf("%s ignoring invalid payload %s (int out of range %i:%i)\n",strFriendlyName.c_str(),strNewValue.c_str(),min,max);
#endif
					return false;
				}
			}

			strValue=String(newvalue);
			return true;
		}
		break;
	case homieFloat:
		{
			double newvalue=atof(strNewValue.c_str());

			double min,max;

			if(ValidateFormat_Double(min,max))
			{
				if(newvalue<min || newvalue>max)
				{
#ifdef HOMIELIB_VERBOSE
					csprintf("%s ignoring invalid payload %s (float out of range %.04f:%.04f)\n",strFriendlyName.c_str(),strNewValue.c_str(),min,max);
#endif
					return false;
				}
			}

			strValue=strNewValue/*String(newvalue)*/;
			return true;
		}
	case homieBool:
		if(strNewValue=="true") strValue="true"; else if(strNewValue=="false") strValue="false";
		else
		{
#ifdef HOMIELIB_VERBOSE
			csprintf("%s ignoring invalid payload %s (bool needs true or false)\n",strFriendlyName.c_str(),strNewValue.c_str());
#endif
			return false;
		}
		return true;
	case homieEnum:
		{
			int start=0;
			int comma;

			bool bValid=false;

			while((comma=strFormat.indexOf(',',start))>0)
			{
				if(strNewValue==strFormat.substring(start, comma))
				{
					bValid=true;
					break;
				}
				start=comma+1;
			}

			if(!bValid)
			{
				if(strNewValue==strFormat.substring(start)) bValid=true;
			}

			if(bValid)
			{
				strValue=strNewValue;
				return true;
			}
			else
			{
#ifdef HOMIELIB_VERBOSE
				csprintf("%s ignoring invalid payload %s (not one of %s)\n",strFriendlyName.c_str(),strNewValue.c_str(),strFormat.c_str());
#endif
				return false;
			}
		}

		break;
	case homieColor:
		strValue=strNewValue;
		return true;
		break;
	};

	return true;
}

#if defined(USE_PANGOLIN)
void HomieProperty::OnMqttMessage(const char* topic, uint8_t * payload, PANGO_PROPS & properties, size_t len, size_t index, size_t total)
{
	(void)(properties); (void)(total);
#elif defined(USE_ASYNCMQTTCLIENT)
void HomieProperty::OnMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties & properties, size_t len, size_t index, size_t total)
{
	(void)(properties); (void)(total);
#elif defined(USE_ARDUINOMQTT)
void HomieProperty::OnMqttMessage(char* topic, char* payload, void * properties, size_t len, size_t index, size_t total)
{
	(void)(properties); (void)(total);
#elif defined(USE_PUBSUBCLIENT)
void HomieProperty::OnMqttMessage(char* topic, byte* payload, void * properties, unsigned int len, int index, int total)
{
	(void)(properties); (void)(total);
#endif

	if(index==0)
	{

		std::string temp;
		temp.assign((const char *) payload,len);

		bool bValid=SetValueConstrained(String(temp.c_str()));
		//pProp->strValue.
		if(bValid)
		{
			DoCallback();
		}

		if(GetRetained() && !strcmp(topic,GetTopic().c_str()) && !GetIsStandardMQTT())
		{
#ifdef HOMIELIB_VERBOSE
			csprintf("%s received initial value for base topic %s. Unsubscribing.\n",strFriendlyName.c_str(),GetTopic().c_str());
#endif
			pParent->pParent->InitialUnsubscribe(this);
			SetReceivedRetained(true);
		}
		else
		{
			if(bValid)
			{
				Publish();
			}
		}

		if(GetIsStandardMQTT())
		{
			SetReceivedRetained(true);
		}

	}


	if(GetClearPayloadAfterCallback())
	{
#ifdef HOMIELIB_VERBOSE
		csprintf("%s CLEAR PAYLOAD!\n", topic);
#endif
		strValue="";
	}

}

String HomieProperty::GetTopic()
{
	if(GetIsStandardMQTT()) return strID;
	return pParent->GetTopic()+"/"+strID;
}

String HomieProperty::GetSetTopic()
{
	if(GetIsStandardMQTT()) return strID;
	return pParent->GetTopic()+"/"+strID+"/set";
}



HomieNode::HomieNode()
{

}

void HomieNode::Init()
{

//	strTopic=pParent->strTopic+"/"+strID;
	for(size_t a=0;a<vecProperty.size();a++)
	{
		vecProperty[a]->Init();
	}
}

String HomieNode::GetTopic()
{
	return pParent->strTopic+"/"+strID;
}

void HomieNode::PublishDefaults()
{
	for(size_t a=0;a<vecProperty.size();a++)
	{
		vecProperty[a]->PublishDefault();
	}
}

void HomieNode::AddProperty(HomieProperty * pProp)
{
	vecProperty.push_back(pProp);
	pProp->pParent=this;
}

HomieProperty * HomieNode::NewProperty()
{
	HomieProperty * ret=new HomieProperty;
	AddProperty(ret);
	return ret;
}

HomieDevice * HomieNode::GetParentDevice()
{
	return pParent;
}


void HomieProperty::SetSettable(bool bEnable){if(bEnable) flags |= 0x1; else flags &= ~0x1;}
void HomieProperty::SetRetained(bool bEnable){if(bEnable) flags |= 0x2; else flags &= ~0x2;}
void HomieProperty::SetFakeRetained(bool bEnable){if(bEnable) flags |= 0x4; else flags &= ~0x4;}
void HomieProperty::SetPublishEmptyString(bool bEnable){if(bEnable) flags |= 0x8; else flags &= ~0x8;}
void HomieProperty::SetInitialized(bool bEnable){if(bEnable) flags |= 0x10; else flags &= ~0x10;}
void HomieProperty::SetReceivedRetained(bool bEnable){if(bEnable) flags |= 0x20; else flags &= ~0x20;}
void HomieProperty::SetIsStandardMQTT(bool bEnable){if(bEnable) flags |= 0x40; else flags &= ~0x40;}
void HomieProperty::SetInitialPublishingDone(bool bEnable){if(bEnable) flags |= 0x80; else flags &= ~0x80;}
void HomieProperty::SetDebug(bool bEnable){if(bEnable) flags |= 0x100; else flags &= ~0x100;}
void HomieProperty::SetClearPayloadAfterCallback(bool bEnable){if(bEnable) flags |= 0x200; else flags &= ~0x200;}


bool HomieProperty::GetSettable(){return (flags & 0x1)!=0;}
bool HomieProperty::GetRetained(){return (flags & 0x2)!=0;}
bool HomieProperty::GetFakeRetained(){return (flags & 0x4)!=0;}
bool HomieProperty::GetPublishEmptyString(){return (flags & 0x8)!=0;}
bool HomieProperty::GetInitialized(){return (flags & 0x10)!=0;}
bool HomieProperty::GetReceivedRetained(){return (flags & 0x20)!=0;}
bool HomieProperty::GetIsStandardMQTT(){return (flags & 0x40)!=0;}
bool HomieProperty::GetInitialPublishingDone(){return (flags & 0x80)!=0;}
bool HomieProperty::GetDebug(){return (flags & 0x100)!=0;}
bool HomieProperty::GetClearPayloadAfterCallback(){return (flags & 0x200)!=0;}

