# 2 Ship 2 Harkinian Android Port
A port of 2 Ship 2 Harkinian to Android. <br>

Original Repository: https://github.com/HarbourMasters/2ship2harkinian <br>
<br>

NOTE: Controller only. No touch controls yet except for in the enhancements menu. <br>

Supported (probably): Android 7+ (OpenGL ES 3.0+ required) <br>
Tested On: Android 14 <br>

<h3>Installation:</h3>
1. Install the apk from here: https://github.com/Waterdish/2ship2harkinian-Android/releases. <br>
2. Open the app once. It will generate the directory for your rom. Allow all file permissions and then close and reopen the app.<br>
3. Select "Yes" when prompted by the app if you would like to generate an O2R. Select "Yes" when it asks to look for a rom. Navigate to your "MM.z64" and select it. The extraction should start.<br>
5. It will launch straight into the game on subsequent plays. <br>
<br>
  
Use Back/Select/- controller button, or the Android back button (swipe left if using gesture controls) to open Enhancements menu. Use touch controls to navigate menus. <br>

<h3>FAQ:</h3>
Q: Where do I add mods? <br>
  A: Android/data/com.dishii.mm/files <br> <br>

Q: Why is it immediately crashing? <br>
  A: Try deleting and re-extracting the O2R file (mm.o2r). <br> <br>

Q: The game opened once, but now it's just a black screen. <br>
  A: Reinstall and don't raise MSAA above 1 in Settings->Graphics <br><br>

Q: The GUI scaling is too big/too small. <br>
  A: There is no GUI scaling option implemented yet. This will come in a future update. <br><br>

Q: Gyro Aim? <br>
  A: Working on it. <br> <br>

Q: My controller is not doing anything. <br>
  A: Close the Enhancements Menu. If the Enhancements Menu is not open, open it with the Android back button and check if it is detected in Settings->Controller->Controller Mapping. If it is, press refresh. <br><br>

<b>Known Bugs</b>:<br>
Re-connected controllers do not open the GUI anymore. Use the back button or restart the app.<br>
Orientation Lock does not work. https://github.com/libsdl-org/SDL/issues/6090<br>
Near-plane clipping when the camera is close to walls.<br>
Picto box images render black. <br>

<h3>Build Instructions:</h3>
1. Edit the app/build.gradle file to point to your ndk folder. NDK 26+ tested as working.<br>
2. Open the project in android studio and build.<br>


