#include <httplib.h>
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

int main() {
    httplib::Server svr;

    svr.Get("/benchmarks", [](const httplib::Request& req, httplib::Response& res) {
        json benchmarks = {
            {"status", "ok"},
            {"benchmarks", json::array()}
        };
        res.set_content(benchmarks.dump(), "application/json");
    });

    // Serve static files from web directory
    svr.set_mount_point("/", "./web");

    std::cout << "Backend server running on http://localhost:8080\n";
    svr.listen("localhost", 8080);
    return 0;
}
