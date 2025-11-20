#include "parser.h"
#include <iostream>

bool QueryCondition::matches(const DocumentWrapper& doc) const {
    if (!doc.hasField(field)) {
        return false;
    }
    try {
        if (operator_ == "$eq" || operator_ == "") {
            return doc.getField<Document>(field) == value;
            
        } else if (operator_ == "$like") {
            std::string field_value = doc.getField<std::string>(field);
            std::string pattern = value.get<std::string>();
            return matchLikePattern(field_value, pattern);
            
        } else if (operator_ == "$in") {
            return matchInCondition(doc);
            
        } else {
            return matchComparison(doc); // операторы сравнения
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error comparing field '" << field << "': " << e.what() << std::endl;
        return false;
    }
}

bool QueryCondition::matchLikePattern(const std::string& field_value, const std::string& pattern) const {
    return matchSimplePattern(field_value, pattern, 0, 0); // рекурсия для простого сравнения
}

bool QueryCondition::matchSimplePattern(const std::string& text, const std::string& pattern, size_t text_pos, size_t pattern_pos) const {
    if (pattern_pos == pattern.length()) {
        return text_pos == text.length();
    }
    char pattern_char = pattern[pattern_pos];
    if (pattern_char == '%') {
        // пробуем все возможные позиции, начиная с текущей
        for (size_t i = text_pos; i <= text.length(); ++i) {
            if (matchSimplePattern(text, pattern, i, pattern_pos + 1)) {
                return true;
            }
        }
        return false;
    } 
    else if (pattern_char == '_') {
        // хотя бы один символ в тексте
        if (text_pos < text.length()) {
            return matchSimplePattern(text, pattern, text_pos + 1, pattern_pos + 1);
        }
        return false;
    }
    else {
        // обычный символ - должен точно совпадать
        if (text_pos < text.length() && text[text_pos] == pattern_char) {
            return matchSimplePattern(text, pattern, text_pos + 1, pattern_pos + 1);
        }
        return false;
    }
}

bool QueryCondition::matchInCondition(const DocumentWrapper& doc) const {
    if (!value.is_array()) {
        return false;
    }
    try {
        Document field_value = doc.getField<Document>(field);
        // ручной перебор элементов массива
        for (auto it = value.begin(); it != value.end(); ++it) {
            if (*it == field_value) {
                return true;
            }
        }
        return false;
    } catch (const std::exception& e) {
        return false;
    }
}

bool QueryCondition::matchComparison(const DocumentWrapper& doc) const {
    try {
        Document field_value = doc.getField<Document>(field);
        
        if (operator_ == "$gt") {
            return field_value > value;
        } else if (operator_ == "$lt") {
            return field_value < value;
        } else if (operator_ == "$gte") {
            return field_value >= value;
        } else if (operator_ == "$lte") {
            return field_value <= value;
        } else if (operator_ == "$ne") {
            return field_value != value;
        }
    } catch (const std::exception& e) {
        return false;
    }
    
    return false;
}

bool ParsedQuery::matches(const DocumentWrapper& doc) const {
    if (has_or_operator) {
        for (size_t i = 0; i < or_conditions.size(); ++i) {
            if (or_conditions[i].matches(doc)) {
                return true;
            }
        }
        return false;
    } else {
        for (size_t i = 0; i < conditions.size(); ++i) {
            if (!conditions[i].matches(doc)) {
                return false;
            }
        }
        return true;
    }
}

ParsedQuery QueryParser::parse(const std::string& json_query) const {
    ParsedQuery result;
    try {
        Document query_doc = nlohmann::json::parse(json_query);
        parseCondition(query_doc, result);
    } catch (const std::exception& e) {
        std::cerr << "Query parsing error: " << e.what() << std::endl;
        std::cerr << "Invalid JSON query: " << json_query << std::endl;
    }
    
    return result;
}

void QueryParser::parseCondition(const Document& condition_doc, ParsedQuery& result) const {
    // ручной перебор 
    for (auto it = condition_doc.begin(); it != condition_doc.end(); ++it) {
        std::string field = it.key();
        auto value = it.value();
        
        if (isLogicalOperator(field)) {
            parseLogicalOperator(field, value, result);
        } else if (value.is_object()) {
            // обрабатываем операторы сравнения
            for (auto op_it = value.begin(); op_it != value.end(); ++op_it) {
                std::string op = op_it.key();
                auto op_value = op_it.value();
                
                if (isComparisonOperator(op)) {
                    QueryCondition condition;
                    condition.field = field;
                    condition.operator_ = op;
                    condition.value = op_value;
                    result.conditions.push_back(condition);
                }
            }
        } else {
            // простое равенство
            QueryCondition condition;
            condition.field = field;
            condition.operator_ = "$eq"; // неявный оператор
            condition.value = value;
            result.conditions.push_back(condition);
        }
    }
}

void QueryParser::parseLogicalOperator(const std::string& op, const Document& op_doc, ParsedQuery& result) const {
    if (op == "$or") {
        result.has_or_operator = true;
        
        if (op_doc.is_array()) {
            //перебор массива
            for (auto it = op_doc.begin(); it != op_doc.end(); ++it) {
                ParsedQuery or_query;
                parseCondition(*it, or_query);
                result.or_conditions.push_back(or_query);
            }
        }
    } else if (op == "$and") {
        if (op_doc.is_array()) {
            for (auto it = op_doc.begin(); it != op_doc.end(); ++it) {
                parseCondition(*it, result);
            }
        }
    }
}

bool QueryParser::isComparisonOperator(const std::string& field) const {
    return field == "$eq" || field == "$gt" || field == "$lt" || 
           field == "$gte" || field == "$lte" || field == "$ne" || 
           field == "$like" || field == "$in";
}

bool QueryParser::isLogicalOperator(const std::string& field) const {
    return field == "$or" || field == "$and";
}