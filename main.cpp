#include "database.h"
#include "parser.h"
#include <iostream>
#include <string>

void printUsage() {
    std::cout << "Usage: ./no_sql_dbms <database> <command> [collection] <arguments...>" << std::endl;
    std::cout << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "  insert [collection] <json_document>    - Insert document (default collection: 'default')" << std::endl;
    std::cout << "  find [collection] <query_json>         - Find documents (default collection: 'default')" << std::endl;
    std::cout << "  delete [collection] <query_json>       - Delete documents (default collection: 'default')" << std::endl;
    std::cout << "  stats                                 - Show database statistics" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  ./no_sql_dbms mydb insert '{\"name\": \"Alice\"}'          # Default collection" << std::endl;
    std::cout << "  ./no_sql_dbms mydb insert users '{\"name\": \"Alice\"}'    # Specific collection" << std::endl;
    std::cout << "  ./no_sql_dbms mydb find '{\"age\": 25}'                    # Default collection" << std::endl;
    std::cout << "  ./no_sql_dbms mydb find users '{\"age\": 25}'              # Specific collection" << std::endl;
    std::cout << "  ./no_sql_dbms mydb stats                                   # Database stats" << std::endl;
}

// функция для определения, является ли аргумент названием коллекции
bool looksLikeCollectionName(const std::string& arg) {
    if (arg.empty()) return false;
    if (arg[0] == '{') return false; // это JSON
    if (arg == "insert" || arg == "find" || arg == "delete" || arg == "stats") return false;
    return true;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printUsage();
        return 1;
    }
    std::string database_name = argv[1];
    std::string command = argv[2];

    try {
        Database db(database_name);
        if (command == "insert") {
            std::string collection_name;
            std::string json_document;
            if (argc == 4) {
                // Формат: ./no_sql_dbms db insert '{"doc"}'
                collection_name = "default";
                json_document = argv[3];
            } else if (argc == 5 && looksLikeCollectionName(argv[3])) {
                // Формат: ./no_sql_dbms db insert collection '{"doc"}'
                collection_name = argv[3];
                json_document = argv[4];
            } else {
                std::cerr << "Error: insert requires <json_document> or <collection> <json_document>" << std::endl;
                std::cout << "Usage: ./no_sql_dbms <database> insert [collection] <json_document>" << std::endl;
                return 1;
            }
            
            Collection& collection = db.getCollection(collection_name);
            if (collection.insert(json_document)) {
                std::cout << "Document inserted successfully into collection '" << collection_name << "'." << std::endl;
            } else {
                std::cerr << "Failed to insert document." << std::endl;
                return 1;
            }
        } else if (command == "find") {
            std::string collection_name;
            std::string query_json;
            if (argc == 4) {
                collection_name = "default";
                query_json = argv[3];
            } else if (argc == 5 && looksLikeCollectionName(argv[3])) {
                collection_name = argv[3];
                query_json = argv[4];
            } else {
                std::cerr << "Error: find requires <query_json> or <collection> <query_json>" << std::endl;
                std::cout << "Usage: ./no_sql_dbms <database> find [collection] <query_json>" << std::endl;
                return 1;
            }
            
            Collection& collection = db.getCollection(collection_name);
            auto results = collection.find(query_json);
            
            if (results.empty()) {
                std::cout << "No documents found in collection '" << collection_name << "'." << std::endl;
            } else {
                std::cout << "Found " << results.size() << " documents in collection '" << collection_name << "':" << std::endl;
                for (size_t i = 0; i < results.size(); ++i) {
                    std::cout << results[i].toJson() << std::endl;
                }
            }
            
        } else if (command == "delete") {
            std::string collection_name;
            std::string query_json;
            if (argc == 4) {
                collection_name = "default";
                query_json = argv[3];
            } else if (argc == 5 && looksLikeCollectionName(argv[3])) {
                collection_name = argv[3];
                query_json = argv[4];
            } else {
                std::cerr << "Error: delete requires <query_json> or <collection> <query_json>" << std::endl;
                std::cout << "Usage: ./no_sql_dbms <database> delete [collection] <query_json>" << std::endl;
                return 1;
            }
            Collection& collection = db.getCollection(collection_name);
            size_t deleted_count = collection.remove(query_json);
            
            std::cout << "Deleted " << deleted_count << " documents from collection '" << collection_name << "'." << std::endl;
        } else {
            std::cerr << "Unknown command: " << command << std::endl;
            printUsage();
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}