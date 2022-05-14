<kbd>![Town of Wakewood - Banner](Screenshots/TownofWakewood_Banner.png?)</kbd>

## PROJECT TITLE
The Town Of Wakewood

## PROJECT BRIEF
This Optimised Developer Tool will be used to present varying OpenGL Techniques. These include, a Geometry Shader, Shadows, Noise, and Particles and Animation. This application will be used to showcase these elements in real time and allow you to understand the different concepts within OpenGL. 

### FEATURES
  * Dynamic Small Town Scene
  * Geometry Shading
  * Shadow Mapping/Volumes
  * Particle Effects & Animations
  * Noise Decay
  * Custom HDR Skybox
  * Background SFX
  * Animated Models, Camera, and Lighting
* ImGUI
  * Model & Animation Controller
    * Toggle Plane Model On/Off 
    * Adjust Game/Animation Speed
    * Adjust Distance Vehicles Travel
    * Adjust Vehicle Moving Speed
  * Lighting Controller
    * Adjust Global Lighting on the X, Y, and Z Axis
  * House Customization
    * Toggle White House Model On/Off
    * Toggle Yellow House Model On/Off
    * Toggle Red House Model On/Off
    * Toggle Blue House Model On/Off
  * Volume Settings
    * Adjust Forest Ambience SFX
    * Adjust Car Ambience SFX
    * Toggle Ambience On/Off
  * Tool Customization
    * Toggle Animation On/Off
    * Exit Application
    * View Performance


--------------


### DOCUMENTATION

#### How does the user interact with your executable? How do you open and control the software you wrote (exe file)?
The Town of Wakewood can be download [here](https://github.com/caleb-long19/COMP3015_Optimised-Developer-Tool/releases/tag/v1.0). Once downloaded, extract and open the folder. Double click "Project_Wakewood.exe" to open the application. The loading time of the application will vary based on your systems hardware. Once the program has loaded, you will be presented with a low-poly town. In the top left corner you will see a graphical user interface (GUI), developed using ImGUI. Use your mouse to interact with the GUI. Left click on the buttons or tick boxes to interact with them. Hold the left mouse button on the sliders and drag left/right to alter their values. Each button/slider has a title above them, indicating what they do. Close the application by clicking the "x", located at the top right of the window, or press the "Exit Application" button on the GUI.

#### How does the program code work? How do the classes and functions fit together and who does what?
The Town of Wakewood utilizes four primary techniques. Geometry Shading, Shadow Volumes, Particles & Animation, and Noise. Geometry Shading & Shadow Volumes require one another to work properly. Six GLSL Shaders were created to begin implementation, thse shaders are as follows:

* shadowVolume-comp.frag
* shadowVolume-comp.vert 
* shadowVolume-render.frag
* shadowVolume-render.vert
* shadowVolume-vol.frag
* shadwowVolume-vol.geom
* shadowVolume-vol.vert

Before discussing the shaders in further detail, context is required on where the data that is sent to the shaders is from. A file called "scenebasic_uniform.cpp" contains a vast amount of methods, each containing unique instructions to provide an efficient application. To communicate with the shaders, we must first compile them. This is done in the compile method.

<kbd>![Compile Code](Screenshots/Code_Compile.png?)</kbd>

The code sample above displays the previously discussed shaders being compiled and linked, by using the function of .use(). We can activate the shader. Once activated, the data we send to the .vert, .frag, and .geom files can be used.

Once all the data from the shaders has been set via 

All vertices shaders are used to retrieve/output data from the scenebasic_uniform.cpp file.

#### What makes your shader program special and how does it compare to similar things?
The Town of Wakewood has been built to simulate a small, top-down, open world setting of a small town. The visual look of the tool is used to represent games such as Cities: Skylines (Developed by Colossal Order, 2015).

In Cities: Skylines, you design large cities, containing residential, industrial, and business districts. AI is present in the game and move around the city without the input of the player. The car models present in The Town of Wakewood are animated, going back and forth on the road, simulating AI. 

The tools visual apperance mimics that of a residential area in Cities: Skylines.

| Features             | Cities: Skylines    | Wakewood            |
| -------------        | -------------       | -------------       |
| Pause Time           | :heavy_check_mark:  | :heavy_check_mark:  |
| Alter Game Speed     | :heavy_check_mark:  | :heavy_check_mark:  |
| Build Cities         | :heavy_check_mark:  | :x:                 |
| Vehicle AI           | :heavy_check_mark:  | :x:                 |
| Music/SFX            | :heavy_check_mark:  | :heavy_check_mark:  |

To begin development of the tool, my previous project [COMP3015: Coursework 1](https://github.com/caleb-long19/COMP3015-Custom-Shader-Project-OpenGL) was used as a base. All aspects of the previous project [COMP3015: Coursework 1](https://github.com/caleb-long19/COMP3015-Custom-Shader-Project-OpenGL) were removed by the end of development, besides two features, the ImGUI library, which provides an easy to use GUI which manipulates the scene, and the irrKlang library, which is used to play audio samples during runtime. Everything else in this project is completely unique in comparison. The Town of Wakewood was originally going to contain only two techinques, geometry and shadow volumes. Due to the extra time after they were implemented, a decision was made to implement particles & animation. Smoke particles were added to chimney's, making it seem that the houses were being lived in. Lastly, the noise decay technique was added, simply because spare time was available and to increase the quality of this portfolio piece.

To make the tool unique from Cities: Skylines, two features were implemented. The first allows the user to alter the speed of the vehicles/traffic separate from the games speed. The second feature allows the user to change the distance the vehicles travel on the road. These features combined with the four techniques, creates both a visually pleasing and dynamic tool that serves as a professional portfolio piece.


--------------


### INSTALLATION INSTRUCTIONS & USER MANUAL

#### How To Install & Use The Tool

<kbd>![Release Location](Screenshots/Release_Location.png?)</kbd>
Go to the Releases Page. This is located on the right-hand side of the GitHub Repository

<kbd>![Release Download](Screenshots/Release_Download.png?)</kbd>
Download the The Town Of Wakewood: Developer Tool - Verison "N"

<kbd>![Extract .zip Folder](Screenshots/Extract_To_Folder.png?)</kbd>
After Downloading the .zip file. Extract to any location you want. 

<kbd>![Open Application](Screenshots/Exe_Location.png?)</kbd>
Enter the folder and double-click the "Project_Wakewood.exe"

<kbd>![Application Demo](Screenshots/Wakewood_Demo.png?)</kbd>
The Window will load and you will be presented with a scene displaying a small town surrounded by a forest.

A Graphical User Interface will be present on the left-hand side of the window.
* Model & Animation Controller
  * Toggle Plane Model On/Off 
  * Adjust Game/Animation Speed
  * Adjust Distance Vehicles Travel
  * Adjust Vehicle Moving Speed
* Lighting Controller
  * Adjust Global Lighting on the X, Y, and Z Axis
* House Customization
  * Toggle White House Model On/Off
  * Toggle Yellow House Model On/Off
  * Toggle Red House Model On/Off
  * Toggle Blue House Model On/Off
* Volume Settings
  * Adjust Forest Ambience SFX
  * Adjust Car Ambience SFX
  * Turn Music On/Off
* Tool Customization
  * Turn Animation On/Off
  * Exit Application
  * View Performance

#### TUTORIAL & WALKTHROUGH VIDEO - CLICK ME!
[![Tutorial Thumnbail](https://img.youtube.com/vi/sglTpRQdkmU/0.jpg)](https://www.youtube.com/watch?v=sglTpRQdkmU)


--------------


### THE TOWN OF WAKEWOOD: PRESENTATION - CLICK ME!
[![Presentation Thumnbail](https://img.youtube.com/vi/VIDEO_ID/0.jpg)](https://www.youtube.com/watch?v=VIDEO_ID)

### SCREENSHOTS

#### Morning In Wakewood
<kbd>![Town of Wakewood - Morning](Screenshots/Town_of_Wakewood-Morning.png?)</kbd>

#### Afternoon In The Town of Wakewood
<kbd>![Town of Wakewood - Afternoon](Screenshots/Town_of_Wakewood-Afternoon.png?)</kbd>

#### Getting Dark In Wakewood
<kbd>![Town of Wakewood - Evening](Screenshots/Town_of_Wakewood-Evening.png?)</kbd>

#### Flying Around Wakewood
<kbd>![Town of Wakewood - Flying](Screenshots/Town_of_Wakewood-Plane.png?)</kbd>

--------------


### REFERENCES

#### YouTube
* The Cherno: https://www.youtube.com/channel/UCQ-W1KE9EYfdxhL6S4twUNw
* ThinMatrix: https://www.youtube.com/watch?v=C8FK9Xn1gUM
* Victor Gordan: https://www.youtube.com/watch?v=9g-4aJhCnyY

#### Books
* OpenGL Shading Lagauge Cookbooks: David Wolff
* Learn OpenGL: Frahaan Hussain

#### Websites
* Plymouth University DLE Resources: Marius Varga

#### OpenGL Libraries
* ImGUI: https://github.com/ocornut/imgui
* IrrKlang: https://www.ambiera.com/irrklang/

#### Music
* Art of Silence - Uniq: https://www.youtube.com/watch?v=3V-pYCGx0C4
* Zapsplat - Forest Ambience: https://www.zapsplat.com/music/forest-ambience-birds-wind-in-trees-leaves-falling-queensland-australia/
* Sound Library - Car Drive By: https://www.youtube.com/watch?v=LSduo2nbAtI

### Additional Notes
ALL MODELS USED IN THE TOWN OF WAKEWOOD WERE CREATED BY MYSELF USING BLENDER!
