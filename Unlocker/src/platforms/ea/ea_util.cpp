#include "pch.h"
#include "ea_util.h"
#include "util.h"
#include "Logger.h"
#include <constants.h>

const auto ETAG_PATH = getCacheDirPath() / "origin-entitlements.etag";
const auto ORIGIN_XML_PATH = getCacheDirPath() / "origin-entitlements.xml";
const auto EA_DESKTOP_XML_PATH = getCacheDirPath() / "ea-desktop-entitlements.xml";


XMLDocument originEntitlementsXML;
XMLDocument eaDesktopEntitlementsXML;

struct OriginEntitlement
{
	string entitlementTag;
	string entitlementType;
	string groupName;
	string productId;
};

void from_json(const json& j, OriginEntitlement& p)
{
	j["entitlementTag"].get_to(p.entitlementTag);
	j["entitlementType"].get_to(p.entitlementType);
	j["groupName"].get_to(p.groupName);
	j["productId"].get_to(p.productId);
}

void fetchEntitlements()
{
	logger->debug("Fetching Origin entitlements");

	auto originXml = readFileContents(ORIGIN_XML_PATH.string());
	auto eaDesktopXml = readFileContents(EA_DESKTOP_XML_PATH.string());
	auto etag = readFileContents(ETAG_PATH.string());

	// If the file is empty, then reset etag
	if(originXml.empty() || eaDesktopXml.empty())
		etag = "";

	auto r = fetch(origin_entitlements_url, cpr::Header{ {"If-None-Match", etag} });

	if(r.status_code == 304)
	{
		logger->debug("Cached Origin entitlements have not changed");
		return;
	}

	if(r.status_code != 200)
	{
		logger->error("Failed to fetch Origin entitlements: {} - {}", r.error.code, r.error.message);
		return;
	}

	vector<OriginEntitlement> jsonEntitlements;

	try
	{
		// Parse json into our vector
		json::parse(r.text, nullptr, true, true).get_to(jsonEntitlements);
	} catch(json::exception& ex)
	{
		logger->error("Error parsing Origin entitlements json: {}", ex.what());
		return;
	}

	originEntitlementsXML.Clear();
	eaDesktopEntitlementsXML.Clear();

	auto pOriginEntitlements = (XMLElement*) originEntitlementsXML.InsertFirstChild(
		originEntitlementsXML.NewElement("Entitlements")
	);
	auto pEADesktopEntitlements = (XMLElement*) eaDesktopEntitlementsXML.InsertFirstChild(
		eaDesktopEntitlementsXML.NewElement("entitlements")
	);

	int index = 1000000;
	for(const auto& e : jsonEntitlements)
	{
		auto pEntitlement = pOriginEntitlements->InsertNewChildElement("Entitlement");
		pEntitlement->SetAttribute("EntitlementTag", e.entitlementTag.c_str());
		pEntitlement->SetAttribute("ItemId", e.productId.c_str());
		pEntitlement->SetAttribute("Group", e.groupName.c_str());
		pEntitlement->SetAttribute("Type", e.entitlementType.c_str());
		pEntitlement->SetAttribute("EntitlementId", index);
		pEntitlement->SetAttribute("Source", "ORIGIN-OIG");
		pEntitlement->SetAttribute("UseCount", 0);
		pEntitlement->SetAttribute("Version", 0);
		pEntitlement->SetAttribute("ResourceId", "");
		pEntitlement->SetAttribute("LastModifiedDate", "2021-01-01T00:00:00Z");
		pEntitlement->SetAttribute("Expiration", "0000-00-00T00:00:00");
		pEntitlement->SetAttribute("GrantDate", "2021-01-01T00:00:00Z");

		pEntitlement = pEADesktopEntitlements->InsertNewChildElement("entitlement");
		pEntitlement->InsertNewChildElement("entitlementId")->SetText(index++);
		pEntitlement->InsertNewChildElement("version")->SetText(0);
		pEntitlement->InsertNewChildElement("productId")->SetText(e.productId.c_str());
		pEntitlement->InsertNewChildElement("productCatalog")->SetText("OFB");
		pEntitlement->InsertNewChildElement("entitlementTag")->SetText(e.entitlementTag.c_str());
		pEntitlement->InsertNewChildElement("status")->SetText("ACTIVE");
		pEntitlement->InsertNewChildElement("useCount")->SetText(0);
		pEntitlement->InsertNewChildElement("entitlementSource")->SetText("ORIGIN-OIG");
		pEntitlement->InsertNewChildElement("entitlementType")->SetText(e.entitlementType.c_str());
		pEntitlement->InsertNewChildElement("originPermissions")->SetText(0);
		pEntitlement->InsertNewChildElement("isConsumable")->SetText(false);
		pEntitlement->InsertNewChildElement("lastModifiedDate")->SetText("2020-01-01T00:00Z");
		pEntitlement->InsertNewChildElement("grantDate")->SetText("2020-01-01T00:00:00Z");
	}

	// Make a printer to convert the document object into string
	XMLPrinter printer;

	// Cache Origin entitlements
	originEntitlementsXML.Print(&printer);
	auto r1 = writeFileContents(ORIGIN_XML_PATH, printer.CStr());

	// Cache EA Desktop entitlements
	printer.ClearBuffer();
	eaDesktopEntitlementsXML.Print(&printer);
	auto r2 = writeFileContents(EA_DESKTOP_XML_PATH, printer.CStr());

	// Cache etag
	auto r3 = writeFileContents(ETAG_PATH, r.header["etag"]);

	if(r1 && r2 && r3)
		logger->info("Origin entitlements were successfully fetched and cached");
	else
		logger->error("Failed to cache origin entitlements. r1: {}, r2: {}, r3: {}", r1, r2, r3);
}

void readEntitlementsFromFile()
{
	logger->debug("Reading origin entitlements from cache");

	auto originText = readFileContents(ORIGIN_XML_PATH.string());
	auto eaDesktopText = readFileContents(EA_DESKTOP_XML_PATH.string());

	if(originText.empty() || eaDesktopText.empty())
	{
		logger->error("Origin or EA Desktop entitlements file is empty");
		return;
	}

	auto result1 = originEntitlementsXML.Parse(originText.c_str());
	if(result1 != XMLError::XML_SUCCESS)
		logger->error("Failed to parse Origin entitlements xml file");

	auto result2 = eaDesktopEntitlementsXML.Parse(eaDesktopText.c_str());
	if(result2 != XMLError::XML_SUCCESS)
		logger->error("Failed to parse EA Desktop entitlements xml file");

	if(result1 == XMLError::XML_SUCCESS && result2 == XMLError::XML_SUCCESS)
		logger->info("Origin entitlements were successfully read from file");
}

void fetchEntitlementsAsync()
{
	// Execute blocking operations in a new thread
	std::thread fetchingThread([]{
		logger->debug("Entitlement fetching thread started");
		fetchEntitlements();
		readEntitlementsFromFile();
		logger->debug("Entitlement fetching thread finished");
	});
	fetchingThread.detach();
}