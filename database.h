#ifndef DATABASE_H
#define DATABASE_H

#include "collection.h"
#include "hash_map.h"
#include <string>

class Database {
private:
    std::string name;
    std::string storage_path;//путь к месту хранения
    HashMap<std::string, Collection*> collections;
    
    void ensureStorageDirectory() const;
    void loadExistingCollections();

public:
    Database(const std::string& db_name, const std::string& base_path = "./data");
    ~Database();  
    
    // Управление коллекциями
    bool createCollection(const std::string& collection_name);
    Collection& getCollection(const std::string& collection_name);
    bool collectionExists(const std::string& collection_name) const;
    bool dropCollection(const std::string& collection_name);
    
    // Информация о БД
    std::string getName() const;
    std::string getStoragePath() const;
    size_t getCollectionCount() const;
    Vector<std::string> getCollectionNames() const;
    
    // Статистика
    void printStats() const;
    
    // Персистентность
    bool saveAllCollections();
};

#endif