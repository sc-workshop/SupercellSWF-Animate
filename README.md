<p align="center">
<img src="./assets/workshop.png"  width="10%">
<h1 align="center" style="font-size: 32px;"> Supercell Animate </h1>
</p>

### Supercell Animate is a custom platform addon and extension for Adobe Animate. This extension will allow you to create custom documents and export content of your library directly to a .sc file.

<br/>

> [!IMPORTANT]
> This repository does not provide any instructions on how to modify any Supercell game or their assets. All information and all Assets were taken from public sources. You assume all risks from the use of this tool.


# Table Of Content

- [Export features](#export-features)
- [How to use](#how-to-use)
- [Limitations](#limitations)
- [Issues](#issues)
- [Requirements](#requirements)
- [Installing](#installing)
- [Updating](#plugin-update)
- [Building](#building)
- [Examples](#examples)
- [Changelog](#changelog)
- [Support](#support)

<br/>

# How to use
- Check [requirements](#requirements), make sure everything is satisfied
- [Install](#installing) or [build](#building) extension
- Open your Adobe Animate and go to New Document panel
- Create new "SupercellSWF" document: 

<p align="center">
<img src="./assets/new_document_panel.png"  width="45%">
</p>

- Create something!  
In [examples](#examples) you can find good and different use cases for extension in different games!  
Also, before creating something, do not forget to read information about ["limitations"](#Limitations) that will help you not to create any problems.

- To set export options you need to click "File -> Publish Settings.."  
You can set all options here for the first time only once and then just press "Publish" button. You can also find a description of all options in ["Publisher options"](#publisher-options)

<p align="center">
<img src="./assets/publish_settings_select.png"  width="35%" height="30%">
</p>

- After setting all options, you can click on publish button. You are great!  
Then you just need to wait until file is saved. 

# Content creation tips
For the most part these tips will help you make a more "correct" and more optimized project.

- Be sure to pay attention to ["Limitations"](#limitations) section.
- Pay attention to linkage name.
    Plugin exports only MovieClips that have an linkage name, it is used as export name of symbol.
    For details, see [Examples](#examples)
- Avoid using sprites, and especially vector graphics in MovieClips.  
    Animate is a fairly flexible program and you will notice very little difference between different types of symbols, but speed of export and rendering performance in general depends on how correctly you use these types.  
    When organizing project structure, note that Shapes or so-called Graphic symbols are intended to contain sprites or vector graphics. They can have only one frame.
    At the same time, MovieClip symbols are used to create animation using shapes. They can have an unlimited (within technical limits) number of frames and they can have a linkage name.
- 

# Support
This project was created with the goal of exploring various interesting tools and to support a small but quite creative community of modders. I put a lot of heart and time into this creation, so if you want to support me, you can buy me a coffee right here :3  
[!["Buy Me A Coffee"](https://www.buymeacoffee.com/assets/img/custom_images/orange_img.png)](https://www.buymeacoffee.com/daniilsivi)
  
# Export features
- Stroke  / Fills / Brushes ✅  
Full support. It can be unstable sometimes especially with big fills and curves so try to work more with sprites

    - Fill styles ✅  
    Currently supported are solid color fill and bitmap fill. Linear fill will be added later if needed.

- Filters ❌  
There will be no support for filters for Symbols

- Scripts (ActionScript, JavaScript) ❌  
.sc file does not support any scripts

- Sounds ❌  
.sc file does not support sounds

- Parenting / Camera / Advanced Layers ✅  
Partial support. 
At the moment, implemented layer parenting (rigging)

- Text ✅  
Partial support

- Symbols (Graphic, MovieClip) ✅  
Full support except for "Button" type

    - Blending modes ✅  
    Full support. But note that some modes may not work or crash.

    - Transform ✅   
    2D Transforms are fully supported. 3D transformations have a potentially small range of uses and a small chance of being supported.

- Layers ✅  
All types of layers are supported: Normal, Masks, Folders, Guides
Also note that all symbols from guide layer will be ignored and will only be used for animation if necessary.

- Classic / Animation tweens ✅  
Absolutely all types of animation tweens and their types of interpolation are supported.

 - Shape animation ✅  
Full support. Be careful when using it, the result can be very unstable.

- Nine-slice ✅  
Full support. But keep in mind that on Animate 2024 and above this may not work correctly.  

# Limitations

- Color transforms  
Be careful when transforming colors. Color values can only be positive, otherwise these values will be set to zero and the result will be undesirable.

- Text Fields  
Text fields are covered with a large layer of mystique.  
But there is also support for basic things here.
Also be careful, not all settings can somehow affect text in final file (because they cannot be disabled, Animate just crashes)  
Also, text must have no more than 255 characters, otherwise text will not be written to file.
To enable text outline you need to add "Glow" filter to text field  
  
# Requirements
For extension to work correctly, you need:

- Adobe Animate 2022 or higher

## Windows
- Windows 10 x64 or Windows 11
- [Visual C++ Redistributable Runtime](https://aka.ms/vs/17/release/vc_redist.x64.exe)

## MacOS
Not supported now
> [!NOTE]
> MacOS support will be available one day, but it is not a priority at all right now.
  
# Installing
First of all, download .zip package from [Releases](https://github.com/sc-workshop/SupercellSWF-Animate/releases)  
Unpack zip to any place convenient for you.  
Inside archive there is a file ```install.jsfl```. You need to run it in Adobe Animate.  
By default, ```.jsfl``` is associated with Animate, so you can double-click on it and run it as a command, but if something is wrong, you can open it in Animate itself as a script and click on run button.  
After installation is complete, be sure to restart Animate.
If you want to remove plugin, run ```install.jsfl``` again, and this time you will see a confirmation button.
  
# Plugin Update
If you have an already installed plugin and want to update, run installation file and <strong>remove</strong> plugin, restart Animate and install plugin as usual.  

> [!IMPORTANT]
> If you are using version 1.2.0 and lower, you <strong>MUST</strong> remove extension files <strong>MANUALLY</strong>. If you do not do this, then installation may not be successful.
  
# Building
Before starting you need to have:
- [npm, node.js and ts-node](https://nodejs.org/)
- [Typescript and ts-node](https://www.npmjs.com/package/ts-node)
- MSVC Build tools (Visual Studio)

To prepare all the necessary dependencies, run the following command in the project folder:  
`
    npm run cook
`  
  
Next, after preparing all modules, you need to open console (as admin in case of Debug) in root of repository.  
- Release  
    To build a installer with all packages, you need to fill in certificate details in `scripts/cert.ts` and then run following command:  
    `
    npm run package
    `  
    After the command completes, installer files will be available in `dist/package` folder.  
    You can run it or zip and share.
- Debug  
    If you want to use a debug build then make sure you have CEP debug mode enabled. You can read more about CEP [Here](https://github.com/Adobe-CEP/CEP-Resources/blob/master/CEP_9.x/Documentation/CEP%209.0%20HTML%20Extension%20Cookbook.md)  
    After that you need to use this command:  
    `
    npm run build:dev
    `  
    After execution, extension should load into Adobe Animate.  
    From now on you can connect any native or typescript debugger to Animate process and do your own stuff.
  
# Examples
You can find a list of all available examples in ["examples"](/examples/README.md) folder.  
  
# Issues
If you find any problem with export process or you just need any help, please report it in [Issues](https://github.com/sc-workshop/SupercellSWF-Animate/issues) or [join our Discord Server](https://discord.gg/uPnDsns6G6)  

> [!TIP]
> Native part of plugin saves logs in "%temp%/org.scWorkshop.SupercellSWF_export_log.txt", please provide them along with a description for a faster solution to the problem.  
  
# Changelog
## v 1.3.6
- Minor exporter crash fixes
- Publisher interface crash fixes
## v 1.3.5
- Added text export selection option
- Minor improvements and fixes
## v 1.3.4
- Added support for layer parenting
- Minor improvements and fixes
## v 1.3.3
- Added automatic installation of different plugin variants for different cpu feature sets (Currently supported AVX2, SSE4.2 and native)
- Minor bugfixes and performance improvements
## v 1.3.2
- Added Bitmap Fill support
- Added Export name replacing and atlas textures repacking
## v 1.3.1
- Improved progress window behavior
- Added object caching
- Various bug fixes
## v 1.3.0
- Improved Atlas Generator
- Added SC2 support
- Many performance improvements and code refactoring
## v 1.2.1
- Added experimental installer
## v 1.2.0
- Complete refactoring of code
- Changed and improved design of Publisher Settings
- More animations for Publisher progress window
- Improved support for Fills and Strokes. Added rasterization to sprites
- Added new algorithm to reduce shape object count 
- Added support for 9slice scaling
- More bugs
## v 1.1.0
- Added support for blend modes
- Added basic support for solid fills and stroke
- Added experimental shape animation
- Added support for group
## v 1.0.1
- Fixed sprite transformation
- Updated localization
## v 1.0 
- First release. Basic export to .sc file
