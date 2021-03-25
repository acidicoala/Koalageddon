#include "pch.h"
#include "PatternMatcher.h"

struct PatternMask
{
	string binaryPattern;
	string mask;
};

// Converts user-friendly hex pattern string into a byte array
// and generate corresponding mask
NOINLINE PatternMask getPatternAndMask(string pattern)
{
	// Remove whitespaces
	pattern = std::regex_replace(pattern, std::regex("\\s+"), "");

	// Convert hex to binary
	std::stringstream patternStream;
	std::stringstream maskStream;
	for(size_t i = 0; i < pattern.length(); i += 2)
	{
		std::string byteString = pattern.substr(i, 2);

		maskStream << (byteString == "??" ? '?' : 'x');

		// Handle wildcards ourselves, rest goes to strtol
		patternStream << (byteString == "??" ? '?' : (char) strtol(byteString.c_str(), nullptr, 16));
	}

	return { patternStream.str(), maskStream.str() };
}

// Credit: superdoc1234
// Source: https://www.unknowncheats.me/forum/1364641-post150.html
NOINLINE PVOID find(PCSTR pBaseAddress, size_t memLength, PCSTR pattern, PCSTR mask)
{
	auto DataCompare = [](const auto* pData, const auto* mask, const auto* cmask, auto chLast, size_t iEnd) -> bool {
		if(pData[iEnd] != chLast) return false;
		for(size_t i = 0; i <= iEnd; ++i)
		{
			if(cmask[i] == 'x' && pData[i] != mask[i])
			{
				return false;
			}
		}

		return true;
	};

	auto iEnd = strlen(mask) - 1;
	auto chLast = pattern[iEnd];

	for(size_t i = 0; i < memLength - strlen(mask); ++i)
	{
		if(DataCompare(pBaseAddress + i, pattern, mask, chLast, iEnd))
		{
			return (PVOID) (pBaseAddress + i);
		}
	}

	return nullptr;
}

// Credit: Rake
// Source: https://guidedhacking.com/threads/external-internal-pattern-scanning-guide.14112/
NOINLINE PVOID PatternMatcher::scanInternal(PCSTR pMemory, size_t length, string pattern)
{
	// logger->debug("DLL Base: {}, Image Size: {:X}", lpBaseOfDll, SizeOfImage);

	PVOID match = nullptr;
	MEMORY_BASIC_INFORMATION mbi{};

	auto [binaryPattern, mask] = getPatternAndMask(pattern);

	auto pCurrentRegion = pMemory;
	do
	{
		// Skip irrelevant code regions
		auto result = VirtualQuery((LPCVOID) pCurrentRegion, &mbi, sizeof(mbi));
		if(result && mbi.State == MEM_COMMIT && mbi.Protect != PAGE_NOACCESS)
		{
			// logger->debug("Current Region: {}, Region Size: {:X}", (void*) pCurrentRegion, mbi.RegionSize);
			match = find(pCurrentRegion, mbi.RegionSize, binaryPattern.c_str(), mask.c_str());

			if(match)
				break;
		}

		pCurrentRegion += mbi.RegionSize;
	} while(pCurrentRegion < pMemory + length);

	return match;
}
