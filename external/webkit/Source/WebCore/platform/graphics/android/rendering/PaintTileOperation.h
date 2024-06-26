/*
 * Copyright 2010, The Android Open Source Project
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

#ifndef PaintTileSetOperation_h
#define PaintTileSetOperation_h

#include "Tile.h"
#include "QueuedOperation.h"
#include "SkRefCnt.h"

namespace WebCore {

class LayerAndroid;
class TilePainter;
class ImageTexture;

class PaintTileOperation : public QueuedOperation {
public:
    PaintTileOperation(Tile* tile, TilePainter* painter,
                       GLWebViewState* state, bool isLowResPrefetch);
    virtual ~PaintTileOperation();
    virtual bool operator==(const QueuedOperation* operation);
    virtual void run(BaseRenderer* renderer);//4.2 Merge : Passing renderer
    virtual void* uniquePtr() { return m_tile; }
    // returns a rendering priority for m_tile, lower values are processed faster
    virtual int priority();

    TilePainter* painter() { return m_painter; }
    void updatePainter(TilePainter* painter);
    float scale() { return m_tile->scale(); }

private:
    Tile* m_tile;
    TilePainter* m_painter;
    GLWebViewState* m_state;
    bool m_isLowResPrefetch;
};

class ScaleFilter : public OperationFilter {
public:
    ScaleFilter(const TilePainter* painter, float scale)
        : m_painter(painter)
        , m_scale(scale) {}
    virtual bool check(QueuedOperation* operation)
    {
        PaintTileOperation* op = static_cast<PaintTileOperation*>(operation);
        return ((op->painter() == m_painter) && (op->scale() != m_scale));
    }
private:
    const TilePainter* m_painter;
    float m_scale;
};


class TilePainterFilter : public OperationFilter {
public:
    TilePainterFilter(TilePainter* painter) : m_painter(painter) {}
    virtual bool check(QueuedOperation* operation)
    {
        PaintTileOperation* op = static_cast<PaintTileOperation*>(operation);
        return op->painter() == m_painter;
    }
private:
    TilePainter* m_painter;
};

}

#endif // PaintTileSetOperation_h
