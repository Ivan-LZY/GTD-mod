# Grand Theft Detection
GTD-mod (Grand Theft Detection) is a user-friendly object detection mod created for the photorealistic video game, Grand Theft Auto V (GTAV). This mod, developed in C++, allows the user to explore the 3D virtual world in GTAV and capture images along with its object detection annotations for pedestrians & vehicles.

Recorded in-game footage:

<p align="center">
  <img src="https://user-images.githubusercontent.com/87157423/140605749-714d2ad3-a012-4a23-91a0-82ef6cb737d6.gif">
  <a><br>Visual demonstration of this mod</a>
</p>
<br>
<p align="center">
  <img src="https://user-images.githubusercontent.com/87157423/140607543-5509dfa7-5542-45c9-a697-8650d65bfb79.png">
  <a>Example of traffic scenes captured in-game. Overlay (Bounding boxes and Keypoints) was applied using the data logged by GTD-mod.</a>
</p>

The exploratory approach to using this mod makes the data collection process in any data science project a fun one!

# Background

GTAV has been used for various research in computer vision, to name a few:

- Semantic Segmentation - "Playing for Data: Ground Truth from Computer Games", EECV 2016 [paper link](https://download.visinf.tu-darmstadt.de/data/from_games/data/eccv-2016-richter-playing_for_data.pdf) 
- Autonomous Vehicle - "Precise Synthetic Image and LiDAR (PreSIL) Dataset for Autonomous Vehicle Perception", 2019 [paper link](https://arxiv.org/pdf/1905.00160.pdf)
- Multiple Object Tracking - "The MTA Dataset for Multi Target Multi Camera Pedestrian Tracking by Weighted Distance Aggregation", CVPR 2020 [paper link](https://openaccess.thecvf.com/content_CVPRW_2020/papers/w70/Kohl_The_MTA_Dataset_for_Multi-Target_Multi-Camera_Pedestrian_Tracking_by_Weighted_CVPRW_2020_paper.pdf)
- Image Enhancement - "Enhancing Photorealism Enhancement", 2021 [paper link](https://arxiv.org/pdf/2105.04619.pdf)

However, only a handful these papers have open sourced their mods and datasets. Of these papers, some of their mods either work in very limiting scenarios or are outdated and crashes frequently in-game.

I have found two open sourced mods with working codes: [JTA-mod](https://github.com/fabbrimatteo/JTA-Mods) & [MTA-mod](https://github.com/koehlp/MTA-Mod). *Note: MTA-mod is derived from JTA-mod*

Using the code from JTA & MTA, I then overhauled the functions and UI so that the new mod can now capture images and log annotations from your in-game character's camera, rather than from a fixed perspective.

The following improvements were added to the mod:
- Spawn/De-spawn pedestrians
- Instantly teleport your character to various locations in-game
- Added a toggle to let your character "fly" so that you can capture images from a height
- Devised a method to robustly gather vehicle information in each scene and log this data for annotation
- Randomized weather conditions, time of day and camera rotations when taking capturing data, so as to get a variety of lighting conditions and view angles

To view more annotation visualizations, go to [Results showcase](#Results-showcase)

# Quick start
***The following set of instructions is adapted from JTA-mod's repository***
- Download and install a legal copy of Grand Theft Auto V
- Download ScriptHook V AND the SDK [here](http://www.dev-c.com/gtav/scripthookv/). 
    - Follow the instructions reported on the web page. Also, follow the instructions of the SDK readme under the section "Runtime asi script reloading"
- Download the GTA Mod: [`GTD.asi`](https://drive.google.com/file/d/1ybk8qgh2hBn8A60IbDRrLHAfSY-9YNoV/view?usp=sharing)
- Copy `GTD.asi` into the Grand Theft Auto V game folder.
- Copy the whole folder `LZY_Anno` into the Grand Theft Auto V game folder.
- If you installed GTA V using Rockstar's Social Club, you have to change the permissions of the game folder (read, write and execute). If you used Steam, everything should be fine.
- Run Grand Theft Auto V and select Story Mode. If it is your first start of the game, you have to complete the first mission.
- The mods will automatically start. Now, by pressing `F6` you can bring up the GTD-mod UI. You can disable the Mods in any time by pressing ctrl+R.
- Remember to disable the HUD in the display settings of GTA V. Also disable the in-game notifications.

If done correctly, your directory should look like this:
```
PlayGTAV.exe			//GTAV main directory
GTD.asi				//GTAV main directory
└───LZY_Anno				//folder to hold annotations/screenshots/process_data
	└───cam_current				//folder where all in-game captured images (.jpg) and annotations (.csv) are saved
	└───parameters.txt			//parameters that GTD.asi will read from
	└───plot_funcs.py			//functions to plot annotations onto the captured images
	└───main.py				//run this to visualise/save captured images with overlay
```
## In-game UI Guide

<p align="center">
  <img src="https://user-images.githubusercontent.com/87157423/140610510-a8609c50-298c-4a5e-9e69-e1f0226083d5.JPG">
  <a></a>
</p>

To plot and visualize the logged data, setup directories correctly and use [main.py](https://github.com/Ivan-LZY/GTD-mod/blob/main/main.py)

## Developer Guide
- Download and install a legal copy of Grand Theft Auto V
- Clone this repository
- Download ScriptHook V AND the SDK here http://www.dev-c.com/gtav/scripthookv/. Follow the instructions reported on the web page. Also, follow the instructions of the SDK readme under the section "Runtime asi script reloading"
- Open the Solution with Visual Studio (I used VS2017), follow these instructions:
	- include the header files to the project by going in Configuration Properties->C/C++->General and set the path to the "inc" directory of the SDK in "Additional Include Directories"
	- include the library to the projects by going to Configuration Properties->Linker->General  and set the path that points to the "lib" folder of the SDK in "Additional Library Directories" 
	- Also go to:  `Configuration Properties -> Linker -> Input`  and set the `ScriptHookV.lib` file name in `Additional Dependencies`
	- Go in `Build-Events -> Post-Build-Event` and, under `Command Line` type `xcopy /Y path/to/asi/file path/to/gta/installation/directory`
	- Under `Configuration-Properties -> General` change the `Target Extension` to `.asi` and `Configuration Type` to `.dll`
- Build the entire solution.

This mod is developed in C++. To modify or implement new functions in-game, I strongly encourage you to visit the database provided by ScriptHookV developer: http://www.dev-c.com/nativedb/

# Results showcase

With this mod, it is possible to generate synthetic datasets for a variety of use-case:

<br>
<p align="center">
  <img src="https://user-images.githubusercontent.com/87157423/140604002-a6ad5ff5-f345-48b5-a511-edbc97eb370d.png">
  <a>Elevated and street views</a>
</p>
<br>
<p align="center">
  <img src="https://user-images.githubusercontent.com/87157423/140603924-25c65f8e-9731-42bc-b521-e9831b066079.png">
  <a>Weather and time of day randomization</a>
</p>
<br>
<p align="center">
  <img src="https://user-images.githubusercontent.com/87157423/140604131-4ecf0c93-6982-4217-9bcb-f87b593b98bf.png">
  <a>Human keypoint annotations at varying ranges with/without occlusion</a>
</p>
<br>
<p align="center">
  <img src="https://user-images.githubusercontent.com/87157423/140604290-36cdf409-1e4d-4cbf-9fdd-c655d519bd3f.png">
  <a>Vehicle detection with range information & traffic scenarios</a>
</p>

# Possible Customization
Given the huge modding community for GTAV, you can always download or make your own cosmetic mods to introduce diversity in your dataset:

<p align="center">
  <img src="https://user-images.githubusercontent.com/87157423/140604915-c848c76d-b23c-465e-97f1-4968317083cd.png">
  <a></a>
</p>

Cosmetic mods can be found at: https://www.gta5-mods.com/
<br>

# Credits
[JTA-mod](https://github.com/fabbrimatteo/JTA-Mods)<br>
[MTA-mod](https://github.com/koehlp/MTA-Mod)<br>
[MenyooSP](https://github.com/MAFINS/MenyooSP)
