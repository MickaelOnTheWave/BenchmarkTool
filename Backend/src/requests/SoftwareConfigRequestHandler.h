#ifndef SOFTWARECONFIGREQUESTHANDLER_H
#define SOFTWARECONFIGREQUESTHANDLER_H

#include "TypeRequestHandler.h"

class SoftwareConfigRequestHandler : public TypeRequestHandler
{
public:
   nlohmann::json List(Database& db, const nlohmann::json &input) override;
   nlohmann::json Create(Database& db, const nlohmann::json &input) override;
   nlohmann::json Delete(Database& db, const nlohmann::json &input) override;
};

#endif // SOFTWARECONFIGREQUESTHANDLER_H
