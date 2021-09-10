#include "pch.h"
#include "UpdateChecker.h"
#include "Logger.h"
#include "constants.h"

void UpdateChecker::checkForUpdates()
{
	std::thread([](){
		// Fetch offsets
		auto r = fetch(latest_release_url_api);

		if(r.status_code != 200)
		{
			logger->error(
				"Failed to check for updates. ErrorCode: {}. StatusCode: {}. Message: {}",
				r.error.code, r.status_code, r.error.message
			);
			return;
		}

		auto release = json::parse(r.text, nullptr, true, true);
		string tagName = release["tag_name"];

		auto currentVersionTag = fmt::format("v{}", VERSION);

		if(tagName != currentVersionTag)
		{
			logger->warn(
				"A new Koalageddon version {} has been released. Get it from: {} ",
				tagName, latest_release_url
			);
		}
	}).detach();
}
