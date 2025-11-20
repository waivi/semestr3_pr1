#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <iostream>
#include <string>
#include <memory>
#include <chrono>
#include <random>
#include <sstream>
#include <nlohmann/json.hpp>

using Document = nlohmann::json;

class DocumentWrapper {
private:
    Document doc;

public:
    DocumentWrapper();
    DocumentWrapper(const Document& document);
    DocumentWrapper(const std::string& json_str);
    DocumentWrapper(const DocumentWrapper& other);
    
    DocumentWrapper& operator=(const DocumentWrapper& other);
    Document& operator[](const std::string& key);
    const Document& operator[](const std::string& key) const;
    
    static std::string generateId();
    void setGeneratedId();
    bool hasField(const std::string& field_name) const;
    
    template<typename T>
    T getField(const std::string& field_name, const T& default_value = T()) const;
    
    template<typename T>
    void setField(const std::string& field_name, const T& value);
    
    std::string toJson() const;
    std::string toPrettyJson() const;
    const Document& getRawDocument() const;
};
template<typename T>
T DocumentWrapper::getField(const std::string& field_name, const T& default_value) const {
    if (doc.contains(field_name)) { //проверяет есть ли поле 
        try {
            return doc[field_name].get<T>(); //получить как тип T
        } catch (const nlohmann::json::exception& e) {
            std::cerr << "Error getting field '" << field_name << "': " << e.what() << std::endl;
            return default_value;
        }
    }
    return default_value;
}
template<typename T>
void DocumentWrapper::setField(const std::string& field_name, const T& value) {
    doc[field_name] = value;
}

#endif