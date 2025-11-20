#include "collection.h"
#include "parser.h"
#include <fstream>
#include <iostream>
#include <cstdlib> 

Collection::Collection(const std::string& collection_name, const std::string& db_path)
    : name(collection_name), storage_path(db_path + "/" + collection_name + ".json") {
    loadFromFile(); //автоматом загружаем данные
}

bool Collection::insert(const DocumentWrapper& document) {
    DocumentWrapper doc_copy = document;
    if (!doc_copy.hasField("_id")) { //нет id - генерируем
        doc_copy.setGeneratedId();
    }
    std::string id = doc_copy.getField<std::string>("_id");
    data.put(id, doc_copy);
    return saveToFile();
}
bool Collection::insert(const std::string& json_str) {
    DocumentWrapper doc(json_str);
    return insert(doc);
}
bool Collection::insert(const Document& json_doc) {
    DocumentWrapper doc(json_doc);
    return insert(doc);
}
bool Collection::findById(const std::string& id, DocumentWrapper& result) const {
    return data.get(id, result);
}

Vector<DocumentWrapper> Collection::findAll() const {
    Vector<DocumentWrapper> all_documents;
    Vector<DocumentWrapper> values = data.values();
    for (size_t i = 0; i < values.size(); ++i) {
        all_documents.push_back(values[i]);
    }
    return all_documents;
}
Vector<std::string> Collection::getAllIds() const {
    Vector<std::string> ids;
    Vector<std::string> keys = data.keys();
    for (size_t i = 0; i < keys.size(); ++i) {
        ids.push_back(keys[i]);
    }
    return ids;
}

bool Collection::removeById(const std::string& id) {
    bool removed = data.remove(id);
    if (removed) {
        saveToFile();
    }
    return removed;
}

bool Collection::saveToFile() const {
    try {
        // JSON объект для хранения всех доков
        nlohmann::json collection_data = nlohmann::json::object();
        Vector<DocumentWrapper> all_docs = findAll();
        // сохраняем каждый в JSON
        for (size_t i = 0; i < all_docs.size(); ++i) {
            const DocumentWrapper& doc = all_docs[i];
            std::string id = doc.getField<std::string>("_id");
            collection_data[id] = doc.getRawDocument();
        }
        // создаем директорию если не существует
        std::string directory = storage_path.substr(0, storage_path.find_last_of('/'));
        system(("mkdir -p " + directory).c_str());
        // Сохраняем в файл
        std::ofstream file(storage_path);
        if (!file.is_open()) {
            std::cerr << "Cannot open file for writing: " << storage_path << std::endl;
            return false;
        }
        file << collection_data.dump(4);  // красивый JSON с отступами
        file.close();
        std::cout << "Collection " << name << " saved to " << storage_path << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving collection: " << e.what() << std::endl;
        return false;
    }
}

bool Collection::loadFromFile() {
    try {
        std::ifstream file(storage_path);
        if (!file.is_open()) {
            std::cout << "Collection file not found, creating new: " << storage_path << std::endl;
            return true;
        }
        nlohmann::json collection_data;
        file >> collection_data; //читаем
        file.close();
        data.clear();
        // загружаем документы из JSON
        for (auto& [id, doc_json] : collection_data.items()) {
            DocumentWrapper doc(doc_json);
            data.put(id, doc);
        }
        std::cout << "Collection " << name << " loaded from " << storage_path << " (" << size() << " documents)" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading collection: " << e.what() << std::endl;
        return false;
    }
}

size_t Collection::size() const {
    return data.size();
}
std::string Collection::getName() const {
    return name;
}
std::string Collection::getStoragePath() const {
    return storage_path;
}

// поиск доков по JSON запросу
Vector<DocumentWrapper> Collection::find(const std::string& query_json) const {
    QueryParser parser;
    ParsedQuery query = parser.parse(query_json);
    return find(query);
}
Vector<DocumentWrapper> Collection::find(const ParsedQuery& query) const {
    Vector<DocumentWrapper> results;
    Vector<DocumentWrapper> all_docs = findAll();
    for (size_t i = 0; i < all_docs.size(); ++i) {
        if (query.matches(all_docs[i])) {
            results.push_back(all_docs[i]);
        }
    }
    return results;
}

size_t Collection::remove(const std::string& query_json) {
    QueryParser parser;
    ParsedQuery query = parser.parse(query_json);
    return remove(query);
}
size_t Collection::remove(const ParsedQuery& query) {
    Vector<DocumentWrapper> docs_to_remove = find(query);
    size_t removed_count = 0;
    for (size_t i = 0; i < docs_to_remove.size(); ++i) {
        std::string id = docs_to_remove[i].getField<std::string>("_id");
        if (removeById(id)) {
            removed_count++;
        }
    }
    return removed_count;
}