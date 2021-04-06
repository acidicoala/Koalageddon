#pragma once
#include "framework.h"

using namespace tinyxml2;

extern XMLDocument originEntitlementsXML;
extern XMLDocument eaDesktopEntitlementsXML;

void fetchEntitlementsAsync();
bool isOriginEntitlementBlacklisted(XMLElement* pEntitlement);
bool saveLangRequestId(const string& data, string* outID);
bool modifyLangResponse(const string& data, const string& langRequestId, string* out);
bool modifyEntitlementReponse(const string& data, string* out);
