#include "document.h"

DocumentWrapper::DocumentWrapper() : doc(nlohmann::json::object()) {}
DocumentWrapper::DocumentWrapper(const Document& document) : doc(document) {}
DocumentWrapper::DocumentWrapper(const std::string& json_str) {
    try {
        doc = nlohmann::json::parse(json_str);
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        doc = nlohmann::json::object(); //пустой объект
    }
}

DocumentWrapper::DocumentWrapper(const DocumentWrapper& other) : doc(other.doc) {}
DocumentWrapper& DocumentWrapper::operator=(const DocumentWrapper& other) {
    if (this != &other) {
        doc = other.doc;
    }
    return *this;
}

std::string DocumentWrapper::generateId() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    
    std::stringstream ss;
    ss << "doc_";
    
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    ss << timestamp << "_";
    
    for (int i = 0; i < 8; ++i) {
        ss << std::hex << dis(gen);
    }
    
    return ss.str();
}

void DocumentWrapper::setGeneratedId() {
    if (!doc.contains("_id")) {
        doc["_id"] = generateId();
    }
}
// проверка наличия поля
bool DocumentWrapper::hasField(const std::string& field_name) const {
    return doc.contains(field_name);
}
//сериализайия
std::string DocumentWrapper::toJson() const {
    return doc.dump(); // из JSON в строку
}
std::string DocumentWrapper::toPrettyJson() const {
    return doc.dump(4);
}
const Document& DocumentWrapper::getRawDocument() const {
    return doc;
}
Document& DocumentWrapper::operator[](const std::string& key) {
    return doc[key];
}
const Document& DocumentWrapper::operator[](const std::string& key) const {
    return doc.at(key);
}