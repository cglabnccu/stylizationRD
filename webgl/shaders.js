'use strict';
// GLSL shader sources — 對應 src/reactionDiffusion.cpp 的各個 AMP kernel 與 postProcessing.cpp
// =================================================================== shaders
const VS = `#version 300 es
void main(){
  vec2 p = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
  gl_Position = vec4(p*2.0-1.0, 0, 1);
}`;

// pass 1: alpha_A（各向異性函數 Eq.6/7）== RD::FastGrayScott 第一個 AMP kernel
const FS_ALPHA = `#version 300 es
precision highp float;
uniform sampler2D uState;   // rg = (A,B)
uniform sampler2D uFlow;    // rg = flow vector
uniform sampler2D uCtrl1;   // segmentation: (F, k, l, theta0)
uniform sampler2D uCtrl2;   // segmentation: (size, sd, dmin, dmax)
uniform bool uSegOn;
uniform float uL, uTheta0, uDmin, uDmax;
out vec4 o;
const float PI = 3.14159265358979;
ivec2 wrap(ivec2 p, ivec2 n){ return (p + n) % n; }
void main(){
  ivec2 n  = textureSize(uState,0);
  ivec2 xy = ivec2(gl_FragCoord.xy);
  float sx = 0.0, sy = 0.0;
  for(int i=-1;i<=1;i++){
    sx += texelFetch(uState, wrap(xy+ivec2( i, 1),n),0).r
        - texelFetch(uState, wrap(xy+ivec2( i,-1),n),0).r;
    sy += texelFetch(uState, wrap(xy+ivec2( 1, i),n),0).r
        - texelFetch(uState, wrap(xy+ivec2(-1, i),n),0).r;
  }
  vec2 f = texelFetch(uFlow, xy, 0).rg;
  float axb = length(vec2(sx,sy)) * length(f);
  float cosT = 0.0, cross_p = 0.0;
  if(axb > 0.0){
    cosT    = clamp((sx*f.x + sy*f.y)/axb, -1.0, 1.0);
    cross_p = (sx*f.y - sy*f.x)/axb;
  }
  float theta = acos(cosT);
  if(cross_p < 0.0) theta = 2.0*PI - theta;

  float l, th0, dmin, dmax;
  if(uSegOn){
    vec4 c1 = texelFetch(uCtrl1, xy, 0);
    vec4 c2 = texelFetch(uCtrl2, xy, 0);
    l = c1.z; th0 = c1.w; dmin = c2.z; dmax = c2.w;
  } else { l = uL; th0 = uTheta0; dmin = uDmin; dmax = uDmax; }

  bool inRange = (dmin <= dmax) ? (theta >= dmin && theta <= dmax)
                                : (theta >= dmin || (theta <= dmax && theta > 0.0));
  float temp = inRange ? 1.0 : 0.5*(1.0 + cos(l*(theta + th0)))*0.9 + 0.1;
  o = vec4(0.1/temp, 0.0, 0.0, 1.0);
}`;

// pass 2: 各向異性擴散 + Gray-Scott 反應 == 第二個 AMP kernel
const FS_UPDATE = `#version 300 es
precision highp float;
uniform sampler2D uState;
uniform sampler2D uAlpha;
uniform sampler2D uAdd;     // rg = (Addition_A, Addition_B)
uniform sampler2D uCtrlKS;  // rg = (k mask, size mask)  == Mask_control_k / _size
uniform sampler2D uCtrl1;   // segmentation
uniform sampler2D uCtrl2;
uniform bool uSegOn;
uniform float uF, uSd, uAddAw, uAddBw;
out vec4 o;
ivec2 wrap(ivec2 p, ivec2 n){ return (p + n) % n; }
void main(){
  ivec2 n  = textureSize(uState,0);
  ivec2 xy = ivec2(gl_FragCoord.xy);
  vec2  ab = texelFetch(uState, xy, 0).rg;
  float ac = texelFetch(uAlpha, xy, 0).r;
  float da = 0.0, db = 0.0;
  for(int q=-1;q<=1;q++)
  for(int p=-1;p<=1;p++){
    ivec2 j = wrap(xy+ivec2(p,q), n);
    vec2 s  = texelFetch(uState, j, 0).rg;
    float w = 0.5*(texelFetch(uAlpha, j, 0).r + ac);
    da += w*(s.r - ab.r);
    db += w*(s.g - ab.g);
  }
  da /= 8.0; db /= 8.0;

  vec4 ks = texelFetch(uCtrlKS, xy, 0);
  float F, kk, size, sdv;
  if(uSegOn){
    vec4 c1 = texelFetch(uCtrl1, xy, 0);
    vec4 c2 = texelFetch(uCtrl2, xy, 0);
    F = c1.x; kk = c1.y; size = c2.x; sdv = c2.y;
  } else { F = uF; kk = ks.r; size = ks.g; sdv = uSd; }

  float a = ab.r, b = ab.g;
  float DA = sdv*1.0*da;
  float DB = sdv*0.5*db;
  float RA = size*(-a*b*b + F*(1.0-a));
  float RB = size*( a*b*b - (kk+F)*b);
  vec2 add = texelFetch(uAdd, xy, 0).rg;
  float A2 = clamp(a + DA + RA, 0.0, 1.0);
  float B2 = clamp(b + DB + RB + 0.04*(uAddBw*add.g - uAddAw*add.r), 0.0, 1.0);
  o = vec4(A2, B2, 0.0, 1.0);
}`;

// 筆刷 / Fill Ink / Clean / Edge2Add / Mask2Add（線段 splat）
const FS_SPLAT = `#version 300 es
precision highp float;
uniform sampler2D uSrc;    // 目前的 state 或 additions
uniform sampler2D uMask;   // r=lum, g=edge
uniform int uMode;
uniform vec2 uP0, uP1;
uniform float uRadius, uRand;
out vec4 o;
float hash(vec2 p){ return fract(sin(dot(p, vec2(127.1,311.7)))*43758.5453); }
float distSeg(vec2 p, vec2 a, vec2 b){
  vec2 ab = b-a;
  float t = clamp(dot(p-a, ab)/max(dot(ab,ab),1e-6), 0.0, 1.0);
  return distance(p, a+t*ab);
}
void main(){
  vec2 xy = gl_FragCoord.xy;
  vec4 st = texelFetch(uSrc, ivec2(xy), 0);
  bool hit = distSeg(xy, uP0, uP1) <= uRadius;
  if(uMode == 0){ o = vec4(1,0,0,1); return; }                     // Clean state
  if(uMode == 7){ o = vec4(0,0,0,1); return; }                     // Clean additions
  if(uMode == 1){                                                  // Fill Ink == Seeds(25,true,.5)
    vec2 cell = floor(xy/50.0);
    vec2 c = cell*50.0 + 25.0 + (vec2(hash(cell+uRand), hash(cell+uRand+7.7))-0.5)*50.0;
    if(distance(xy, c) < uRadius) st.g = 0.5;
    o = st; return;
  }
  if(uMode == 2 && hit){ st.r = 0.0; st.g = 1.0; }                 // paint_to_B
  if(uMode == 3 && hit){ st.r = 0.0; }                             // paint_black → c_A=0
  if(uMode == 4 && hit){ st.r = 1.0; }                             // paint_white → c_A=1
  if(uMode == 5 && hit){ st.g = 1.0; }                             // → Addition_B
  if(uMode == 6 && hit){ st.r = 1.0; }                             // → Addition_A
  float edge = texelFetch(uMask, ivec2(xy), 0).g;
  float lum  = texelFetch(uMask, ivec2(xy), 0).r;
  if(uMode == 8){ st.r = edge; }                                   // Edge2AddA
  if(uMode == 9){ st.g = edge; }                                   // Edge2AddB
  if(uMode == 10){ st.r += 0.5*lum; }                              // Mask2AddA
  if(uMode == 11){ st.g += 0.5*lum; }                              // Mask2AddB
  o = st;
}`;

// k / size mask 均勻填滿 == RD::UpdatekMask / UpdateSizeMask
const FS_FILLKS = `#version 300 es
precision highp float;
uniform float uKv, uSv;
out vec4 o;
void main(){ o = vec4(uKv, uSv, 0.0, 1.0); }`;

// Gradient 筆刷 == RD::Gradient（Linear / Circular / Inverse Circular; target size / k）
const FS_GRADIENT = `#version 300 es
precision highp float;
uniform sampler2D uSrc;    // 現有 ctrlKS
uniform vec2 uStart, uEnd;
uniform int uType;         // 0 Linear, 1 Circular, 2 Inverse Circular
uniform int uTarget;       // 0 size, 1 k
out vec4 o;
void main(){
  vec2 p = gl_FragCoord.xy;
  vec4 ks = texelFetch(uSrc, ivec2(p), 0);
  float L = max(distance(uStart, uEnd), 1e-4);
  float v;
  if(uType == 1){                     // Circular
    float d = distance(p, uStart);
    if(uTarget == 0) v = (d > L) ? 1.0 : d/L;
    else             v = (d > L) ? 0.066 : 0.056 + (d/L)/100.0;
  } else if(uType == 2){              // Inverse Circular
    float d = distance(p, uStart);
    if(uTarget == 0) v = (d > L) ? 0.1 : 1.0 - d/L + 0.1;
    else             v = (d > L) ? 0.056 : 0.066 - (d/L)/100.0;
  } else {                            // Linear
    vec2 dir = (uEnd-uStart)/L;
    float t = dot(p-uStart, dir)/L;
    if(t < 0.0)      v = (uTarget==0) ? 0.1 : 0.056;
    else if(t > 1.0) v = (uTarget==0) ? 1.0 : 0.066;
    else             v = (uTarget==0) ? min(1.0, t+0.1) : 0.066 - (1.0-t)/100.0;
  }
  if(uTarget == 0) ks.g = v; else ks.r = v;
  o = ks;
}`;

// flow 產生（ETF）== RD::ETF；mode 2 = linear（Pattern Picker 預覽用）
const FS_FLOW = `#version 300 es
precision highp float;
uniform sampler2D uMask;
uniform int uMode;
out vec4 o;
ivec2 wrap(ivec2 p, ivec2 n){ return (p + n) % n; }
void main(){
  ivec2 n = textureSize(uMask,0);
  vec2 v;
  if(uMode == 2) v = vec2(1.0, 0.0);
  else {
    ivec2 xy = ivec2(gl_FragCoord.xy);
    float gx = 0.0, gy = 0.0;
    for(int i=-1;i<=1;i++){
      gx += texelFetch(uMask, wrap(xy+ivec2( 2, i*2),n),0).r
          - texelFetch(uMask, wrap(xy+ivec2(-2, i*2),n),0).r;
      gy += texelFetch(uMask, wrap(xy+ivec2( i*2, 2),n),0).r
          - texelFetch(uMask, wrap(xy+ivec2( i*2,-2),n),0).r;
    }
    vec2 g = vec2(gx, gy);
    v = (length(g) < 1e-4) ? vec2(0.0) : normalize(vec2(-g.y, g.x));
  }
  o = vec4(v, 0.0, 1.0);
}`;

const FS_FLOWSMOOTH = `#version 300 es
precision highp float;
uniform sampler2D uFlow;
out vec4 o;
ivec2 wrap(ivec2 p, ivec2 n){ return (p + n) % n; }
void main(){
  ivec2 n  = textureSize(uFlow,0);
  ivec2 xy = ivec2(gl_FragCoord.xy);
  vec2 c = texelFetch(uFlow, xy, 0).rg;
  vec2 sum = vec2(0.0);
  for(int q=-2;q<=2;q++)
  for(int p=-2;p<=2;p++){
    vec2 v = texelFetch(uFlow, wrap(xy+ivec2(p,q),n),0).rg;
    sum += (dot(v,c) < 0.0) ? -v : v;
  }
  o = vec4((length(sum)<1e-5) ? c : normalize(sum), 0.0, 1.0);
}`;

// == RD::RotateFlow
const FS_ROTFLOW = `#version 300 es
precision highp float;
uniform sampler2D uFlow;
uniform float uAng;
out vec4 o;
void main(){
  vec2 v = texelFetch(uFlow, ivec2(gl_FragCoord.xy), 0).rg;
  float c = cos(uAng), s = sin(uAng);
  o = vec4(v.x*c - v.y*s, v.y*c + v.x*s, 0.0, 1.0);
}`;

// 顯示 + 後處理 == BasicDrawPane::render 的 processingS 分支（postProcessing.cpp）
const FS_DISPLAY = `#version 300 es
precision highp float;
uniform sampler2D uState, uMask, uOrig, uTex, uFlow, uNoise;
uniform int uMode;       // 0 dA 1 dB 2 LIC 3 motion 4 tex 5 dirTex 6 polar 7 ada 8 thresh 9 cmap
uniform int uCmMode;     // 1..4
uniform bool uIsAda, uDispRegion;
uniform float uAlpha, uBeta, uRegionGray;
out vec4 o;
const float PI = 3.14159265358979;
ivec2 wrap(ivec2 p, ivec2 n){ return (p + n) % n; }
float cosGF(ivec2 xy, ivec2 n){   // gradient(A) 與 flow 的夾角 cos == SeedGradient + dot
  float gx=0.0, gy=0.0;
  for(int i=-1;i<=1;i++){
    float w = (i==0)?2.0:1.0;
    gx += w*(texelFetch(uState,wrap(xy+ivec2( 1,i),n),0).r - texelFetch(uState,wrap(xy+ivec2(-1,i),n),0).r);
    gy += w*(texelFetch(uState,wrap(xy+ivec2( i,1),n),0).r - texelFetch(uState,wrap(xy+ivec2( i,-1),n),0).r);
  }
  vec2 g = vec2(gx,gy), f = texelFetch(uFlow,xy,0).rg;
  float lg = length(g)*length(f);
  return (lg < 1e-9) ? 0.0 : clamp(dot(g,f)/lg, -1.0, 1.0);
}
void main(){
  ivec2 n  = textureSize(uState,0);
  ivec2 xy = ivec2(gl_FragCoord.xy);
  float A = texelFetch(uState, xy, 0).r;
  float B = texelFetch(uState, xy, 0).g;
  float mask = texelFetch(uMask, xy, 0).r;
  vec3 orig  = texelFetch(uOrig, xy, 0).rgb;
  vec3 col;
  if(uMode == 0) col = vec3(A);
  else if(uMode == 1) col = vec3(B);
  else if(uMode == 2){                       // LIC
    float acc = 0.0, wsum = 0.0;
    for(int dir=0; dir<2; dir++){
      vec2 p = vec2(xy);
      for(int k2=0; k2<10; k2++){
        vec2 v = texelFetch(uFlow, wrap(ivec2(p),n),0).rg;
        if(dir==1) v = -v;
        float sv = abs(v.x)+abs(v.y);
        if(sv > 1e-6){
          p.x += abs(v.x)/sv*sign(v.x);
          p.y += abs(v.y)/sv*sign(v.y);
        }
        float w = exp(-float(k2*k2)/200.0);
        acc += w*texelFetch(uNoise, wrap(ivec2(p),n),0).r;
        wsum += w;
      }
    }
    col = vec3(acc/wsum);
  }
  else if(uMode == 3){                       // Motion Illusion（簡化版：無 distance transform）
    float ct = cosGF(xy,n);
    float range = uAlpha/2.0, center = uBeta;
    if(A > center+range)      col = vec3(1.0,1.0,0.0);
    else if(A < center-range) col = vec3(0.0,0.0,1.0);
    else                      col = vec3(ct > 0.0 ? 0.0 : 1.0);
  }
  else if(uMode == 4){                       // Texture
    float x = clamp((uAlpha+0.5)*(A+(uBeta-0.5)), 0.0, 1.0);
    col = texture(uTex, vec2(x, 0.5)).rgb;
  }
  else if(uMode == 5){                       // dirTexture
    float ct = cosGF(xy,n);
    col = texture(uTex, vec2(clamp(acos(ct)/PI,0.0,1.0), clamp(A,0.0,1.0))).rgb;
  }
  else if(uMode == 6){                       // PolarTexture
    float ct = cosGF(xy,n);
    float R = 1.0-A;
    vec2 t = vec2(0.5+uAlpha+uBeta*R*ct, 0.5+uAlpha+uBeta*R*sin(acos(ct)));
    col = texture(uTex, clamp(t, 0.0, 1.0)).rgb;
  }
  else if(uMode == 7){                       // adaThresholding
    float c = (1.0-mask)*uBeta + mask*uAlpha, r = 0.05;
    col = vec3(clamp((A-(c-r))/(r*2.0), 0.0, 1.0));
  }
  else if(uMode == 8){                       // Thresholding
    float r = uAlpha/2.0, c = uBeta;
    col = vec3(clamp((A-(c-r))/max(r*2.0,1e-5), 0.0, 1.0));
  }
  else {                                     // Color_mapping (mode 1-4, isAda)
    float c, r;
    if(uIsAda){ c = (1.0-mask)*uBeta + mask*uAlpha; r = 0.0; }
    else      { r = uAlpha/2.0; c = uBeta; }
    if(A > c+r){
      if(uCmMode == 1)      col = 0.5 + 0.5*orig;
      else if(uCmMode == 2) col = vec3(1.0);
      else                  col = orig;
    } else if(A > c-r){
      float a = 1.0 - (A-(c-r))/max(r*2.0, 1e-5);
      if(uCmMode == 1){ a = max(a, 0.5); col = (1.0-a) + a*orig; }
      else if(uCmMode == 2) col = (1.0-a) + a*orig;
      else if(uCmMode == 3){ a = 1.0-a; col = (1.0-a) + a*orig; }
      else col = vec3(0.0);
    } else {
      if(uCmMode <= 2)      col = orig;
      else if(uCmMode == 3) col = vec3(1.0);
      else                  col = vec3(0.0);
    }
  }
  if(uDispRegion){                           // == RD::DisplaySeg 棋盤格
    float g255 = texelFetch(uMask, xy, 0).b*255.0;
    if(abs(g255-uRegionGray) > 5.0){
      int qx = (xy.x/3) % 2, qy = (xy.y/3) % 2;
      col = vec3((qx==qy) ? 0.5 : 1.0);
    }
  }
  o = vec4(col, 1.0);
}`;
