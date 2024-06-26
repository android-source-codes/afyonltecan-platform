/*
 * Copyright 2008, The Android Open Source Project
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define LOG_TAG "webcoreglue"

#include <config.h>
#include "WebFrameView.h"

//#include "android_graphics.h" //4.2 Merge Removedin 4.2
#include "GraphicsContext.h"
#include "Frame.h"
#include "FrameTree.h"
#include "FrameView.h"
#include "HostWindow.h"
#include "PlatformGraphicsContext.h"
#include "WebViewCore.h"

#include <SkCanvas.h>

namespace android {

WebFrameView::WebFrameView(WebCore::FrameView* frameView, WebViewCore* webViewCore)
    : WebCoreViewBridge()
    , mFrameView(frameView)
    , mWebViewCore(webViewCore) {
    // attach itself to mFrameView
    mFrameView->setPlatformWidget(this);
    Retain(mWebViewCore);
}

WebFrameView::~WebFrameView() {
//AndroidJB4.3 Merge  - START
//    Release(mWebViewCore); //Removed in 4.3 Merge
     ::Release(mWebViewCore);
//AndroidJB4.3 Merge  - END
}

void WebFrameView::draw(WebCore::GraphicsContext* gc, const WebCore::IntRect& rect) {
    WebCore::Frame* frame = mFrameView->frame();

    if (frame->contentRenderer())
        mFrameView->paintContents(gc, rect);
    else {
        // FIXME: I'm not entirely sure this ever happens or is needed
        gc->setFillColor(WebCore::Color::white, WebCore::ColorSpaceDeviceRGB);
        gc->fillRect(rect);
    }
}

}   // namespace android
