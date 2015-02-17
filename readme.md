# StylizationRD
#### Image stylization using anisotropic reaction diffusion
###### Keywords: image stylization · reaction diffusion · pattern generation

---


### [Required]
 * <a href="https://msdn.microsoft.com/zh-tw/vstudio/aa718325.aspx" target="_blank">Visual Studio</a>(2013 recommand)
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
  * Segmentation
   * size
  * Demo Film
  * [BUG] CAF degree selection

### [Future Work]
  1. more UI friendly
   * Customize Anisotropic Function degree selection
   * Pattern Picker info & size slidebar
   * ???
  2. using faster algorithm
  3. oriental cloud patterns
  4. merge flowfield maker
  5. ???

---

### [Papers' Examples]
|Fig.|Workflow |Remark/Notes　　　　　|
|----------------|:--------|:--------------------- |
|5(a)| **Load Flow** `source.vfb`, Parameters F k l can reference to Table 1, Press **Start** and click **Fill Ink**, after its being stable, select **adathresholding** (beta=0.8)|--|
|5(b)| as above |--|
|5(c)| as above |--|
|5(d)| as above, adjust **theta0** roughly to 220 degrees|--|
|6(a)| **Load Flow** `vortex.vfb`, Parameters F k l can reference to Table 1, check the **Customize Anisotropic Function** to YES, and select 0~180 degree. Then press **Start** and click **Fill Ink**, after its being stable, select **adathresholding** (beta=0.8, alpha=0.1)|--|
|6(b)| as above, **Customize Anisotropic Function** select degree 45~135.|--|
|8(c)| **Load SrcImg** `controlshape-a.png`, **Load Flow** `vortex.vfb`, **Load Control Img** `controlshape_controlImg.png`. Press **Start**, Pattern Size = 0.1, then check the **Activate Segmentation** to YES, and assign each region with its own parameters(recommanded pattern provide at Note). After its stable , select **Thresholding** (alpha=0.4 beta=0.5).|Region<br/>1:SPOT(theta0=90)<br/>2:HOLE(theta0=90)<br/>3:SPOT(theta0=90)<br/>4:LINE(theta0=90)<br/>5:inverse LINE(theta0=90)<br/>|
|12| **Load SrcImg** `cat-source.png`, **Load ETF** `cat-source.png`, The Parameters can use **Pattern Picker** *(Tool>Open Pattern Picker)* to select SQUARE pattern, then Press **Start** and click **Fill Ink** and click **Edge2AddB** *(Tool>Edge2AddB)*, after its being stable, select **adathresholding** (alpha=0.4 beta=0.0)|
|17(c)| **Load SrcImg** `Kremlin-source.png`, **Load ETF** `Kremlin-source.png`, **Load Control Img** `Kremlin-controlImg.png`. Press **Start** and click **Edge2AddB** (addB=0.03), then check the **Activate Segmentation** to YES, and assign each region with its own parameters(recommanded pattern provide at Note). After its stable , select **Thresholding** (alpha=0.05 beta=0.4).|Region<br/>1:SPOT<br/>2~3: BLACK<br/>4~8: LINE<br/>   |
|17(d)| as above.Pattern Size = 0.2 After its stable , select **adaThresholding** (alpha=0.36 beta=0.4).|Regions<br/>1~3: HOLE<br/>4~8: QUADE(theta0=45)<br/>   |
---

### [Table 1]
|Style       | a(theta)| F    | k    | l   |Figure      |
| ---------- | ------- | ---- | ---- | --- | ---------- |
|semicircle  |    Eq. 6| .0375| .0655| 1   |Fig. 5(a)   |
|spot: spidle|    Eq. 6| .0375| .0655| 2   |Fig. 5(b)   |
|hole: spidle|    Eq. 6| .0300| .0546| 2   |Fig. 8 blue |
|stripe      |    Eq. 6| .0300| .0620| 2   |Fig. 8 green|
|triangle    |    Eq. 6| .0375| .0655| 3   |Fig. 5(c)   |
|spot: quad  |    Eq. 6| .0375| .0655| 4   |Fig. 5(d)   |
|water drop  |    Eq. 7| .0375| .0655| 2   |Fig. 6(a)   |
|sector      |    Eq. 7| .0375| .0655| 4   |Fig. 6(b)   |


---

###[Common Q&A]
<dl>
  <dt>Q: Error LNK2019(Build Error)</dt>
  <dd>Open VS2013 Project settings -> Linker > General > System : change Subsystem to "Windows".</dd>

  <dt>Q: Unable to startup program (The system cannot find the file specified)(Build Error)</dt>
  <dd>Solution Explorer -> Select "FlowBase" -> Right Click and "Set as StartUp Project"</dd>
</dl>
---
###[Useful Tutorials]
#####[wxWidgets]
 * <a href="http://www.rhyous.com/2009/12/16/how-to-compile-a-wxwidgets-application-in-visual-studio-2008/" target="_blank">How to compile a wxWidgets application in Visual Studio 2008?</a>
 * <a href="http://changyang319.pixnet.net/blog/post/26984931-wxwidgets%E6%95%99%E5%AD%B8%EF%BC%9A%E7%B7%A8%E8%AD%AF%E9%9D%9C%E6%85%8Blib%E6%AA%94%E5%8F%8A%E7%AC%AC%E4%B8%80%E5%80%8Bhello-world" target="_blank">wxWidgets教學：編譯靜態LIB檔及第一個Hello World程式</a>
 * <a href="http://sourceforge.net/projects/wxformbuilder/?source=typ_redirect" target="_blank">wxFormBuilder - a RAD tool for wxWidgets GUI design.</a>



#####[OpenCV]
 * <a href="http://docs.opencv.org/doc/tutorials/introduction/windows_install/windows_install.html#windowssetpathandenviromentvariable" target="_blank">Set the OpenCV enviroment variable and add it to the systems path</a>
 * <a href="http://docs.opencv.org/doc/tutorials/introduction/windows_visual_studio_Opencv/windows_visual_studio_Opencv.html#windows-visual-studio-how-to" target="_blank">How to build applications with OpenCV inside the Microsoft Visual Studio</a>
