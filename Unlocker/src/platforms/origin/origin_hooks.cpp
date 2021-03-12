#include "pch.h"
#include "origin_hooks.h"
#include "platforms/origin/Origin.h"

XMLDocument entitlementsXML;

auto getOriginConfig()
{
	return config->platforms["Origin"];
}

bool isEntitlementBlacklisted(XMLElement* pEntitlement)
{
	return vectorContains(config->platforms["Origin"].blacklist, string(pEntitlement->FindAttribute("ItemId")->Value()));

}
string* __fastcall encrypt(PARAMS(void* aes, string* message))
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
			if(isEntitlementBlacklisted(pEntitlement))
				pQueryEntitlementsResponse->DeleteChild(pEntitlement);
			pEntitlement = pEntitlement->NextSiblingElement("Entitlement");
		}

		// Insert our entitlements into the original response
		pEntitlement = entitlementsXML.FirstChildElement("Entitlements")->FirstChildElement("Entitlement");
		while(pEntitlement != nullptr)
		{
			// Have to make a copy because TinyXML2 doesn't allow insertion of elements from another doc...
			if(!isEntitlementBlacklisted(pEntitlement))
				pQueryEntitlementsResponse->InsertEndChild(pEntitlement->DeepClone(&xmlDoc));
			pEntitlement = pEntitlement->NextSiblingElement("Entitlement");
		}

		XMLPrinter printer;
		xmlDoc.Print(&printer);
		*message = printer.CStr(); // copy constructor

		logger->info("Modified response: \n{}", printer.CStr());
	} while(false);

	auto result = PLH::FnCast(
		BasePlatform::trampolineMap[mangled_encrypt],
		encrypt
	)(ARGS(aes, message));

	return result;
}
