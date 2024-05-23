#pragma once
#define WINDOWS_LEAN_AND_MEAN
#include <Windows.h>
#include <string>
#include <fstream>
#include "simdjson.h"
#include "rapidxml.hpp"
#include "toml.hpp"

namespace Plasma {
	namespace IO {
		class FileHandler {
		private:
			std::string FileData;
			std::fstream File;
		public:
			FileHandler(std::string& PathToFile, bool IsBinary);
			~FileHandler();
			std::string Read();
			void Write(std::string& Data);
		};

		class JsonFileHandler {
		private:
			std::string RawJsonData;
			simdjson::padded_string FileData;
			simdjson::ondemand::parser parser;
			simdjson::ondemand::document JsonIterator;
		public:
			JsonFileHandler(std::string& PathToFile);
			~JsonFileHandler();
			std::string GetRaw();
			template<typename ReqestedType> ReqestedType GetData(std::string& Key);
		};

		class XmlFileHandler {
		private:
			rapidxml::xml_document<> XmlFileData;
			std::string FileData;
			std::fstream File;
		public:
			XmlFileHandler(std::string& PathToFile);
			~XmlFileHandler();
			std::string GetRaw();
			std::string GetData(const std::string& XPath);
		};

		class TomlFileHandler {
		private:
			std::string FileData;
			toml::table tbl;
		public:
			TomlFileHandler(std::string& PathToFile);
			~TomlFileHandler();
			std::string GetRaw();
			template<typename ReqestedType> ReqestedType GetData(std::string& Key);
		};
	}
}