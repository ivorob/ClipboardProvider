#pragma once

#include <X11/Xlib.h>
#include <string>
#include <map>
#include <vector>

class ClipboardProvider {
public:
    ClipboardProvider(Display *display);

    void provideUtf8Text(const std::string& utf8String);
private:
    using SelectionRequestHandler = void(*)(ClipboardProvider *provider, XSelectionRequestEvent *, const std::string&);
private:
    void initClipboardWindow();
    void initTargets();

    void initUtf8Targets();
    void clearUtf8Targets();

    void handleSelectionRequest(XSelectionRequestEvent *eventRequest, const std::string& utf8String);
    SelectionRequestHandler getTargetHandler(Atom target) const;
private:
    // Handlers
    void sendEmptyResponse(XSelectionRequestEvent *eventRequest);
    void sendTargets(XSelectionRequestEvent *eventRequest);
    void sendUtf8(XSelectionRequestEvent *eventRequest, const std::string& utf8String);
private:
    Display *display;
    Window clipboard;
    SelectionRequestHandler defaultRequestHandler;
    std::map<Atom, SelectionRequestHandler> targetHandlers;
    std::vector<Atom> mimeTypes;
};
