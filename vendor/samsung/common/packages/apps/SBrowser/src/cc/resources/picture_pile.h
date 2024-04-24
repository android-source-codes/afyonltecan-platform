// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_RESOURCES_PICTURE_PILE_H_
#define CC_RESOURCES_PICTURE_PILE_H_

#include "cc/resources/picture_pile_base.h"
#include "ui/gfx/rect.h"

namespace cc {
class PicturePileImpl;
class Region;
class RenderingStatsInstrumentation;

class CC_EXPORT PicturePile : public PicturePileBase {
 public:
  PicturePile();

  // Re-record parts of the picture that are invalid.
  // Invalidations are in layer space.
  // Return true iff the pile was modified.
  bool Update(
      ContentLayerClient* painter,
      SkColor background_color,
      bool contents_opaque,
      const Region& invalidation,
      const gfx::Rect& visible_layer_rect,
      int frame_number,
      RenderingStatsInstrumentation* stats_instrumentation);

  void set_slow_down_raster_scale_factor(int factor) {
    slow_down_raster_scale_factor_for_debug_ = factor;
  }

  void set_show_debug_picture_borders(bool show) {
    show_debug_picture_borders_ = show;
  }
#if defined(SBROWSER_GPU_RASTERIZATION_ENABLE)
  bool is_suitable_for_gpu_rasterization() const {
    return is_suitable_for_gpu_rasterization_;
  }
  void SetUnsuitableForGpuRasterizationForTesting() {
    is_suitable_for_gpu_rasterization_ = false;
  }
#endif
 protected:
  virtual ~PicturePile();

 private:
  friend class PicturePileImpl;
#if defined(SBROWSER_GPU_RASTERIZATION_ENABLE)
  bool is_suitable_for_gpu_rasterization_;
#endif
  DISALLOW_COPY_AND_ASSIGN(PicturePile);
};

}  // namespace cc

#endif  // CC_RESOURCES_PICTURE_PILE_H_