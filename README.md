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

How to build?
-------------
Prerequisites:

* [SCons](http://www.scons.org/) and [Python](http://python.org/)
    * Windows: You may need to modify the `PATH` environment variable to include 
      `<PYTHON_ROOT>\Scripts` (where the scons.bat file gets installed).
* Windows: an installation of Visual Studio. Express versions will work but
  these are limited to 32bit builds only.
* [Markdown in Python](http://www.freewisdom.org/projects/python-markdown) if
  you want to convert this README in Markdown format to HTML.


Run `scons -h` for a list of command-line arguments. Of note are:

* **DEBUG**: Build the shared-library part of the extension with debugging
  support.
* **TARGET_ARCH**: Can be either x86 (32bit build) or x86_64 (64bit build).
* **CHROME_BIN**: Location of the chrome binary. Required if running the
  extension packaging target.
* **PRIVATE_KEY**: Location of the .pem file used for signing a packaged
  extension. If one isn't provided, a key is created when packaging.

Targets:

* **unpacked**: (Default) Build the shared library part of the extension and
  install it and all other extension files in a directory called
  `install-<debug|release>-<arch>/set-wallpaper-extension` resulting in an
  'unpacked' extension that can be loaded with Chrome using _Developer Mode_.
  Intermediate files created during the build are placed in
  `build-<debug|release>-<arch>`.
* **packed**: After building an unpacked extension, package the extension into a
  .crx file. For this target, the `CHROME_BIN` construction variable must be
  set. The `PRIVATE_KEY` variable provides the path to a signing key to use.
  If no key is provided, one will be created. The resulting .crx (and .pem file)
  are placed in `install-<debug|release>-<arch>`.
* **readme**: Use markdown for python to convert README.md into html. Useful
  for previewing the file before a push.

The scons documentation can be read for more details but to start a build, the
command-line should look something like this:

    scons VAR1=value1 VAR2=value2 ... [target]

The target name is optional. If not provided, the default target is used.

How to debug?
-------------
You can debug the extension's Native (NPAPI) instance by setting a property 
for the plugin:
 
    app.debug = true;

---

Mohamed Mansour hello@mohamedmansour.com
