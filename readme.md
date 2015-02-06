# StylizationRD 
#### Image stylization using anisotropic reaction diffusion
###### Keywords: image stylization · reaction diffusion · pattern generation
 
---


### [Required]

 * <a href="http://www.cmake.org/" target="_blank">CMake</a>(3.0 or higher)
 * <a href="https://www.wxwidgets.org/" target="_blank">wxWidgets</a>(3.0.2 or higher)
 * <a href="http://opencv.org/" target="_blank">OpenCV</a>(2.9.7 or higher)
 * amp(C++ Accelerated Massive Parallelism)  visual studio 2013


### [WorkFlow]
 * Open src: Load image into MASK, and generate sobel edge image into MASK_s
 * Flowfield Option: 
  1. Open ETF: use the ETF of load image as flowfield
  2. Open Flow: there are some vfb in /data/

 * Start: use mouse left button to draw patten into current_B, use tool->Edge2addB and slide addB large than 0, can add edge of input image to result of reaction diffusion.


### [in post process]
  1. Segmentation
  2. size of brush (addA, addB)
  3. Pattern Picker use x.vfb
  4. Demo Film


---

### [對應table 1的fig.5 6的產生]
fig5(a)(b)(c)(d): source.vfb, F k l照表設定(Size調到0.5左右)，用seed均勻灑顏料，穩定後用adathresholding(beta=0.8左右)
*fig5(d)theta0調成220度

---

###[Useful Tutorials]
#####[wxWidgets]
 * <a href="http://www.rhyous.com/2009/12/16/how-to-compile-a-wxwidgets-application-in-visual-studio-2008/" target="_blank">How to compile a wxWidgets application in Visual Studio 2008?</a>
 * <a href="http://changyang319.pixnet.net/blog/post/26984931-wxwidgets%E6%95%99%E5%AD%B8%EF%BC%9A%E7%B7%A8%E8%AD%AF%E9%9D%9C%E6%85%8Blib%E6%AA%94%E5%8F%8A%E7%AC%AC%E4%B8%80%E5%80%8Bhello-world" target="_blank">wxWidgets教學：編譯靜態LIB檔及第一個Hello World程式]</a>


#####[OpenCV]
 * <a href="http://docs.opencv.org/doc/tutorials/introduction/windows_install/windows_install.html#windowssetpathandenviromentvariable" target="_blank">Set the OpenCV enviroment variable and add it to the systems path</a>
 * <a href="http://docs.opencv.org/doc/tutorials/introduction/windows_visual_studio_Opencv/windows_visual_studio_Opencv.html#windows-visual-studio-how-to" target="_blank">How to build applications with OpenCV inside the Microsoft Visual Studio</a>

#####[Others]
 - Q1:[Build Error] Error LNK2019
 - A1:Windows, Open VS2013 Project settings -> Linker > General > System : change Subsystem to "Windows"
 - Q2:[Build Error] Unable to startup program (The system cannot find the file specified 系統找不到指定的檔案)
 - A2:Solution Explorer -> Select "FlowBase" -> Right Click and "Set as StartUp Project"
