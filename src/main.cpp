#include <X11/Xlib.h>
#include <iostream>

#include "ClipboardProvider.h"

int
main()
{
    Display *display = XOpenDisplay(nullptr);
    if (display == nullptr) {
        std::cerr << "Cannot open display" << std::endl;
        return -1;
    }

    ClipboardProvider clipboardProvider(display);
    clipboardProvider.provideUtf8Text("Hello");
    return 0;
}
