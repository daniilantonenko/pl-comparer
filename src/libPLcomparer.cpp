#include <PLcomparer/PLcomparer.h>

using json = nlohmann::json;

namespace PackageListComparer
{

	static size_t WriteCallback(void *contents, size_t size, size_t nmemb,
								void *userp)
	{
		((std::string *)userp)->append((char *)contents, size * nmemb);
		return size * nmemb;
	}

	json loadJson(std::string arch)
	{
		CURL *curl;
		CURLcode res;
		std::string readBuffer;

		curl = curl_easy_init(); // returns a CURL easy handle

		if (curl && !arch.empty())
		{
			std::string query = "p10?arch=" + arch;
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
				return json::parse(readBuffer);
			}
		}
		return 0;
	}

	json comparing(const json &source, const json &target)
	{
		std::string difference;
		json result(json::value_t::array);
		// json result = {"missing", "extra", "updated"};

		switch (source.type())
		{
		case json::value_t::array:
		{
			//  first pass: traverse common elements
			size_t i = 0, j = 0;
			json temp_missing, temp_extra, temp_updated;

			// get diffrance packages
			while (i < source.size() && j < target.size())
			{

				if (source[i] == target[j])
				{
					// same packages
				}

				if (source[i]["name"] < target[j]["name"])
				{
					// missing packages
					temp_missing.push_back(source[i]);

					++i;
				}
				else if (source[i]["name"] > target[j]["name"])
				{
					// extra packages
					temp_extra.push_back(target[j]);

					++j;
				}
				else
				{
					// updated packages
					if (source[i]["version"] > target[j]["version"])
					{
						temp_updated.push_back(source[i]);
					}
					++i;
					++j;
				}
			}

			while (i < source.size())
			{

				std::cout << "i < source.size() \n";

				// temp_json.push_back(source[i]);
				++i;
			}
			while (j < target.size())
			{
				std::cout << "i < target.size() \n";
				++j;
			}

			result.insert(result.end(), {"missing", temp_missing});
			result.insert(result.end(), {"extra", temp_extra});
			result.insert(result.end(), {"updated", temp_updated});

			break;
		}
		case json::value_t::object:
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
		case json::value_t::null:
		case json::value_t::string:
		case json::value_t::boolean:
		case json::value_t::number_integer:
		case json::value_t::number_unsigned:
		case json::value_t::number_float:
		case json::value_t::binary:
		case json::value_t::discarded:
		default:
		{
			break;
		}
		}

		return result;
	}

}