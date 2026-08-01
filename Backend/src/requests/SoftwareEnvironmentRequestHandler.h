#ifndef SOFTWAREENVIRONMENTREQUESTHANDLER_H
#define SOFTWAREENVIRONMENTREQUESTHANDLER_H

#include "TypeRequestHandler.h"

class SoftwareEnvironmentRequestHandler : public TypeRequestHandler
{
public:
   nlohmann::json List(Database& db, const nlohmann::json &input) override;
   nlohmann::json Create(Database& db, const nlohmann::json &input) override;
   nlohmann::json Delete(Database& db, const nlohmann::json &input) override;
};

#endif // SOFTWAREENVIRONMENTREQUESTHANDLER_H
