/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkImageDecoder.h"
#include "SkBitmap.h"
#include "SkImagePriv.h"
#include "SkPixelRef.h"
#include "SkStream.h"
#include "SkTemplates.h"
#include "SkCanvas.h"
#include "SkImageInfo.h"
#include <stdio.h>
#include <cutils/properties.h>
#include <cutils/log.h>

#define LOG_TAG "skia"

#define ATRACE_TAG ATRACE_TAG_GRAPHICS
#include <utils/Trace.h>

SkImageDecoder::SkImageDecoder()
    : fPeeker(NULL)
    , fAllocator(NULL)
    , fSampleSize(1)
    , fDefaultPref(kUnknown_SkColorType)
    , fPreserveSrcDepth(false)
    , fDitherImage(true)
    , fSkipWritingZeroes(false)
    , fPreferQualityOverSpeed(false)
    , fRequireUnpremultipliedColors(false) 
    , fIsAllowMultiThreadRegionDecode(0) 
    , fPreferSize(0)
    , fPostProc(0) 
    , fdc(NULL)
    , fISOSpeedRatings(0) {
}

SkImageDecoder::~SkImageDecoder() {
    SkSafeUnref(fPeeker);
    SkSafeUnref(fAllocator);
}

void SkImageDecoder::copyFieldsToOther(SkImageDecoder* other) {
    if (NULL == other) {
        return;
    }
    other->setPeeker(fPeeker);
    other->setAllocator(fAllocator);
    other->setSampleSize(fSampleSize);
    other->setPreserveSrcDepth(fPreserveSrcDepth);
    other->setDitherImage(fDitherImage);
    other->setSkipWritingZeroes(fSkipWritingZeroes);
    other->setPreferQualityOverSpeed(fPreferQualityOverSpeed);
    other->setRequireUnpremultipliedColors(fRequireUnpremultipliedColors);
}

SkImageDecoder::Format SkImageDecoder::getFormat() const {
    return kUnknown_Format;
}

const char* SkImageDecoder::getFormatName() const {
    return GetFormatName(this->getFormat());
}

const char* SkImageDecoder::GetFormatName(Format format) {
    switch (format) {
        case kUnknown_Format:
            return "Unknown Format";
        case kBMP_Format:
            return "BMP";
        case kGIF_Format:
            return "GIF";
        case kICO_Format:
            return "ICO";
        case kPKM_Format:
            return "PKM";
        case kKTX_Format:
            return "KTX";
        case kASTC_Format:
            return "ASTC";
        case kJPEG_Format:
            return "JPEG";
        case kPNG_Format:
            return "PNG";
        case kWBMP_Format:
            return "WBMP";
        case kWEBP_Format:
            return "WEBP";
        default:
            SkDEBUGFAIL("Invalid format type!");
    }
    return "Unknown Format";
}

SkImageDecoder::Peeker* SkImageDecoder::setPeeker(Peeker* peeker) {
    SkRefCnt_SafeAssign(fPeeker, peeker);
    return peeker;
}

SkBitmap::Allocator* SkImageDecoder::setAllocator(SkBitmap::Allocator* alloc) {
    SkRefCnt_SafeAssign(fAllocator, alloc);
    return alloc;
}

void SkImageDecoder::setSampleSize(int size) {
    if (size < 1) {
        size = 1;
    }
    fSampleSize = size;
}

bool SkImageDecoder::allocPixelRef(SkBitmap* bitmap,
                                   SkColorTable* ctable) const {
    return bitmap->tryAllocPixels(fAllocator, ctable);
}

///////////////////////////////////////////////////////////////////////////////

SkColorType SkImageDecoder::getPrefColorType(SrcDepth srcDepth, bool srcHasAlpha) const {
    SkColorType ct = fDefaultPref;
    if (fPreserveSrcDepth) {
        switch (srcDepth) {
            case kIndex_SrcDepth:
                ct = kIndex_8_SkColorType;
                break;
            case k8BitGray_SrcDepth:
                ct = kN32_SkColorType;
                break;
            case k32Bit_SrcDepth:
                ct = kN32_SkColorType;
                break;
        }
    }
    return ct;
}


SkImageDecoder::Result SkImageDecoder::decode(SkStream* stream, SkBitmap* bm, SkColorType pref,
                                              Mode mode) {
	// we reset this to false before calling onDecode
    fShouldCancelDecode = false;
    // assign this, for use by getPrefColorType(), in case fUsePrefTable is false
    fDefaultPref = pref;

    // pass a temporary bitmap, so that if we return false, we are assured of
    // leaving the caller's bitmap untouched.
    SkBitmap    tmp;
	if (this->getFormat() != kPNG_Format) {
		MtkSkDebugf("onDecode start stream=%p,bm=%p,pref=%d,mode=%d,format=%s\n",
				stream, bm, pref, mode, this->getFormatName());
	}
    const Result result = this->onDecode(stream, &tmp, mode);	
    if (kFailure == result) {
        return kFailure;
    }
	if (this->getFormat() != kPNG_Format) {
		MtkSkDebugf("onDecode return true,format=%s\n",this->getFormatName());
	}		
    bm->swap(tmp);
    return result;
}

bool SkImageDecoder::decodeSubset(SkBitmap* bm, const SkIRect& rect, SkColorType pref) {
    // we reset this to false before calling onDecodeSubset
    fShouldCancelDecode = false;
    // assign this, for use by getPrefColorType(), in case fUsePrefTable is false
    fDefaultPref = pref;
    MtkSkDebugf("onDecodeSubset,bm=%p,pref=%d,format=%s\n", bm, pref,this->getFormatName());
    if (! this->onDecodeSubset(bm, rect)) {
        return false;
    }
    MtkSkDebugf("decodeSubset %s End,return true",this->getFormatName());
    return true;
}

bool SkImageDecoder::buildTileIndex(SkStreamRewindable* stream, int *width, int *height) {
    // we reset this to false before calling onBuildTileIndex
    fShouldCancelDecode = false;

    return this->onBuildTileIndex(stream, width, height);
}

bool SkImageDecoder::onBuildTileIndex(SkStreamRewindable* stream, int* /*width*/,
                                      int* /*height*/) {
    SkDELETE(stream);
    return false;
}


bool SkImageDecoder::cropBitmap(SkBitmap *dst, SkBitmap *src, int sampleSize,
                                int dstX, int dstY, int width, int height,
                                int srcX, int srcY) {
    int w = width / sampleSize;
    int h = height / sampleSize;

    // if the destination has no pixels then we must allocate them.
    if(sampleSize > 1 && width > 0 && w == 0) {
		ALOGW("Skia::cropBitmap W/H %d %d->%d %d, Sample %d, force width != 0 !!!!!!\n", width, height,w, h, sampleSize );
		w = 1;
    }
    if(sampleSize > 1 && height > 0 && h == 0) {
		ALOGW("Skia::cropBitmap W/H %d %d->%d %d, Sample %d, force height != 0 !!!!!!\n", width, height,w, h, sampleSize );
		h = 1;
    }	

    if (src->colorType() == kIndex_8_SkColorType) {
        // kIndex8 does not allow drawing via an SkCanvas, as is done below.
        // Instead, use extractSubset. Note that this shares the SkPixelRef and
        // SkColorTable.
        // FIXME: Since src is discarded in practice, this holds on to more
        // pixels than is strictly necessary. Switch to a copy if memory
        // savings are more important than speed here. This also means
        // that the pixels in dst can not be reused (though there is no
        // allocation, which was already done on src).
        int x = (dstX - srcX) / sampleSize;
        int y = (dstY - srcY) / sampleSize;
        SkIRect subset = SkIRect::MakeXYWH(x, y, w, h);
        return src->extractSubset(dst, subset);
    }
    // if the destination has no pixels then we must allocate them.
    if (dst->isNull()) {
        dst->setInfo(src->info().makeWH(w, h));

        if (!this->allocPixelRef(dst, NULL)) {
            SkDEBUGF(("failed to allocate pixels needed to crop the bitmap"));
            return false;
        }
    }
    // check to see if the destination is large enough to decode the desired
    // region. If this assert fails we will just draw as much of the source
    // into the destination that we can.
    if (dst->width() < w || dst->height() < h) {
        SkDEBUGF(("SkImageDecoder::cropBitmap does not have a large enough bitmap.\n"));
    }

    // Set the Src_Mode for the paint to prevent transparency issue in the
    // dest in the event that the dest was being re-used.
    SkPaint paint;
    paint.setXfermodeMode(SkXfermode::kSrc_Mode);

    SkCanvas canvas(*dst);
    canvas.drawSprite(*src, (srcX - dstX) / sampleSize,
                            (srcY - dstY) / sampleSize,
                            &paint);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool SkImageDecoder::DecodeFile(const char file[], SkBitmap* bm, SkColorType pref,  Mode mode,
                                Format* format) {
    SkASSERT(file);
    SkASSERT(bm);

    SkAutoTDelete<SkStreamRewindable> stream(SkStream::NewFromFile(file));
    if (stream.get()) {
        if (SkImageDecoder::DecodeStream(stream, bm, pref, mode, format)) {
            if (SkPixelRef* pr = bm->pixelRef()) {
                pr->setURI(file);
            }
            return true;
        }
    }
    return false;
}

bool SkImageDecoder::DecodeMemory(const void* buffer, size_t size, SkBitmap* bm, SkColorType pref,
                                  Mode mode, Format* format) {
    if (0 == size) {
        return false;
    }
    SkASSERT(buffer);

    SkMemoryStream  stream(buffer, size);
    return SkImageDecoder::DecodeStream(&stream, bm, pref, mode, format);
}

bool SkImageDecoder::DecodeStream(SkStreamRewindable* stream, SkBitmap* bm, SkColorType pref,
                                  Mode mode, Format* format) {
    SkASSERT(stream);
    SkASSERT(bm);

    bool success = false;
    SkImageDecoder* codec = SkImageDecoder::Factory(stream);

    if (codec) {
        success = codec->decode(stream, bm, pref, mode) != kFailure;
        if (success && format) {
            *format = codec->getFormat();
            if (kUnknown_Format == *format) {
                if (stream->rewind()) {
                    *format = GetStreamFormat(stream);
                }
            }
        }
        delete codec;
    }
    return success;
}

bool SkImageDecoder::decodeYUV8Planes(SkStream* stream, SkISize componentSizes[3], void* planes[3],
                                      size_t rowBytes[3], SkYUVColorSpace* colorSpace) {
    // we reset this to false before calling onDecodeYUV8Planes
    fShouldCancelDecode = false;

    return this->onDecodeYUV8Planes(stream, componentSizes, planes, rowBytes, colorSpace);
}

#ifdef MTK_JPEG_ImageDecoder
void SkImageDecoder::setPreferSize(int size) {
    if (size < 0) {
        size = 0;
    }
    fPreferSize = size;
}

void SkImageDecoder::setPostProcFlag(int flag) {
    fPostProc = flag;
}
#endif

#ifdef MTK_IMAGE_DC_SUPPORT 
void SkImageDecoder::setDynamicCon(void* pointer, int size) {
    fdc = pointer;
	fsize = size;
	//XLOGD("setDynamicCon fsize=%d", fsize);
}
#endif

#ifdef MTK_SKIA_MULTI_THREAD_JPEG_REGION
bool SkImageDecoder::decodeSubset(SkBitmap* bm, const SkIRect& rect, SkColorType pref, 
                                  int sampleSize, void* fdc) {
    // we reset this to false before calling onDecodeSubset
    fShouldCancelDecode = false;
    // assign this, for use by getPrefColorType(), in case fUsePrefTable is false
    fDefaultPref = pref;
    MtkSkDebugf("multi onDecodeSubset,bm=%p,pref=%d,format=%s\n", bm, pref,this->getFormatName());
    
#ifdef MTK_IMAGE_DC_SUPPORT
    if (! this->onDecodeSubset(bm, rect, sampleSize, fdc)) {
        return false;
    }
#else
    if (! this->onDecodeSubset(bm, rect, sampleSize, NULL)) {
        return false;
    }
#endif
    MtkSkDebugf("multi decodeSubset %s End,return true",this->getFormatName());
    return true;
}
#endif  //MTK_SKIA_MULTI_THREAD_JPEG_REGION
