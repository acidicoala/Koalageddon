#include "pch.h"
#include "origin_hooks.h"
#include "Origin.h"

#define GET_ORIGINAL_FUNC(FUNC) PLH::FnCast(BasePlatform::trampolineMap[mangled_##FUNC], FUNC);

auto& getOriginConfig()
{
	return config->platformRefs.Origin;
}

bool isOriginEntitlementBlacklisted(XMLElement* pEntitlement)
{
	return vectorContains(getOriginConfig().blacklist, string(pEntitlement->FindAttribute("ItemId")->Value()));
}

#ifndef _WIN64

string* __fastcall encrypt(PARAMS(void* mystery, string* message))
{
	do
	{
		XMLDocument xmlDoc;
		if(xmlDoc.Parse(message->c_str()) != XMLError::XML_SUCCESS)
			break;

		auto pLSX = xmlDoc.FirstChildElement("LSX");
		if(pLSX == nullptr)
			break;

		auto pResponse = pLSX->FirstChildElement("Response");
		if(pResponse == nullptr)
			break;

		auto pQueryEntitlementsResponse = pResponse->FirstChildElement("QueryEntitlementsResponse");
		if(pQueryEntitlementsResponse == nullptr)
			break;

		logger->info("Intercepted QueryEntitlementsResponse");

		// Origin magic happens here

		// First filter out blacklisted DLCs from the legit response
		auto pEntitlement = pQueryEntitlementsResponse->FirstChildElement("Entitlement");
		while(pEntitlement != nullptr)
		{
			if(isOriginEntitlementBlacklisted(pEntitlement))
				pQueryEntitlementsResponse->DeleteChild(pEntitlement);
			pEntitlement = pEntitlement->NextSiblingElement("Entitlement");
		}

		// Insert our entitlements into the original response
		pEntitlement = originEntitlementsXML.FirstChildElement("Entitlements")->FirstChildElement("Entitlement");
		while(pEntitlement != nullptr)
		{
			// Have to make a copy because TinyXML2 doesn't allow insertion of elements from another doc...
			if(!isOriginEntitlementBlacklisted(pEntitlement))
				pQueryEntitlementsResponse->InsertEndChild(pEntitlement->DeepClone(&xmlDoc));
			pEntitlement = pEntitlement->NextSiblingElement("Entitlement");
		}

		XMLPrinter printer;
		xmlDoc.Print(&printer);
		*message = printer.CStr(); // copy constructor

		logger->info("Modified response: \n{}", printer.CStr());
	} while(false);

	static auto original = GET_ORIGINAL_FUNC(encrypt);

	return original(ARGS(mystery, message));
}

#endif
