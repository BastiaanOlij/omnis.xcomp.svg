#define NANOSVG_IMPLEMENTATION
#define NANOSVGRAST_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "oSVG.h"

// Generic is a framework you can use to develop OMNIS external components.
// It does not support any extra calls. See other generic samples for extra support,
// such as properties, functions, events etc. 
//
// When this generic object is added to a window in window design mode, it will have
// only standard properties that apply to all window objects. e.g. left,top etc.

// our properties
ECOproperty oSVGProperties[] = {
//  propid      resourceid,     datatype,       propflags                                   propFlags2, enumStart,  enumEnd
    svgdata,    4000,           fftCharacter,   EXTD_FLAG_PROPDATA|EXTD_FLAG_PWINDMLINE,    0,          0,          0,
    svgdpi,     4002,           fftNumber,      EXTD_FLAG_PROPDATA,                         0,          0,          0,
    svgpng,     4001,           fftBinary,      EXTD_FLAG_PROPDATA|EXTD_FLAG_RUNTIMEONLY,   0,          0,          0,
    svgwidth,   4003,           fftInteger,     EXTD_FLAG_PROPDATA|EXTD_FLAG_RUNTIMEONLY,   0,          0,          0,
    svgheight,  4004,           fftInteger,     EXTD_FLAG_PROPDATA|EXTD_FLAG_RUNTIMEONLY,   0,          0,          0,
};

// Your generic object constructor
oSVG::oSVG( HWND pFieldHWnd ) {
    mHWnd = pFieldHWnd;	// we remember the objects child window for later drawing
    mDPI = 96.0;
    mSVGString = NULL;
    mSVGData = NULL;
    mImageWidth = 0;
    mImageHeight = 0;
    mImageData = NULL;
    mPixMap = 0;
};

// Generic destruction
oSVG::~oSVG() {
    // Insert any memory deletion code or general cleanup code
    if (mSVGString != NULL) {
        free(mSVGString);
        mSVGString = NULL;
    };
    freeSVGData();
    freeImageData();
};

// release our CVG data
void  oSVG::freeSVGData() {
    if (mSVGData != NULL) {
        nsvgDelete(mSVGData);
        mSVGData = NULL;
    };
};

// release our image data
void oSVG::freeImageData() {
    mImageWidth = 0;
    mImageHeight = 0;
    if (mImageData != NULL) {
        free(mImageData);
        mImageData = NULL;
    };
    if (mPixMap!=0) {
        GDIdeleteHPIXMAP(mPixMap);
        mPixMap = 0;
    };
};

void  oSVG::updateImage() {
    if (mSVGString != NULL) {
        if (mSVGData == NULL) {
            // convert our string to UTF8 data
            qlong len = OMstrlen(mSVGString);
            char * svgutf8 = (char *)malloc(sizeof(char) * UTF8_MAX_BYTES_PER_CHAR * (len+1)); // sizeof(char) is a bit overdone, it should return 1
            if (svgutf8 != NULL) {
                CHRunicode::charToUtf8(mSVGString, len, (qbyte *)svgutf8);
                
                // Rebuild our CVG data, note, the scaling of nanosvg is based on the output, so we're converting from points to inches.
                // the result is the image scaling 'down" with a higher DPI
                // we want to render the image at a higher DPI so we're assuming our loaded image is at 72 dpi and we're scaling it to our set DPI
                // It's a bit of a hack but it works...
                mSVGData = nsvgParse(svgutf8, "in", 72.0 / mDPI);
            };
        };
        
        if (mSVGData != NULL) {
            // for now take our width and height as is scaled up by our DPI setting
            int width = mSVGData->width * mDPI / 72.0;
            int height = mSVGData->height * mDPI / 72.0;
            
            if ((mImageWidth != width) || (mImageHeight != height)) {
                freeImageData();
            };

            if (mImageData == NULL) {
                NSVGrasterizer * rast = nsvgCreateRasterizer();
                if (rast != NULL) {
                    mImageData = (unsigned char *) malloc(width*height*4);
                    if (mImageData != NULL) {
                        nsvgRasterize(rast, mSVGData, 0, 0, 1, mImageData, width, height, width*4);
                    
                        // remember our size
                        mImageWidth = width;
                        mImageHeight = height;
                    };
                    
                    // we can cleanup this...
                    nsvgDeleteRasterizer(rast);
                };
            };
        };
    };
};

HPIXMAP oSVG::getPixmap() {
    if ((mImageData == NULL) || (mImageWidth == 0) || (mImageHeight ==0 )) {
        return 0;
    } else if (mPixMap == 0) {
        mPixMap = GDIcreateHPIXMAP(mImageWidth, mImageHeight, 32, true);
        unsigned char * imgdata = (unsigned char *) GDIlockHPIXMAP(mPixMap);
        if (imgdata != NULL) {
            // RGBA => ARGB...
            int datasize = mImageWidth*mImageHeight*4;
            for (int i = 0; i < datasize; i+=4) {
                imgdata[i+0] = mImageData[i+3] >= 128 ? 255 : 0; // alpha
                imgdata[i+1] = mImageData[i+0]; // red
                imgdata[i+2] = mImageData[i+1]; // green
                imgdata[i+3] = mImageData[i+2]; // blue
            };
            // memcpy(imgdata, mImageData, datasize);
            GDIunlockHPIXMAP(mPixMap);
        };
    };
    return mPixMap;
};

// returns our image as a PNG (calling method is responsible for freeing up the memory using free)
qbyte * oSVG::asPng(int &pLen) {
    if ((mImageData == NULL) || (mImageWidth == 0) || (mImageHeight ==0 )) {
        pLen = 0;
        return NULL;
    } else {;
        int len;
        unsigned char *png = stbi_write_png_to_mem((unsigned char *) mImageData, 0, mImageWidth, mImageHeight, 4, &len);
        if (png == NULL) {
            pLen = 0;
        } else {
            pLen = len;
        };
        return png;
    };
};

// can we assign this property?
qbool oSVG::propCanAssign(qlong pPropId) {
    switch( pPropId ) {
        case svgdata: {
            return 1L;
        }; break;
        case svgdpi: {
            return 1L;
        }; break;
        default:
            // huh?
            break;
    };
    return 0L;
};

// set a property value
qbool oSVG::setProperty(qlong pPropId, EXTfldval * pSetVal) {
    switch( pPropId ) {
        case svgdata: {
            qlong len = pSetVal->getCharLen();
            qlong reallen = 0;
            
            if (len > 0) {
                mSVGString = (qchar *) realloc(mSVGString, sizeof(qchar) * (len+1));
                if (mSVGString != NULL) {
                    pSetVal->getChar(len+1, mSVGString, reallen);
                    mSVGString[reallen] = 0; // zero terminate it!
                } else {
                    // couldn't allocate memory... maybe error somehow?
                };
            } else if (mSVGString != NULL) {
                // just free our string...
                free(mSVGString);
                mSVGString = NULL;
            };
            
            // need to update our image
            freeSVGData();
            freeImageData();
            updateImage();
            
            WNDinvalidateRect( mHWnd, NULL );
            return 1L;
        }; break;
        case svgdpi: {
            qshort subtype = dpDefault; /* only has not properly declared our getNum parameter as constant so just to get our compiler to be happy */
            pSetVal->getNum(mDPI, subtype);
            if (mDPI < 10) {
                mDPI = 10;
            } else if (mDPI > 2400) {
                mDPI = 2400;
            };
            
            // need to update our image
            freeSVGData();
            freeImageData();
            updateImage();
            
            WNDinvalidateRect( mHWnd, NULL );
            return 1L;
        }; break;
        case svgpng:
            // read only
            break;
        case svgwidth:
            // read only
            break;
        case svgheight:
            // read only
            break;
        default:
            // huh?
            break;
    };
    return 0L;
};

// get a property value
qbool oSVG::getProperty(qlong pPropId, EXTfldval * pRetVal) {
    switch( pPropId) {
        case svgdata: {
            if (mSVGString != NULL) {
                pRetVal->setChar(mSVGString, OMstrlen(mSVGString));
            } else {
                // return an empty string
                str255 empty(QTEXT(""));
                pRetVal->setChar(empty);
            };
            
            return 1L;
        }; break;
        case svgdpi: {
            pRetVal->setNum(mDPI);
            return 1L;
        }; break;
        case svgpng: {
            qlong   len = 0;
            qbyte * png = asPng(len);
            
            if (png != NULL) {
                pRetVal->setBinary(fftBinary, png, len);
                free(png);
            } else {
                pRetVal->setEmpty(fftBinary, 0);
            };
            return 1L;
        }; break;
        case svgwidth: {
            pRetVal->setLong(mImageWidth);
            return 1L;
        }; break;
        case svgheight:
            pRetVal->setLong(mImageHeight);
            return 1L;
            break;
        default:
            // huh?
            break;
    };
    
    return 0L;
};

// You need to paint your control
qbool oSVG::paint() {
    WNDpaintStruct  paintStruct;
	qrect           clientRect;
    qrect           updateRect;
    HDC             hdc;
    void *          offScreenPaint;
    
    WNDbeginPaint( mHWnd, &paintStruct );
	
	// get the client rect for our component
	WNDgetClientRect( mHWnd, &clientRect );
    
    // build a backscreen buffer to prevent flickering...
    updateRect = paintStruct.rcPaint;
    hdc = paintStruct.hdc;
	offScreenPaint = GDIoffscreenPaintBegin(NULL, hdc, clientRect, updateRect);
	if (offScreenPaint) {
    
        // lets clear our background.
        HBRUSH brush = GDIgetStockBrush	( BLACK_BRUSH );
        GDIsetTextColor( hdc, GDI_COLOR_QGRAY );
        GDIfillRect( hdc, &clientRect, brush );
    
        // draw our image
        // omnis does not have a way to show an RGBA image directly, have to convert it to a PIXMAP first
        // so weird... :(
        HPIXMAP pixmap = getPixmap();
        if (pixmap != 0) {
            qrect sourcerect;
        
            sourcerect.left = 0;
            sourcerect.top = 0;
            sourcerect.right = mImageWidth-1;
            sourcerect.bottom = mImageHeight-1;
        
            // and draw it, for some reason our alpha is being ignored here
            GDIdrawHPIXMAP(hdc, pixmap, &sourcerect, &clientRect, qfalse);
        };
    
        // If in design mode, then call drawNumber & drawMultiKnobs to draw design
        // numbers and multiknobs, if required.
        if ( ECOisDesign(mHWnd) ) {
            ECOdrawDesignName(mHWnd,hdc);
            ECOdrawNumber(mHWnd,hdc);
            ECOdrawMultiKnobs(mHWnd,hdc);
        };
    
        GDIoffscreenPaintEnd(offScreenPaint);
	};
    
	WNDendPaint( mHWnd, &paintStruct );	
	return qtrue;
};

// Component library entry point (name as declared in resource 31000 )
extern "C" LRESULT OMNISWNDPROC oSVGWndProc(HWND hwnd, UINT Msg,WPARAM wParam,LPARAM lParam,EXTCompInfo* eci) {
    // Initialize callback tables - THIS MUST BE DONE
    ECOsetupCallbacks(hwnd, eci);
    switch (Msg) {
        // WM_PAINT - standard paint message
        case WM_PAINT: {
            // First find the object in the libraries chain of objects
            oSVG* object = (oSVG*)ECOfindObject( eci, hwnd );
            // and if its good, call the paint function
            if ( NULL!=object && object->paint() ) {
                return qtrue;
            };
        }; break;
			
        // ECM_OBJCONSTRUCT - this is a message to create a new object.
        case ECM_OBJCONSTRUCT: {
            // Allocate a new object
            oSVG* object = new oSVG( hwnd );
            // and insert into a chain of objects. The OMNIS library will maintain this chain
            ECOinsertObject( eci, hwnd, (void*)object );
            return qtrue;
        }; break;
			
        // ECM_OBJDESTRUCT - this is a message to inform you to delete the object
        case ECM_OBJDESTRUCT: {
            // First find the object in the libraries chain of objects,
            // this call if ok also removes the object from the chain.
            oSVG* object = (oSVG*)ECOremoveObject( eci, hwnd );
            if ( NULL!=object ) {
                // Now you can delete the object you previous allocated
                // Note: The hwnd passed on ECM_OBJCONSTRUCT should not be deleted, as
                // it was created and will be destroyed by OMNIS
                delete object;
            }
            return qtrue;
        }; break;
	 		
        // ECM_CONNECT - this message is sent once per OMNIS session and should not be confused
        // with ECM_OBJCONSTRUCT which is sent once per object. ECM_CONNECT can be used if load other libraries
        // once or other general global actions that need to be done only once.
        //
        // For most components this can be removed - see other BLYTH component examples
        case ECM_CONNECT: {
            return EXT_FLAG_LOADED; // Return external flags
        }; break;
      
        // ECM_DISCONNECT - this message is sent only once when the OMNIS session is ending and should not be confused
        // with ECM_OBJDESTRUCT which is sent once per object. ECM_DISCONNECT can be used to free other libraries
        // loaded on ECM_CONNECT or other general global actions that need to be done only once.
        //
        // For most components this can be removed - see other BLYTH component examples
        case ECM_DISCONNECT: {
            return qtrue;
        }; break;

        // ECM_GETCOMPLIBINFO - this is sent by OMNIS to find out the name of the library, and
        // the number of components this library supports
        case ECM_GETCOMPLIBINFO: {
            return ECOreturnCompInfo( gInstLib, eci, LIB_RES_NAME, OBJECT_COUNT );
        }; break;

        // ECM_GETCOMPICON - this is sent by OMNIS to get an icon for the OMNIS component store and
        // external component browser. You need to always supply an icon in your resource file.
        case ECM_GETCOMPICON: {
            // OMNIS will call you once per component for an icon.
            // GENERIC_ICON is defined in the header and included in the resource file
            if ( eci->mCompId==OBJECT_ID1 ) {
                return ECOreturnIcon( gInstLib, eci, GENERIC_ICON );
            };
            return qfalse;
        }; break;

        // ECM_GETCOMPID - this message is sent by OMNIS to get information about each component in this library
        // wParam is a number from 1 to the number of components return on the ECM_GETCOMPLIBINFO message.
        //
        // For each call you should return the internal object ID and the type of object it is.
        // The object id will be used for other calls such as ECM_GETCOMPICON
        //
        // The object type is for example : cObjType_Basic 		- a basic window object or
        //																	cRepObjType_Basic	- a basic report object.
        // 																	There are others 	- see BLYTH examples and headers
        case ECM_GETCOMPID: {
            if ( wParam==1 )
                return ECOreturnCompID( gInstLib, eci, OBJECT_ID1, cObjType_Basic );
            return 0L;
        }; break;
        
        case WM_WINDOWPOSCHANGED: {
            // always invalidate
            WNDinvalidateRect( hwnd, NULL );
        }; break;
        
        ////////////////////////////////////////////////////////////////////////
        // stuff for properties
        
        case ECM_GETPROPNAME: {
            return ECOreturnProperties( gInstLib, eci, &oSVGProperties[0], svglast-1 );
        }; break;
        
        case ECM_PROPERTYCANASSIGN: {
            oSVG* object = (oSVG*)ECOfindObject( eci, hwnd );
            if (object != NULL) {
                return object->propCanAssign(ECOgetId(eci));
            };
        }; break;

        case ECM_SETPROPERTY: {
            oSVG* object = (oSVG*)ECOfindObject( eci, hwnd );
            if (object != NULL) {
                EXTParamInfo* param = ECOfindParamNum( eci, 1 );
                if ( param ) {
                    EXTfldval fval( (qfldval)param->mData );
                    qbool result = object->setProperty(ECOgetId(eci), &fval);
                    if (result == 1L) {
                        return 1L;
                    };
                };
            };
        }; break;

        case ECM_GETPROPERTY: {
            oSVG* object = (oSVG*)ECOfindObject( eci, hwnd );
            if (object != NULL) {
                EXTfldval fval;
                qbool result = object->getProperty(ECOgetId(eci), &fval);
                if (result == 1L) {
                    ECOaddParam(eci,&fval);
                    return 1L;
                };
            };
        }; break;
	 };

	 // As a final result this must ALWAYS be called. It handles all other messages that this component
	 // decides to ignore.
	 return WNDdefWindowProc(hwnd,Msg,wParam,lParam,eci);
};
