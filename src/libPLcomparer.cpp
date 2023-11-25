#include <PLcomparer/PLcomparer.h>

namespace PackageListComparer
{

	static size_t WriteCallback(void *contents, size_t size, size_t nmemb,
								void *userp)
	{
		((std::string *)userp)->append((char *)contents, size * nmemb);
		return size * nmemb;
	}

	std::vector<int> splitVersion(const std::string &version)
	{
		std::vector<int> result;
		size_t start = 0;
		size_t end = version.find('.');
		std::string substring;

		while (end != std::string::npos)
		{

			std::string substring = version.substr(start, end - start);

			if (!substring.empty() && std::all_of(substring.begin(), substring.end(), ::isdigit))
			{
				result.push_back(std::stoi(substring));
			}

			start = end + 1;
			end = version.find('.', start);
		}

		substring = version.substr(start);

		// skipping strings with letters
		if (!substring.empty() && std::all_of(substring.begin(), substring.end(), ::isdigit))
		{
			result.push_back(std::stoi(substring));
		}
		return result;
	}

	int CompareVersions(std::string first, std::string second)
	{
		// Replace bad characters
		std::replace(first.begin(), first.end(), '_', '.');
		std::replace(second.begin(), second.end(), '_', '.');

		std::vector<int> version1 = splitVersion(first);
		std::vector<int> version2 = splitVersion(second);

		int maxLength = std::max(version1.size(), version2.size());

		for (int i = 0; i < maxLength; ++i)
		{

			int value1 = (i < version1.size()) ? version1[i] : 0;
			int value2 = (i < version2.size()) ? version2[i] : 0;

			if (value1 > value2)
			{
				return 1;
			}
			else if (value1 < value2)
			{
				return -1;
			}
		}

		return 0;
	}

	nlohmann::json loadJson(const std::string branch, const std::string arch)
	{
		CURL *curl;
		CURLcode res;
		std::string readBuffer;

		curl = curl_easy_init(); // returns a CURL easy handle

		if (curl && !branch.empty() && !arch.empty())
		{
			std::string query = branch + "?arch=" + arch;
			std::string url =
				"https://rdb.altlinux.org/api/export/branch_binary_packages/" + query;
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
			res = curl_easy_perform(curl);

			curl_easy_cleanup(curl); // close connection

			if (res != CURLE_OK)
			{
				fprintf(stderr, "curl_easy_perform() failed: %s\n",
						curl_easy_strerror(res));
			}
			else
			{
				return nlohmann::json::parse(readBuffer);
			}
		}
		return 0;
	}

	nlohmann::json comparing(const nlohmann::json &source, const nlohmann::json &target)
	{
		std::string difference;
		nlohmann::json result(nlohmann::json::value_t::array);

		switch (source.type())
		{
		case nlohmann::json::value_t::array:
		{
			//  first pass: traverse common elements
			size_t i = 0, j = 0;
			nlohmann::json temp_missing, temp_extra, temp_updated;

			// get diffrance packages
			while (i < source.size() && j < target.size())
			{
				if (source[i]["name"] > target[j]["name"])
				{
					// extra packages
					++j;
				}
				else if (source[i]["name"] < target[j]["name"])
				{
					// missing packages
					temp_extra.push_back(source[i]);

					++i;
				}
				else
				{

					// updated packages
					std::string versionSource = source[i]["version"];
					std::string versionTarget = target[j]["version"];
					if (CompareVersions(versionSource, versionTarget) == 1)
					{
						temp_updated.push_back(source[i]);
					}
					++i;
					++j;
				}
			}

			while (i < source.size())
			{

				++i;
			}
			while (j < target.size())
			{
				// missing packages
				temp_missing.push_back(target[j]);
				++j;
			}

			result.insert(result.end(), {"missing", temp_missing});
			result.insert(result.end(), {"extra", temp_extra});
			result.insert(result.end(), {"updated", temp_updated});

			break;
		}
		case nlohmann::json::value_t::object:
		{
			//  first pass: traverse this object's elements
			for (auto it = source.cbegin(); it != source.cend(); ++it)
			{
				if (target.find(it.key()) != target.end())
				{
					auto temp_json = comparing(it.value(), target[it.key()]);

					// is not 0 temporary json (only true)
					if (temp_json.size())
					{
						result.insert(result.end(), temp_json.begin(), temp_json.end());
					}
				}
				else
				{
					std::cerr << "found a key that is not in o -> remove it \n";
				}
			}

			break;
		}
		case nlohmann::json::value_t::null:
		case nlohmann::json::value_t::string:
		case nlohmann::json::value_t::boolean:
		case nlohmann::json::value_t::number_integer:
		case nlohmann::json::value_t::number_unsigned:
		case nlohmann::json::value_t::number_float:
		case nlohmann::json::value_t::binary:
		case nlohmann::json::value_t::discarded:
		default:
		{
			break;
		}
		}

		return result;
	}

}
