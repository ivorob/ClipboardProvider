#include <iostream>
#include <X11/Xatom.h>
#include "ClipboardProvider.h"

namespace {

const std::vector<std::string> utf8Targets = {
    "UTF8_STRING",
    "text/plain;charset=utf-8"
};

}

ClipboardProvider::ClipboardProvider(Display *display)
    : display(display),
      clipboard(),
      defaultRequestHandler()
{
    initClipboardWindow();
    initTargets();
}

void
ClipboardProvider::initClipboardWindow()
{
    int screen = DefaultScreen(this->display);
    clipboard = XCreateSimpleWindow(this->display, RootWindow(this->display, screen),
                    0, 0, 1, 1, 0,
                    BlackPixel(this->display, screen),
                    WhitePixel(this->display, screen));
}

void
ClipboardProvider::initTargets()
{
    targetHandlers[XInternAtom(this->display, "TARGETS", False)] =
        [](ClipboardProvider *provider, XSelectionRequestEvent *event, const std::string&) {
            provider->sendTargets(event);
        };

    this->defaultRequestHandler = [](ClipboardProvider *provider, XSelectionRequestEvent *event, const std::string&) {
        provider->sendEmptyResponse(event);
    };
}

void
ClipboardProvider::sendEmptyResponse(XSelectionRequestEvent *eventRequest)
{
    XSelectionEvent selectionEvent = {0};
    selectionEvent.type = SelectionNotify;
    selectionEvent.requestor = eventRequest->requestor;
    selectionEvent.selection = eventRequest->selection;
    selectionEvent.target = eventRequest->target;
    selectionEvent.property = None;
    selectionEvent.time = eventRequest->time;

    XSendEvent(this->display, eventRequest->requestor, True, NoEventMask,
            reinterpret_cast<XEvent *>(&selectionEvent));
}

void
ClipboardProvider::sendTargets(XSelectionRequestEvent *requestEvent)
{
    XChangeProperty(this->display, requestEvent->requestor, requestEvent->property, XA_ATOM, 32, PropModeReplace,
            reinterpret_cast<const unsigned char *>(&this->mimeTypes[0]), this->mimeTypes.size() * sizeof(Atom));

    XSelectionEvent selectionEvent = {0};
    selectionEvent.type = SelectionNotify;
    selectionEvent.requestor = requestEvent->requestor;
    selectionEvent.selection = requestEvent->selection;
    selectionEvent.target = requestEvent->target;
    selectionEvent.property = requestEvent->property;
    selectionEvent.time = requestEvent->time;

    XSendEvent(display, requestEvent->requestor, True, NoEventMask, reinterpret_cast<XEvent *>(&selectionEvent));
}

void
ClipboardProvider::sendUtf8(XSelectionRequestEvent *requestEvent, const std::string& utf8String)
{
    XChangeProperty(display, requestEvent->requestor, requestEvent->property, requestEvent->target, 8, PropModeReplace,
            reinterpret_cast<const unsigned char *>(utf8String.c_str()), utf8String.size());

    XSelectionEvent selectionEvent = {0};
    selectionEvent.type = SelectionNotify;
    selectionEvent.requestor = requestEvent->requestor;
    selectionEvent.selection = requestEvent->selection;
    selectionEvent.target = requestEvent->target;
    selectionEvent.property = requestEvent->property;
    selectionEvent.time = requestEvent->time;

    XSendEvent(display, requestEvent->requestor, True, NoEventMask, reinterpret_cast<XEvent *>(&selectionEvent));
}

void
ClipboardProvider::provideUtf8Text(const std::string& utf8String)
{
    initUtf8Targets();

    Atom clipboardSelection = XInternAtom(this->display, "CLIPBOARD", False);
    XSetSelectionOwner(this->display, clipboardSelection, this->clipboard, CurrentTime);

    bool done = false;
    while (!done) {
        XEvent event = {0};
        XNextEvent(this->display, &event);

        switch (event.type) {
            case SelectionClear:
                done = true;
                break;
            case SelectionRequest:
                handleSelectionRequest(&event.xselectionrequest, utf8String);
                break;
        }
    }

    clearUtf8Targets();
}

void
ClipboardProvider::initUtf8Targets()
{
    for (const auto& utf8Target : utf8Targets) {
        auto targetAtom = XInternAtom(this->display, utf8Target.c_str(), False);
        this->mimeTypes.push_back(targetAtom);
        this->targetHandlers[targetAtom] =
            [](ClipboardProvider *provider, XSelectionRequestEvent *event, const std::string& utf8String) {
                provider->sendUtf8(event, utf8String);
            };
    }
}

void
ClipboardProvider::clearUtf8Targets()
{
    this->mimeTypes.clear();

    for (const auto& utf8Target : utf8Targets) {
        auto targetAtom = XInternAtom(this->display, utf8Target.c_str(), False);
        this->targetHandlers.erase(targetAtom);
    }
}

void
ClipboardProvider::handleSelectionRequest(XSelectionRequestEvent *eventRequest, const std::string& utf8String)
{
    auto targetHandler = getTargetHandler(eventRequest->target);
    targetHandler(this, eventRequest, utf8String);
}

ClipboardProvider::SelectionRequestHandler
ClipboardProvider::getTargetHandler(Atom target) const
{
    auto it = this->targetHandlers.find(target);
    if (it != this->targetHandlers.end()) {
        return it->second;
    }

    return this->defaultRequestHandler;
}
