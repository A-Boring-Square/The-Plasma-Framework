#define WINDOWS_LEAN_AND_MEAN
#include <Windows.h>
#include <string>
#include <fstream>
#include <iostream>
#include "simdjson.h"
#include "rapidxml.hpp"
#include "toml.hpp"
#include "Plasma.hpp"

namespace Plasma {
	namespace IO {

		// Implementation of the FileHandler class
		FileHandler::FileHandler(std::string& PathToFile, bool IsBinary) {
			std::string Line;
			if (IsBinary) {
				this->File.open(PathToFile, std::ios::in | std::ios::out | std::ios::binary);
			}
			else {
				this->File.open(PathToFile, std::ios::in | std::ios::out);
			}

			if (this->File.is_open()) {
				while (std::getline(this->File, Line)) {
					this->FileData.append(Line);
				}
				// Move the file cursor to the end for subsequent writes
				this->File.clear();
				this->File.seekp(0, std::ios::end);
			}
			else {
				throw std::runtime_error("Can't Open File: " + PathToFile);
			}
		}

		std::string FileHandler::Read() {
			return this->FileData;
		}

		void FileHandler::Write(std::string& Data) {
			this->FileData.append(Data);
			this->File << Data;
		}

		FileHandler::~FileHandler() {
			if (this->File.is_open()) {
				this->File.close();
			}
		}

		// Implementation of the JsonFileHandler class
		JsonFileHandler::JsonFileHandler(std::string& PathToFile) {
			std::ifstream file(PathToFile);
			if (!file.is_open()) {
				throw std::runtime_error("Can't Open File: " + PathToFile);
			}

			// Read the entire file content into a string
			this->RawJsonData = std::string((std::istreambuf_iterator<char>(file)),
				std::istreambuf_iterator<char>());

			// Initialize the simdjson parser with the raw JSON data
			this->FileData = simdjson::padded_string(this->RawJsonData);
			this->JsonIterator = parser.iterate(this->FileData);
		}

		JsonFileHandler::~JsonFileHandler() {}

		std::string JsonFileHandler::GetRaw() {
			return this->RawJsonData;
		}

		template<typename RequestedType>
		RequestedType JsonFileHandler::GetData(std::string& Key) {
			for (auto field : this->JsonIterator.get_object()) {
				if (field.key() == Key) {
					if constexpr (std::is_same_v<RequestedType, std::string>) {
						return std::string(field.value().get_c_str());
					}
					else if constexpr (std::is_same_v<RequestedType, int64_t>) {
						return field.value().get_int64();
					}
					else if constexpr (std::is_same_v<RequestedType, double>) {
						return field.value().get_double();
					}
					else if constexpr (std::is_same_v<RequestedType, bool>) {
						return field.value().get_bool();
					}
					else if constexpr (std::is_same_v<RequestedType, simdjson::ondemand::object>) {
						return field.value().get_object();
					}
					else if constexpr (std::is_same_v<RequestedType, simdjson::ondemand::array>) {
						return field.value().get_array();
					}
					// Add more type checks as needed
				}
			}
			throw std::runtime_error("Key not found or type mismatch: " + Key);
		}

		// Implementation of the XmlFileHandler class
		XmlFileHandler::XmlFileHandler(std::string& PathToFile) {
			std::string Line;
			this->File.open(PathToFile);
			if (!this->File.is_open()) {
				throw std::runtime_error("Can't Open File: " + PathToFile);
			}
			while (std::getline(this->File, Line)) {
				this->FileData.append(Line);
			}
			this->XmlFileData.parse<0>(&FileData[0]);
		}

		XmlFileHandler::~XmlFileHandler() {
			if (this->File.is_open()) {
				this->File.close();
			}
		}

		std::string XmlFileHandler::GetRaw() {
			return this->FileData;
		}

		std::string XmlFileHandler::GetData(const std::string& XPath) {
			rapidxml::xml_node<>* currentNode = this->XmlFileData.first_node();
			size_t pos = 0;

			while (XPath[pos] == '/') {
				++pos; // Skip leading slashes
			}

			while (pos < XPath.size()) {
				// Find the next separator or end of string
				size_t next_separator = XPath.find_first_of("/[", pos);
				size_t end = (next_separator == std::string::npos) ? XPath.size() : next_separator;

				std::string token = XPath.substr(pos, end - pos);

				// Handle special tokens
				if (token == "..") { // Parent axis
					currentNode = currentNode->parent();
				}
				else if (token == ".") { // Current node
					// Do nothing
				}
				else if (token == "*") { // Wildcard
					currentNode = currentNode->first_node();
				}
				else if (token[0] == '[') { // Predicate
					// Parse predicate
					size_t close_bracket = token.find(']');
					if (close_bracket == std::string::npos || close_bracket == 1) {
						throw std::runtime_error("Invalid predicate: " + token);
					}
					std::string predicate = token.substr(1, close_bracket - 1);
					// Handle predicates here...

					// Move to the next token
					pos = close_bracket + 1;
					continue;
				}
				else if (token[0] == '@') { // Attribute axis
					// Find attribute name
					std::string attribute_name = token.substr(1);
					// Find attribute node
					rapidxml::xml_attribute<>* attribute = currentNode->first_attribute(attribute_name.c_str());
					if (!attribute) {
						throw std::runtime_error("Attribute not found: " + token);
					}
					// Return attribute value
					return attribute->value();
				}
				else { // Child axis
					currentNode = currentNode->first_node(token.c_str());
				}

				// Move to the next token
				pos = end;
			}

			if (!currentNode) {
				throw std::runtime_error("XPath not found: " + XPath);
			}

			return currentNode->value();
		}

		TomlFileHandler::TomlFileHandler(std::string& PathToFile) {
			std::string Line;
			try {
				this->tbl = toml::parse_file(PathToFile);
				std::fstream TomlFile;
				TomlFile.open(PathToFile, std::ios::in);
				if (TomlFile.is_open()) {
					while (std::getline(TomlFile, Line)) {
						this->FileData.append(Line + "\n");
					}
				}
				else {
					throw std::runtime_error("Can't Open File: " + PathToFile);
				}
			}
			catch (const toml::parse_error& err) {
				throw std::runtime_error("Toml Parsing Failed");
			}
		}

		TomlFileHandler::~TomlFileHandler() {}

		std::string TomlFileHandler::GetRaw() {
			return this->FileData;
		}

		template<typename ReqestedType>
		ReqestedType TomlFileHandler::GetData(std::string& Key) {

		}
	}
}
