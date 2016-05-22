# omnis.xcomp.svg
Omnis XCOMP wrapper around nanosvg

This is a very straight forward Omnis XCOMP wrapper for nanosvg.
Nanosvg is a lightweight SVG (Scalable Vector Graphics) parser and renderer that does an adequate job at rendering SVG. It currently doesn't support text and has limited support for certain effects.
It is very well suited for rendering out things like icons or simple graphs. I've included some sample svg files from the w3c site, you can see which ones render properly and where there is room for improvement.
As it is a full software based implementation creating large output files may be resource hungry and slow.

Find out more about Nanosvg here:
https://github.com/memononen/nanosvg

Note also that nanosvg itself is released under a ZLIB license.

For PNG output of the image this XCOMP uses logic from the STB library that can be found in full here:
https://github.com/nothings/stb

Note that STB fall under a public domain license.

All code related to the above two libraries are used in this project within their original and unmodified files. 

The XCOMP itself is written based on the Omnis Studio SDK, please refer to the license details of the SDK for further licensing information.

The code I've added to create the XCOMP I'm releasing under an MIT license.
I'm also providing binaries for some versions of Omnis Studio to benefit those people that don't want to go through the trouble of compiling themselves.
These can be used freely.
