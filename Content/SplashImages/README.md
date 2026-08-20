# Splash Images

Put editor splash screen candidates in this plugin content folder as `.png` files.

Configure the active list in Unreal Editor under:

`Project Settings > Plugins > Splash Screen Cycler`

If the configured array is empty, the plugin falls back to every PNG in this folder.

On editor startup, the Splash Screen Cycler plugin copies the next configured PNG to:

`Hell_Run/Content/Splash/EdSplash.png`

The editor reads that native splash before plugins can run, so the image selected during one startup is the splash shown on the next startup.
