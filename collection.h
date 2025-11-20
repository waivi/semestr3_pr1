#ifndef COLLECTION_H
#define COLLECTION_H

#include "document.h"
#include "hash_map.h"  
#include <string>
#include "vector.h"

class QueryParser;
struct ParsedQuery;
//коллекция документов, использует хэш табл для хранения
class Collection {
private:
    std::string name;                    
    HashMap<std::string, DocumentWrapper> data;  // хранилище доков (id, doc)
    std::string storage_path;            // путь для сохранения на диск

public:
    Collection(): name(), data(), storage_path() {};
    Collection(const std::string& collection_name, const std::string& db_path);
    
    bool insert(const DocumentWrapper& document);
    bool insert(const std::string& json_str);
    bool insert(const Document& json_doc);
    bool findById(const std::string& id, DocumentWrapper& result) const;
    //для парсера
    Vector<DocumentWrapper> find(const std::string& query_json) const;
    Vector<DocumentWrapper> find(const ParsedQuery& query) const;
    
    size_t remove(const std::string& query_json);
    size_t remove(const ParsedQuery& query);

    Vector<DocumentWrapper> findAll() const; 
    Vector<std::string> getAllIds() const;
    
    bool removeById(const std::string& id);
    bool saveToFile() const;
    bool loadFromFile();
    size_t size() const;
    std::string getName() const;
    std::string getStoragePath() const;
};

#endif