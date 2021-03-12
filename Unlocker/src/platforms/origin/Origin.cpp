#include "pch.h"
#include "Origin.h"
#include "origin_hooks.h"
#include "util.h"

// Can't include it globally because it pollutes global namespace.
// More specifically, the `interface` name is colliding with steam hooks
#pragma warning(push) // Disable 3rd party library warnings
#pragma warning(disable: ALL_CODE_ANALYSIS_WARNINGS)
#include <codeanalysis\warnings.h>
#include <cpr/cpr.h> 
#pragma warning(pop)

using nlohmann::json;

constexpr auto url = "https://raw.githubusercontent.com/acidicoala/public-entitlements/main/origin/v1/entitlements.json";

const auto XML_PATH = getWorkingDirPath() / "cache" / "origin-entitlements.xml";
const auto ETAG_PATH = getWorkingDirPath() / "cache" / "origin-entitlements.etag";

struct Entitlement
{
	string entitlementTag;
	string entitlementType;
	string groupName;
	string productId;
};

void from_json(const json& j, Entitlement& p)
{
	j["entitlementTag"].get_to(p.entitlementTag);
	j["entitlementType"].get_to(p.entitlementType);
	j["groupName"].get_to(p.groupName);
	j["productId"].get_to(p.productId);
}

void fetchEntitlements()
{
	logger->debug("Fetching Origin entitlements");

	auto xml = readFileContents(XML_PATH.string());
	auto etag = readFileContents(ETAG_PATH.string());

	// If the file is empty, then reset etag
	if(xml.empty())
		etag = "";

	cpr::Response r = cpr::Get(
		cpr::Url{ url },
		cpr::Header{ {"If-None-Match", etag} },
		cpr::Timeout{ 3 * 1000 } // 3s
	);

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

	vector<Entitlement> jsonEntitlements;

	try
	{
		// Parse json into our vector
		json::parse(r.text, nullptr, true, true).get_to(jsonEntitlements);
	} catch(json::exception& ex)
	{
		logger->error("Error parsing Origin entitlements json: {}", ex.what());
		return;
	}

	entitlementsXML.Clear();
	auto pEntitlements = entitlementsXML.NewElement("Entitlements");
	entitlementsXML.InsertFirstChild(pEntitlements);

	int index = 1000000;
	for(const auto& e : jsonEntitlements)
	{
		auto pEntitlement = entitlementsXML.NewElement("Entitlement");
		pEntitlement->SetAttribute("EntitlementTag", e.entitlementTag.c_str());
		pEntitlement->SetAttribute("ItemId", e.productId.c_str());
		pEntitlement->SetAttribute("Group", e.groupName.c_str());
		pEntitlement->SetAttribute("Type", e.entitlementType.c_str());
		pEntitlement->SetAttribute("EntitlementId", index++);
		pEntitlement->SetAttribute("Source", "ORIGIN-OIG");
		pEntitlement->SetAttribute("UseCount", 0);
		pEntitlement->SetAttribute("Version", 0);
		pEntitlement->SetAttribute("ResourceId", "");
		pEntitlement->SetAttribute("LastModifiedDate", "2021-01-01T00:00:00Z");
		pEntitlement->SetAttribute("Expiration", "0000-00-00T00:00:00");
		pEntitlement->SetAttribute("GrantDate", "2021-01-01T00:00:00Z");

		pEntitlements->InsertEndChild(pEntitlement);
	}

	// Make a printer to convert the document object into string
	XMLPrinter printer;
	entitlementsXML.Print(&printer);

	// Cache entitlements
	auto r1 = writeFileContents(XML_PATH, printer.CStr());

	// Cache etag
	auto r2 = writeFileContents(ETAG_PATH, r.header["etag"]);

	if(r1 && r2)
		logger->info("Origin entitlements were successfully fetched and cached");
}

void readEntitlementsFromFile()
{
	logger->debug("Reading origin entitlements from cache");

	auto text = readFileContents(XML_PATH.string());

	if(text.empty())
	{
		logger->error("Origin entitlements file is empty");
		return;
	}

	auto result = entitlementsXML.Parse(text.c_str());
	if(result != XMLError::XML_SUCCESS)
	{
		logger->error("Failed to parse entitlements xml file");
		return;
	}

	logger->info("Origin entitlements were successfully read from file");
}

void Origin::init()
{
	if(initialized || handle == NULL)
		return;

	logger->debug("Initializing Origin platform");

	// Execute blocking operations in a new thread
	std::thread fetchingThread([]{
		logger->debug("Entitlement fetching thread started");
		fetchEntitlements();
		readEntitlementsFromFile();
		logger->debug("Entitlement fetching thread finished");
	});
	fetchingThread.detach();

	installDetourHook(hooks, encrypt, mangled_encrypt);

	logger->info("Origin platform was initialized");
	initialized = true;
}

void Origin::shutdown()
{
	if(!initialized)
		return;

	logger->debug("Shutting down Origin platform");

	for(auto& hook : hooks)
	{
		hook->unHook();
	}
	hooks.clear();

	logger->debug("Origin platform was shut down");
}
