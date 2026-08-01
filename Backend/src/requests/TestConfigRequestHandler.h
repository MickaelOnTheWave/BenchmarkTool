#ifndef TESTCONFIGREQUESTHANDLER_H
#define TESTCONFIGREQUESTHANDLER_H

#include "TypeRequestHandler.h"

class TestConfigRequestHandler : public TypeRequestHandler
{
public:
   nlohmann::json List(Database& db, const nlohmann::json &input) override;
   nlohmann::json Create(Database& db, const nlohmann::json &input) override;
   nlohmann::json Delete(Database& db, const nlohmann::json &input) override;
};

#endif // TESTCONFIGREQUESTHANDLER_H
