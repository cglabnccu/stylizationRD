'use strict';
// =================================================================== GL setup
const canvas = document.getElementById('c');
const gl = canvas.getContext('webgl2', {preserveDrawingBuffer:true});
if(!gl){ alert('需要 WebGL2 支援'); throw new Error('no webgl2'); }
if(!gl.getExtension('EXT_color_buffer_float')){ alert('需要 EXT_color_buffer_float'); throw new Error('no float fbo'); }

function compileProg(fs){
  const mk = (t,src)=>{ const s=gl.createShader(t); gl.shaderSource(s,src); gl.compileShader(s);
    if(!gl.getShaderParameter(s,gl.COMPILE_STATUS)) throw new Error(gl.getShaderInfoLog(s)); return s; };
  const p = gl.createProgram();
  gl.attachShader(p, mk(gl.VERTEX_SHADER, VS));
  gl.attachShader(p, mk(gl.FRAGMENT_SHADER, fs));
  gl.linkProgram(p);
  if(!gl.getProgramParameter(p,gl.LINK_STATUS)) throw new Error(gl.getProgramInfoLog(p));
  return p;
}
const progAlpha   = compileProg(FS_ALPHA);
const progUpdate  = compileProg(FS_UPDATE);
const progSplat   = compileProg(FS_SPLAT);
const progFillKS  = compileProg(FS_FILLKS);
const progGrad    = compileProg(FS_GRADIENT);
const progFlow    = compileProg(FS_FLOW);
const progSmooth  = compileProg(FS_FLOWSMOOTH);
const progRot     = compileProg(FS_ROTFLOW);
const progDisplay = compileProg(FS_DISPLAY);
const U = (p,n) => gl.getUniformLocation(p,n);

let W = 256, H = 256;
let texState, fboState, cur = 0;
let texAlpha, fboAlpha, texAdd, fboAdd, texKS, fboKS;
let texFlow, fboFlow, texMask, texOrig, texTex = null, texNoise, texCtrl1, texCtrl2, texETFsrc;
let hasSrc = false;
let curAdd = 0, curKS = 0, curFlow = 0;
const D2R = Math.PI/180;

function makeTexN(w, h, internal, fmt, type, data, filter){
  const t = gl.createTexture();
  gl.bindTexture(gl.TEXTURE_2D, t);
  gl.texImage2D(gl.TEXTURE_2D, 0, internal, w, h, 0, fmt, type, data || null);
  const f = filter || gl.NEAREST;
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, f);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, f);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
  return t;
}
const makeTex = (internal, fmt, type) => makeTexN(W, H, internal, fmt, type);
function makeFbo(tex){
  const f = gl.createFramebuffer();
  gl.bindFramebuffer(gl.FRAMEBUFFER, f);
  gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, tex, 0);
  return f;
}
function bindTex(prog, name, unit, tex){
  gl.activeTexture(gl.TEXTURE0 + unit);
  gl.bindTexture(gl.TEXTURE_2D, tex);
  gl.uniform1i(U(prog, name), unit);
}
function drawN(prog, fbo, w, h){
  gl.useProgram(prog);
  gl.bindFramebuffer(gl.FRAMEBUFFER, fbo);
  gl.viewport(0, 0, w, h);
  gl.drawArrays(gl.TRIANGLES, 0, 3);
}
const draw = (prog, fbo) => drawN(prog, fbo, W, H);

function allocate(){
  canvas.width = W; canvas.height = H;
  texState = [makeTex(gl.RGBA16F, gl.RGBA, gl.HALF_FLOAT), makeTex(gl.RGBA16F, gl.RGBA, gl.HALF_FLOAT)];
  fboState = [makeFbo(texState[0]), makeFbo(texState[1])];
  texAlpha = makeTex(gl.RGBA16F, gl.RGBA, gl.HALF_FLOAT);  fboAlpha = makeFbo(texAlpha);
  texAdd   = [makeTex(gl.RGBA16F, gl.RGBA, gl.HALF_FLOAT), makeTex(gl.RGBA16F, gl.RGBA, gl.HALF_FLOAT)];
  fboAdd   = [makeFbo(texAdd[0]), makeFbo(texAdd[1])];
  texKS    = [makeTex(gl.RGBA16F, gl.RGBA, gl.HALF_FLOAT), makeTex(gl.RGBA16F, gl.RGBA, gl.HALF_FLOAT)];
  fboKS    = [makeFbo(texKS[0]), makeFbo(texKS[1])];
  texFlow  = [makeTex(gl.RGBA32F, gl.RGBA, gl.FLOAT), makeTex(gl.RGBA32F, gl.RGBA, gl.FLOAT)];
  fboFlow  = [makeFbo(texFlow[0]), makeFbo(texFlow[1])];
  texMask  = makeTex(gl.RGBA8, gl.RGBA, gl.UNSIGNED_BYTE);
  texOrig  = makeTex(gl.RGBA8, gl.RGBA, gl.UNSIGNED_BYTE);
  texCtrl1 = makeTex(gl.RGBA32F, gl.RGBA, gl.FLOAT);
  texCtrl2 = makeTex(gl.RGBA32F, gl.RGBA, gl.FLOAT);
  texETFsrc = makeTex(gl.RGBA8, gl.RGBA, gl.UNSIGNED_BYTE);
  // LIC noise（半解析度 nearest 放大 == PP::LIC）
  const nz = new Uint8Array(W*H*4);
  for(let y=0;y<H;y+=2) for(let x=0;x<W;x+=2){
    const v = Math.random()*255;
    for(let dy=0;dy<2 && y+dy<H;dy++) for(let dx=0;dx<2 && x+dx<W;dx++)
      nz[((y+dy)*W+(x+dx))*4] = v;
  }
  texNoise = makeTexN(W, H, gl.RGBA8, gl.RGBA, gl.UNSIGNED_BYTE, nz);
  cur = 0; curAdd = 0; curKS = 0; curFlow = 0;
  cleanState();
  fillKS();
}

// =================================================================== UI helpers
const $ = id => document.getElementById(id);
function addlog(msg, color){
  const d = document.createElement('div');
  d.textContent = msg;
  if(color) d.style.color = color;
  $('log').appendChild(d);
  $('log').scrollTop = $('log').scrollHeight;
}
function setStatus(id, txt){ $(id).textContent = txt; }

// slider 全域參數（== drawPane->element 成員）
const P = { brushSize:4, addA:0.5, addB:0.5, s:0.6, sd:0.3, F:0.0375, k:0.0589,
            l:0, theta0:0, mindeg:0, maxdeg:0, alpha:0.2, beta:0.5 };
const FMT = { brushSize:v=>`Brush Size : ${v}`, addA:v=>`addA : ${(+v).toFixed(3)}`,
  addB:v=>`addB : ${(+v).toFixed(3)}`, s:v=>`Size : ${(+v).toFixed(3)}`,
  sd:v=>`Spacing : ${(+v).toFixed(3)}`, F:v=>`F : ${(+v).toFixed(4)}`,
  k:v=>`k : ${(+v).toFixed(4)}`, l:v=>`l : ${v}`, theta0:v=>`theta0 : ${v}`,
  mindeg:v=>`min degree : ${v}`, maxdeg:v=>`max degree : ${v}`,
  alpha:v=>`alpha : ${(+v).toFixed(3)}`, beta:v=>`beta : ${(+v).toFixed(3)}` };

// =================================================================== state ops
function splatState(mode, p0, p1){
  gl.useProgram(progSplat);
  bindTex(progSplat, 'uSrc', 0, texState[cur]);
  bindTex(progSplat, 'uMask', 1, texMask);
  gl.uniform1i(U(progSplat,'uMode'), mode);
  gl.uniform2f(U(progSplat,'uP0'), p0?p0[0]:0, p0?p0[1]:0);
  gl.uniform2f(U(progSplat,'uP1'), p1?p1[0]:0, p1?p1[1]:0);
  gl.uniform1f(U(progSplat,'uRadius'), P.brushSize);
  gl.uniform1f(U(progSplat,'uRand'), Math.random()*100);
  draw(progSplat, fboState[1-cur]); cur = 1-cur;
}
function splatAdd(mode, p0, p1){
  gl.useProgram(progSplat);
  bindTex(progSplat, 'uSrc', 0, texAdd[curAdd]);
  bindTex(progSplat, 'uMask', 1, texMask);
  gl.uniform1i(U(progSplat,'uMode'), mode);
  gl.uniform2f(U(progSplat,'uP0'), p0?p0[0]:0, p0?p0[1]:0);
  gl.uniform2f(U(progSplat,'uP1'), p1?p1[0]:0, p1?p1[1]:0);
  gl.uniform1f(U(progSplat,'uRadius'), P.brushSize);
  gl.uniform1f(U(progSplat,'uRand'), 0);
  draw(progSplat, fboAdd[1-curAdd]); curAdd = 1-curAdd;
}
function cleanState(){ splatState(0); splatAdd(7); }
function fillKS(){    // == UpdatekMask + UpdateSizeMask
  gl.useProgram(progFillKS);
  gl.uniform1f(U(progFillKS,'uKv'), P.k);
  gl.uniform1f(U(progFillKS,'uSv'), 1.0 - P.s);
  draw(progFillKS, fboKS[curKS]);
}
function applyGradient(target, start, end, type){  // == RD::Gradient
  gl.useProgram(progGrad);
  bindTex(progGrad, 'uSrc', 0, texKS[curKS]);
  gl.uniform2f(U(progGrad,'uStart'), start[0], start[1]);
  gl.uniform2f(U(progGrad,'uEnd'), end[0], end[1]);
  gl.uniform1i(U(progGrad,'uType'), {Linear:0, Circular:1, 'Inverse Circular':2}[type]);
  gl.uniform1i(U(progGrad,'uTarget'), target === 'size' ? 0 : 1);
  draw(progGrad, fboKS[1-curKS]); curKS = 1-curKS;
}
function rotateFlow(deg){   // == RD::RotateFlow
  gl.useProgram(progRot);
  bindTex(progRot, 'uFlow', 0, texFlow[curFlow]);
  gl.uniform1f(U(progRot,'uAng'), deg*D2R);
  draw(progRot, fboFlow[1-curFlow]); curFlow = 1-curFlow;
}

// =================================================================== undo / redo
const undoStack = [], redoStack = [];
function snapshotTex(tex){
  const f = gl.createFramebuffer();
  gl.bindFramebuffer(gl.FRAMEBUFFER, f);
  gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, tex, 0);
  const buf = new Float32Array(W*H*4);
  gl.readPixels(0, 0, W, H, gl.RGBA, gl.FLOAT, buf);
  gl.deleteFramebuffer(f);
  return buf;
}
function restoreTex(tex, buf){
  gl.bindTexture(gl.TEXTURE_2D, tex);
  gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA16F, W, H, 0, gl.RGBA, gl.FLOAT, buf);
}
function takeSnapshot(){
  return { w:W, state: snapshotTex(texState[cur]), add: snapshotTex(texAdd[curAdd]), ks: snapshotTex(texKS[curKS]) };
}
function applySnapshot(s){
  if(s.w !== W) return;
  restoreTex(texState[cur], s.state);
  restoreTex(texAdd[curAdd], s.add);
  restoreTex(texKS[curKS], s.ks);
}
function pushUndo(){
  undoStack.push(takeSnapshot());
  if(undoStack.length > 6) undoStack.shift();
  redoStack.length = 0;
  updateUndoBtns();
}
function updateUndoBtns(){
  $('btnUndo').disabled = undoStack.length === 0;
  $('btnRedo').disabled = redoStack.length === 0;
}
$('btnUndo').addEventListener('click', () => {
  if(!undoStack.length) return;
  redoStack.push(takeSnapshot());
  applySnapshot(undoStack.pop());
  updateUndoBtns();
});
$('btnRedo').addEventListener('click', () => {
  if(!redoStack.length) return;
  undoStack.push(takeSnapshot());
  applySnapshot(redoStack.pop());
  updateUndoBtns();
});

// =================================================================== segmentation
// == vector<PixelPattern> + RD::UpdateControlMask
let regions = [];
let ctrlGray = null;   // Mask_control（0-255）
function rebuildControlMask(){
  if(!ctrlGray) return;
  const c1 = new Float32Array(W*H*4), c2 = new Float32Array(W*H*4);
  for(let i=0;i<W*H;i++){
    const g = ctrlGray[i];
    for(const r of regions){
      if(g >= r.gray-5 && g <= r.gray+5){
        c1[i*4  ] = r.F;
        c1[i*4+1] = r.k;
        c1[i*4+2] = r.l;
        c1[i*4+3] = r.theta0*D2R;
        c2[i*4  ] = 1 - r.size;
        c2[i*4+1] = 0.7 + r.sd;
        c2[i*4+2] = r.dmin*D2R;
        c2[i*4+3] = r.dmax*D2R;
        break;
      }
    }
  }
  gl.bindTexture(gl.TEXTURE_2D, texCtrl1);
  gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA32F, W, H, 0, gl.RGBA, gl.FLOAT, c1);
  gl.bindTexture(gl.TEXTURE_2D, texCtrl2);
  gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA32F, W, H, 0, gl.RGBA, gl.FLOAT, c2);
}
function selectedRegion(){
  const i = $('segBox').selectedIndex;
  return (i >= 0 && i < regions.length) ? regions[i] : null;
}

// =================================================================== sliders
for(const id of Object.keys(P)){
  $(id).addEventListener('input', () => {
    const v = parseFloat($(id).value);
    if($(id+'_t')) $(id+'_t').textContent = FMT[id]($(id).value);
    // segmentation on：滑桿改的是選中 region 的參數（== C++ OnSlider* 分支）
    const segEditing = $('segCb').checked && selectedRegion() &&
                       ['F','k','l','s','sd','theta0','mindeg','maxdeg'].includes(id);
    if(segEditing){
      const r = selectedRegion();
      if(id==='s') r.size = v;
      else if(id==='mindeg') r.dmin = v;
      else if(id==='maxdeg') r.dmax = v;
      else r[id] = v;
      rebuildControlMask();
    } else {
      P[id] = v;
      if(id === 'k' || id === 's') fillKS();
    }
  });
  if($(id+'_t')) $(id+'_t').textContent = FMT[id]($(id).value);
}

// =================================================================== menus
document.querySelectorAll('.menu > button').forEach(b => {
  b.addEventListener('click', e => {
    const m = b.parentElement;
    document.querySelectorAll('.menu').forEach(x => { if(x!==m) x.classList.remove('open'); });
    m.classList.toggle('open');
    e.stopPropagation();
  });
});
document.addEventListener('click', () => document.querySelectorAll('.menu').forEach(m => m.classList.remove('open')));
document.querySelectorAll('.menu li[data-act]').forEach(li => {
  li.addEventListener('click', () => menuAction(li.dataset.act));
});
function menuAction(act){
  switch(act){
    case 'openSrc':  $('fileSrc').click(); break;
    case 'openVfb':  $('fileVfb').click(); break;
    case 'openETF':  $('fileETF').click(); break;
    case 'openTex':  $('fileTex').click(); break;
    case 'openCtrl': $('fileCtrl').click(); break;
    case 'openSize': $('fileSize').click(); break;
    case 'save': {
      const a = document.createElement('a');
      a.download = 'result.png';
      a.href = canvas.toDataURL('image/png');
      a.click();
      addlog('Result saved.');
      break;
    }
    case 'edge2AddA': pushUndo(); splatAdd(8);  addlog('Edge2AddA.'); break;
    case 'edge2AddB': pushUndo(); splatAdd(9);  addlog('Edge2AddB.'); break;
    case 'mask2AddA': pushUndo(); splatAdd(10); addlog('Mask2AddA.'); break;
    case 'mask2AddB': pushUndo(); splatAdd(11); addlog('Mask2AddB.'); break;
    case 'picker': openPicker(); break;
    case 'about':
      alert('Image stylization using anisotropic reaction diffusion\nChi, Liu, Hsu — The Visual Computer 32(12), 2016\nWebGL port of the original C++/AMP system.');
      break;
    case 'log': $('log').classList.toggle('hidden'); break;
  }
}

// =================================================================== file loads
function lumOf(imgData, w, h){
  const lum = new Float32Array(w*h);
  for(let i=0;i<w*h;i++)
    lum[i] = (0.299*imgData.data[i*4] + 0.587*imgData.data[i*4+1] + 0.114*imgData.data[i*4+2])/255;
  return lum;
}
function drawToImageData(bmp, w, h){
  const cv = document.createElement('canvas');
  cv.width = w; cv.height = h;
  const ctx = cv.getContext('2d');
  ctx.drawImage(bmp, 0, 0, w, h);
  return ctx.getImageData(0, 0, w, h);
}
// == RD::ReadSrc：Mask（亮度 normalize）、Mask_s（sobel edge）、Original_img
function loadSrc(bmp, name){
  W = Math.min(bmp.width, 1024); H = Math.min(bmp.height, 1024);
  allocate();
  const img = drawToImageData(bmp, W, H);
  const lum = lumOf(img, W, H);
  let lo=1e9, hi=-1e9;
  for(const v of lum){ if(v<lo)lo=v; if(v>hi)hi=v; }
  const rng = Math.max(hi-lo, 1e-6);
  const at = (x,y) => lum[((y+H)%H)*W + ((x+W)%W)];
  const edge = new Float32Array(W*H);
  let emax = 1e-6;
  for(let y=0;y<H;y++) for(let x=0;x<W;x++){
    const gx = at(x+1,y-1)+2*at(x+1,y)+at(x+1,y+1) - at(x-1,y-1)-2*at(x-1,y)-at(x-1,y+1);
    const gy = at(x-1,y+1)+2*at(x,y+1)+at(x+1,y+1) - at(x-1,y-1)-2*at(x,y-1)-at(x+1,y-1);
    const e2 = 0.5*Math.abs(gx) + 0.5*Math.abs(gy);
    edge[y*W+x] = e2; if(e2>emax) emax = e2;
  }
  const m = new Uint8Array(W*H*4);
  for(let y=0;y<H;y++) for(let x=0;x<W;x++){
    const i = y*W+x, j = (H-1-y)*W+x;    // GL flip
    m[i*4  ] = 255*(lum[j]-lo)/rng;
    m[i*4+1] = 255*edge[j]/emax;
    m[i*4+2] = 0;
    m[i*4+3] = 255;
  }
  gl.bindTexture(gl.TEXTURE_2D, texMask);
  gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA8, W, H, 0, gl.RGBA, gl.UNSIGNED_BYTE, m);
  const flip = new Uint8Array(W*H*4);
  for(let y=0;y<H;y++) flip.set(img.data.subarray((H-1-y)*W*4, (H-y)*W*4), y*W*4);
  gl.bindTexture(gl.TEXTURE_2D, texOrig);
  gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA8, W, H, 0, gl.RGBA, gl.UNSIGNED_BYTE, flip);
  hasSrc = true;
  $('btnFill').disabled = false;
  ctrlGray = null; regions = [];
  undoStack.length = 0; redoStack.length = 0; updateUndoBtns();
  setStatus('stSrc', `SrcImg: ${name} (${W}x${H})`);
  addlog('SrcImg loaded: ' + name);
}
// == RD::ETF（簡化：亮度梯度的垂直方向 + 方向平滑）
function loadETF(bmp, name){
  const img = drawToImageData(bmp, W, H);
  const m = new Uint8Array(W*H*4);
  for(let y=0;y<H;y++) for(let x=0;x<W;x++){
    const i = y*W+x, j = ((H-1-y)*W+x)*4;
    m[i*4] = 0.299*img.data[j] + 0.587*img.data[j+1] + 0.114*img.data[j+2];
    m[i*4+3] = 255;
  }
  gl.bindTexture(gl.TEXTURE_2D, texETFsrc);
  gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA8, W, H, 0, gl.RGBA, gl.UNSIGNED_BYTE, m);
  gl.useProgram(progFlow);
  bindTex(progFlow, 'uMask', 0, texETFsrc);
  gl.uniform1i(U(progFlow,'uMode'), 3);
  draw(progFlow, fboFlow[curFlow]);
  for(let i=0;i<6;i++){
    gl.useProgram(progSmooth);
    bindTex(progSmooth, 'uFlow', 0, texFlow[curFlow]);
    draw(progSmooth, fboFlow[1-curFlow]); curFlow = 1-curFlow;
  }
  setStatus('stFlow', `Flowfield: ETF(${name})`);
  addlog('ETF loaded: ' + name);
}
// == RD::ReadFlow（.vfb: int32 w, int32 h, float (dy,dx) 交錯）
async function loadVfb(file){
  const buf = await file.arrayBuffer();
  const dv = new DataView(buf);
  const vw = dv.getInt32(0, true), vh = dv.getInt32(4, true);
  if(vw <= 0 || vh <= 0 || 8 + vw*vh*8 > buf.byteLength){
    addlog("ERROR!! Can't read " + file.name, '#ff6b6b'); return;
  }
  const data = new Float32Array(buf, 8, vw*vh*2);
  const flow = new Float32Array(W*H*4);
  for(let g=0; g<H; g++){
    const sj = Math.min(vh-1, Math.round(g*(vh-1)/Math.max(H-1,1)));
    for(let x=0; x<W; x++){
      const si = Math.min(vw-1, Math.round(x*(vw-1)/Math.max(W-1,1)));
      const idx = sj*vw + si;
      flow[(g*W+x)*4  ] = data[idx*2+1];    // dx
      flow[(g*W+x)*4+1] = -data[idx*2];     // -dy（同 ReadFlow）
    }
  }
  gl.bindTexture(gl.TEXTURE_2D, texFlow[curFlow]);
  gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA32F, W, H, 0, gl.RGBA, gl.FLOAT, flow);
  setStatus('stFlow', 'Flowfield: ' + file.name);
  addlog(`Flow loaded: ${file.name} (${vw}x${vh})`);
}
// == PP::ReadTexture
function loadTex(bmp, name){
  const w = Math.min(bmp.width, 512), h = Math.min(bmp.height, 512);
  const img = drawToImageData(bmp, w, h);
  const flip = new Uint8Array(w*h*4);
  for(let y=0;y<h;y++) flip.set(img.data.subarray((h-1-y)*w*4, (h-y)*w*4), y*w*4);
  texTex = makeTexN(w, h, gl.RGBA8, gl.RGBA, gl.UNSIGNED_BYTE, flip, gl.LINEAR);
  setStatus('stTex', 'Texture: ' + name);
  addlog('Texture loaded: ' + name);
}
// == RD::ReadControlImg（灰階分區，容差 5，剔除 <1% 面積）
function loadCtrl(bmp, name){
  const img = drawToImageData(bmp, W, H);
  ctrlGray = new Uint8Array(W*H);
  for(let y=0;y<H;y++) for(let x=0;x<W;x++){
    const j = ((H-1-y)*W+x)*4;
    ctrlGray[y*W+x] = Math.round(0.299*img.data[j] + 0.587*img.data[j+1] + 0.114*img.data[j+2]);
  }
  const counts = new Map();
  regions = [];
  for(const g of ctrlGray){
    let r = regions.find(r => g >= r.gray-5 && g <= r.gray+5);
    if(!r){ r = { gray:g, F:P.F, k:P.k, l:P.l, size:P.s, sd:P.sd, theta0:P.theta0, dmin:0, dmax:0 }; regions.push(r); }
    counts.set(r, (counts.get(r)||0)+1);
  }
  regions = regions.filter(r => (counts.get(r)||0) >= W*H*0.01);
  regions.sort((a,b) => a.gray-b.gray);
  $('segBox').innerHTML = regions.map((r,i) => `<option>Region ${i+1} (gray ${r.gray})</option>`).join('');
  // gray 存進 texMask.b 供 Display Region 用
  const m = new Uint8Array(W*H*4);
  {
    const f = gl.createFramebuffer();
    gl.bindFramebuffer(gl.FRAMEBUFFER, f);
    gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, texMask, 0);
    gl.readPixels(0, 0, W, H, gl.RGBA, gl.UNSIGNED_BYTE, m);
    gl.deleteFramebuffer(f);
  }
  for(let i=0;i<W*H;i++) m[i*4+2] = ctrlGray[i];
  gl.bindTexture(gl.TEXTURE_2D, texMask);
  gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA8, W, H, 0, gl.RGBA, gl.UNSIGNED_BYTE, m);
  rebuildControlMask();
  setStatus('stCtrl', `Control-Img: ${name} (${regions.length} regions)`);
  addlog(`Control image loaded: ${name}, ${regions.length} regions.`);
}
// == RD::ReadSizeControlImg：size mask = max(gray/255, 0.1)
function loadSizeImg(bmp, name){
  const img = drawToImageData(bmp, W, H);
  const ks = snapshotTex(texKS[curKS]);
  for(let y=0;y<H;y++) for(let x=0;x<W;x++){
    const i = y*W+x, j = ((H-1-y)*W+x)*4;
    const g = (0.299*img.data[j] + 0.587*img.data[j+1] + 0.114*img.data[j+2])/255;
    ks[i*4+1] = Math.max(g, 0.1);
  }
  restoreTex(texKS[curKS], ks);
  addlog('Size image loaded: ' + name);
}

$('fileSrc').addEventListener('change',  async e => { if(e.target.files[0]) loadSrc(await createImageBitmap(e.target.files[0]), e.target.files[0].name); });
$('fileVfb').addEventListener('change',  async e => { if(e.target.files[0]) loadVfb(e.target.files[0]); });
$('fileETF').addEventListener('change',  async e => { if(e.target.files[0]) loadETF(await createImageBitmap(e.target.files[0]), e.target.files[0].name); });
$('fileTex').addEventListener('change',  async e => { if(e.target.files[0]) loadTex(await createImageBitmap(e.target.files[0]), e.target.files[0].name); });
$('fileCtrl').addEventListener('change', async e => { if(e.target.files[0]) loadCtrl(await createImageBitmap(e.target.files[0]), e.target.files[0].name); });
$('fileSize').addEventListener('change', async e => { if(e.target.files[0]) loadSizeImg(await createImageBitmap(e.target.files[0]), e.target.files[0].name); });

// =================================================================== toolbar
let running = false;
$('btnStart').addEventListener('click', () => {
  running = !running;
  $('btnStart').textContent = running ? 'Pause' : 'Start';
});
$('btnFill').addEventListener('click', () => splatState(1));   // == Seeds(25,true,.5)
$('btnClean').addEventListener('click', () => { cleanState(); addlog('Draw Panel Cleaned.'); });
$('btnAddDeg').addEventListener('click', () => rotateFlow(22.5));
$('btnSubDeg').addEventListener('click', () => rotateFlow(-22.5));

// == OnProcessingBox：檢查前置條件 + 顯示對應控制項
$('processingBox').addEventListener('change', () => {
  const v = $('processingBox').value;
  if((v === 'dirTexture' || v === 'PolarTexture' || v === 'Texture') && !texTex){
    addlog(`Must Loaded Texture First! (${v})`, '#ff6b6b');
    $('processingBox').value = 'distribution_A';
  }
  if(v === 'Color_mapping' && !hasSrc){
    addlog('Must Loaded src before using Color_mapping!', '#ff6b6b');
    $('processingBox').value = 'distribution_A';
  }
  const cur2 = $('processingBox').value;
  const showAB = !['distribution_A','distribution_B','LIC'].includes(cur2);
  document.querySelectorAll('.ab').forEach(x => x.classList.toggle('hidden', !showAB));
  document.querySelectorAll('.cm').forEach(x => x.classList.toggle('hidden', cur2 !== 'Color_mapping'));
  $('ppHint').classList.toggle('hidden', showAB);
});
// == OnControllingBox：Gradient 模式顯示 type 選單
$('controllingBox').addEventListener('change', () => {
  const v = $('controllingBox').value;
  $('gradTypeS_wrap').classList.toggle('hidden', v !== 'Gradient_Size');
  $('gradTypeK_wrap').classList.toggle('hidden', v !== 'Gradient_k');
});
// CAF checkbox → min/max degree 滑桿 + ±22.5 按鈕
$('cafCb').addEventListener('change', () => {
  const on = $('cafCb').checked;
  document.querySelectorAll('.caf').forEach(x => x.classList.toggle('hidden', !on));
  $('btnAddDeg').classList.toggle('hidden', !on);
  $('btnSubDeg').classList.toggle('hidden', !on);
  if(!on){
    P.mindeg = 0; P.maxdeg = 0;
    $('mindeg').value = 0; $('maxdeg').value = 0;
    $('mindeg_t').textContent = FMT.mindeg(0);
    $('maxdeg_t').textContent = FMT.maxdeg(0);
  }
});
// Segmentation checkbox
$('segCb').addEventListener('change', () => {
  const on = $('segCb').checked;
  if(on && !regions.length){
    addlog('Must Loaded Control Image first!', '#ff6b6b');
    $('segCb').checked = false;
    return;
  }
  document.querySelectorAll('.seg').forEach(x => x.classList.toggle('hidden', !on));
});
$('segBox').addEventListener('change', () => {
  const r = selectedRegion();
  if(!r) return;   // 把 region 參數帶回滑桿顯示
  $('F').value = r.F; $('k').value = r.k; $('l').value = r.l; $('s').value = r.size;
  $('sd').value = r.sd; $('theta0').value = r.theta0; $('mindeg').value = r.dmin; $('maxdeg').value = r.dmax;
  for(const id of ['F','k','l','s','sd','theta0','mindeg','maxdeg'])
    $(id+'_t').textContent = FMT[id]($(id).value);
});

// =================================================================== paint
let painting = false, lastPos = null, gradStart = null;
function canvasPos(e){
  const r = canvas.getBoundingClientRect();
  return [ (e.clientX - r.left)/r.width * W,
           (1 - (e.clientY - r.top)/r.height) * H ];
}
canvas.addEventListener('pointerdown', e => {
  const p = canvasPos(e);
  const mode = $('controllingBox').value;
  pushUndo();
  if(mode === 'Gradient_Size' || mode === 'Gradient_k'){ gradStart = p; }
  else if(mode === 'paint_black'){ splatState(3, p, p); splatAdd(5, p, p); }
  else if(mode === 'paint_white'){ splatState(4, p, p); splatAdd(6, p, p); }
  else { splatState(2, p, p); }
  painting = true; lastPos = p;
});
canvas.addEventListener('pointermove', e => {
  if(!painting) return;
  const p = canvasPos(e);
  const mode = $('controllingBox').value;
  if(mode === 'Gradient_Size' || mode === 'Gradient_k'){ lastPos = p; return; }
  if(mode === 'paint_black'){ splatState(3, lastPos, p); splatAdd(5, lastPos, p); }
  else if(mode === 'paint_white'){ splatState(4, lastPos, p); splatAdd(6, lastPos, p); }
  else { splatState(2, lastPos, p); }
  lastPos = p;
});
window.addEventListener('pointerup', () => {
  if(painting && gradStart){
    const mode = $('controllingBox').value;
    if(mode === 'Gradient_Size'){
      applyGradient('size', gradStart, lastPos, $('gradTypeS').value);
      $('s_t').textContent = 'Size :   ' + $('gradTypeS').value + ' Gradient';
    } else if(mode === 'Gradient_k'){
      applyGradient('k', gradStart, lastPos, $('gradTypeK').value);
      $('k_t').textContent = 'k :   ' + $('gradTypeK').value + ' Gradient';
    }
    gradStart = null;
  }
  painting = false;
});

// =================================================================== pattern picker
// == MyPatternPicker：點 l_all.jpg 選 (k,l)，F=0.0375，即時 128² 預覽
const PN = 128;
const pick = { F: 0.0375, k: 0.0655, l: 2 };
let pvState, pvFboS, pvCur = 0, pvAlphaT, pvAlphaF, pvFlowT, pvFlowF,
    pvMask, pvKS, pvKSF, pvOutT, pvOutF, pvBuf, pvCtx2d, pvImgData, pvZero;
function pvTex(internal, fmt, type){ return makeTexN(PN, PN, internal, fmt, type); }
function pvDraw(prog, fbo){ drawN(prog, fbo, PN, PN); }
function pvInit(){
  pvState = [pvTex(gl.RGBA16F, gl.RGBA, gl.HALF_FLOAT), pvTex(gl.RGBA16F, gl.RGBA, gl.HALF_FLOAT)];
  pvFboS  = [makeFbo(pvState[0]), makeFbo(pvState[1])];
  pvAlphaT = pvTex(gl.RGBA16F, gl.RGBA, gl.HALF_FLOAT); pvAlphaF = makeFbo(pvAlphaT);
  pvFlowT  = pvTex(gl.RGBA32F, gl.RGBA, gl.FLOAT);      pvFlowF  = makeFbo(pvFlowT);
  pvKS     = pvTex(gl.RGBA16F, gl.RGBA, gl.HALF_FLOAT); pvKSF    = makeFbo(pvKS);
  pvMask   = pvTex(gl.RGBA8, gl.RGBA, gl.UNSIGNED_BYTE);
  pvZero   = pvTex(gl.RGBA32F, gl.RGBA, gl.FLOAT);
  pvOutT   = pvTex(gl.RGBA8, gl.RGBA, gl.UNSIGNED_BYTE); pvOutF  = makeFbo(pvOutT);
  pvBuf    = new Uint8Array(PN*PN*4);
  pvCtx2d  = $('pickerCanvas').getContext('2d');
  pvImgData = pvCtx2d.createImageData(PN, PN);
  gl.useProgram(progFlow);
  bindTex(progFlow, 'uMask', 0, pvMask);
  gl.uniform1i(U(progFlow,'uMode'), 2);      // linear flow
  pvDraw(progFlow, pvFlowF);
}
function pvFillKS(){
  gl.useProgram(progFillKS);
  gl.uniform1f(U(progFillKS,'uKv'), pick.k);
  gl.uniform1f(U(progFillKS,'uSv'), 1.0 - parseFloat($('pickerS').value));
  pvDraw(progFillKS, pvKSF);
}
function pvReset(){
  gl.useProgram(progSplat);
  bindTex(progSplat, 'uSrc', 0, pvState[pvCur]);
  bindTex(progSplat, 'uMask', 1, pvMask);
  gl.uniform1i(U(progSplat,'uMode'), 0);
  pvDraw(progSplat, pvFboS[1-pvCur]); pvCur = 1-pvCur;
  gl.useProgram(progSplat);
  bindTex(progSplat, 'uSrc', 0, pvState[pvCur]);
  gl.uniform1i(U(progSplat,'uMode'), 1);
  gl.uniform1f(U(progSplat,'uRadius'), 4);
  gl.uniform1f(U(progSplat,'uRand'), Math.random()*100);
  pvDraw(progSplat, pvFboS[1-pvCur]); pvCur = 1-pvCur;
  pvFillKS();
}
function pvStep(){
  gl.useProgram(progAlpha);
  bindTex(progAlpha, 'uState', 0, pvState[pvCur]);
  bindTex(progAlpha, 'uFlow',  1, pvFlowT);
  bindTex(progAlpha, 'uCtrl1', 2, pvZero);
  bindTex(progAlpha, 'uCtrl2', 3, pvZero);
  gl.uniform1i(U(progAlpha,'uSegOn'), 0);
  gl.uniform1f(U(progAlpha,'uL'), pick.l);
  gl.uniform1f(U(progAlpha,'uTheta0'), 0);
  gl.uniform1f(U(progAlpha,'uDmin'), 0);
  gl.uniform1f(U(progAlpha,'uDmax'), 0);
  pvDraw(progAlpha, pvAlphaF);

  gl.useProgram(progUpdate);
  bindTex(progUpdate, 'uState', 0, pvState[pvCur]);
  bindTex(progUpdate, 'uAlpha', 1, pvAlphaT);
  bindTex(progUpdate, 'uAdd',   2, pvZero);
  bindTex(progUpdate, 'uCtrlKS',3, pvKS);
  bindTex(progUpdate, 'uCtrl1', 4, pvZero);
  bindTex(progUpdate, 'uCtrl2', 5, pvZero);
  gl.uniform1i(U(progUpdate,'uSegOn'), 0);
  gl.uniform1f(U(progUpdate,'uF'), pick.F);
  gl.uniform1f(U(progUpdate,'uSd'), 0.7 + P.sd);
  gl.uniform1f(U(progUpdate,'uAddAw'), 0);
  gl.uniform1f(U(progUpdate,'uAddBw'), 0);
  pvDraw(progUpdate, pvFboS[1-pvCur]); pvCur = 1-pvCur;
}
function pvRender(){
  gl.useProgram(progDisplay);
  bindTex(progDisplay, 'uState', 0, pvState[pvCur]);
  bindTex(progDisplay, 'uMask',  1, pvMask);
  bindTex(progDisplay, 'uOrig',  2, pvMask);
  bindTex(progDisplay, 'uTex',   3, pvMask);
  bindTex(progDisplay, 'uFlow',  4, pvFlowT);
  bindTex(progDisplay, 'uNoise', 5, pvMask);
  gl.uniform1i(U(progDisplay,'uMode'), 8);     // Thresholding
  gl.uniform1f(U(progDisplay,'uAlpha'), 0.6);
  gl.uniform1f(U(progDisplay,'uBeta'), 0.6);   // == StartPreview: alpha=beta=0.6
  gl.uniform1i(U(progDisplay,'uCmMode'), 1);
  gl.uniform1i(U(progDisplay,'uIsAda'), 0);
  gl.uniform1i(U(progDisplay,'uDispRegion'), 0);
  gl.uniform1f(U(progDisplay,'uRegionGray'), 0);
  pvDraw(progDisplay, pvOutF);
  gl.readPixels(0, 0, PN, PN, gl.RGBA, gl.UNSIGNED_BYTE, pvBuf);
  for(let y=0;y<PN;y++)
    pvImgData.data.set(pvBuf.subarray((PN-1-y)*PN*4, (PN-y)*PN*4), y*PN*4);
  pvCtx2d.putImageData(pvImgData, 0, 0);
}
function pvShowVals(){
  $('pickerVals').textContent = `F : ${pick.F.toFixed(4)}\nk : ${pick.k.toFixed(4)}\nl : ${pick.l}`;
}
const overlay = $('pickerOverlay');
function openPicker(){
  pick.F = P.F; pick.k = P.k; pick.l = P.l;
  $('pickerS').value = P.s;
  $('pickerS_t').textContent = FMT.s(P.s);
  pvShowVals(); pvReset();
  overlay.classList.add('open');
}
$('btnPickClose').addEventListener('click', () => overlay.classList.remove('open'));
overlay.addEventListener('click', e => { if(e.target === overlay) overlay.classList.remove('open'); });
$('pickerS').addEventListener('input', () => {
  $('pickerS_t').textContent = FMT.s($('pickerS').value);
  pvFillKS();
});
// == Picker::MouseLDown：k = 0.056 + 0.0000238·x、l = y/70、F = 0.0375
$('pickerImg').addEventListener('pointerdown', e => {
  const r = e.currentTarget.getBoundingClientRect();
  const x = (e.clientX - r.left)/r.width * 500;
  const y = (e.clientY - r.top)/r.height * 500;
  pick.k = 0.056 + 0.0000238*x;
  pick.F = 0.0375;
  pick.l = Math.min(6, Math.floor(y/70));
  const mark = $('pickerMark');
  mark.style.left = (e.clientX - r.left) + 'px';
  mark.style.top  = (e.clientY - r.top)  + 'px';
  pvShowVals(); pvReset();
});
// == MyPatternPicker::OnSelect
$('btnPickSelect').addEventListener('click', () => {
  if(!$('segCb').checked){
    P.F = pick.F; P.k = pick.k; P.l = pick.l; P.s = parseFloat($('pickerS').value);
    $('F').value = P.F; $('k').value = P.k; $('l').value = P.l; $('s').value = P.s;
    for(const id of ['F','k','l','s']) $(id+'_t').textContent = FMT[id]($(id).value);
    fillKS();
  } else {
    const r = selectedRegion();
    if(r){
      r.F = pick.F; r.k = pick.k; r.l = pick.l; r.size = parseFloat($('pickerS').value);
      rebuildControlMask();
    }
  }
  if(!running) $('btnStart').click();   // == activateRenderLoop(true)
  overlay.classList.remove('open');
});

// =================================================================== main loop
// innerAMPloopsize 自動調速（== FastGrayScott 尾端的調整邏輯）
let iters = 4;
function step(){
  const segOn = $('segCb').checked && regions.length > 0;
  gl.useProgram(progAlpha);
  bindTex(progAlpha, 'uState', 0, texState[cur]);
  bindTex(progAlpha, 'uFlow',  1, texFlow[curFlow]);
  bindTex(progAlpha, 'uCtrl1', 2, texCtrl1);
  bindTex(progAlpha, 'uCtrl2', 3, texCtrl2);
  gl.uniform1i(U(progAlpha,'uSegOn'), segOn ? 1 : 0);
  gl.uniform1f(U(progAlpha,'uL'), P.l);
  gl.uniform1f(U(progAlpha,'uTheta0'), P.theta0*D2R);
  gl.uniform1f(U(progAlpha,'uDmin'), P.mindeg*D2R);
  gl.uniform1f(U(progAlpha,'uDmax'), P.maxdeg*D2R);
  draw(progAlpha, fboAlpha);

  gl.useProgram(progUpdate);
  bindTex(progUpdate, 'uState', 0, texState[cur]);
  bindTex(progUpdate, 'uAlpha', 1, texAlpha);
  bindTex(progUpdate, 'uAdd',   2, texAdd[curAdd]);
  bindTex(progUpdate, 'uCtrlKS',3, texKS[curKS]);
  bindTex(progUpdate, 'uCtrl1', 4, texCtrl1);
  bindTex(progUpdate, 'uCtrl2', 5, texCtrl2);
  gl.uniform1i(U(progUpdate,'uSegOn'), segOn ? 1 : 0);
  gl.uniform1f(U(progUpdate,'uF'), P.F);
  gl.uniform1f(U(progUpdate,'uSd'), 0.7 + P.sd);
  gl.uniform1f(U(progUpdate,'uAddAw'), P.addA);
  gl.uniform1f(U(progUpdate,'uAddBw'), P.addB);
  draw(progUpdate, fboState[1-cur]); cur = 1-cur;
}
const PROC_IDX = { distribution_A:0, distribution_B:1, LIC:2, Motion_Illusion:3,
  Texture:4, dirTexture:5, PolarTexture:6, adaThresholding:7, Thresholding:8, Color_mapping:9 };
let lastFrameT = 0;
function render(frameT){
  // 用 rAF 的幀間隔調整每幀迭代數（== innerAMPloopsize 自動調速），
  // 不用 gl.finish() 強制同步 — 那會讓 CPU 空轉等 GPU
  const dt = frameT - lastFrameT;
  lastFrameT = frameT;
  if(running){
    for(let i=0;i<iters;i++) step();
    if(dt > 0 && dt < 1000/58) iters = Math.min(1024, Math.round(iters*1.5));
    else if(dt > 1000/30) iters = Math.max(2, Math.round(iters*0.75));
  }
  gl.useProgram(progDisplay);
  bindTex(progDisplay, 'uState', 0, texState[cur]);
  bindTex(progDisplay, 'uMask',  1, texMask);
  bindTex(progDisplay, 'uOrig',  2, texOrig);
  bindTex(progDisplay, 'uTex',   3, texTex || texMask);
  bindTex(progDisplay, 'uFlow',  4, texFlow[curFlow]);
  bindTex(progDisplay, 'uNoise', 5, texNoise);
  gl.uniform1i(U(progDisplay,'uMode'), PROC_IDX[$('processingBox').value] ?? 0);
  gl.uniform1i(U(progDisplay,'uCmMode'), $('cmMode').selectedIndex + 1);
  gl.uniform1i(U(progDisplay,'uIsAda'), $('cmIsAda').checked ? 1 : 0);
  gl.uniform1f(U(progDisplay,'uAlpha'), P.alpha);
  gl.uniform1f(U(progDisplay,'uBeta'), P.beta);
  const dr = $('dispRegionCb').checked && $('segCb').checked && selectedRegion();
  gl.uniform1i(U(progDisplay,'uDispRegion'), dr ? 1 : 0);
  gl.uniform1f(U(progDisplay,'uRegionGray'), dr ? selectedRegion().gray : 0);
  draw(progDisplay, null);

  if(overlay.classList.contains('open')){
    for(let i=0;i<6;i++) pvStep();
    pvRender();
  }
  requestAnimationFrame(render);
}

// =================================================================== boot
const PICKER_IMG = '../l_all.jpg';   // 直接使用 repo 根目錄的原始檔（img 顯示不受 file:// CORS 限制）
const DEFAULT_IMG = '../data/cat-source.png';
$('pickerImg').src = PICKER_IMG;
allocate();
pvInit();
addlog('Hello CRD!');
let exampleBmp = null;
async function getExampleBmp(){
  if(exampleBmp) return exampleBmp;
  const img = new Image();
  img.src = DEFAULT_IMG;
  await img.decode();
  exampleBmp = await createImageBitmap(img);
  return exampleBmp;
}
// 「載入範例」workflow：
// Load SrcImg + ETF + Control Image(cat) → pattern(F=0.0375, k=0.0578, l=0)
// → Fill Ink → Edge2AddB → Activate Segmentation(Region 1) → adaThresholding(alpha=0, beta=0.45)
async function loadExample(){
  const bmp = await getExampleBmp();
  loadSrc(bmp, 'cat-source.png');
  loadETF(bmp, 'cat-source.png');
  P.F = 0.0375; P.k = 0.0578; P.l = 0;
  $('F').value = P.F; $('k').value = P.k; $('l').value = P.l;
  for(const id of ['F','k','l']) $(id+'_t').textContent = FMT[id]($(id).value);
  fillKS();
  loadCtrl(bmp, 'cat-source.png');   // region 會繼承上面設定的圖樣參數
  splatState(1);   // Fill Ink
  splatAdd(9);     // Edge2AddB
  // 啟動 segmentation 並選 Region 1
  $('segCb').checked = true;
  $('segCb').dispatchEvent(new Event('change'));
  $('segBox').selectedIndex = 0;
  $('segBox').dispatchEvent(new Event('change'));
  // Post processing：adaThresholding, alpha=0, beta=0.45
  P.alpha = 0.0; P.beta = 0.45;
  $('alpha').value = 0.0; $('beta').value = 0.45;
  $('alpha_t').textContent = FMT.alpha(0.0);
  $('beta_t').textContent = FMT.beta(0.45);
  $('processingBox').value = 'adaThresholding';
  $('processingBox').dispatchEvent(new Event('change'));
  addlog('範例已載入（貓 + ETF + Control Image + Segmentation Region 1），按 Start 開始生成。');
}
$('btnExample').addEventListener('click', loadExample);
// 啟動時預設載入貓圖（== Open SrcImg + Open ETF）
(async () => {
  const bmp = await getExampleBmp();
  loadSrc(bmp, 'cat-source.png');
  loadETF(bmp, 'cat-source.png');
})();
requestAnimationFrame(render);
