// =============================================================================
//
// Copyright (c) 2013 Christopher Baker <http://christopherbaker.net>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// =============================================================================


#include "ImageUtils.h"


namespace ofx {
namespace ESCPOS {

    
ofPixels_<unsigned char> ImageUtils::scaleAndCropTo(const ofPixels_<unsigned char>& pixels,
                                                    int width,
                                                    int height,
                                                    ofScaleMode scaleMode)
{
    ofRectangle inRect(0,0,pixels.getWidth(),pixels.getHeight());
    ofRectangle outRect(0,0,width,height);

    inRect.scaleTo(outRect,scaleMode);

    ofPixels_<unsigned char> inPixels = pixels;

    inPixels.resize(inRect.getWidth(),inRect.getHeight());

    ofPixels_<unsigned char> outPixels;

    inPixels.cropTo(outPixels,
                    outRect.x - inRect.x,
                    0,
                    outRect.width,
                    outRect.height);

    return outPixels;
}


//------------------------------------------------------------------------------
ofPixels_<unsigned char> ImageUtils::dither(const ofPixels_<unsigned char>& pixels,
                                            float threshold,
                                            float quantWeight)
{

    // Special thanks to @julapy / ofxDither

    ofPixels_<unsigned char> pixelsIn = pixels;

    // ensure the image is grayscale
    if(OF_IMAGE_GRAYSCALE != pixelsIn.getImageType())
    {
        pixelsIn = toGrayscale(pixels);
    }

    // make a copy
    ofPixels_<unsigned char> pixelsOut =  pixelsIn;

    // set up the quantization error
    int width  = pixelsOut.getWidth();
    int height = pixelsOut.getHeight();

    std::size_t numPixels = width * height; // 1 byte / pixel

    float qErrors[numPixels];
    std::fill(qErrors, qErrors + numPixels, 0.0);

    //unsigned char* inPix  = pixelsIn.getPixels();
    unsigned char* outPix = pixelsOut.getPixels();

    float limit = ofColor_<unsigned char>::limit();

    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            int p = pixelsIn.getPixelIndex(x, y);

            int oldPx = outPix[p] + qErrors[p]; // add error
            int newPx = (oldPx < (threshold * limit)) ? 0 : limit;  // threshold

            outPix[p] = newPx;

            int qError = oldPx - newPx;

            accumulateDitherError(x+1,y  ,pixelsOut,qError,qErrors,quantWeight); // check east
            accumulateDitherError(x+2,y  ,pixelsOut,qError,qErrors,quantWeight); // check east east
            accumulateDitherError(x-1,y+1,pixelsOut,qError,qErrors,quantWeight); // check southwest
            accumulateDitherError(x  ,y+1,pixelsOut,qError,qErrors,quantWeight); // check south
            accumulateDitherError(x+1,y+1,pixelsOut,qError,qErrors,quantWeight); // check southeast
            accumulateDitherError(x  ,y+2,pixelsOut,qError,qErrors,quantWeight); // check south south
        }
    }

    return pixelsOut;
}

////------------------------------------------------------------------------------
//BitImage ImageUtils::makeBitImage(const ofPixels& pixelsIn,
//                                  BitImage::ByteFormat format,
//                                  float threshold)
//{
//    if(pixelsIn.getImageType() != OF_IMAGE_GRAYSCALE) {
//        // warning ... must be grayscale
//    }
//
//    ByteBuffer buffer;
//
//    int width  = pixelsIn.getWidth();
//    int height = pixelsIn.getHeight();
//
//    const unsigned char* pixels = pixelsIn.getPixels();
//
//    unsigned char currentByte = 0;
//    unsigned char bitIndex = 0;
//
//    cout << "w/h=" << width << " / " << height << endl;
//
//    if(format == BitImage::BYTE_FORMAT_COLUMN) {
//        for(int x = 0; x < width; x++) {
//            for(int y = 0; y < height; y++) {
//                
//                int p = pixelsIn.getPixelIndex(x,y);
//
//                buffer.write(pixels[p] < threshold * 255 ? 0 : 1);
//
////                currentByte |= (pixels[p] < threshold * 255 ? 0 : 1) << bitIndex;
////                bitIndex++;
////
////                if(y + 1 < height && bitIndex >= 8) {
////                    buffer.write(currentByte);
////                    currentByte = 0;
////                    bitIndex = 0;
////                }
//            }
//
////            buffer.write(currentByte);
////            currentByte = 0;
////            bitIndex    = 0;
//        }
//    }
//
////    unsigned char numBytesHorizontal = width / 8;
////    unsigned char numBytesVertical   = height;
//
//
//    cout << pixelsIn.getWidth() << "/" << pixelsIn.getHeight() << " : " << (int)(width - 7)/8 << "/" << (int)height << " currentBytes=" << (int)buffer.size() << endl;
//
//    return BitImage(buffer,format,(width - 7)/8,height);
//
//}


ofPixels_<unsigned char> ImageUtils::toGrayscale(const ofPixels_<unsigned char>& pixels)
{
    if(OF_IMAGE_GRAYSCALE == pixels.getImageType()) return pixels;

    ofPixels pix;

    pix.allocate(pixels.getWidth(), pixels.getHeight(), OF_IMAGE_GRAYSCALE);

    for(std::size_t x = 0; x < pixels.getWidth(); ++x)
    {
        for(std::size_t y = 0; y < pixels.getHeight(); ++y)
        {
            ofColor_<unsigned char> c = pixels.getColor(x, y);
            pix.setColor(x, y, 0.21 * c.r + 0.71 * c.g + 0.07 * c.b);
        }
    }

    return pix;
    
}


} } // namespace ofx::ESCPOS
