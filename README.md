Set wallpaper extension
=====================================

This Google Chrome extension adds a context menu to every single image that
allows you to set that image as a wallpaper. It allows you to change the style
of the wallpaper by simply pressing one of the TILE, CENTER, STRETCH, FILL, and
FIT types.

Contributors
-------------

- Mohamed Mansour (Maintainer, Lead developer) - https://plus.google.com/116805285176805120365/posts
- Edwin Vane (Developer, build system, plugin enhancements) - https://plus.google.com/106364473100192535271/posts
                                               - 
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
  you want to generate this README from its Markdown source.


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
* **msvs_project**: Generate a Visual Studio Project. Refer to the
  [Generating MSVS Projects](#msvs) section below for more details.

The scons documentation can be read for more details but to start a build, the
command-line should look something like this:

    scons VAR1=value1 VAR2=value2 ... [target]

The target name is optional. If not provided, the default target is used. Build
results and intermediate files are placed in a directory with the following
format:

    build-<debug|release>-<x86|x86_64>

This scheme of including the build variant in the directory name enables build
results for varying values of the **DEBUG** and **TARGET_ARCH** command-line
variables to exist at the same time.

Generating MSVS <a id="msvs">Projects</a>
------------------------

The **msvs_project** target is used to build a Visual Studio Project using
SCon's built-in functionality. This functionality has some caveats however:

* The resulting solution file (`.sln` file) is placed in the build directory
  with other build results and intermediate files whereas the project files
  (`.vcxproj` files) are placed in the `source` directory.
    * As a result, project files have the build variant as part of their names
      to prevent naming collisions.

The generated solution file can be opened with Visual Studio to navigate, edit,
and build source. Building the project within Visual Studio will invoke scons
with the same **DEBUG** and **TARGET_ARCH** command-line variable values as
were provided when the project files were generated. The result of the build is
the NPAPI DLL component of the plugin. Since this DLL is not a stand-alone
executable using the run or run-in-debug-mode commands in Visual Studio will
cause Visual Studio to complain.

How to debug the plugin
-----------------------
You can debug the extension's Native (NPAPI) instance by setting a property 
for the plugin:
 
    app.debug = true;

---

Mohamed Mansour hello@mohamedmansour.com
