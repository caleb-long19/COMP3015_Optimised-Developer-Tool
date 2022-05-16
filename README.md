<kbd>![Town of Wakewood - Banner](Screenshots/TownofWakewood_Banner.png?)</kbd>

## PROJECT TITLE
The Town Of Wakewood

## PROJECT BRIEF
This Optimised Developer Tool will be used to present varying OpenGL Techniques. These include a Geometry Shader, Shadows, Noise, and Particles and Animation. This application will be used to showcase these elements in real time and allow you to understand the different concepts within OpenGL. 

### FEATURES
  * Dynamic Small Town Scene
  * Geometry Shading
  * Shadow Mapping/Volumes
  * Particle Effects & Animations
  * Noise (Disintegration)
  * Custom HDR Skybox
  * Background SFX
  * Animated Models, Camera, and Lighting
* ImGUI
  * Switch Shader Type
    * Toon Shading
    * Phong Shading
    * Phong Shading (Noise Disintegration)  
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

### IMPORTANT
When you download the source code you may be greeted with an error about Frustrum. Remove it from the helper file and re-add it to the helper filer. It should fix the issue!

--------------


### INSTALLATION INSTRUCTIONS & USER MANUAL


#### TUTORIAL & WALKTHROUGH VIDEO - CLICK ME!
[![Tutorial Thumnbail](https://img.youtube.com/vi/sglTpRQdkmU/0.jpg)](https://www.youtube.com/watch?v=sglTpRQdkmU)


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

<kbd>![GUI](Screenshots/GUI_Example.png?)</kbd>

* ImGUI
  * Switch Shader Type
    * Toon Shading
    * Phong Shading
    * Phong Shading (Noise Disintegration)  
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


### DOCUMENTATION

#### How does the user interact with your executable? How do you open and control the software you wrote (exe file)?
The Town of Wakewood can be downloaded [here](https://github.com/caleb-long19/COMP3015_Optimised-Developer-Tool/releases). Once downloaded, extract and open the folder. Double click "Project_Wakewood.exe" to open the application. The loading time of the application will vary based on your system's hardware. Once the program has loaded, you will need to input the window size you wish the application to be rendered in, hit enter after inputting a value. 

Afterwards, you will be presented with a low-poly town. 

In the top left corner you will see a graphical user interface (GUI), developed using ImGUI. Use your mouse to interact with the GUI. Left click on the buttons or tick boxes to interact with them. Hold the left mouse button on the sliders and drag left/right to alter their values. Each button/slider has a title beside them, indicating what they do. Close the application by clicking the "X", located at the top right of the window, or press the "Exit Application" button on the GUI.

#### How does the program code work? How do the classes and functions fit together and who does what?
The Town of Wakewood utilizes four primary techniques. Geometry Shading, Shadow Volumes, Particles & Animation, and Noise (Disintegration). A total of 8 GLSL Shaders were created during development to implement these techniques, these shaders are as follows:

* Geometry & Shadow Volume Shader
  * shadowVolume-comp.frag
  * shadowVolume-comp.vert 
  * shadowVolume-render.frag
  * shadowVolume-render.vert
  * shadowVolume-vol.frag
  * shadwowVolume-vol.geom
  * shadowVolume-vol.vert

* Smoke Particles & Animations
  * smokeParticles.frag
  * smokeParticles.vert

Before discussing the shaders in further detail, context is required on where the data that is sent to the shaders is from. A file called "scenebasic_uniform.cpp" contains a vast amount of methods, each containing unique instructions to provide an efficient application. To communicate with the shaders, we must first compile them. This is done in the compile method.

<kbd>![Compile Code](Screenshots/Code_Compile.png?)</kbd>

The code sample above displays the previously discussed shaders being compiled and linked, by using the function of .use(). We can activate the shader. Once activated, we can send our data to our shaders, allowing them to use it for various calculations.

After the compiler has linked all the shaders. We can begin to set up all the data that needs to be sent to the shaders. This information is set via using the function of .setUniform("Name_In_Shader", value).

<kbd>![Uniform Code](Screenshots/Code_setupParticles.png?)</kbd>

The uniform code above shows us loading the texture for the smoke particles. Before we set the uniform data, we need to activate the particle shader, or the application will not execute and provide an error. The uniform data ranges from the lifetime of the particles (How long they are alive before being recycled) to the min and max size of the smoke particles.

<kbd>![Particle Shader Code](Screenshots/Code_particleShader.png?)</kbd>

The uniform data is sent to the smokeParticles.vert shader. The data is used to calculate the velocity of the smoke, this is used to give the effect of smoke rising over time, just like in real life. The lifetime is calculated, checking to see if the particle age is greater than the lifetime, when it is, we recycle that particle, if not, we keep updating the velocity. We can now draw our particles in the drawScene method. We bind our feedback buffers, and particle arrays, set the matrices for the particles and draw them.

The smokeParticle.vert is the most unique vertex shader used in the project. Containing a lot more uniforms, outputs, and methods than all the others.

<kbd>![vertex Shader Code](Screenshots/Code_vertShaders.png?)</kbd>

The screenshot above displays what all the other vertex shader code looks like. Uniforms from the scenebasic_uniform.cpp are calculated here, once all calculations are done in the main method, they are sent to its fragment counterpart. Vertex commonly contains coordinate data for both position and textures, and the Model View Projection Matrix. Model is used for an object's local position into world space, view for the world space to camera, and projection from the camera to the screen.

<kbd>![Model Code](Screenshots/Code_models.png?)</kbd>

An example of manipulating the matrix data can be seen above. We reset our model data, and use the functions of translate, rotate, and scale for our model. Translate means the position of the object e.g. X, Y, Z Coordinates. Rotate is self explanitory, and scale means to increase or decrease the object size.

Setting up the noise requires multiple steps, first we must set the noise texture and bind it. For an object to appear disintegrated, the discard keyword is used and combined with the noise effect, simulating the look of decay on the chosen model. Fragments of the noise are discarded if the noise value is above or below a set threshold. 

<kbd>![Noise Setup Code](Screenshots/Code_setupNoise.png?)</kbd>

We generate a perfect noise texture that will bind to our models, and set the threshold values. This uniform data will be sent to shadowVolume-rander.frag.

<kbd>![Noise Fragment Code](Screenshots/Code_noiseFrag.png?)</kbd>

The main method inside the render fragment shader takes in the noise texture, and texture coordinates into a noise vector. We then check the noise vector value by comparing it to our thresholds, discarding any fragments that meet the criteria. Once discarded, we run the phong model shading method.

<kbd>![Noise Decay Effect](Screenshots/decayModels.png?)</kbd>

Finally, our models can be rendered with the noise disintegration effect. 

Lastly, we have the geometry shader and shadow volumes. These shadow volumes provide high quality rendered shadows for all of the models in the scene, making it visually pleasing. Shadow volumes require boundaries, quads are formed by extending the edges of an object to produce a shadow effect, similar to real life shadows. Each triangle consists of 3 of these quads (Extending upon each edge and caps).

<kbd>![Shadow Setup Code](Screenshots/Code_setupShadowVolumes.png?)</kbd>

Firstly, we need to set up the framebuffer objects, a framebuffer is a combination of multiple buffers including, colour, depth, and stencil. Our framebuffer includes the depth buffer and two colour buffers (ambient, and diffuse + specular). Once the framebuffer has been created, we can return to the set up. A VAO (Vertex Array Object) for the quads is made, we then load the texture/s our models are going to use, and activate the shadow rendering and composition shaders.

The primary use of the geometry shader is to produce the shadow volumes/boundaries, adjacency information of the model's triangles are sent to the geometry shader. Adjacency information is used to check those triangles for a silhouette edge (check for triangle facing the light & check the adjacent triangles). A polygon is then created for the shadow volume. 

<kbd>![Shadow Geometry Code](Screenshots/Code_geometryShader.png?)</kbd>. 

Inside the geometry shader, we are calculating the quads that have formed from the object edges. After, we need to check the triangles and its neighbours, to see which is facing towards or away from the light, a silhouette/shadow is produced on the triangles that are facing away from the light. 

<kbd>![Pass 1 Code](Screenshots/Code_renderGeometry.png?)</kbd>. 

For the geometry and shadows to be processed and rendered correctly, we must follow a triple pass system. Pass 1 is used to render the geometry normally with phong shading; our ambient, and diffuse + specular components created earlier, are separated into individual buffers. Pass 2 generates the shadow volumes/casting objects with the help of the geometry shader. We set up the stencil test buffer so that the stencil test is always successful, front faces return an increment, while back faces return a decrement. Pass 3 sets the stencil buffer, followed by combining the ambient, and diffuse + specular buffers, only if the stencil test succeeds. A full screen quad is then rendered onto the screen, finishing the process of shadow volumes.

<kbd>![Shading Code](Screenshots/codel_shadingModels.png?)</kbd>. 

The phong and toon shading is calculated in the shadowVolume-render.vert file. Phong uses ambient, diffuse and specular lighting techniques, as well as taking in a reflective model and the direction, the phong model calculation is the clamped dot product of the light reflection and direction, to produce semi-realistic surface shading. Toon Shading utilizes the 3 previously listed techniques. We lock a dot product (multiplication of 2 vectors and output a float) for the specular to create a fixed number of values, simulating a cell shaded appearance.

Two libraries were used during development, irrKlang, and ImGUI. irrKlang is a sound library that can be used to play audio clips. To use it, we must initialise the ISoundEngine. Afterwards, we can create ISound components. ISound allows us to use commands such as .play("insert location of sound file") and .volume("insert float value to determine volume level"). 

<kbd>![Sound Code 1](Screenshots/soundLibrary_part1.png?)</kbd>.

<kbd>![Sound Code 2](Screenshots/soundLibrary_part2.png?)</kbd>.

ImGUI provides an interface on the screen. Containing features such as buttons, sliders, checkboxes, etc. The project uses all 3 of the listed features. Sliders are used to adjust animation speeds, positional data, and the audio volume. ImGUI contains various .cpp files containing extensive code and calculations, we just need to call those methods into our own .cpp file and assign the correct data to them.

<kbd>![GUI Code](Screenshots/imGUI_Example.png?)</kbd>.

The example above shows both slider & button code. To communicate with ImGUI, we use ImGUI::"Method Name". Our method name in this case is slider float. We assign our ambience volume float, and set a min/max value for it. The button is used to turn the ambience on/off. We check the current bool, and switch it based on its current state. 

#### Software Engineering Issues
During development, I was careful about my coding practices. The project contains many instances of good coding practices, commenting and encapsulation is plentiful. The code was proofread to remove duplicate code, some could possibly remain. To my knowledge, the code present in the project is efficient and the program performs perfectly well. Various issues occurred during development. A common issue that was repeated, was sending incorrect data through the uniforms, causing the program to fail compiling, render a black screen, or glitch out entirely. GLSL code was very difficult to handle at times, having no particular way of debugging the code, solving errors could range from minutes to hours. 9 times out of 10, the error was caused by an incorrect value being used in a calculation, the rest being syntax errors. Reading the lab sheets and searching google provided most solutions to my problems.

#### What makes your shader program special and how does it compare to similar things?
The Town of Wakewood has been built to simulate a small, top-down, open world setting of a small town. The visual look of the tool is used to represent games such as Cities: Skylines (Developed by Colossal Order, 2015).

<kbd>![Cities: Example](Screenshots/Cities_Reference.png?)</kbd>.

In Cities: Skylines, you design large cities, containing residential, industrial, and business districts. AI is present in the game and moves around the city without the input of the player. The car models present in The Town of Wakewood are animated, going back and forth on the road, simulating AI. 

The tools visual appearance mimics that of a residential area in Cities: Skylines.

| Features             | Cities: Skylines    | Wakewood            |
| -------------        | -------------       | -------------       |
| Pause Time           | :heavy_check_mark:  | :heavy_check_mark:  |
| Alter Game Speed     | :heavy_check_mark:  | :heavy_check_mark:  |
| Build Cities         | :heavy_check_mark:  | :x:                 |
| Vehicle AI           | :heavy_check_mark:  | :x:                 |
| Music/SFX            | :heavy_check_mark:  | :heavy_check_mark:  |

To begin development of the tool, my previous project [COMP3015: Coursework 1](https://github.com/caleb-long19/COMP3015-Custom-Shader-Project-OpenGL) was used as a base. All aspects of the previous project [COMP3015: Coursework 1](https://github.com/caleb-long19/COMP3015-Custom-Shader-Project-OpenGL) were removed by the end of development, besides two features, the ImGUI library, which provides an easy to use GUI which manipulates the scene, and the irrKlang library, which is used to play audio samples during runtime. Everything else in this project is completely unique in comparison. The Town of Wakewood was originally going to contain only two techniques, geometry and shadow volumes. Due to the extra time after they were implemented, a decision was made to implement particles & animation. Smoke particles were added to chimney's, making it seem that the houses were being lived in. Lastly, the noise disintegration technique was added, simply because spare time was available and to increase the quality of this portfolio piece.

To make the tool unique for itself and from Cities: Skylines, two features were implemented. The first allows the user to alter the speed of the vehicles/traffic separate from the game's speed. The second feature allows the user to change the distance the vehicles travel on the road. 

As well as providing a GUI which allows the user to manipulate the scene in multiple ways, creating a dynamic and unique experience for each user. 
These features combined with the four techniques, creates both a visually pleasing and dynamic tool that serves as a professional portfolio piece.

The Town of Wakewood uses a GUI which manipulates the scene in real time, similar to the previously mentioned, Cities: Skylines. The user can switch shaders with the click of a button. Animations Speeds and coordinates can be altered with sliders. Checkboxes are used to trigger booleans which turn the smoke particles on or off. Sliders are used again to change the volume.

#### THE TOWN OF WAKEWOOD: PRESENTATION - CLICK ME!
[![Presentation Thumnbail](https://img.youtube.com/vi/H4-ZPZqtje0/0.jpg)](https://www.youtube.com/watch?v=H4-ZPZqtje0)


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
  * Lecture & Lab 6: Geometry Shaders
  * Lecture & Lab 7: Vertex Animations
  * Lecture & Lab 8: Shadows
  * Lecture & Lab 9: Noise

#### OpenGL Libraries
* ImGUI: https://github.com/ocornut/imgui
* IrrKlang: https://www.ambiera.com/irrklang/

#### Music
* Art of Silence - Uniq: https://www.youtube.com/watch?v=3V-pYCGx0C4
* Zapsplat - Forest Ambience: https://www.zapsplat.com/music/forest-ambience-birds-wind-in-trees-leaves-falling-queensland-australia/
* Sound Library - Car Drive By: https://www.youtube.com/watch?v=LSduo2nbAtI

### Additional Notes
ALL MODELS USED IN THE TOWN OF WAKEWOOD WERE CREATED BY MYSELF USING BLENDER!
