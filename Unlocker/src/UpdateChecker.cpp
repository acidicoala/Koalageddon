#include "pch.h"
#include "UpdateChecker.h"
#include "Logger.h"
#include "constants.h"

void UpdateChecker::checkForUpdates()
{
	std::thread([](){
		// Fetch offsets
		cpr::Response r = cpr::Get(
			cpr::Url{ latest_release_url_api },
			cpr::Timeout{ 3 * 1000 } // 3s
		);

		if(r.status_code != 200)
		{
			logger->error("Failed to check for updates: {} - {}", r.error.code, r.error.message);
			return;
		}

		auto release = json::parse(r.text, nullptr, true, true);
		string tagName = release["tag_name"];

		auto currentVersionTag = fmt::format("v{}", VERSION);

		if(tagName != currentVersionTag)
		{
			logger->warn(
				"A new Koalageddon version {} has been released. Get it from: {} ",
				currentVersionTag, latest_release_url
			);
		}
	}).detach();
}
