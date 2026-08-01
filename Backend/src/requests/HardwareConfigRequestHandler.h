#ifndef HARDWARECONFIGREQUESTHANDLER_H
#define HARDWARECONFIGREQUESTHANDLER_H

#include "TypeRequestHandler.h"

class HardwareConfigRequestHandler : public TypeRequestHandler
{
public:
   nlohmann::json List(Database& db, const nlohmann::json &input) override;
   nlohmann::json Create(Database& db, const nlohmann::json &input) override;
   nlohmann::json Delete(Database& db, const nlohmann::json &input) override;
};

#endif // HARDWARECONFIGREQUESTHANDLER_H
