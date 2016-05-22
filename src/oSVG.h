#include <extcomp.he>
#include <extfval.he>
#include <hwnd.he>
#include <gdi.he>

// For more info on SVG see:
// https://www.w3.org/TR/SVG/

// Need this all for our SVG rasteriser
// See for more details and NANOSVG license:
// https://github.com/memononen/nanosvg
#include <stdio.h>
#include <string.h>
#include <float.h>
#include "stb_image_write.h"
#define NANOSVG_ALL_COLOR_KEYWORDS  // Include full list of color keywords.
#include "nanosvg.h"
#include "nanosvgrast.h"

#define OBJECT_COUNT 	1				/* Number of controls within library */
#define LIB_RES_NAME    1000            /* Resource id of library name */
#define OBJECT_ID1		2000            /* Resource id of control within library */
#define GENERIC_ICON 	1				/* Resource bitmap id */

enum svgprops {
    svgdata = 1,
    svgpng,
    svgdpi,
    svgwidth,
    svgheight,
    svglast
};

class oSVG {
private:
	HWND                mHWnd;
    qreal               mDPI;
    qchar *             mSVGString;
    NSVGimage *         mSVGData;
    
    int                 mImageWidth;
    int                 mImageHeight;
    unsigned char *     mImageData;
    HPIXMAP             mPixMap;

public:
	oSVG( HWND pFieldHWnd );
	~oSVG();
    
    void    freeSVGData();
    void    freeImageData();
    
    void    updateImage();
    HPIXMAP getPixmap();
    qbyte * asPng(int &pLen);
    
    qbool propCanAssign(qlong pPropId);
    qbool setProperty(qlong pPropId, EXTfldval * pSetVal);
    qbool getProperty(qlong pPropId, EXTfldval * pRetVal);
	qbool paint();
};

/* EOF */