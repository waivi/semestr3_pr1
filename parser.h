#ifndef PARSER_H
#define PARSER_H

#include "document.h"
#include "vector.h"
#include <string>

struct QueryCondition {
    std::string field;
    std::string operator_; // "$eq" и тд
    Document value;
    
    // проверяет, удовлетворяет ли документ условию (состоит из поле+оерат+знач)
    bool matches(const DocumentWrapper& doc) const;
    
private:
    bool matchLikePattern(const std::string& field_value, const std::string& pattern) const;
    bool matchSimplePattern(const std::string& text, const std::string& pattern, size_t text_pos, size_t pattern_pos) const;
    bool matchInCondition(const DocumentWrapper& doc) const;
    bool matchComparison(const DocumentWrapper& doc) const;
};

struct ParsedQuery { // парсированный запрос
    Vector<QueryCondition> conditions;
    Vector<ParsedQuery> or_conditions; 
    bool has_or_operator = false;
    
    // проверяет, удовлетворяет ли документ всему запросу
    bool matches(const DocumentWrapper& doc) const;
};

class QueryParser {
public:
    // парсит JSON запрос в структурированный формат
    ParsedQuery parse(const std::string& json_query) const;
    
private:
    void parseCondition(const Document& condition_doc, ParsedQuery& result) const; 
    void parseLogicalOperator(const std::string& op, const Document& op_doc, ParsedQuery& result) const;
    
    // Вспомогательные методы для парсинга
    bool isComparisonOperator(const std::string& field) const;
    bool isLogicalOperator(const std::string& field) const;
};

#endif