# stylizationRD 
 Image stylization using anisotropic reaction diffusion

---


### [Required]
 * [CMake](www.cmake.org/)(3.0 or higher)
 * [wxWidgets](https://www.wxwidgets.org/)(3.0.2 or higher)
 * [OpenCV](http://opencv.org/)(2.9.7 or higher)
 * amp(C++ Accelerated Massive Parallelism)  visual studio 2013


### [workflow]
 * Open src: Load image into MASK, and generate sobel edge image into MASK_s
 * Flowfield Option: 
  1. Open ETF: use the ETF of load image as flowfield
  2. Open Flow: there are some vfb in /data/

 * Start: use mouse left button to draw patten into current_B, use tool->Edge2addB and slide addB large than 0, can add edge of input image to result of reaction diffusion.


### [in post process]
beta to threshold the density to black and white


---

### [對應table 1的fig.5 6的產生]
fig5(a)(b)(c)(d): source.vfb, F k l照表設定(Size調到0.5左右)，用seed均勻灑顏料，穩定後用adathresholding(beta=0.8左右)
*fig5(d)theta0調成220度

---

###[Useful Tutorials]
#####[wxWidgets]
 * [How to compile a wxWidgets application in Visual Studio 2008?](
http://www.rhyous.com/2009/12/16/how-to-compile-a-wxwidgets-application-in-visual-studio-2008/)
 * [wxWidgets教學：編譯靜態LIB檔及第一個Hello World程式](
http://changyang319.pixnet.net/blog/post/26984931-wxwidgets%E6%95%99%E5%AD%B8%EF%BC%9A%E7%B7%A8%E8%AD%AF%E9%9D%9C%E6%85%8Blib%E6%AA%94%E5%8F%8A%E7%AC%AC%E4%B8%80%E5%80%8Bhello-world)

#####[OpenCV]
 * [Set the OpenCV enviroment variable and add it to the systems path](http://docs.opencv.org/doc/tutorials/introduction/windows_install/windows_install.html#windowssetpathandenviromentvariable)
 * [How to build applications with OpenCV inside the Microsoft Visual Studio](http://docs.opencv.org/doc/tutorials/introduction/windows_visual_studio_Opencv/windows_visual_studio_Opencv.html#windows-visual-studio-how-to)

#####[Others]
 - Q:error LNK2019
 - A:Windows, Open VS2013 Project settings -> Linker > General > System : change Subsystem to "Windows"
