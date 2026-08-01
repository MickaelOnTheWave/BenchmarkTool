#ifndef MACHINEREQUESTHANDLER_H
#define MACHINEREQUESTHANDLER_H

#include "TypeRequestHandler.h"

class MachineRequestHandler : public TypeRequestHandler
{
public:
   nlohmann::json List(Database& db, const nlohmann::json &input) override;
   nlohmann::json Create(Database& db, const nlohmann::json &input) override;
   nlohmann::json Delete(Database& db, const nlohmann::json &input) override;
};

#endif // MACHINEREQUESTHANDLER_H
