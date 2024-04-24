// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_THUMBNAILS_THUMBNAIL_SERVICE_IMPL_H_
#define CHROME_BROWSER_THUMBNAILS_THUMBNAIL_SERVICE_IMPL_H_

#include "base/memory/ref_counted.h"
#include "chrome/browser/history/top_sites.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/thumbnails/thumbnail_service.h"

namespace base {
class RefCountedMemory;
}

namespace thumbnails {

// An implementation of ThumbnailService which delegates storage and most of
// logic to an underlying TopSites instances.
class ThumbnailServiceImpl : public ThumbnailService {
 public:
  explicit ThumbnailServiceImpl(Profile* profile);

  // Implementation of ThumbnailService.
  virtual bool SetPageThumbnail(const ThumbnailingContext& context,
                                const gfx::Image& thumbnail) OVERRIDE;

#if defined(S_NATIVE_SUPPORT)
  virtual bool SetPageThumbnailWithURL(const ThumbnailingContext& context,
                                       const GURL& gurl,
                                       const gfx::Image& thumbnail) OVERRIDE;
#endif

  virtual ThumbnailingAlgorithm* GetThumbnailingAlgorithm() const OVERRIDE;
  virtual bool GetPageThumbnail(
      const GURL& url,
      bool prefix_match,
      scoped_refptr<base::RefCountedMemory>* bytes) OVERRIDE;
  virtual void AddForcedURL(const GURL& url) OVERRIDE;
  virtual bool ShouldAcquirePageThumbnail(const GURL& url) OVERRIDE;

#if defined(S_NATIVE_SUPPORT)
  virtual bool ShouldAcquirePage(const GURL& url) OVERRIDE;
#endif

  // Implementation of RefcountedProfileKeyedService.
  virtual void ShutdownOnUIThread() OVERRIDE;

 private:
  virtual ~ThumbnailServiceImpl();

  scoped_refptr<history::TopSites> top_sites_;
  bool use_thumbnail_retargeting_;

  DISALLOW_COPY_AND_ASSIGN(ThumbnailServiceImpl);
};

}  // namespace thumbnails

#endif  // CHROME_BROWSER_THUMBNAILS_THUMBNAIL_SERVICE_IMPL_H_
