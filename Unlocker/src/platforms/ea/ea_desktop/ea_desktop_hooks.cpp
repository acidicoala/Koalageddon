#include "pch.h"
#include "ea_desktop_hooks.h"
#include "EADesktop.h"

#define GET_PROXY_FUNC(FUNC) PLH::FnCast(BasePlatform::trampolineMap[mangled_##FUNC], FUNC);

bool isEADesktopEntitlementBlacklisted(XMLElement* pEntitlement)
{
	return vectorContains(
		config->platformRefs.EADesktop.blacklist,
		string(pEntitlement->FirstChildElement("productId")->GetText())
	);
}

#ifdef _WIN64

string* toStdString(PARAMS(void* mystery))
{
	static auto proxy = GET_PROXY_FUNC(toStdString);
	auto str = proxy(ARGS(mystery));
	if(str == nullptr)
	{
		logger->warn("toStdString -> null str");
		return str;
	}

	logger->debug("toStdString -> {}", *str);

	if(!startsWith(*str, R"(<?xml version="1.0" encoding="UTF-8" standalone="yes"?><entitlements>)"))
	{ // irrelevant
		return str;
	}

	do
	{
		XMLDocument xmlDoc;
		if(xmlDoc.Parse(str->c_str()) != XMLError::XML_SUCCESS)
			break;

		auto pEntitlements = xmlDoc.FirstChildElement("entitlements");
		if(pEntitlements == nullptr)
			break;

		logger->info("Intercepted entitlements xml");

		// EA Desktop magic happens here

		// First filter out blacklisted DLCs from the legit response
		auto pEntitlement = pEntitlements->FirstChildElement("entitlement");
		while(pEntitlement != nullptr)
		{
			if(isEADesktopEntitlementBlacklisted(pEntitlement))
				pEntitlements->DeleteChild(pEntitlement);
			pEntitlement = pEntitlement->NextSiblingElement("Entitlement");
		}

		// Insert our entitlements into the original response
		auto inserted = 0;
		pEntitlement = eaDesktopEntitlementsXML.FirstChildElement("entitlements")->FirstChildElement("entitlement");
		while(pEntitlement != nullptr)
		{
			inserted++;
			// Have to make a copy because TinyXML2 doesn't allow insertion of elements from another doc...
			if(!isEADesktopEntitlementBlacklisted(pEntitlement))
				pEntitlements->InsertEndChild(pEntitlement->DeepClone(&xmlDoc));
			pEntitlement = pEntitlement->NextSiblingElement("entitlement");
		}

		XMLPrinter printer;
		xmlDoc.Print(&printer);

		*str = printer.CStr(); // copy constructor

		logger->debug("Modified response: \n{}", printer.CStr());
		logger->info("Inserted {} entitlements: {}", inserted);
	} while(false);

	return str;
}

#endif
