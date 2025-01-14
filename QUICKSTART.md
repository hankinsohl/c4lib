# Quick Start Guide for c4edit

## Configuring c4edit for your computer

c4edit needs to be configured for your computer before it will work. To configure c4edit, open
config.xml in a text editor. Then scroll down to the "\<config\>" block, and make changes to
the following options:

### LOAD_SAVE

Use LOAD_SAVE to specify the name of the save you want to load. In the config block, edit the
entry for LOAD_SAVE.

* Replace "\*your-username\*" with your Windows username
* Replace "\*save-name\*" with the name of the save you want to load.

To load a save, the following options are required: SCHEMA, BTS_INSTALL_DIR and CUSTOM_ASSETS_DIR.
MOD_NAME and USE_MODULAR_LOADING may also be used, but are not required, and should not be
specified in most cases.

### SCHEMA

c4edit is installed in the same directory as its schema file. Unless you have moved c4edit or the
schema file, "BTS.schema" will work.

### BTS_INSTALL_DIR

Set this option to the value where you installed Beyond the Sword. Some example values are as
follows:

If you installed Beyond the Sword from CD:

* "C:\Program Files (x86)\Firaxis Games\Sid Meier's Civilization 4\Beyond the Sword"

If you installed Beyond the Sword from Good Old Games (GOG):

* "C:\Program Files (x86)\GOG Galaxy\Games\Civilization IV Complete\Civ4\Beyond the Sword"

If you installed Beyond the Sword from Steam:

* "C:\Program Files (x86)\Steam\steamapps\common\Sid Meier's Civilization IV Beyond the Sword\Beyond
  the Sword"

In the config block, the value for Steam is used. Change this value as described above, if needed.

### CUSTOM_ASSETS_DIR

Set the value of this option to the directory Beyond the Sword uses to store custom assets.

* Replace "\*your-username\*" with your Windows username.

### WRITE_TRANSLATION

Use this option to specify the name of the translation you want to create.

## Running c4edit

Once the configuration file has been updated you can run c4edit from a command line prompt using
the following command:

```
c4edit -config_file="config.xml"
```

## Additional options

Additional options for loading info files, and for writing info and save files are documented
in config.xml. Set these options as necessary. In particular, if you want to edit a BTS
save, use WRITE_INFO to generate an info file for the save. You can edit the info file in a
text editor. When done with changes, use LOAD_INFO to load the modified info file and use
WRITE_SAVE to generate a new BTS save based on the modified info file.