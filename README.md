Set background as wallpaper extension
=====================================

This Google Chrome extension adds a context menu to every single image that
allows you to set that image as a wallpaper. It allows you to change the style
of the wallpaper by simply pressing one of the TILE, CENTER, and STRETCH types.

How does it work?
----------------
It uses the Google Chrome Extension API to inject context menus to every image.
Then once you clicked on an image, it triggers a callback to the background
page which opens up the preview page.

The preview page is an HTML5 canvas, where I load the image into the canvas.
Once the image loads, we use simple 2D math to figure out where the center is,
how to stretch it, and many scaled photos needed to tile.

When the user chooses save background, it will go to a NPAPI plugin which is
programmed in C++ that hooks itself to the Windows API.


Mohamed Mansour hello@mohamedmansour.com