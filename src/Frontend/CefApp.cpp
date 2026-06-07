#include <include/cef_app.h>
#include <include/cef_client.h>
#include <include/wrapper/cef_helpers.h>

class SimpleHandler : public CefClient, public CefLifeSpanHandler, public CefLoadHandler {
public:
    SimpleHandler() = default;

    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override {
        return this;
    }

    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override {
        return this;
    }

    void OnAfterCreated(CefRefPtr<CefBrowser> browser) override {
        browser->GetMainFrame()->LoadURL("http://localhost:8080/index.html");
    }

    void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, TransitionType transition_type) override {}
    void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) override {}

    IMPLEMENT_REFCOUNTING(SimpleHandler);
};

class SimpleApp : public CefApp, public CefBrowserProcessHandler {
public:
    SimpleApp() = default;

    virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
        return this;
    }

    void OnContextInitialized() override {
        CefRefPtr<SimpleHandler> handler = new SimpleHandler();
        CefWindowInfo window_info;
        window_info.SetAsWindowless(0);

        CefBrowserHost::CreateBrowser(window_info, handler, "http://localhost:8080/index.html", CefBrowserSettings(), nullptr, nullptr);
    }

    void OnBeforeChildProcessLaunch(CefRefPtr<CefCommandLine> command_line) override {}

    IMPLEMENT_REFCOUNTING(SimpleApp);
};

int main(int argc, char* argv[]) {
    CefMainArgs main_args(argc, argv);
    CefSettings settings;
    settings.no_sandbox = true;
    settings.multi_threaded_message_loop = true;

    CefInitialize(main_args, settings, new SimpleApp(), nullptr);
    CefRunMessageLoop();
    CefShutdown();
    return 0;
}
