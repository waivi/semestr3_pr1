#include "database.h"
#include <iostream>
#include <fstream>
#include <cstdio>

Database::Database(const std::string& db_name, const std::string& base_path) 
    : name(db_name), storage_path(base_path + "/" + db_name) {
    
    ensureStorageDirectory();
    loadExistingCollections();
    
    std::cout << "Database '" << name << "' initialized at: " << storage_path << std::endl;
}

Database::~Database() {
    Vector<std::string> keys = collections.keys();
    for (size_t i = 0; i < keys.size(); ++i) {
        Collection* collection = nullptr;
        if (collections.get(keys[i], collection)) {
            delete collection;
        }
    }
    std::cout << "Database '" << name << "' destroyed." << std::endl;
}

// создание директории если не существует
void Database::ensureStorageDirectory() const {
    system(("mkdir -p " + storage_path).c_str());
}

// загрузка существующих коллекций
void Database::loadExistingCollections() {
    std::string command = "find " + storage_path + " -name \"*.json\" 2>/dev/null";
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) return;
    
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        std::string file_path(buffer);
        file_path.erase(file_path.find_last_not_of(" \n\r\t") + 1);
        size_t last_slash = file_path.find_last_of('/');
        size_t last_dot = file_path.find_last_of('.');
        if (last_slash == std::string::npos || last_dot == std::string::npos) continue;
        std::string collection_name = file_path.substr(last_slash + 1, last_dot - last_slash - 1);
        Collection* collection = new Collection(collection_name, storage_path);
        collections.put(collection_name, collection);
        std::cout << "Loaded collection: " << collection_name << std::endl;
    }
    pclose(pipe);
}

bool Database::createCollection(const std::string& collection_name) {
    if (collectionExists(collection_name)) {
        std::cerr << "Collection '" << collection_name << "' already exists." << std::endl;
        return false;
    }
    Collection* new_collection = new Collection(collection_name, storage_path);
    collections.put(collection_name, new_collection);
    std::cout << "Collection '" << collection_name << "' created successfully." << std::endl;
    return true;
}

Collection& Database::getCollection(const std::string& collection_name) {
    Collection* collection = nullptr;
    
    if (collections.get(collection_name, collection)) {
        return *collection; 
    }
    
    collection = new Collection(collection_name, storage_path);
    collections.put(collection_name, collection);
    return *collection;  
}

// Проверка существования коллекции
bool Database::collectionExists(const std::string& collection_name) const {
    Collection* temp = nullptr;
    return collections.get(collection_name, temp);
}

bool Database::dropCollection(const std::string& collection_name) {
    Collection* collection = nullptr;
    if (!collections.get(collection_name, collection)) {
        std::cerr << "Collection '" << collection_name << "' does not exist." << std::endl;
        return false;
    }
    
    std::string file_path = storage_path + "/" + collection_name + ".json";
    if (remove(file_path.c_str()) == 0) {
        // сначала удаляем из памяти, потом из HashMap
        delete collection;
        collections.remove(collection_name);
        std::cout << "Collection '" << collection_name << "' dropped successfully." << std::endl;
        return true;
    } else {
        std::cerr << "Failed to delete collection file: " << file_path << std::endl;
        return false;
    }
}

// Информационные методы (без изменений)
std::string Database::getName() const {
    return name;
}

std::string Database::getStoragePath() const {
    return storage_path;
}

size_t Database::getCollectionCount() const {
    return collections.size();
}

Vector<std::string> Database::getCollectionNames() const {
    return collections.keys();
}

bool Database::saveAllCollections() {
    bool success = true;
    Vector<std::string> collection_names = collections.keys();
    
    for (size_t i = 0; i < collection_names.size(); ++i) {
        Collection* collection = nullptr;
        if (collections.get(collection_names[i], collection)) {
            if (!collection->saveToFile()) { 
                success = false;
            }
        }
    }
    
    return success;
}
