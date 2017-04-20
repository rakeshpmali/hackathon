Links: 
1. Original app source: https://github.com/seek-for-android/open-mobile-api-sample
2. ReadMe about adding org.simalliance.openmobileapi for build: https://github.com/seek-for-android/pool/wiki/UsingSmartCardAPI
3. Sample code to read library version: https://github.com/seek-for-android/pool/wiki/RetrievingVersionInfo
4. Howo add new library to Android Studio: http://stackoverflow.com/questions/25610727/adding-external-library-in-android-studio

Steps:
1. Import project to Android Studio
2. Using SDK manager, add org.simalliance.openmobileapi for API21.
3. Modify the app\build.gradle for "compileSdkVersion" and "targetSdkVersion". Set value to 21
4. Build APK, which would throw errors for org.simalliance.openmobileapi
5. Add a new folder into app, lets say app\libs
6. Drag and drop Android\sdk\add-ons\addon-open_mobile_api-giesecke_devrient_gmbh-21\libs\org.simalliance.openmobileapi.jar
7. Accept all dialog boxes and add it to project.
8. Right-click on app\libs\org.simalliance.openmobileapi.jar --> Add As Library (the UI would ask to "add to module 'app'", continue)
9. Now clean and recompile the APK.
