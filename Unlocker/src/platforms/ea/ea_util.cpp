#include "pch.h"
#include "ea_util.h"
#include "util.h"
#include "Logger.h"
#include "Config.h"
#include "constants.h"

const auto ETAG_PATH = getCacheDirPath() / "origin-entitlements.etag";
const auto ORIGIN_XML_PATH = getCacheDirPath() / "origin-entitlements.xml";

XMLDocument originEntitlementsXML;

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

	auto originXml = readFileContents(ORIGIN_XML_PATH);
	auto etag = readFileContents(ETAG_PATH);

	// If the file is empty, then reset etag
	if(originXml.empty())
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

	auto pOriginEntitlements = (XMLElement*) originEntitlementsXML.InsertFirstChild(
		originEntitlementsXML.NewElement("Entitlements")
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
	}

	// Make a printer to convert the document object into string
	XMLPrinter printer;

	// Cache Origin entitlements
	originEntitlementsXML.Print(&printer);
	auto r1 = writeFileContents(ORIGIN_XML_PATH, printer.CStr());

	// Cache etag
	auto r2 = writeFileContents(ETAG_PATH, r.header["etag"]);

	if(r1 && r2)
		logger->info("Origin entitlements were successfully fetched and cached");
	else
		logger->error("Failed to cache origin entitlements. r1: {}, r2: {}", r1, r2);
}

void readEntitlementsFromFile()
{
	logger->debug("Reading origin entitlements from cache");

	auto fileContent = readFileContents(ORIGIN_XML_PATH);

	if(fileContent.empty())
	{
		logger->error("Origin entitlements file is empty");
		return;
	}

	if(originEntitlementsXML.Parse(fileContent.c_str()) == XMLError::XML_SUCCESS)
		logger->info("Origin entitlements were successfully read from file");
	else
		logger->error("Failed to parse Origin entitlements xml file");
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

auto& getOriginConfig()
{
	return config->platformRefs.Origin;
}

bool isOriginEntitlementBlacklisted(XMLElement* pEntitlement)
{
	return vectorContains(getOriginConfig().blacklist, string(pEntitlement->FindAttribute("ItemId")->Value()));
}


bool saveLangRequestId(const string& data, string* langRequestId)
{
	static std::regex gameInfoRequest(R"aw(<Request[\S\s]*id="(\w+)"[\S\s]*GameInfoId="(\w+)")aw");

	std::smatch match;
	if(!std::regex_search(data, match, gameInfoRequest)) // Is this a GetGameInfo request?
		return false;

	auto id = match[1].str();
	auto gameInfoId = match[2].str();

	if(stringsAreEqual(gameInfoId, "LANGUAGES")) // Is this a language request?
	{
		*langRequestId = id;
		logger->info("Intercepted LSX language request. Saved id: {}", id);
	}

	return true;
}

bool modifyLangResponse(const string& data, const string& langRequestId, string* out)
{
	static std::regex gameInfoResponse(R"aw(<Response[\S\s]*id=\"(\w+)\"[\S\s]*GameInfo=\"(.+)\")aw");
	static string allLanguages = R"(GameInfo="cs_CZ,da_DK,de_DE,en_US,es_ES,fi_FI,fr_FR,it_IT,ja_JP,ko_KR,nl_NL,no_NO,pl_PL,pt_BR,ru_RU,sv_SE,zh_CN,zh_TW")";

	std::smatch match;
	if(!std::regex_search(data, match, gameInfoResponse)) // Is this a GetGameInfo response?
		return false;

	auto id = match[1].str();

	if(stringsAreEqual(id, langRequestId)) // Is this a language response?
	{
		auto spoof = new string(std::regex_replace(data, std::regex(R"aw(GameInfo="(.+)")aw"), allLanguages));
		*out = spoof->c_str();
		logger->info("Intercepted LSX language response");
		logger->debug("Spoofing with: \n{}", *spoof);
	}
	return true;
}

bool modifyEntitlementReponse(const string& data, string* out)
{
	XMLDocument xmlDoc;
	if(xmlDoc.Parse(data.c_str()) != XMLError::XML_SUCCESS)
		return false;

	auto pLSX = xmlDoc.FirstChildElement("LSX");
	if(pLSX == nullptr)
		return false;

	auto pResponse = pLSX->FirstChildElement("Response");
	if(pResponse == nullptr)
		return false;

	auto pQueryEntitlementsResponse = pResponse->FirstChildElement("QueryEntitlementsResponse");
	if(pQueryEntitlementsResponse == nullptr)
		return false;

	logger->info("Intercepted QueryEntitlementsResponse");

	// Origin / EA Desktop magic happens here

	// First filter out blacklisted DLCs from the legit response
	auto pEntitlement = pQueryEntitlementsResponse->FirstChildElement("Entitlement");
	while(pEntitlement != nullptr)
	{
		if(isOriginEntitlementBlacklisted(pEntitlement))
			pQueryEntitlementsResponse->DeleteChild(pEntitlement);
		pEntitlement = pEntitlement->NextSiblingElement("Entitlement");
	}

	// Insert our entitlements into the original response
	auto inserted = 0;
	pEntitlement = originEntitlementsXML.FirstChildElement("Entitlements")->FirstChildElement("Entitlement");
	while(pEntitlement != nullptr)
	{
		inserted++;
		// Have to make a copy because TinyXML2 doesn't allow insertion of elements from another doc...
		if(!isOriginEntitlementBlacklisted(pEntitlement))
			pQueryEntitlementsResponse->InsertEndChild(pEntitlement->DeepClone(&xmlDoc));
		pEntitlement = pEntitlement->NextSiblingElement("Entitlement");
	}

	XMLPrinter printer;
	xmlDoc.Print(&printer);

	logger->debug("Modified response: \n{}", printer.CStr());
	logger->info("Inserted {} entitlements: {}", inserted);

	*out = printer.CStr(); // copy constructor
	return true;
}
