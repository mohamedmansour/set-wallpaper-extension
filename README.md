Set wallpaper extension
=====================================

This Google Chrome extension adds a context menu to every single image that
allows you to set that image as a wallpaper. It allows you to change the style
of the wallpaper by simply pressing one of the TILE, CENTER, STRETCH, FILL, and
FIT types.

How does it work?
----------------
It uses the Google Chrome Extension API to inject context menus to every image.
Then once you clicked on an image, it triggers a callback to the background
page which opens up the preview page.

The preview page is an HTML5 canvas, where I load the image into the canvas.
Once the image loads, we use simple 2D math to figure out where the center is,
how to stretch it, and many scaled photos needed to tile. The preview should
look like exactly how the image will be scaled on your screen.

When the user chooses save background, it will go to a NPAPI plugin which is
programmed in C++ that hooks itself to the Windows API.

How to debug?
-------------
You can debug the extension's Native (NPAPI) instance by setting a property 
for the plugin:
 
    app.debug = true;

![Screenshot of the Chrome Extension](https://chrome.google.com/extensions/img/ddkmiidlgnkhnfhigdpadkaamogngkin/1288980317.71/screenshot/22002)


Mohamed Mansour hello@mohamedmansour.com