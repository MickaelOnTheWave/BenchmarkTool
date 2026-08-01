#ifndef TESTREQUESTHANDLER_H
#define TESTREQUESTHANDLER_H

#include "TypeRequestHandler.h"

class TestRequestHandler : public TypeRequestHandler
{
public:
   nlohmann::json List(Database& db, const nlohmann::json &input) override;
   nlohmann::json Create(Database& db, const nlohmann::json &input) override;
   nlohmann::json Delete(Database& db, const nlohmann::json &input) override;
};

#endif // TESTREQUESTHANDLER_H
