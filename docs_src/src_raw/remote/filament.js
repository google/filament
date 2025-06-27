
var Filament = (() => {
  var _scriptName = typeof document != 'undefined' ? document.currentScript?.src : undefined;
  if (typeof __filename != 'undefined') _scriptName ||= __filename;
  return (
function(moduleArg = {}) {
  var moduleRtn;

var Module=Object.assign({},moduleArg);var readyPromiseResolve,readyPromiseReject;var readyPromise=new Promise((resolve,reject)=>{readyPromiseResolve=resolve;readyPromiseReject=reject});var ENVIRONMENT_IS_WEB=typeof window=="object";var ENVIRONMENT_IS_WORKER=typeof importScripts=="function";var ENVIRONMENT_IS_NODE=typeof process=="object"&&typeof process.versions=="object"&&typeof process.versions.node=="string";if(ENVIRONMENT_IS_NODE){}var moduleOverrides=Object.assign({},Module);var arguments_=[];var thisProgram="./this.program";var quit_=(status,toThrow)=>{throw toThrow};var scriptDirectory="";function locateFile(path){if(Module["locateFile"]){return Module["locateFile"](path,scriptDirectory)}return scriptDirectory+path}var read_,readAsync,readBinary;if(ENVIRONMENT_IS_NODE){var fs=require("fs");var nodePath=require("path");scriptDirectory=__dirname+"/";read_=(filename,binary)=>{filename=isFileURI(filename)?new URL(filename):nodePath.normalize(filename);return fs.readFileSync(filename,binary?undefined:"utf8")};readBinary=filename=>{var ret=read_(filename,true);if(!ret.buffer){ret=new Uint8Array(ret)}return ret};readAsync=(filename,onload,onerror,binary=true)=>{filename=isFileURI(filename)?new URL(filename):nodePath.normalize(filename);fs.readFile(filename,binary?undefined:"utf8",(err,data)=>{if(err)onerror(err);else onload(binary?data.buffer:data)})};if(!Module["thisProgram"]&&process.argv.length>1){thisProgram=process.argv[1].replace(/\\/g,"/")}arguments_=process.argv.slice(2);quit_=(status,toThrow)=>{process.exitCode=status;throw toThrow}}else if(ENVIRONMENT_IS_WEB||ENVIRONMENT_IS_WORKER){if(ENVIRONMENT_IS_WORKER){scriptDirectory=self.location.href}else if(typeof document!="undefined"&&document.currentScript){scriptDirectory=document.currentScript.src}if(_scriptName){scriptDirectory=_scriptName}if(scriptDirectory.startsWith("blob:")){scriptDirectory=""}else{scriptDirectory=scriptDirectory.substr(0,scriptDirectory.replace(/[?#].*/,"").lastIndexOf("/")+1)}{read_=url=>{var xhr=new XMLHttpRequest;xhr.open("GET",url,false);xhr.send(null);return xhr.responseText};if(ENVIRONMENT_IS_WORKER){readBinary=url=>{var xhr=new XMLHttpRequest;xhr.open("GET",url,false);xhr.responseType="arraybuffer";xhr.send(null);return new Uint8Array(xhr.response)}}readAsync=(url,onload,onerror)=>{var xhr=new XMLHttpRequest;xhr.open("GET",url,true);xhr.responseType="arraybuffer";xhr.onload=()=>{if(xhr.status==200||xhr.status==0&&xhr.response){onload(xhr.response);return}onerror()};xhr.onerror=onerror;xhr.send(null)}}}else{}var out=Module["print"]||console.log.bind(console);var err=Module["printErr"]||console.error.bind(console);Object.assign(Module,moduleOverrides);moduleOverrides=null;if(Module["arguments"])arguments_=Module["arguments"];if(Module["thisProgram"])thisProgram=Module["thisProgram"];if(Module["quit"])quit_=Module["quit"];var wasmBinary;if(Module["wasmBinary"])wasmBinary=Module["wasmBinary"];var wasmMemory;var ABORT=false;var EXITSTATUS;var HEAP8,HEAPU8,HEAP16,HEAPU16,HEAP32,HEAPU32,HEAPF32,HEAPF64;function updateMemoryViews(){var b=wasmMemory.buffer;Module["HEAP8"]=HEAP8=new Int8Array(b);Module["HEAP16"]=HEAP16=new Int16Array(b);Module["HEAPU8"]=HEAPU8=new Uint8Array(b);Module["HEAPU16"]=HEAPU16=new Uint16Array(b);Module["HEAP32"]=HEAP32=new Int32Array(b);Module["HEAPU32"]=HEAPU32=new Uint32Array(b);Module["HEAPF32"]=HEAPF32=new Float32Array(b);Module["HEAPF64"]=HEAPF64=new Float64Array(b)}var __ATPRERUN__=[];var __ATINIT__=[];var __ATPOSTRUN__=[];var runtimeInitialized=false;function preRun(){if(Module["preRun"]){if(typeof Module["preRun"]=="function")Module["preRun"]=[Module["preRun"]];while(Module["preRun"].length){addOnPreRun(Module["preRun"].shift())}}callRuntimeCallbacks(__ATPRERUN__)}function initRuntime(){runtimeInitialized=true;if(!Module["noFSInit"]&&!FS.init.initialized)FS.init();FS.ignorePermissions=false;TTY.init();callRuntimeCallbacks(__ATINIT__)}function postRun(){if(Module["postRun"]){if(typeof Module["postRun"]=="function")Module["postRun"]=[Module["postRun"]];while(Module["postRun"].length){addOnPostRun(Module["postRun"].shift())}}callRuntimeCallbacks(__ATPOSTRUN__)}function addOnPreRun(cb){__ATPRERUN__.unshift(cb)}function addOnInit(cb){__ATINIT__.unshift(cb)}function addOnPostRun(cb){__ATPOSTRUN__.unshift(cb)}var runDependencies=0;var runDependencyWatcher=null;var dependenciesFulfilled=null;function getUniqueRunDependency(id){return id}function addRunDependency(id){runDependencies++;Module["monitorRunDependencies"]?.(runDependencies)}function removeRunDependency(id){runDependencies--;Module["monitorRunDependencies"]?.(runDependencies);if(runDependencies==0){if(runDependencyWatcher!==null){clearInterval(runDependencyWatcher);runDependencyWatcher=null}if(dependenciesFulfilled){var callback=dependenciesFulfilled;dependenciesFulfilled=null;callback()}}}function abort(what){Module["onAbort"]?.(what);what="Aborted("+what+")";err(what);ABORT=true;EXITSTATUS=1;what+=". Build with -sASSERTIONS for more info.";var e=new WebAssembly.RuntimeError(what);readyPromiseReject(e);throw e}var dataURIPrefix="data:application/octet-stream;base64,";var isDataURI=filename=>filename.startsWith(dataURIPrefix);var isFileURI=filename=>filename.startsWith("file://");function findWasmBinary(){var f="filament.wasm";if(!isDataURI(f)){return locateFile(f)}return f}var wasmBinaryFile;function getBinarySync(file){if(file==wasmBinaryFile&&wasmBinary){return new Uint8Array(wasmBinary)}if(readBinary){return readBinary(file)}throw"both async and sync fetching of the wasm failed"}function getBinaryPromise(binaryFile){if(!wasmBinary&&(ENVIRONMENT_IS_WEB||ENVIRONMENT_IS_WORKER)){if(typeof fetch=="function"&&!isFileURI(binaryFile)){return fetch(binaryFile,{credentials:"same-origin"}).then(response=>{if(!response["ok"]){throw`failed to load wasm binary file at '${binaryFile}'`}return response["arrayBuffer"]()}).catch(()=>getBinarySync(binaryFile))}else if(readAsync){return new Promise((resolve,reject)=>{readAsync(binaryFile,response=>resolve(new Uint8Array(response)),reject)})}}return Promise.resolve().then(()=>getBinarySync(binaryFile))}function instantiateArrayBuffer(binaryFile,imports,receiver){return getBinaryPromise(binaryFile).then(binary=>WebAssembly.instantiate(binary,imports)).then(receiver,reason=>{err(`failed to asynchronously prepare wasm: ${reason}`);abort(reason)})}function instantiateAsync(binary,binaryFile,imports,callback){if(!binary&&typeof WebAssembly.instantiateStreaming=="function"&&!isDataURI(binaryFile)&&!isFileURI(binaryFile)&&!ENVIRONMENT_IS_NODE&&typeof fetch=="function"){return fetch(binaryFile,{credentials:"same-origin"}).then(response=>{var result=WebAssembly.instantiateStreaming(response,imports);return result.then(callback,function(reason){err(`wasm streaming compile failed: ${reason}`);err("falling back to ArrayBuffer instantiation");return instantiateArrayBuffer(binaryFile,imports,callback)})})}return instantiateArrayBuffer(binaryFile,imports,callback)}function getWasmImports(){return{a:wasmImports}}function createWasm(){var info=getWasmImports();function receiveInstance(instance,module){wasmExports=instance.exports;wasmMemory=wasmExports["sc"];updateMemoryViews();wasmTable=wasmExports["vc"];addOnInit(wasmExports["tc"]);removeRunDependency("wasm-instantiate");return wasmExports}addRunDependency("wasm-instantiate");function receiveInstantiationResult(result){receiveInstance(result["instance"])}if(Module["instantiateWasm"]){try{return Module["instantiateWasm"](info,receiveInstance)}catch(e){err(`Module.instantiateWasm callback failed with error: ${e}`);readyPromiseReject(e)}}if(!wasmBinaryFile)wasmBinaryFile=findWasmBinary();instantiateAsync(wasmBinary,wasmBinaryFile,info,receiveInstantiationResult).catch(readyPromiseReject);return{}}var tempDouble;var tempI64;var ASM_CONSTS={1699756:()=>{const options=window.filament_glOptions;const context=window.filament_glContext;const handle=GL.registerContext(context,options);window.filament_contextHandle=handle;GL.makeContextCurrent(handle)},1699970:()=>{const handle=window.filament_contextHandle;GL.makeContextCurrent(handle)},1700051:($0,$1,$2,$3,$4,$5)=>{const fn=Emval.toValue($0);fn({renderable:Emval.toValue($1),depth:$2,fragCoords:[$3,$4,$5]})}};var callRuntimeCallbacks=callbacks=>{while(callbacks.length>0){callbacks.shift()(Module)}};var noExitRuntime=Module["noExitRuntime"]||true;function syscallGetVarargI(){var ret=HEAP32[+SYSCALLS.varargs>>2];SYSCALLS.varargs+=4;return ret}var syscallGetVarargP=syscallGetVarargI;var PATH={isAbs:path=>path.charAt(0)==="/",splitPath:filename=>{var splitPathRe=/^(\/?|)([\s\S]*?)((?:\.{1,2}|[^\/]+?|)(\.[^.\/]*|))(?:[\/]*)$/;return splitPathRe.exec(filename).slice(1)},normalizeArray:(parts,allowAboveRoot)=>{var up=0;for(var i=parts.length-1;i>=0;i--){var last=parts[i];if(last==="."){parts.splice(i,1)}else if(last===".."){parts.splice(i,1);up++}else if(up){parts.splice(i,1);up--}}if(allowAboveRoot){for(;up;up--){parts.unshift("..")}}return parts},normalize:path=>{var isAbsolute=PATH.isAbs(path),trailingSlash=path.substr(-1)==="/";path=PATH.normalizeArray(path.split("/").filter(p=>!!p),!isAbsolute).join("/");if(!path&&!isAbsolute){path="."}if(path&&trailingSlash){path+="/"}return(isAbsolute?"/":"")+path},dirname:path=>{var result=PATH.splitPath(path),root=result[0],dir=result[1];if(!root&&!dir){return"."}if(dir){dir=dir.substr(0,dir.length-1)}return root+dir},basename:path=>{if(path==="/")return"/";path=PATH.normalize(path);path=path.replace(/\/$/,"");var lastSlash=path.lastIndexOf("/");if(lastSlash===-1)return path;return path.substr(lastSlash+1)},join:(...paths)=>PATH.normalize(paths.join("/")),join2:(l,r)=>PATH.normalize(l+"/"+r)};var initRandomFill=()=>{if(typeof crypto=="object"&&typeof crypto["getRandomValues"]=="function"){return view=>crypto.getRandomValues(view)}else if(ENVIRONMENT_IS_NODE){try{var crypto_module=require("crypto");var randomFillSync=crypto_module["randomFillSync"];if(randomFillSync){return view=>crypto_module["randomFillSync"](view)}var randomBytes=crypto_module["randomBytes"];return view=>(view.set(randomBytes(view.byteLength)),view)}catch(e){}}abort("initRandomDevice")};var randomFill=view=>(randomFill=initRandomFill())(view);var PATH_FS={resolve:(...args)=>{var resolvedPath="",resolvedAbsolute=false;for(var i=args.length-1;i>=-1&&!resolvedAbsolute;i--){var path=i>=0?args[i]:FS.cwd();if(typeof path!="string"){throw new TypeError("Arguments to path.resolve must be strings")}else if(!path){return""}resolvedPath=path+"/"+resolvedPath;resolvedAbsolute=PATH.isAbs(path)}resolvedPath=PATH.normalizeArray(resolvedPath.split("/").filter(p=>!!p),!resolvedAbsolute).join("/");return(resolvedAbsolute?"/":"")+resolvedPath||"."},relative:(from,to)=>{from=PATH_FS.resolve(from).substr(1);to=PATH_FS.resolve(to).substr(1);function trim(arr){var start=0;for(;start<arr.length;start++){if(arr[start]!=="")break}var end=arr.length-1;for(;end>=0;end--){if(arr[end]!=="")break}if(start>end)return[];return arr.slice(start,end-start+1)}var fromParts=trim(from.split("/"));var toParts=trim(to.split("/"));var length=Math.min(fromParts.length,toParts.length);var samePartsLength=length;for(var i=0;i<length;i++){if(fromParts[i]!==toParts[i]){samePartsLength=i;break}}var outputParts=[];for(var i=samePartsLength;i<fromParts.length;i++){outputParts.push("..")}outputParts=outputParts.concat(toParts.slice(samePartsLength));return outputParts.join("/")}};var UTF8Decoder=typeof TextDecoder!="undefined"?new TextDecoder("utf8"):undefined;var UTF8ArrayToString=(heapOrArray,idx,maxBytesToRead)=>{var endIdx=idx+maxBytesToRead;var endPtr=idx;while(heapOrArray[endPtr]&&!(endPtr>=endIdx))++endPtr;if(endPtr-idx>16&&heapOrArray.buffer&&UTF8Decoder){return UTF8Decoder.decode(heapOrArray.subarray(idx,endPtr))}var str="";while(idx<endPtr){var u0=heapOrArray[idx++];if(!(u0&128)){str+=String.fromCharCode(u0);continue}var u1=heapOrArray[idx++]&63;if((u0&224)==192){str+=String.fromCharCode((u0&31)<<6|u1);continue}var u2=heapOrArray[idx++]&63;if((u0&240)==224){u0=(u0&15)<<12|u1<<6|u2}else{u0=(u0&7)<<18|u1<<12|u2<<6|heapOrArray[idx++]&63}if(u0<65536){str+=String.fromCharCode(u0)}else{var ch=u0-65536;str+=String.fromCharCode(55296|ch>>10,56320|ch&1023)}}return str};var FS_stdin_getChar_buffer=[];var lengthBytesUTF8=str=>{var len=0;for(var i=0;i<str.length;++i){var c=str.charCodeAt(i);if(c<=127){len++}else if(c<=2047){len+=2}else if(c>=55296&&c<=57343){len+=4;++i}else{len+=3}}return len};var stringToUTF8Array=(str,heap,outIdx,maxBytesToWrite)=>{if(!(maxBytesToWrite>0))return 0;var startIdx=outIdx;var endIdx=outIdx+maxBytesToWrite-1;for(var i=0;i<str.length;++i){var u=str.charCodeAt(i);if(u>=55296&&u<=57343){var u1=str.charCodeAt(++i);u=65536+((u&1023)<<10)|u1&1023}if(u<=127){if(outIdx>=endIdx)break;heap[outIdx++]=u}else if(u<=2047){if(outIdx+1>=endIdx)break;heap[outIdx++]=192|u>>6;heap[outIdx++]=128|u&63}else if(u<=65535){if(outIdx+2>=endIdx)break;heap[outIdx++]=224|u>>12;heap[outIdx++]=128|u>>6&63;heap[outIdx++]=128|u&63}else{if(outIdx+3>=endIdx)break;heap[outIdx++]=240|u>>18;heap[outIdx++]=128|u>>12&63;heap[outIdx++]=128|u>>6&63;heap[outIdx++]=128|u&63}}heap[outIdx]=0;return outIdx-startIdx};function intArrayFromString(stringy,dontAddNull,length){var len=length>0?length:lengthBytesUTF8(stringy)+1;var u8array=new Array(len);var numBytesWritten=stringToUTF8Array(stringy,u8array,0,u8array.length);if(dontAddNull)u8array.length=numBytesWritten;return u8array}var FS_stdin_getChar=()=>{if(!FS_stdin_getChar_buffer.length){var result=null;if(ENVIRONMENT_IS_NODE){var BUFSIZE=256;var buf=Buffer.alloc(BUFSIZE);var bytesRead=0;var fd=process.stdin.fd;try{bytesRead=fs.readSync(fd,buf,0,BUFSIZE)}catch(e){if(e.toString().includes("EOF"))bytesRead=0;else throw e}if(bytesRead>0){result=buf.slice(0,bytesRead).toString("utf-8")}}else if(typeof window!="undefined"&&typeof window.prompt=="function"){result=window.prompt("Input: ");if(result!==null){result+="\n"}}else{}if(!result){return null}FS_stdin_getChar_buffer=intArrayFromString(result,true)}return FS_stdin_getChar_buffer.shift()};var TTY={ttys:[],init(){},shutdown(){},register(dev,ops){TTY.ttys[dev]={input:[],output:[],ops:ops};FS.registerDevice(dev,TTY.stream_ops)},stream_ops:{open(stream){var tty=TTY.ttys[stream.node.rdev];if(!tty){throw new FS.ErrnoError(43)}stream.tty=tty;stream.seekable=false},close(stream){stream.tty.ops.fsync(stream.tty)},fsync(stream){stream.tty.ops.fsync(stream.tty)},read(stream,buffer,offset,length,pos){if(!stream.tty||!stream.tty.ops.get_char){throw new FS.ErrnoError(60)}var bytesRead=0;for(var i=0;i<length;i++){var result;try{result=stream.tty.ops.get_char(stream.tty)}catch(e){throw new FS.ErrnoError(29)}if(result===undefined&&bytesRead===0){throw new FS.ErrnoError(6)}if(result===null||result===undefined)break;bytesRead++;buffer[offset+i]=result}if(bytesRead){stream.node.timestamp=Date.now()}return bytesRead},write(stream,buffer,offset,length,pos){if(!stream.tty||!stream.tty.ops.put_char){throw new FS.ErrnoError(60)}try{for(var i=0;i<length;i++){stream.tty.ops.put_char(stream.tty,buffer[offset+i])}}catch(e){throw new FS.ErrnoError(29)}if(length){stream.node.timestamp=Date.now()}return i}},default_tty_ops:{get_char(tty){return FS_stdin_getChar()},put_char(tty,val){if(val===null||val===10){out(UTF8ArrayToString(tty.output,0));tty.output=[]}else{if(val!=0)tty.output.push(val)}},fsync(tty){if(tty.output&&tty.output.length>0){out(UTF8ArrayToString(tty.output,0));tty.output=[]}},ioctl_tcgets(tty){return{c_iflag:25856,c_oflag:5,c_cflag:191,c_lflag:35387,c_cc:[3,28,127,21,4,0,1,0,17,19,26,0,18,15,23,22,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]}},ioctl_tcsets(tty,optional_actions,data){return 0},ioctl_tiocgwinsz(tty){return[24,80]}},default_tty1_ops:{put_char(tty,val){if(val===null||val===10){err(UTF8ArrayToString(tty.output,0));tty.output=[]}else{if(val!=0)tty.output.push(val)}},fsync(tty){if(tty.output&&tty.output.length>0){err(UTF8ArrayToString(tty.output,0));tty.output=[]}}}};var mmapAlloc=size=>{abort()};var MEMFS={ops_table:null,mount(mount){return MEMFS.createNode(null,"/",16384|511,0)},createNode(parent,name,mode,dev){if(FS.isBlkdev(mode)||FS.isFIFO(mode)){throw new FS.ErrnoError(63)}MEMFS.ops_table||={dir:{node:{getattr:MEMFS.node_ops.getattr,setattr:MEMFS.node_ops.setattr,lookup:MEMFS.node_ops.lookup,mknod:MEMFS.node_ops.mknod,rename:MEMFS.node_ops.rename,unlink:MEMFS.node_ops.unlink,rmdir:MEMFS.node_ops.rmdir,readdir:MEMFS.node_ops.readdir,symlink:MEMFS.node_ops.symlink},stream:{llseek:MEMFS.stream_ops.llseek}},file:{node:{getattr:MEMFS.node_ops.getattr,setattr:MEMFS.node_ops.setattr},stream:{llseek:MEMFS.stream_ops.llseek,read:MEMFS.stream_ops.read,write:MEMFS.stream_ops.write,allocate:MEMFS.stream_ops.allocate,mmap:MEMFS.stream_ops.mmap,msync:MEMFS.stream_ops.msync}},link:{node:{getattr:MEMFS.node_ops.getattr,setattr:MEMFS.node_ops.setattr,readlink:MEMFS.node_ops.readlink},stream:{}},chrdev:{node:{getattr:MEMFS.node_ops.getattr,setattr:MEMFS.node_ops.setattr},stream:FS.chrdev_stream_ops}};var node=FS.createNode(parent,name,mode,dev);if(FS.isDir(node.mode)){node.node_ops=MEMFS.ops_table.dir.node;node.stream_ops=MEMFS.ops_table.dir.stream;node.contents={}}else if(FS.isFile(node.mode)){node.node_ops=MEMFS.ops_table.file.node;node.stream_ops=MEMFS.ops_table.file.stream;node.usedBytes=0;node.contents=null}else if(FS.isLink(node.mode)){node.node_ops=MEMFS.ops_table.link.node;node.stream_ops=MEMFS.ops_table.link.stream}else if(FS.isChrdev(node.mode)){node.node_ops=MEMFS.ops_table.chrdev.node;node.stream_ops=MEMFS.ops_table.chrdev.stream}node.timestamp=Date.now();if(parent){parent.contents[name]=node;parent.timestamp=node.timestamp}return node},getFileDataAsTypedArray(node){if(!node.contents)return new Uint8Array(0);if(node.contents.subarray)return node.contents.subarray(0,node.usedBytes);return new Uint8Array(node.contents)},expandFileStorage(node,newCapacity){var prevCapacity=node.contents?node.contents.length:0;if(prevCapacity>=newCapacity)return;var CAPACITY_DOUBLING_MAX=1024*1024;newCapacity=Math.max(newCapacity,prevCapacity*(prevCapacity<CAPACITY_DOUBLING_MAX?2:1.125)>>>0);if(prevCapacity!=0)newCapacity=Math.max(newCapacity,256);var oldContents=node.contents;node.contents=new Uint8Array(newCapacity);if(node.usedBytes>0)node.contents.set(oldContents.subarray(0,node.usedBytes),0)},resizeFileStorage(node,newSize){if(node.usedBytes==newSize)return;if(newSize==0){node.contents=null;node.usedBytes=0}else{var oldContents=node.contents;node.contents=new Uint8Array(newSize);if(oldContents){node.contents.set(oldContents.subarray(0,Math.min(newSize,node.usedBytes)))}node.usedBytes=newSize}},node_ops:{getattr(node){var attr={};attr.dev=FS.isChrdev(node.mode)?node.id:1;attr.ino=node.id;attr.mode=node.mode;attr.nlink=1;attr.uid=0;attr.gid=0;attr.rdev=node.rdev;if(FS.isDir(node.mode)){attr.size=4096}else if(FS.isFile(node.mode)){attr.size=node.usedBytes}else if(FS.isLink(node.mode)){attr.size=node.link.length}else{attr.size=0}attr.atime=new Date(node.timestamp);attr.mtime=new Date(node.timestamp);attr.ctime=new Date(node.timestamp);attr.blksize=4096;attr.blocks=Math.ceil(attr.size/attr.blksize);return attr},setattr(node,attr){if(attr.mode!==undefined){node.mode=attr.mode}if(attr.timestamp!==undefined){node.timestamp=attr.timestamp}if(attr.size!==undefined){MEMFS.resizeFileStorage(node,attr.size)}},lookup(parent,name){throw FS.genericErrors[44]},mknod(parent,name,mode,dev){return MEMFS.createNode(parent,name,mode,dev)},rename(old_node,new_dir,new_name){if(FS.isDir(old_node.mode)){var new_node;try{new_node=FS.lookupNode(new_dir,new_name)}catch(e){}if(new_node){for(var i in new_node.contents){throw new FS.ErrnoError(55)}}}delete old_node.parent.contents[old_node.name];old_node.parent.timestamp=Date.now();old_node.name=new_name;new_dir.contents[new_name]=old_node;new_dir.timestamp=old_node.parent.timestamp;old_node.parent=new_dir},unlink(parent,name){delete parent.contents[name];parent.timestamp=Date.now()},rmdir(parent,name){var node=FS.lookupNode(parent,name);for(var i in node.contents){throw new FS.ErrnoError(55)}delete parent.contents[name];parent.timestamp=Date.now()},readdir(node){var entries=[".",".."];for(var key of Object.keys(node.contents)){entries.push(key)}return entries},symlink(parent,newname,oldpath){var node=MEMFS.createNode(parent,newname,511|40960,0);node.link=oldpath;return node},readlink(node){if(!FS.isLink(node.mode)){throw new FS.ErrnoError(28)}return node.link}},stream_ops:{read(stream,buffer,offset,length,position){var contents=stream.node.contents;if(position>=stream.node.usedBytes)return 0;var size=Math.min(stream.node.usedBytes-position,length);if(size>8&&contents.subarray){buffer.set(contents.subarray(position,position+size),offset)}else{for(var i=0;i<size;i++)buffer[offset+i]=contents[position+i]}return size},write(stream,buffer,offset,length,position,canOwn){if(buffer.buffer===HEAP8.buffer){canOwn=false}if(!length)return 0;var node=stream.node;node.timestamp=Date.now();if(buffer.subarray&&(!node.contents||node.contents.subarray)){if(canOwn){node.contents=buffer.subarray(offset,offset+length);node.usedBytes=length;return length}else if(node.usedBytes===0&&position===0){node.contents=buffer.slice(offset,offset+length);node.usedBytes=length;return length}else if(position+length<=node.usedBytes){node.contents.set(buffer.subarray(offset,offset+length),position);return length}}MEMFS.expandFileStorage(node,position+length);if(node.contents.subarray&&buffer.subarray){node.contents.set(buffer.subarray(offset,offset+length),position)}else{for(var i=0;i<length;i++){node.contents[position+i]=buffer[offset+i]}}node.usedBytes=Math.max(node.usedBytes,position+length);return length},llseek(stream,offset,whence){var position=offset;if(whence===1){position+=stream.position}else if(whence===2){if(FS.isFile(stream.node.mode)){position+=stream.node.usedBytes}}if(position<0){throw new FS.ErrnoError(28)}return position},allocate(stream,offset,length){MEMFS.expandFileStorage(stream.node,offset+length);stream.node.usedBytes=Math.max(stream.node.usedBytes,offset+length)},mmap(stream,length,position,prot,flags){if(!FS.isFile(stream.node.mode)){throw new FS.ErrnoError(43)}var ptr;var allocated;var contents=stream.node.contents;if(!(flags&2)&&contents.buffer===HEAP8.buffer){allocated=false;ptr=contents.byteOffset}else{if(position>0||position+length<contents.length){if(contents.subarray){contents=contents.subarray(position,position+length)}else{contents=Array.prototype.slice.call(contents,position,position+length)}}allocated=true;ptr=mmapAlloc(length);if(!ptr){throw new FS.ErrnoError(48)}HEAP8.set(contents,ptr)}return{ptr:ptr,allocated:allocated}},msync(stream,buffer,offset,length,mmapFlags){MEMFS.stream_ops.write(stream,buffer,0,length,offset,false);return 0}}};var asyncLoad=(url,onload,onerror,noRunDep)=>{var dep=!noRunDep?getUniqueRunDependency(`al ${url}`):"";readAsync(url,arrayBuffer=>{onload(new Uint8Array(arrayBuffer));if(dep)removeRunDependency(dep)},event=>{if(onerror){onerror()}else{throw`Loading data file "${url}" failed.`}});if(dep)addRunDependency(dep)};var FS_createDataFile=(parent,name,fileData,canRead,canWrite,canOwn)=>{FS.createDataFile(parent,name,fileData,canRead,canWrite,canOwn)};var preloadPlugins=Module["preloadPlugins"]||[];var FS_handledByPreloadPlugin=(byteArray,fullname,finish,onerror)=>{if(typeof Browser!="undefined")Browser.init();var handled=false;preloadPlugins.forEach(plugin=>{if(handled)return;if(plugin["canHandle"](fullname)){plugin["handle"](byteArray,fullname,finish,onerror);handled=true}});return handled};var FS_createPreloadedFile=(parent,name,url,canRead,canWrite,onload,onerror,dontCreateFile,canOwn,preFinish)=>{var fullname=name?PATH_FS.resolve(PATH.join2(parent,name)):parent;var dep=getUniqueRunDependency(`cp ${fullname}`);function processData(byteArray){function finish(byteArray){preFinish?.();if(!dontCreateFile){FS_createDataFile(parent,name,byteArray,canRead,canWrite,canOwn)}onload?.();removeRunDependency(dep)}if(FS_handledByPreloadPlugin(byteArray,fullname,finish,()=>{onerror?.();removeRunDependency(dep)})){return}finish(byteArray)}addRunDependency(dep);if(typeof url=="string"){asyncLoad(url,processData,onerror)}else{processData(url)}};var FS_modeStringToFlags=str=>{var flagModes={r:0,"r+":2,w:512|64|1,"w+":512|64|2,a:1024|64|1,"a+":1024|64|2};var flags=flagModes[str];if(typeof flags=="undefined"){throw new Error(`Unknown file open mode: ${str}`)}return flags};var FS_getMode=(canRead,canWrite)=>{var mode=0;if(canRead)mode|=292|73;if(canWrite)mode|=146;return mode};var FS={root:null,mounts:[],devices:{},streams:[],nextInode:1,nameTable:null,currentPath:"/",initialized:false,ignorePermissions:true,ErrnoError:class{constructor(errno){this.name="ErrnoError";this.errno=errno}},genericErrors:{},filesystems:null,syncFSRequests:0,FSStream:class{constructor(){this.shared={}}get object(){return this.node}set object(val){this.node=val}get isRead(){return(this.flags&2097155)!==1}get isWrite(){return(this.flags&2097155)!==0}get isAppend(){return this.flags&1024}get flags(){return this.shared.flags}set flags(val){this.shared.flags=val}get position(){return this.shared.position}set position(val){this.shared.position=val}},FSNode:class{constructor(parent,name,mode,rdev){if(!parent){parent=this}this.parent=parent;this.mount=parent.mount;this.mounted=null;this.id=FS.nextInode++;this.name=name;this.mode=mode;this.node_ops={};this.stream_ops={};this.rdev=rdev;this.readMode=292|73;this.writeMode=146}get read(){return(this.mode&this.readMode)===this.readMode}set read(val){val?this.mode|=this.readMode:this.mode&=~this.readMode}get write(){return(this.mode&this.writeMode)===this.writeMode}set write(val){val?this.mode|=this.writeMode:this.mode&=~this.writeMode}get isFolder(){return FS.isDir(this.mode)}get isDevice(){return FS.isChrdev(this.mode)}},lookupPath(path,opts={}){path=PATH_FS.resolve(path);if(!path)return{path:"",node:null};var defaults={follow_mount:true,recurse_count:0};opts=Object.assign(defaults,opts);if(opts.recurse_count>8){throw new FS.ErrnoError(32)}var parts=path.split("/").filter(p=>!!p);var current=FS.root;var current_path="/";for(var i=0;i<parts.length;i++){var islast=i===parts.length-1;if(islast&&opts.parent){break}current=FS.lookupNode(current,parts[i]);current_path=PATH.join2(current_path,parts[i]);if(FS.isMountpoint(current)){if(!islast||islast&&opts.follow_mount){current=current.mounted.root}}if(!islast||opts.follow){var count=0;while(FS.isLink(current.mode)){var link=FS.readlink(current_path);current_path=PATH_FS.resolve(PATH.dirname(current_path),link);var lookup=FS.lookupPath(current_path,{recurse_count:opts.recurse_count+1});current=lookup.node;if(count++>40){throw new FS.ErrnoError(32)}}}}return{path:current_path,node:current}},getPath(node){var path;while(true){if(FS.isRoot(node)){var mount=node.mount.mountpoint;if(!path)return mount;return mount[mount.length-1]!=="/"?`${mount}/${path}`:mount+path}path=path?`${node.name}/${path}`:node.name;node=node.parent}},hashName(parentid,name){var hash=0;for(var i=0;i<name.length;i++){hash=(hash<<5)-hash+name.charCodeAt(i)|0}return(parentid+hash>>>0)%FS.nameTable.length},hashAddNode(node){var hash=FS.hashName(node.parent.id,node.name);node.name_next=FS.nameTable[hash];FS.nameTable[hash]=node},hashRemoveNode(node){var hash=FS.hashName(node.parent.id,node.name);if(FS.nameTable[hash]===node){FS.nameTable[hash]=node.name_next}else{var current=FS.nameTable[hash];while(current){if(current.name_next===node){current.name_next=node.name_next;break}current=current.name_next}}},lookupNode(parent,name){var errCode=FS.mayLookup(parent);if(errCode){throw new FS.ErrnoError(errCode)}var hash=FS.hashName(parent.id,name);for(var node=FS.nameTable[hash];node;node=node.name_next){var nodeName=node.name;if(node.parent.id===parent.id&&nodeName===name){return node}}return FS.lookup(parent,name)},createNode(parent,name,mode,rdev){var node=new FS.FSNode(parent,name,mode,rdev);FS.hashAddNode(node);return node},destroyNode(node){FS.hashRemoveNode(node)},isRoot(node){return node===node.parent},isMountpoint(node){return!!node.mounted},isFile(mode){return(mode&61440)===32768},isDir(mode){return(mode&61440)===16384},isLink(mode){return(mode&61440)===40960},isChrdev(mode){return(mode&61440)===8192},isBlkdev(mode){return(mode&61440)===24576},isFIFO(mode){return(mode&61440)===4096},isSocket(mode){return(mode&49152)===49152},flagsToPermissionString(flag){var perms=["r","w","rw"][flag&3];if(flag&512){perms+="w"}return perms},nodePermissions(node,perms){if(FS.ignorePermissions){return 0}if(perms.includes("r")&&!(node.mode&292)){return 2}else if(perms.includes("w")&&!(node.mode&146)){return 2}else if(perms.includes("x")&&!(node.mode&73)){return 2}return 0},mayLookup(dir){if(!FS.isDir(dir.mode))return 54;var errCode=FS.nodePermissions(dir,"x");if(errCode)return errCode;if(!dir.node_ops.lookup)return 2;return 0},mayCreate(dir,name){try{var node=FS.lookupNode(dir,name);return 20}catch(e){}return FS.nodePermissions(dir,"wx")},mayDelete(dir,name,isdir){var node;try{node=FS.lookupNode(dir,name)}catch(e){return e.errno}var errCode=FS.nodePermissions(dir,"wx");if(errCode){return errCode}if(isdir){if(!FS.isDir(node.mode)){return 54}if(FS.isRoot(node)||FS.getPath(node)===FS.cwd()){return 10}}else{if(FS.isDir(node.mode)){return 31}}return 0},mayOpen(node,flags){if(!node){return 44}if(FS.isLink(node.mode)){return 32}else if(FS.isDir(node.mode)){if(FS.flagsToPermissionString(flags)!=="r"||flags&512){return 31}}return FS.nodePermissions(node,FS.flagsToPermissionString(flags))},MAX_OPEN_FDS:4096,nextfd(){for(var fd=0;fd<=FS.MAX_OPEN_FDS;fd++){if(!FS.streams[fd]){return fd}}throw new FS.ErrnoError(33)},getStreamChecked(fd){var stream=FS.getStream(fd);if(!stream){throw new FS.ErrnoError(8)}return stream},getStream:fd=>FS.streams[fd],createStream(stream,fd=-1){stream=Object.assign(new FS.FSStream,stream);if(fd==-1){fd=FS.nextfd()}stream.fd=fd;FS.streams[fd]=stream;return stream},closeStream(fd){FS.streams[fd]=null},dupStream(origStream,fd=-1){var stream=FS.createStream(origStream,fd);stream.stream_ops?.dup?.(stream);return stream},chrdev_stream_ops:{open(stream){var device=FS.getDevice(stream.node.rdev);stream.stream_ops=device.stream_ops;stream.stream_ops.open?.(stream)},llseek(){throw new FS.ErrnoError(70)}},major:dev=>dev>>8,minor:dev=>dev&255,makedev:(ma,mi)=>ma<<8|mi,registerDevice(dev,ops){FS.devices[dev]={stream_ops:ops}},getDevice:dev=>FS.devices[dev],getMounts(mount){var mounts=[];var check=[mount];while(check.length){var m=check.pop();mounts.push(m);check.push(...m.mounts)}return mounts},syncfs(populate,callback){if(typeof populate=="function"){callback=populate;populate=false}FS.syncFSRequests++;if(FS.syncFSRequests>1){err(`warning: ${FS.syncFSRequests} FS.syncfs operations in flight at once, probably just doing extra work`)}var mounts=FS.getMounts(FS.root.mount);var completed=0;function doCallback(errCode){FS.syncFSRequests--;return callback(errCode)}function done(errCode){if(errCode){if(!done.errored){done.errored=true;return doCallback(errCode)}return}if(++completed>=mounts.length){doCallback(null)}}mounts.forEach(mount=>{if(!mount.type.syncfs){return done(null)}mount.type.syncfs(mount,populate,done)})},mount(type,opts,mountpoint){var root=mountpoint==="/";var pseudo=!mountpoint;var node;if(root&&FS.root){throw new FS.ErrnoError(10)}else if(!root&&!pseudo){var lookup=FS.lookupPath(mountpoint,{follow_mount:false});mountpoint=lookup.path;node=lookup.node;if(FS.isMountpoint(node)){throw new FS.ErrnoError(10)}if(!FS.isDir(node.mode)){throw new FS.ErrnoError(54)}}var mount={type:type,opts:opts,mountpoint:mountpoint,mounts:[]};var mountRoot=type.mount(mount);mountRoot.mount=mount;mount.root=mountRoot;if(root){FS.root=mountRoot}else if(node){node.mounted=mount;if(node.mount){node.mount.mounts.push(mount)}}return mountRoot},unmount(mountpoint){var lookup=FS.lookupPath(mountpoint,{follow_mount:false});if(!FS.isMountpoint(lookup.node)){throw new FS.ErrnoError(28)}var node=lookup.node;var mount=node.mounted;var mounts=FS.getMounts(mount);Object.keys(FS.nameTable).forEach(hash=>{var current=FS.nameTable[hash];while(current){var next=current.name_next;if(mounts.includes(current.mount)){FS.destroyNode(current)}current=next}});node.mounted=null;var idx=node.mount.mounts.indexOf(mount);node.mount.mounts.splice(idx,1)},lookup(parent,name){return parent.node_ops.lookup(parent,name)},mknod(path,mode,dev){var lookup=FS.lookupPath(path,{parent:true});var parent=lookup.node;var name=PATH.basename(path);if(!name||name==="."||name===".."){throw new FS.ErrnoError(28)}var errCode=FS.mayCreate(parent,name);if(errCode){throw new FS.ErrnoError(errCode)}if(!parent.node_ops.mknod){throw new FS.ErrnoError(63)}return parent.node_ops.mknod(parent,name,mode,dev)},create(path,mode){mode=mode!==undefined?mode:438;mode&=4095;mode|=32768;return FS.mknod(path,mode,0)},mkdir(path,mode){mode=mode!==undefined?mode:511;mode&=511|512;mode|=16384;return FS.mknod(path,mode,0)},mkdirTree(path,mode){var dirs=path.split("/");var d="";for(var i=0;i<dirs.length;++i){if(!dirs[i])continue;d+="/"+dirs[i];try{FS.mkdir(d,mode)}catch(e){if(e.errno!=20)throw e}}},mkdev(path,mode,dev){if(typeof dev=="undefined"){dev=mode;mode=438}mode|=8192;return FS.mknod(path,mode,dev)},symlink(oldpath,newpath){if(!PATH_FS.resolve(oldpath)){throw new FS.ErrnoError(44)}var lookup=FS.lookupPath(newpath,{parent:true});var parent=lookup.node;if(!parent){throw new FS.ErrnoError(44)}var newname=PATH.basename(newpath);var errCode=FS.mayCreate(parent,newname);if(errCode){throw new FS.ErrnoError(errCode)}if(!parent.node_ops.symlink){throw new FS.ErrnoError(63)}return parent.node_ops.symlink(parent,newname,oldpath)},rename(old_path,new_path){var old_dirname=PATH.dirname(old_path);var new_dirname=PATH.dirname(new_path);var old_name=PATH.basename(old_path);var new_name=PATH.basename(new_path);var lookup,old_dir,new_dir;lookup=FS.lookupPath(old_path,{parent:true});old_dir=lookup.node;lookup=FS.lookupPath(new_path,{parent:true});new_dir=lookup.node;if(!old_dir||!new_dir)throw new FS.ErrnoError(44);if(old_dir.mount!==new_dir.mount){throw new FS.ErrnoError(75)}var old_node=FS.lookupNode(old_dir,old_name);var relative=PATH_FS.relative(old_path,new_dirname);if(relative.charAt(0)!=="."){throw new FS.ErrnoError(28)}relative=PATH_FS.relative(new_path,old_dirname);if(relative.charAt(0)!=="."){throw new FS.ErrnoError(55)}var new_node;try{new_node=FS.lookupNode(new_dir,new_name)}catch(e){}if(old_node===new_node){return}var isdir=FS.isDir(old_node.mode);var errCode=FS.mayDelete(old_dir,old_name,isdir);if(errCode){throw new FS.ErrnoError(errCode)}errCode=new_node?FS.mayDelete(new_dir,new_name,isdir):FS.mayCreate(new_dir,new_name);if(errCode){throw new FS.ErrnoError(errCode)}if(!old_dir.node_ops.rename){throw new FS.ErrnoError(63)}if(FS.isMountpoint(old_node)||new_node&&FS.isMountpoint(new_node)){throw new FS.ErrnoError(10)}if(new_dir!==old_dir){errCode=FS.nodePermissions(old_dir,"w");if(errCode){throw new FS.ErrnoError(errCode)}}FS.hashRemoveNode(old_node);try{old_dir.node_ops.rename(old_node,new_dir,new_name)}catch(e){throw e}finally{FS.hashAddNode(old_node)}},rmdir(path){var lookup=FS.lookupPath(path,{parent:true});var parent=lookup.node;var name=PATH.basename(path);var node=FS.lookupNode(parent,name);var errCode=FS.mayDelete(parent,name,true);if(errCode){throw new FS.ErrnoError(errCode)}if(!parent.node_ops.rmdir){throw new FS.ErrnoError(63)}if(FS.isMountpoint(node)){throw new FS.ErrnoError(10)}parent.node_ops.rmdir(parent,name);FS.destroyNode(node)},readdir(path){var lookup=FS.lookupPath(path,{follow:true});var node=lookup.node;if(!node.node_ops.readdir){throw new FS.ErrnoError(54)}return node.node_ops.readdir(node)},unlink(path){var lookup=FS.lookupPath(path,{parent:true});var parent=lookup.node;if(!parent){throw new FS.ErrnoError(44)}var name=PATH.basename(path);var node=FS.lookupNode(parent,name);var errCode=FS.mayDelete(parent,name,false);if(errCode){throw new FS.ErrnoError(errCode)}if(!parent.node_ops.unlink){throw new FS.ErrnoError(63)}if(FS.isMountpoint(node)){throw new FS.ErrnoError(10)}parent.node_ops.unlink(parent,name);FS.destroyNode(node)},readlink(path){var lookup=FS.lookupPath(path);var link=lookup.node;if(!link){throw new FS.ErrnoError(44)}if(!link.node_ops.readlink){throw new FS.ErrnoError(28)}return PATH_FS.resolve(FS.getPath(link.parent),link.node_ops.readlink(link))},stat(path,dontFollow){var lookup=FS.lookupPath(path,{follow:!dontFollow});var node=lookup.node;if(!node){throw new FS.ErrnoError(44)}if(!node.node_ops.getattr){throw new FS.ErrnoError(63)}return node.node_ops.getattr(node)},lstat(path){return FS.stat(path,true)},chmod(path,mode,dontFollow){var node;if(typeof path=="string"){var lookup=FS.lookupPath(path,{follow:!dontFollow});node=lookup.node}else{node=path}if(!node.node_ops.setattr){throw new FS.ErrnoError(63)}node.node_ops.setattr(node,{mode:mode&4095|node.mode&~4095,timestamp:Date.now()})},lchmod(path,mode){FS.chmod(path,mode,true)},fchmod(fd,mode){var stream=FS.getStreamChecked(fd);FS.chmod(stream.node,mode)},chown(path,uid,gid,dontFollow){var node;if(typeof path=="string"){var lookup=FS.lookupPath(path,{follow:!dontFollow});node=lookup.node}else{node=path}if(!node.node_ops.setattr){throw new FS.ErrnoError(63)}node.node_ops.setattr(node,{timestamp:Date.now()})},lchown(path,uid,gid){FS.chown(path,uid,gid,true)},fchown(fd,uid,gid){var stream=FS.getStreamChecked(fd);FS.chown(stream.node,uid,gid)},truncate(path,len){if(len<0){throw new FS.ErrnoError(28)}var node;if(typeof path=="string"){var lookup=FS.lookupPath(path,{follow:true});node=lookup.node}else{node=path}if(!node.node_ops.setattr){throw new FS.ErrnoError(63)}if(FS.isDir(node.mode)){throw new FS.ErrnoError(31)}if(!FS.isFile(node.mode)){throw new FS.ErrnoError(28)}var errCode=FS.nodePermissions(node,"w");if(errCode){throw new FS.ErrnoError(errCode)}node.node_ops.setattr(node,{size:len,timestamp:Date.now()})},ftruncate(fd,len){var stream=FS.getStreamChecked(fd);if((stream.flags&2097155)===0){throw new FS.ErrnoError(28)}FS.truncate(stream.node,len)},utime(path,atime,mtime){var lookup=FS.lookupPath(path,{follow:true});var node=lookup.node;node.node_ops.setattr(node,{timestamp:Math.max(atime,mtime)})},open(path,flags,mode){if(path===""){throw new FS.ErrnoError(44)}flags=typeof flags=="string"?FS_modeStringToFlags(flags):flags;if(flags&64){mode=typeof mode=="undefined"?438:mode;mode=mode&4095|32768}else{mode=0}var node;if(typeof path=="object"){node=path}else{path=PATH.normalize(path);try{var lookup=FS.lookupPath(path,{follow:!(flags&131072)});node=lookup.node}catch(e){}}var created=false;if(flags&64){if(node){if(flags&128){throw new FS.ErrnoError(20)}}else{node=FS.mknod(path,mode,0);created=true}}if(!node){throw new FS.ErrnoError(44)}if(FS.isChrdev(node.mode)){flags&=~512}if(flags&65536&&!FS.isDir(node.mode)){throw new FS.ErrnoError(54)}if(!created){var errCode=FS.mayOpen(node,flags);if(errCode){throw new FS.ErrnoError(errCode)}}if(flags&512&&!created){FS.truncate(node,0)}flags&=~(128|512|131072);var stream=FS.createStream({node:node,path:FS.getPath(node),flags:flags,seekable:true,position:0,stream_ops:node.stream_ops,ungotten:[],error:false});if(stream.stream_ops.open){stream.stream_ops.open(stream)}if(Module["logReadFiles"]&&!(flags&1)){if(!FS.readFiles)FS.readFiles={};if(!(path in FS.readFiles)){FS.readFiles[path]=1}}return stream},close(stream){if(FS.isClosed(stream)){throw new FS.ErrnoError(8)}if(stream.getdents)stream.getdents=null;try{if(stream.stream_ops.close){stream.stream_ops.close(stream)}}catch(e){throw e}finally{FS.closeStream(stream.fd)}stream.fd=null},isClosed(stream){return stream.fd===null},llseek(stream,offset,whence){if(FS.isClosed(stream)){throw new FS.ErrnoError(8)}if(!stream.seekable||!stream.stream_ops.llseek){throw new FS.ErrnoError(70)}if(whence!=0&&whence!=1&&whence!=2){throw new FS.ErrnoError(28)}stream.position=stream.stream_ops.llseek(stream,offset,whence);stream.ungotten=[];return stream.position},read(stream,buffer,offset,length,position){if(length<0||position<0){throw new FS.ErrnoError(28)}if(FS.isClosed(stream)){throw new FS.ErrnoError(8)}if((stream.flags&2097155)===1){throw new FS.ErrnoError(8)}if(FS.isDir(stream.node.mode)){throw new FS.ErrnoError(31)}if(!stream.stream_ops.read){throw new FS.ErrnoError(28)}var seeking=typeof position!="undefined";if(!seeking){position=stream.position}else if(!stream.seekable){throw new FS.ErrnoError(70)}var bytesRead=stream.stream_ops.read(stream,buffer,offset,length,position);if(!seeking)stream.position+=bytesRead;return bytesRead},write(stream,buffer,offset,length,position,canOwn){if(length<0||position<0){throw new FS.ErrnoError(28)}if(FS.isClosed(stream)){throw new FS.ErrnoError(8)}if((stream.flags&2097155)===0){throw new FS.ErrnoError(8)}if(FS.isDir(stream.node.mode)){throw new FS.ErrnoError(31)}if(!stream.stream_ops.write){throw new FS.ErrnoError(28)}if(stream.seekable&&stream.flags&1024){FS.llseek(stream,0,2)}var seeking=typeof position!="undefined";if(!seeking){position=stream.position}else if(!stream.seekable){throw new FS.ErrnoError(70)}var bytesWritten=stream.stream_ops.write(stream,buffer,offset,length,position,canOwn);if(!seeking)stream.position+=bytesWritten;return bytesWritten},allocate(stream,offset,length){if(FS.isClosed(stream)){throw new FS.ErrnoError(8)}if(offset<0||length<=0){throw new FS.ErrnoError(28)}if((stream.flags&2097155)===0){throw new FS.ErrnoError(8)}if(!FS.isFile(stream.node.mode)&&!FS.isDir(stream.node.mode)){throw new FS.ErrnoError(43)}if(!stream.stream_ops.allocate){throw new FS.ErrnoError(138)}stream.stream_ops.allocate(stream,offset,length)},mmap(stream,length,position,prot,flags){if((prot&2)!==0&&(flags&2)===0&&(stream.flags&2097155)!==2){throw new FS.ErrnoError(2)}if((stream.flags&2097155)===1){throw new FS.ErrnoError(2)}if(!stream.stream_ops.mmap){throw new FS.ErrnoError(43)}return stream.stream_ops.mmap(stream,length,position,prot,flags)},msync(stream,buffer,offset,length,mmapFlags){if(!stream.stream_ops.msync){return 0}return stream.stream_ops.msync(stream,buffer,offset,length,mmapFlags)},ioctl(stream,cmd,arg){if(!stream.stream_ops.ioctl){throw new FS.ErrnoError(59)}return stream.stream_ops.ioctl(stream,cmd,arg)},readFile(path,opts={}){opts.flags=opts.flags||0;opts.encoding=opts.encoding||"binary";if(opts.encoding!=="utf8"&&opts.encoding!=="binary"){throw new Error(`Invalid encoding type "${opts.encoding}"`)}var ret;var stream=FS.open(path,opts.flags);var stat=FS.stat(path);var length=stat.size;var buf=new Uint8Array(length);FS.read(stream,buf,0,length,0);if(opts.encoding==="utf8"){ret=UTF8ArrayToString(buf,0)}else if(opts.encoding==="binary"){ret=buf}FS.close(stream);return ret},writeFile(path,data,opts={}){opts.flags=opts.flags||577;var stream=FS.open(path,opts.flags,opts.mode);if(typeof data=="string"){var buf=new Uint8Array(lengthBytesUTF8(data)+1);var actualNumBytes=stringToUTF8Array(data,buf,0,buf.length);FS.write(stream,buf,0,actualNumBytes,undefined,opts.canOwn)}else if(ArrayBuffer.isView(data)){FS.write(stream,data,0,data.byteLength,undefined,opts.canOwn)}else{throw new Error("Unsupported data type")}FS.close(stream)},cwd:()=>FS.currentPath,chdir(path){var lookup=FS.lookupPath(path,{follow:true});if(lookup.node===null){throw new FS.ErrnoError(44)}if(!FS.isDir(lookup.node.mode)){throw new FS.ErrnoError(54)}var errCode=FS.nodePermissions(lookup.node,"x");if(errCode){throw new FS.ErrnoError(errCode)}FS.currentPath=lookup.path},createDefaultDirectories(){FS.mkdir("/tmp");FS.mkdir("/home");FS.mkdir("/home/web_user")},createDefaultDevices(){FS.mkdir("/dev");FS.registerDevice(FS.makedev(1,3),{read:()=>0,write:(stream,buffer,offset,length,pos)=>length});FS.mkdev("/dev/null",FS.makedev(1,3));TTY.register(FS.makedev(5,0),TTY.default_tty_ops);TTY.register(FS.makedev(6,0),TTY.default_tty1_ops);FS.mkdev("/dev/tty",FS.makedev(5,0));FS.mkdev("/dev/tty1",FS.makedev(6,0));var randomBuffer=new Uint8Array(1024),randomLeft=0;var randomByte=()=>{if(randomLeft===0){randomLeft=randomFill(randomBuffer).byteLength}return randomBuffer[--randomLeft]};FS.createDevice("/dev","random",randomByte);FS.createDevice("/dev","urandom",randomByte);FS.mkdir("/dev/shm");FS.mkdir("/dev/shm/tmp")},createSpecialDirectories(){FS.mkdir("/proc");var proc_self=FS.mkdir("/proc/self");FS.mkdir("/proc/self/fd");FS.mount({mount(){var node=FS.createNode(proc_self,"fd",16384|511,73);node.node_ops={lookup(parent,name){var fd=+name;var stream=FS.getStreamChecked(fd);var ret={parent:null,mount:{mountpoint:"fake"},node_ops:{readlink:()=>stream.path}};ret.parent=ret;return ret}};return node}},{},"/proc/self/fd")},createStandardStreams(){if(Module["stdin"]){FS.createDevice("/dev","stdin",Module["stdin"])}else{FS.symlink("/dev/tty","/dev/stdin")}if(Module["stdout"]){FS.createDevice("/dev","stdout",null,Module["stdout"])}else{FS.symlink("/dev/tty","/dev/stdout")}if(Module["stderr"]){FS.createDevice("/dev","stderr",null,Module["stderr"])}else{FS.symlink("/dev/tty1","/dev/stderr")}var stdin=FS.open("/dev/stdin",0);var stdout=FS.open("/dev/stdout",1);var stderr=FS.open("/dev/stderr",1)},staticInit(){[44].forEach(code=>{FS.genericErrors[code]=new FS.ErrnoError(code);FS.genericErrors[code].stack="<generic error, no stack>"});FS.nameTable=new Array(4096);FS.mount(MEMFS,{},"/");FS.createDefaultDirectories();FS.createDefaultDevices();FS.createSpecialDirectories();FS.filesystems={MEMFS:MEMFS}},init(input,output,error){FS.init.initialized=true;Module["stdin"]=input||Module["stdin"];Module["stdout"]=output||Module["stdout"];Module["stderr"]=error||Module["stderr"];FS.createStandardStreams()},quit(){FS.init.initialized=false;for(var i=0;i<FS.streams.length;i++){var stream=FS.streams[i];if(!stream){continue}FS.close(stream)}},findObject(path,dontResolveLastLink){var ret=FS.analyzePath(path,dontResolveLastLink);if(!ret.exists){return null}return ret.object},analyzePath(path,dontResolveLastLink){try{var lookup=FS.lookupPath(path,{follow:!dontResolveLastLink});path=lookup.path}catch(e){}var ret={isRoot:false,exists:false,error:0,name:null,path:null,object:null,parentExists:false,parentPath:null,parentObject:null};try{var lookup=FS.lookupPath(path,{parent:true});ret.parentExists=true;ret.parentPath=lookup.path;ret.parentObject=lookup.node;ret.name=PATH.basename(path);lookup=FS.lookupPath(path,{follow:!dontResolveLastLink});ret.exists=true;ret.path=lookup.path;ret.object=lookup.node;ret.name=lookup.node.name;ret.isRoot=lookup.path==="/"}catch(e){ret.error=e.errno}return ret},createPath(parent,path,canRead,canWrite){parent=typeof parent=="string"?parent:FS.getPath(parent);var parts=path.split("/").reverse();while(parts.length){var part=parts.pop();if(!part)continue;var current=PATH.join2(parent,part);try{FS.mkdir(current)}catch(e){}parent=current}return current},createFile(parent,name,properties,canRead,canWrite){var path=PATH.join2(typeof parent=="string"?parent:FS.getPath(parent),name);var mode=FS_getMode(canRead,canWrite);return FS.create(path,mode)},createDataFile(parent,name,data,canRead,canWrite,canOwn){var path=name;if(parent){parent=typeof parent=="string"?parent:FS.getPath(parent);path=name?PATH.join2(parent,name):parent}var mode=FS_getMode(canRead,canWrite);var node=FS.create(path,mode);if(data){if(typeof data=="string"){var arr=new Array(data.length);for(var i=0,len=data.length;i<len;++i)arr[i]=data.charCodeAt(i);data=arr}FS.chmod(node,mode|146);var stream=FS.open(node,577);FS.write(stream,data,0,data.length,0,canOwn);FS.close(stream);FS.chmod(node,mode)}},createDevice(parent,name,input,output){var path=PATH.join2(typeof parent=="string"?parent:FS.getPath(parent),name);var mode=FS_getMode(!!input,!!output);if(!FS.createDevice.major)FS.createDevice.major=64;var dev=FS.makedev(FS.createDevice.major++,0);FS.registerDevice(dev,{open(stream){stream.seekable=false},close(stream){if(output?.buffer?.length){output(10)}},read(stream,buffer,offset,length,pos){var bytesRead=0;for(var i=0;i<length;i++){var result;try{result=input()}catch(e){throw new FS.ErrnoError(29)}if(result===undefined&&bytesRead===0){throw new FS.ErrnoError(6)}if(result===null||result===undefined)break;bytesRead++;buffer[offset+i]=result}if(bytesRead){stream.node.timestamp=Date.now()}return bytesRead},write(stream,buffer,offset,length,pos){for(var i=0;i<length;i++){try{output(buffer[offset+i])}catch(e){throw new FS.ErrnoError(29)}}if(length){stream.node.timestamp=Date.now()}return i}});return FS.mkdev(path,mode,dev)},forceLoadFile(obj){if(obj.isDevice||obj.isFolder||obj.link||obj.contents)return true;if(typeof XMLHttpRequest!="undefined"){throw new Error("Lazy loading should have been performed (contents set) in createLazyFile, but it was not. Lazy loading only works in web workers. Use --embed-file or --preload-file in emcc on the main thread.")}else if(read_){try{obj.contents=intArrayFromString(read_(obj.url),true);obj.usedBytes=obj.contents.length}catch(e){throw new FS.ErrnoError(29)}}else{throw new Error("Cannot load without read() or XMLHttpRequest.")}},createLazyFile(parent,name,url,canRead,canWrite){class LazyUint8Array{constructor(){this.lengthKnown=false;this.chunks=[]}get(idx){if(idx>this.length-1||idx<0){return undefined}var chunkOffset=idx%this.chunkSize;var chunkNum=idx/this.chunkSize|0;return this.getter(chunkNum)[chunkOffset]}setDataGetter(getter){this.getter=getter}cacheLength(){var xhr=new XMLHttpRequest;xhr.open("HEAD",url,false);xhr.send(null);if(!(xhr.status>=200&&xhr.status<300||xhr.status===304))throw new Error("Couldn't load "+url+". Status: "+xhr.status);var datalength=Number(xhr.getResponseHeader("Content-length"));var header;var hasByteServing=(header=xhr.getResponseHeader("Accept-Ranges"))&&header==="bytes";var usesGzip=(header=xhr.getResponseHeader("Content-Encoding"))&&header==="gzip";var chunkSize=1024*1024;if(!hasByteServing)chunkSize=datalength;var doXHR=(from,to)=>{if(from>to)throw new Error("invalid range ("+from+", "+to+") or no bytes requested!");if(to>datalength-1)throw new Error("only "+datalength+" bytes available! programmer error!");var xhr=new XMLHttpRequest;xhr.open("GET",url,false);if(datalength!==chunkSize)xhr.setRequestHeader("Range","bytes="+from+"-"+to);xhr.responseType="arraybuffer";if(xhr.overrideMimeType){xhr.overrideMimeType("text/plain; charset=x-user-defined")}xhr.send(null);if(!(xhr.status>=200&&xhr.status<300||xhr.status===304))throw new Error("Couldn't load "+url+". Status: "+xhr.status);if(xhr.response!==undefined){return new Uint8Array(xhr.response||[])}return intArrayFromString(xhr.responseText||"",true)};var lazyArray=this;lazyArray.setDataGetter(chunkNum=>{var start=chunkNum*chunkSize;var end=(chunkNum+1)*chunkSize-1;end=Math.min(end,datalength-1);if(typeof lazyArray.chunks[chunkNum]=="undefined"){lazyArray.chunks[chunkNum]=doXHR(start,end)}if(typeof lazyArray.chunks[chunkNum]=="undefined")throw new Error("doXHR failed!");return lazyArray.chunks[chunkNum]});if(usesGzip||!datalength){chunkSize=datalength=1;datalength=this.getter(0).length;chunkSize=datalength;out("LazyFiles on gzip forces download of the whole file when length is accessed")}this._length=datalength;this._chunkSize=chunkSize;this.lengthKnown=true}get length(){if(!this.lengthKnown){this.cacheLength()}return this._length}get chunkSize(){if(!this.lengthKnown){this.cacheLength()}return this._chunkSize}}if(typeof XMLHttpRequest!="undefined"){if(!ENVIRONMENT_IS_WORKER)throw"Cannot do synchronous binary XHRs outside webworkers in modern browsers. Use --embed-file or --preload-file in emcc";var lazyArray=new LazyUint8Array;var properties={isDevice:false,contents:lazyArray}}else{var properties={isDevice:false,url:url}}var node=FS.createFile(parent,name,properties,canRead,canWrite);if(properties.contents){node.contents=properties.contents}else if(properties.url){node.contents=null;node.url=properties.url}Object.defineProperties(node,{usedBytes:{get:function(){return this.contents.length}}});var stream_ops={};var keys=Object.keys(node.stream_ops);keys.forEach(key=>{var fn=node.stream_ops[key];stream_ops[key]=(...args)=>{FS.forceLoadFile(node);return fn(...args)}});function writeChunks(stream,buffer,offset,length,position){var contents=stream.node.contents;if(position>=contents.length)return 0;var size=Math.min(contents.length-position,length);if(contents.slice){for(var i=0;i<size;i++){buffer[offset+i]=contents[position+i]}}else{for(var i=0;i<size;i++){buffer[offset+i]=contents.get(position+i)}}return size}stream_ops.read=(stream,buffer,offset,length,position)=>{FS.forceLoadFile(node);return writeChunks(stream,buffer,offset,length,position)};stream_ops.mmap=(stream,length,position,prot,flags)=>{FS.forceLoadFile(node);var ptr=mmapAlloc(length);if(!ptr){throw new FS.ErrnoError(48)}writeChunks(stream,HEAP8,ptr,length,position);return{ptr:ptr,allocated:true}};node.stream_ops=stream_ops;return node}};var UTF8ToString=(ptr,maxBytesToRead)=>ptr?UTF8ArrayToString(HEAPU8,ptr,maxBytesToRead):"";var SYSCALLS={DEFAULT_POLLMASK:5,calculateAt(dirfd,path,allowEmpty){if(PATH.isAbs(path)){return path}var dir;if(dirfd===-100){dir=FS.cwd()}else{var dirstream=SYSCALLS.getStreamFromFD(dirfd);dir=dirstream.path}if(path.length==0){if(!allowEmpty){throw new FS.ErrnoError(44)}return dir}return PATH.join2(dir,path)},doStat(func,path,buf){var stat=func(path);HEAP32[buf>>2]=stat.dev;HEAP32[buf+4>>2]=stat.mode;HEAPU32[buf+8>>2]=stat.nlink;HEAP32[buf+12>>2]=stat.uid;HEAP32[buf+16>>2]=stat.gid;HEAP32[buf+20>>2]=stat.rdev;tempI64=[stat.size>>>0,(tempDouble=stat.size,+Math.abs(tempDouble)>=1?tempDouble>0?+Math.floor(tempDouble/4294967296)>>>0:~~+Math.ceil((tempDouble-+(~~tempDouble>>>0))/4294967296)>>>0:0)],HEAP32[buf+24>>2]=tempI64[0],HEAP32[buf+28>>2]=tempI64[1];HEAP32[buf+32>>2]=4096;HEAP32[buf+36>>2]=stat.blocks;var atime=stat.atime.getTime();var mtime=stat.mtime.getTime();var ctime=stat.ctime.getTime();tempI64=[Math.floor(atime/1e3)>>>0,(tempDouble=Math.floor(atime/1e3),+Math.abs(tempDouble)>=1?tempDouble>0?+Math.floor(tempDouble/4294967296)>>>0:~~+Math.ceil((tempDouble-+(~~tempDouble>>>0))/4294967296)>>>0:0)],HEAP32[buf+40>>2]=tempI64[0],HEAP32[buf+44>>2]=tempI64[1];HEAPU32[buf+48>>2]=atime%1e3*1e3;tempI64=[Math.floor(mtime/1e3)>>>0,(tempDouble=Math.floor(mtime/1e3),+Math.abs(tempDouble)>=1?tempDouble>0?+Math.floor(tempDouble/4294967296)>>>0:~~+Math.ceil((tempDouble-+(~~tempDouble>>>0))/4294967296)>>>0:0)],HEAP32[buf+56>>2]=tempI64[0],HEAP32[buf+60>>2]=tempI64[1];HEAPU32[buf+64>>2]=mtime%1e3*1e3;tempI64=[Math.floor(ctime/1e3)>>>0,(tempDouble=Math.floor(ctime/1e3),+Math.abs(tempDouble)>=1?tempDouble>0?+Math.floor(tempDouble/4294967296)>>>0:~~+Math.ceil((tempDouble-+(~~tempDouble>>>0))/4294967296)>>>0:0)],HEAP32[buf+72>>2]=tempI64[0],HEAP32[buf+76>>2]=tempI64[1];HEAPU32[buf+80>>2]=ctime%1e3*1e3;tempI64=[stat.ino>>>0,(tempDouble=stat.ino,+Math.abs(tempDouble)>=1?tempDouble>0?+Math.floor(tempDouble/4294967296)>>>0:~~+Math.ceil((tempDouble-+(~~tempDouble>>>0))/4294967296)>>>0:0)],HEAP32[buf+88>>2]=tempI64[0],HEAP32[buf+92>>2]=tempI64[1];return 0},doMsync(addr,stream,len,flags,offset){if(!FS.isFile(stream.node.mode)){throw new FS.ErrnoError(43)}if(flags&2){return 0}var buffer=HEAPU8.slice(addr,addr+len);FS.msync(stream,buffer,offset,len,flags)},getStreamFromFD(fd){var stream=FS.getStreamChecked(fd);return stream},varargs:undefined,getStr(ptr){var ret=UTF8ToString(ptr);return ret}};function ___syscall_fcntl64(fd,cmd,varargs){SYSCALLS.varargs=varargs;try{var stream=SYSCALLS.getStreamFromFD(fd);switch(cmd){case 0:{var arg=syscallGetVarargI();if(arg<0){return-28}while(FS.streams[arg]){arg++}var newStream;newStream=FS.dupStream(stream,arg);return newStream.fd}case 1:case 2:return 0;case 3:return stream.flags;case 4:{var arg=syscallGetVarargI();stream.flags|=arg;return 0}case 12:{var arg=syscallGetVarargP();var offset=0;HEAP16[arg+offset>>1]=2;return 0}case 13:case 14:return 0}return-28}catch(e){if(typeof FS=="undefined"||!(e.name==="ErrnoError"))throw e;return-e.errno}}function ___syscall_ioctl(fd,op,varargs){SYSCALLS.varargs=varargs;try{var stream=SYSCALLS.getStreamFromFD(fd);switch(op){case 21509:{if(!stream.tty)return-59;return 0}case 21505:{if(!stream.tty)return-59;if(stream.tty.ops.ioctl_tcgets){var termios=stream.tty.ops.ioctl_tcgets(stream);var argp=syscallGetVarargP();HEAP32[argp>>2]=termios.c_iflag||0;HEAP32[argp+4>>2]=termios.c_oflag||0;HEAP32[argp+8>>2]=termios.c_cflag||0;HEAP32[argp+12>>2]=termios.c_lflag||0;for(var i=0;i<32;i++){HEAP8[argp+i+17]=termios.c_cc[i]||0}return 0}return 0}case 21510:case 21511:case 21512:{if(!stream.tty)return-59;return 0}case 21506:case 21507:case 21508:{if(!stream.tty)return-59;if(stream.tty.ops.ioctl_tcsets){var argp=syscallGetVarargP();var c_iflag=HEAP32[argp>>2];var c_oflag=HEAP32[argp+4>>2];var c_cflag=HEAP32[argp+8>>2];var c_lflag=HEAP32[argp+12>>2];var c_cc=[];for(var i=0;i<32;i++){c_cc.push(HEAP8[argp+i+17])}return stream.tty.ops.ioctl_tcsets(stream.tty,op,{c_iflag:c_iflag,c_oflag:c_oflag,c_cflag:c_cflag,c_lflag:c_lflag,c_cc:c_cc})}return 0}case 21519:{if(!stream.tty)return-59;var argp=syscallGetVarargP();HEAP32[argp>>2]=0;return 0}case 21520:{if(!stream.tty)return-59;return-28}case 21531:{var argp=syscallGetVarargP();return FS.ioctl(stream,op,argp)}case 21523:{if(!stream.tty)return-59;if(stream.tty.ops.ioctl_tiocgwinsz){var winsize=stream.tty.ops.ioctl_tiocgwinsz(stream.tty);var argp=syscallGetVarargP();HEAP16[argp>>1]=winsize[0];HEAP16[argp+2>>1]=winsize[1]}return 0}case 21524:{if(!stream.tty)return-59;return 0}case 21515:{if(!stream.tty)return-59;return 0}default:return-28}}catch(e){if(typeof FS=="undefined"||!(e.name==="ErrnoError"))throw e;return-e.errno}}function ___syscall_openat(dirfd,path,flags,varargs){SYSCALLS.varargs=varargs;try{path=SYSCALLS.getStr(path);path=SYSCALLS.calculateAt(dirfd,path);var mode=varargs?syscallGetVarargI():0;return FS.open(path,flags,mode).fd}catch(e){if(typeof FS=="undefined"||!(e.name==="ErrnoError"))throw e;return-e.errno}}function ___syscall_stat64(path,buf){try{path=SYSCALLS.getStr(path);return SYSCALLS.doStat(FS.stat,path,buf)}catch(e){if(typeof FS=="undefined"||!(e.name==="ErrnoError"))throw e;return-e.errno}}var __abort_js=()=>{abort("")};var tupleRegistrations={};var runDestructors=destructors=>{while(destructors.length){var ptr=destructors.pop();var del=destructors.pop();del(ptr)}};function readPointer(pointer){return this["fromWireType"](HEAPU32[pointer>>2])}var awaitingDependencies={};var registeredTypes={};var typeDependencies={};var InternalError;var throwInternalError=message=>{throw new InternalError(message)};var whenDependentTypesAreResolved=(myTypes,dependentTypes,getTypeConverters)=>{myTypes.forEach(function(type){typeDependencies[type]=dependentTypes});function onComplete(typeConverters){var myTypeConverters=getTypeConverters(typeConverters);if(myTypeConverters.length!==myTypes.length){throwInternalError("Mismatched type converter count")}for(var i=0;i<myTypes.length;++i){registerType(myTypes[i],myTypeConverters[i])}}var typeConverters=new Array(dependentTypes.length);var unregisteredTypes=[];var registered=0;dependentTypes.forEach((dt,i)=>{if(registeredTypes.hasOwnProperty(dt)){typeConverters[i]=registeredTypes[dt]}else{unregisteredTypes.push(dt);if(!awaitingDependencies.hasOwnProperty(dt)){awaitingDependencies[dt]=[]}awaitingDependencies[dt].push(()=>{typeConverters[i]=registeredTypes[dt];++registered;if(registered===unregisteredTypes.length){onComplete(typeConverters)}})}});if(0===unregisteredTypes.length){onComplete(typeConverters)}};var __embind_finalize_value_array=rawTupleType=>{var reg=tupleRegistrations[rawTupleType];delete tupleRegistrations[rawTupleType];var elements=reg.elements;var elementsLength=elements.length;var elementTypes=elements.map(elt=>elt.getterReturnType).concat(elements.map(elt=>elt.setterArgumentType));var rawConstructor=reg.rawConstructor;var rawDestructor=reg.rawDestructor;whenDependentTypesAreResolved([rawTupleType],elementTypes,elementTypes=>{elements.forEach((elt,i)=>{var getterReturnType=elementTypes[i];var getter=elt.getter;var getterContext=elt.getterContext;var setterArgumentType=elementTypes[i+elementsLength];var setter=elt.setter;var setterContext=elt.setterContext;elt.read=ptr=>getterReturnType["fromWireType"](getter(getterContext,ptr));elt.write=(ptr,o)=>{var destructors=[];setter(setterContext,ptr,setterArgumentType["toWireType"](destructors,o));runDestructors(destructors)}});return[{name:reg.name,fromWireType:ptr=>{var rv=new Array(elementsLength);for(var i=0;i<elementsLength;++i){rv[i]=elements[i].read(ptr)}rawDestructor(ptr);return rv},toWireType:(destructors,o)=>{if(elementsLength!==o.length){throw new TypeError(`Incorrect number of tuple elements for ${reg.name}: expected=${elementsLength}, actual=${o.length}`)}var ptr=rawConstructor();for(var i=0;i<elementsLength;++i){elements[i].write(ptr,o[i])}if(destructors!==null){destructors.push(rawDestructor,ptr)}return ptr},argPackAdvance:GenericWireTypeSize,readValueFromPointer:readPointer,destructorFunction:rawDestructor}]})};var structRegistrations={};var __embind_finalize_value_object=structType=>{var reg=structRegistrations[structType];delete structRegistrations[structType];var rawConstructor=reg.rawConstructor;var rawDestructor=reg.rawDestructor;var fieldRecords=reg.fields;var fieldTypes=fieldRecords.map(field=>field.getterReturnType).concat(fieldRecords.map(field=>field.setterArgumentType));whenDependentTypesAreResolved([structType],fieldTypes,fieldTypes=>{var fields={};fieldRecords.forEach((field,i)=>{var fieldName=field.fieldName;var getterReturnType=fieldTypes[i];var getter=field.getter;var getterContext=field.getterContext;var setterArgumentType=fieldTypes[i+fieldRecords.length];var setter=field.setter;var setterContext=field.setterContext;fields[fieldName]={read:ptr=>getterReturnType["fromWireType"](getter(getterContext,ptr)),write:(ptr,o)=>{var destructors=[];setter(setterContext,ptr,setterArgumentType["toWireType"](destructors,o));runDestructors(destructors)}}});return[{name:reg.name,fromWireType:ptr=>{var rv={};for(var i in fields){rv[i]=fields[i].read(ptr)}rawDestructor(ptr);return rv},toWireType:(destructors,o)=>{for(var fieldName in fields){if(!(fieldName in o)){throw new TypeError(`Missing field: "${fieldName}"`)}}var ptr=rawConstructor();for(fieldName in fields){fields[fieldName].write(ptr,o[fieldName])}if(destructors!==null){destructors.push(rawDestructor,ptr)}return ptr},argPackAdvance:GenericWireTypeSize,readValueFromPointer:readPointer,destructorFunction:rawDestructor}]})};var __embind_register_bigint=(primitiveType,name,size,minRange,maxRange)=>{};var embind_init_charCodes=()=>{var codes=new Array(256);for(var i=0;i<256;++i){codes[i]=String.fromCharCode(i)}embind_charCodes=codes};var embind_charCodes;var readLatin1String=ptr=>{var ret="";var c=ptr;while(HEAPU8[c]){ret+=embind_charCodes[HEAPU8[c++]]}return ret};var BindingError;var throwBindingError=message=>{throw new BindingError(message)};function sharedRegisterType(rawType,registeredInstance,options={}){var name=registeredInstance.name;if(!rawType){throwBindingError(`type "${name}" must have a positive integer typeid pointer`)}if(registeredTypes.hasOwnProperty(rawType)){if(options.ignoreDuplicateRegistrations){return}else{throwBindingError(`Cannot register type '${name}' twice`)}}registeredTypes[rawType]=registeredInstance;delete typeDependencies[rawType];if(awaitingDependencies.hasOwnProperty(rawType)){var callbacks=awaitingDependencies[rawType];delete awaitingDependencies[rawType];callbacks.forEach(cb=>cb())}}function registerType(rawType,registeredInstance,options={}){if(!("argPackAdvance"in registeredInstance)){throw new TypeError("registerType registeredInstance requires argPackAdvance")}return sharedRegisterType(rawType,registeredInstance,options)}var GenericWireTypeSize=8;var __embind_register_bool=(rawType,name,trueValue,falseValue)=>{name=readLatin1String(name);registerType(rawType,{name:name,fromWireType:function(wt){return!!wt},toWireType:function(destructors,o){return o?trueValue:falseValue},argPackAdvance:GenericWireTypeSize,readValueFromPointer:function(pointer){return this["fromWireType"](HEAPU8[pointer])},destructorFunction:null})};var shallowCopyInternalPointer=o=>({count:o.count,deleteScheduled:o.deleteScheduled,preservePointerOnDelete:o.preservePointerOnDelete,ptr:o.ptr,ptrType:o.ptrType,smartPtr:o.smartPtr,smartPtrType:o.smartPtrType});var throwInstanceAlreadyDeleted=obj=>{function getInstanceTypeName(handle){return handle.$$.ptrType.registeredClass.name}throwBindingError(getInstanceTypeName(obj)+" instance already deleted")};var finalizationRegistry=false;var detachFinalizer=handle=>{};var runDestructor=$$=>{if($$.smartPtr){$$.smartPtrType.rawDestructor($$.smartPtr)}else{$$.ptrType.registeredClass.rawDestructor($$.ptr)}};var releaseClassHandle=$$=>{$$.count.value-=1;var toDelete=0===$$.count.value;if(toDelete){runDestructor($$)}};var downcastPointer=(ptr,ptrClass,desiredClass)=>{if(ptrClass===desiredClass){return ptr}if(undefined===desiredClass.baseClass){return null}var rv=downcastPointer(ptr,ptrClass,desiredClass.baseClass);if(rv===null){return null}return desiredClass.downcast(rv)};var registeredPointers={};var getInheritedInstanceCount=()=>Object.keys(registeredInstances).length;var getLiveInheritedInstances=()=>{var rv=[];for(var k in registeredInstances){if(registeredInstances.hasOwnProperty(k)){rv.push(registeredInstances[k])}}return rv};var deletionQueue=[];var flushPendingDeletes=()=>{while(deletionQueue.length){var obj=deletionQueue.pop();obj.$$.deleteScheduled=false;obj["delete"]()}};var delayFunction;var setDelayFunction=fn=>{delayFunction=fn;if(deletionQueue.length&&delayFunction){delayFunction(flushPendingDeletes)}};var init_embind=()=>{Module["getInheritedInstanceCount"]=getInheritedInstanceCount;Module["getLiveInheritedInstances"]=getLiveInheritedInstances;Module["flushPendingDeletes"]=flushPendingDeletes;Module["setDelayFunction"]=setDelayFunction};var registeredInstances={};var getBasestPointer=(class_,ptr)=>{if(ptr===undefined){throwBindingError("ptr should not be undefined")}while(class_.baseClass){ptr=class_.upcast(ptr);class_=class_.baseClass}return ptr};var getInheritedInstance=(class_,ptr)=>{ptr=getBasestPointer(class_,ptr);return registeredInstances[ptr]};var makeClassHandle=(prototype,record)=>{if(!record.ptrType||!record.ptr){throwInternalError("makeClassHandle requires ptr and ptrType")}var hasSmartPtrType=!!record.smartPtrType;var hasSmartPtr=!!record.smartPtr;if(hasSmartPtrType!==hasSmartPtr){throwInternalError("Both smartPtrType and smartPtr must be specified")}record.count={value:1};return attachFinalizer(Object.create(prototype,{$$:{value:record,writable:true}}))};function RegisteredPointer_fromWireType(ptr){var rawPointer=this.getPointee(ptr);if(!rawPointer){this.destructor(ptr);return null}var registeredInstance=getInheritedInstance(this.registeredClass,rawPointer);if(undefined!==registeredInstance){if(0===registeredInstance.$$.count.value){registeredInstance.$$.ptr=rawPointer;registeredInstance.$$.smartPtr=ptr;return registeredInstance["clone"]()}else{var rv=registeredInstance["clone"]();this.destructor(ptr);return rv}}function makeDefaultHandle(){if(this.isSmartPointer){return makeClassHandle(this.registeredClass.instancePrototype,{ptrType:this.pointeeType,ptr:rawPointer,smartPtrType:this,smartPtr:ptr})}else{return makeClassHandle(this.registeredClass.instancePrototype,{ptrType:this,ptr:ptr})}}var actualType=this.registeredClass.getActualType(rawPointer);var registeredPointerRecord=registeredPointers[actualType];if(!registeredPointerRecord){return makeDefaultHandle.call(this)}var toType;if(this.isConst){toType=registeredPointerRecord.constPointerType}else{toType=registeredPointerRecord.pointerType}var dp=downcastPointer(rawPointer,this.registeredClass,toType.registeredClass);if(dp===null){return makeDefaultHandle.call(this)}if(this.isSmartPointer){return makeClassHandle(toType.registeredClass.instancePrototype,{ptrType:toType,ptr:dp,smartPtrType:this,smartPtr:ptr})}else{return makeClassHandle(toType.registeredClass.instancePrototype,{ptrType:toType,ptr:dp})}}var attachFinalizer=handle=>{if("undefined"===typeof FinalizationRegistry){attachFinalizer=handle=>handle;return handle}finalizationRegistry=new FinalizationRegistry(info=>{releaseClassHandle(info.$$)});attachFinalizer=handle=>{var $$=handle.$$;var hasSmartPtr=!!$$.smartPtr;if(hasSmartPtr){var info={$$:$$};finalizationRegistry.register(handle,info,handle)}return handle};detachFinalizer=handle=>finalizationRegistry.unregister(handle);return attachFinalizer(handle)};var init_ClassHandle=()=>{Object.assign(ClassHandle.prototype,{isAliasOf(other){if(!(this instanceof ClassHandle)){return false}if(!(other instanceof ClassHandle)){return false}var leftClass=this.$$.ptrType.registeredClass;var left=this.$$.ptr;other.$$=other.$$;var rightClass=other.$$.ptrType.registeredClass;var right=other.$$.ptr;while(leftClass.baseClass){left=leftClass.upcast(left);leftClass=leftClass.baseClass}while(rightClass.baseClass){right=rightClass.upcast(right);rightClass=rightClass.baseClass}return leftClass===rightClass&&left===right},clone(){if(!this.$$.ptr){throwInstanceAlreadyDeleted(this)}if(this.$$.preservePointerOnDelete){this.$$.count.value+=1;return this}else{var clone=attachFinalizer(Object.create(Object.getPrototypeOf(this),{$$:{value:shallowCopyInternalPointer(this.$$)}}));clone.$$.count.value+=1;clone.$$.deleteScheduled=false;return clone}},delete(){if(!this.$$.ptr){throwInstanceAlreadyDeleted(this)}if(this.$$.deleteScheduled&&!this.$$.preservePointerOnDelete){throwBindingError("Object already scheduled for deletion")}detachFinalizer(this);releaseClassHandle(this.$$);if(!this.$$.preservePointerOnDelete){this.$$.smartPtr=undefined;this.$$.ptr=undefined}},isDeleted(){return!this.$$.ptr},deleteLater(){if(!this.$$.ptr){throwInstanceAlreadyDeleted(this)}if(this.$$.deleteScheduled&&!this.$$.preservePointerOnDelete){throwBindingError("Object already scheduled for deletion")}deletionQueue.push(this);if(deletionQueue.length===1&&delayFunction){delayFunction(flushPendingDeletes)}this.$$.deleteScheduled=true;return this}})};function ClassHandle(){}var createNamedFunction=(name,body)=>Object.defineProperty(body,"name",{value:name});var ensureOverloadTable=(proto,methodName,humanName)=>{if(undefined===proto[methodName].overloadTable){var prevFunc=proto[methodName];proto[methodName]=function(...args){if(!proto[methodName].overloadTable.hasOwnProperty(args.length)){throwBindingError(`Function '${humanName}' called with an invalid number of arguments (${args.length}) - expects one of (${proto[methodName].overloadTable})!`)}return proto[methodName].overloadTable[args.length].apply(this,args)};proto[methodName].overloadTable=[];proto[methodName].overloadTable[prevFunc.argCount]=prevFunc}};var exposePublicSymbol=(name,value,numArguments)=>{if(Module.hasOwnProperty(name)){if(undefined===numArguments||undefined!==Module[name].overloadTable&&undefined!==Module[name].overloadTable[numArguments]){throwBindingError(`Cannot register public name '${name}' twice`)}ensureOverloadTable(Module,name,name);if(Module.hasOwnProperty(numArguments)){throwBindingError(`Cannot register multiple overloads of a function with the same number of arguments (${numArguments})!`)}Module[name].overloadTable[numArguments]=value}else{Module[name]=value;if(undefined!==numArguments){Module[name].numArguments=numArguments}}};var char_0=48;var char_9=57;var makeLegalFunctionName=name=>{if(undefined===name){return"_unknown"}name=name.replace(/[^a-zA-Z0-9_]/g,"$");var f=name.charCodeAt(0);if(f>=char_0&&f<=char_9){return`_${name}`}return name};function RegisteredClass(name,constructor,instancePrototype,rawDestructor,baseClass,getActualType,upcast,downcast){this.name=name;this.constructor=constructor;this.instancePrototype=instancePrototype;this.rawDestructor=rawDestructor;this.baseClass=baseClass;this.getActualType=getActualType;this.upcast=upcast;this.downcast=downcast;this.pureVirtualFunctions=[]}var upcastPointer=(ptr,ptrClass,desiredClass)=>{while(ptrClass!==desiredClass){if(!ptrClass.upcast){throwBindingError(`Expected null or instance of ${desiredClass.name}, got an instance of ${ptrClass.name}`)}ptr=ptrClass.upcast(ptr);ptrClass=ptrClass.baseClass}return ptr};function constNoSmartPtrRawPointerToWireType(destructors,handle){if(handle===null){if(this.isReference){throwBindingError(`null is not a valid ${this.name}`)}return 0}if(!handle.$$){throwBindingError(`Cannot pass "${embindRepr(handle)}" as a ${this.name}`)}if(!handle.$$.ptr){throwBindingError(`Cannot pass deleted object as a pointer of type ${this.name}`)}var handleClass=handle.$$.ptrType.registeredClass;var ptr=upcastPointer(handle.$$.ptr,handleClass,this.registeredClass);return ptr}function genericPointerToWireType(destructors,handle){var ptr;if(handle===null){if(this.isReference){throwBindingError(`null is not a valid ${this.name}`)}if(this.isSmartPointer){ptr=this.rawConstructor();if(destructors!==null){destructors.push(this.rawDestructor,ptr)}return ptr}else{return 0}}if(!handle||!handle.$$){throwBindingError(`Cannot pass "${embindRepr(handle)}" as a ${this.name}`)}if(!handle.$$.ptr){throwBindingError(`Cannot pass deleted object as a pointer of type ${this.name}`)}if(!this.isConst&&handle.$$.ptrType.isConst){throwBindingError(`Cannot convert argument of type ${handle.$$.smartPtrType?handle.$$.smartPtrType.name:handle.$$.ptrType.name} to parameter type ${this.name}`)}var handleClass=handle.$$.ptrType.registeredClass;ptr=upcastPointer(handle.$$.ptr,handleClass,this.registeredClass);if(this.isSmartPointer){if(undefined===handle.$$.smartPtr){throwBindingError("Passing raw pointer to smart pointer is illegal")}switch(this.sharingPolicy){case 0:if(handle.$$.smartPtrType===this){ptr=handle.$$.smartPtr}else{throwBindingError(`Cannot convert argument of type ${handle.$$.smartPtrType?handle.$$.smartPtrType.name:handle.$$.ptrType.name} to parameter type ${this.name}`)}break;case 1:ptr=handle.$$.smartPtr;break;case 2:if(handle.$$.smartPtrType===this){ptr=handle.$$.smartPtr}else{var clonedHandle=handle["clone"]();ptr=this.rawShare(ptr,Emval.toHandle(()=>clonedHandle["delete"]()));if(destructors!==null){destructors.push(this.rawDestructor,ptr)}}break;default:throwBindingError("Unsupporting sharing policy")}}return ptr}function nonConstNoSmartPtrRawPointerToWireType(destructors,handle){if(handle===null){if(this.isReference){throwBindingError(`null is not a valid ${this.name}`)}return 0}if(!handle.$$){throwBindingError(`Cannot pass "${embindRepr(handle)}" as a ${this.name}`)}if(!handle.$$.ptr){throwBindingError(`Cannot pass deleted object as a pointer of type ${this.name}`)}if(handle.$$.ptrType.isConst){throwBindingError(`Cannot convert argument of type ${handle.$$.ptrType.name} to parameter type ${this.name}`)}var handleClass=handle.$$.ptrType.registeredClass;var ptr=upcastPointer(handle.$$.ptr,handleClass,this.registeredClass);return ptr}var init_RegisteredPointer=()=>{Object.assign(RegisteredPointer.prototype,{getPointee(ptr){if(this.rawGetPointee){ptr=this.rawGetPointee(ptr)}return ptr},destructor(ptr){this.rawDestructor?.(ptr)},argPackAdvance:GenericWireTypeSize,readValueFromPointer:readPointer,fromWireType:RegisteredPointer_fromWireType})};function RegisteredPointer(name,registeredClass,isReference,isConst,isSmartPointer,pointeeType,sharingPolicy,rawGetPointee,rawConstructor,rawShare,rawDestructor){this.name=name;this.registeredClass=registeredClass;this.isReference=isReference;this.isConst=isConst;this.isSmartPointer=isSmartPointer;this.pointeeType=pointeeType;this.sharingPolicy=sharingPolicy;this.rawGetPointee=rawGetPointee;this.rawConstructor=rawConstructor;this.rawShare=rawShare;this.rawDestructor=rawDestructor;if(!isSmartPointer&&registeredClass.baseClass===undefined){if(isConst){this["toWireType"]=constNoSmartPtrRawPointerToWireType;this.destructorFunction=null}else{this["toWireType"]=nonConstNoSmartPtrRawPointerToWireType;this.destructorFunction=null}}else{this["toWireType"]=genericPointerToWireType}}var replacePublicSymbol=(name,value,numArguments)=>{if(!Module.hasOwnProperty(name)){throwInternalError("Replacing nonexistent public symbol")}if(undefined!==Module[name].overloadTable&&undefined!==numArguments){Module[name].overloadTable[numArguments]=value}else{Module[name]=value;Module[name].argCount=numArguments}};var dynCallLegacy=(sig,ptr,args)=>{sig=sig.replace(/p/g,"i");var f=Module["dynCall_"+sig];return f(ptr,...args)};var wasmTableMirror=[];var wasmTable;var getWasmTableEntry=funcPtr=>{var func=wasmTableMirror[funcPtr];if(!func){if(funcPtr>=wasmTableMirror.length)wasmTableMirror.length=funcPtr+1;wasmTableMirror[funcPtr]=func=wasmTable.get(funcPtr)}return func};var dynCall=(sig,ptr,args=[])=>{if(sig.includes("j")){return dynCallLegacy(sig,ptr,args)}var rtn=getWasmTableEntry(ptr)(...args);return rtn};var getDynCaller=(sig,ptr)=>(...args)=>dynCall(sig,ptr,args);var embind__requireFunction=(signature,rawFunction)=>{signature=readLatin1String(signature);function makeDynCaller(){if(signature.includes("j")){return getDynCaller(signature,rawFunction)}return getWasmTableEntry(rawFunction)}var fp=makeDynCaller();if(typeof fp!="function"){throwBindingError(`unknown function pointer with signature ${signature}: ${rawFunction}`)}return fp};var extendError=(baseErrorType,errorName)=>{var errorClass=createNamedFunction(errorName,function(message){this.name=errorName;this.message=message;var stack=new Error(message).stack;if(stack!==undefined){this.stack=this.toString()+"\n"+stack.replace(/^Error(:[^\n]*)?\n/,"")}});errorClass.prototype=Object.create(baseErrorType.prototype);errorClass.prototype.constructor=errorClass;errorClass.prototype.toString=function(){if(this.message===undefined){return this.name}else{return`${this.name}: ${this.message}`}};return errorClass};var UnboundTypeError;var getTypeName=type=>{var ptr=___getTypeName(type);var rv=readLatin1String(ptr);_free(ptr);return rv};var throwUnboundTypeError=(message,types)=>{var unboundTypes=[];var seen={};function visit(type){if(seen[type]){return}if(registeredTypes[type]){return}if(typeDependencies[type]){typeDependencies[type].forEach(visit);return}unboundTypes.push(type);seen[type]=true}types.forEach(visit);throw new UnboundTypeError(`${message}: `+unboundTypes.map(getTypeName).join([", "]))};var __embind_register_class=(rawType,rawPointerType,rawConstPointerType,baseClassRawType,getActualTypeSignature,getActualType,upcastSignature,upcast,downcastSignature,downcast,name,destructorSignature,rawDestructor)=>{name=readLatin1String(name);getActualType=embind__requireFunction(getActualTypeSignature,getActualType);upcast&&=embind__requireFunction(upcastSignature,upcast);downcast&&=embind__requireFunction(downcastSignature,downcast);rawDestructor=embind__requireFunction(destructorSignature,rawDestructor);var legalFunctionName=makeLegalFunctionName(name);exposePublicSymbol(legalFunctionName,function(){throwUnboundTypeError(`Cannot construct ${name} due to unbound types`,[baseClassRawType])});whenDependentTypesAreResolved([rawType,rawPointerType,rawConstPointerType],baseClassRawType?[baseClassRawType]:[],base=>{base=base[0];var baseClass;var basePrototype;if(baseClassRawType){baseClass=base.registeredClass;basePrototype=baseClass.instancePrototype}else{basePrototype=ClassHandle.prototype}var constructor=createNamedFunction(name,function(...args){if(Object.getPrototypeOf(this)!==instancePrototype){throw new BindingError("Use 'new' to construct "+name)}if(undefined===registeredClass.constructor_body){throw new BindingError(name+" has no accessible constructor")}var body=registeredClass.constructor_body[args.length];if(undefined===body){throw new BindingError(`Tried to invoke ctor of ${name} with invalid number of parameters (${args.length}) - expected (${Object.keys(registeredClass.constructor_body).toString()}) parameters instead!`)}return body.apply(this,args)});var instancePrototype=Object.create(basePrototype,{constructor:{value:constructor}});constructor.prototype=instancePrototype;var registeredClass=new RegisteredClass(name,constructor,instancePrototype,rawDestructor,baseClass,getActualType,upcast,downcast);if(registeredClass.baseClass){registeredClass.baseClass.__derivedClasses??=[];registeredClass.baseClass.__derivedClasses.push(registeredClass)}var referenceConverter=new RegisteredPointer(name,registeredClass,true,false,false);var pointerConverter=new RegisteredPointer(name+"*",registeredClass,false,false,false);var constPointerConverter=new RegisteredPointer(name+" const*",registeredClass,false,true,false);registeredPointers[rawType]={pointerType:pointerConverter,constPointerType:constPointerConverter};replacePublicSymbol(legalFunctionName,constructor);return[referenceConverter,pointerConverter,constPointerConverter]})};function usesDestructorStack(argTypes){for(var i=1;i<argTypes.length;++i){if(argTypes[i]!==null&&argTypes[i].destructorFunction===undefined){return true}}return false}function newFunc(constructor,argumentList){if(!(constructor instanceof Function)){throw new TypeError(`new_ called with constructor type ${typeof constructor} which is not a function`)}var dummy=createNamedFunction(constructor.name||"unknownFunctionName",function(){});dummy.prototype=constructor.prototype;var obj=new dummy;var r=constructor.apply(obj,argumentList);return r instanceof Object?r:obj}function createJsInvoker(argTypes,isClassMethodFunc,returns,isAsync){var needsDestructorStack=usesDestructorStack(argTypes);var argCount=argTypes.length;var argsList="";var argsListWired="";for(var i=0;i<argCount-2;++i){argsList+=(i!==0?", ":"")+"arg"+i;argsListWired+=(i!==0?", ":"")+"arg"+i+"Wired"}var invokerFnBody=`\n        return function (${argsList}) {\n        if (arguments.length !== ${argCount-2}) {\n          throwBindingError('function ' + humanName + ' called with ' + arguments.length + ' arguments, expected ${argCount-2}');\n        }`;if(needsDestructorStack){invokerFnBody+="var destructors = [];\n"}var dtorStack=needsDestructorStack?"destructors":"null";var args1=["humanName","throwBindingError","invoker","fn","runDestructors","retType","classParam"];if(isClassMethodFunc){invokerFnBody+="var thisWired = classParam['toWireType']("+dtorStack+", this);\n"}for(var i=0;i<argCount-2;++i){invokerFnBody+="var arg"+i+"Wired = argType"+i+"['toWireType']("+dtorStack+", arg"+i+");\n";args1.push("argType"+i)}if(isClassMethodFunc){argsListWired="thisWired"+(argsListWired.length>0?", ":"")+argsListWired}invokerFnBody+=(returns||isAsync?"var rv = ":"")+"invoker(fn"+(argsListWired.length>0?", ":"")+argsListWired+");\n";if(needsDestructorStack){invokerFnBody+="runDestructors(destructors);\n"}else{for(var i=isClassMethodFunc?1:2;i<argTypes.length;++i){var paramName=i===1?"thisWired":"arg"+(i-2)+"Wired";if(argTypes[i].destructorFunction!==null){invokerFnBody+=`${paramName}_dtor(${paramName});\n`;args1.push(`${paramName}_dtor`)}}}if(returns){invokerFnBody+="var ret = retType['fromWireType'](rv);\n"+"return ret;\n"}else{}invokerFnBody+="}\n";return[args1,invokerFnBody]}function craftInvokerFunction(humanName,argTypes,classType,cppInvokerFunc,cppTargetFunc,isAsync){var argCount=argTypes.length;if(argCount<2){throwBindingError("argTypes array size mismatch! Must at least get return value and 'this' types!")}var isClassMethodFunc=argTypes[1]!==null&&classType!==null;var needsDestructorStack=usesDestructorStack(argTypes);var returns=argTypes[0].name!=="void";var closureArgs=[humanName,throwBindingError,cppInvokerFunc,cppTargetFunc,runDestructors,argTypes[0],argTypes[1]];for(var i=0;i<argCount-2;++i){closureArgs.push(argTypes[i+2])}if(!needsDestructorStack){for(var i=isClassMethodFunc?1:2;i<argTypes.length;++i){if(argTypes[i].destructorFunction!==null){closureArgs.push(argTypes[i].destructorFunction)}}}let[args,invokerFnBody]=createJsInvoker(argTypes,isClassMethodFunc,returns,isAsync);args.push(invokerFnBody);var invokerFn=newFunc(Function,args)(...closureArgs);return createNamedFunction(humanName,invokerFn)}var heap32VectorToArray=(count,firstElement)=>{var array=[];for(var i=0;i<count;i++){array.push(HEAPU32[firstElement+i*4>>2])}return array};var getFunctionName=signature=>{signature=signature.trim();const argsIndex=signature.indexOf("(");if(argsIndex!==-1){return signature.substr(0,argsIndex)}else{return signature}};var __embind_register_class_class_function=(rawClassType,methodName,argCount,rawArgTypesAddr,invokerSignature,rawInvoker,fn,isAsync)=>{var rawArgTypes=heap32VectorToArray(argCount,rawArgTypesAddr);methodName=readLatin1String(methodName);methodName=getFunctionName(methodName);rawInvoker=embind__requireFunction(invokerSignature,rawInvoker);whenDependentTypesAreResolved([],[rawClassType],classType=>{classType=classType[0];var humanName=`${classType.name}.${methodName}`;function unboundTypesHandler(){throwUnboundTypeError(`Cannot call ${humanName} due to unbound types`,rawArgTypes)}if(methodName.startsWith("@@")){methodName=Symbol[methodName.substring(2)]}var proto=classType.registeredClass.constructor;if(undefined===proto[methodName]){unboundTypesHandler.argCount=argCount-1;proto[methodName]=unboundTypesHandler}else{ensureOverloadTable(proto,methodName,humanName);proto[methodName].overloadTable[argCount-1]=unboundTypesHandler}whenDependentTypesAreResolved([],rawArgTypes,argTypes=>{var invokerArgsArray=[argTypes[0],null].concat(argTypes.slice(1));var func=craftInvokerFunction(humanName,invokerArgsArray,null,rawInvoker,fn,isAsync);if(undefined===proto[methodName].overloadTable){func.argCount=argCount-1;proto[methodName]=func}else{proto[methodName].overloadTable[argCount-1]=func}if(classType.registeredClass.__derivedClasses){for(const derivedClass of classType.registeredClass.__derivedClasses){if(!derivedClass.constructor.hasOwnProperty(methodName)){derivedClass.constructor[methodName]=func}}}return[]});return[]})};var __embind_register_class_constructor=(rawClassType,argCount,rawArgTypesAddr,invokerSignature,invoker,rawConstructor)=>{var rawArgTypes=heap32VectorToArray(argCount,rawArgTypesAddr);invoker=embind__requireFunction(invokerSignature,invoker);whenDependentTypesAreResolved([],[rawClassType],classType=>{classType=classType[0];var humanName=`constructor ${classType.name}`;if(undefined===classType.registeredClass.constructor_body){classType.registeredClass.constructor_body=[]}if(undefined!==classType.registeredClass.constructor_body[argCount-1]){throw new BindingError(`Cannot register multiple constructors with identical number of parameters (${argCount-1}) for class '${classType.name}'! Overload resolution is currently only performed using the parameter count, not actual type info!`)}classType.registeredClass.constructor_body[argCount-1]=()=>{throwUnboundTypeError(`Cannot construct ${classType.name} due to unbound types`,rawArgTypes)};whenDependentTypesAreResolved([],rawArgTypes,argTypes=>{argTypes.splice(1,0,null);classType.registeredClass.constructor_body[argCount-1]=craftInvokerFunction(humanName,argTypes,null,invoker,rawConstructor);return[]});return[]})};var __embind_register_class_function=(rawClassType,methodName,argCount,rawArgTypesAddr,invokerSignature,rawInvoker,context,isPureVirtual,isAsync)=>{var rawArgTypes=heap32VectorToArray(argCount,rawArgTypesAddr);methodName=readLatin1String(methodName);methodName=getFunctionName(methodName);rawInvoker=embind__requireFunction(invokerSignature,rawInvoker);whenDependentTypesAreResolved([],[rawClassType],classType=>{classType=classType[0];var humanName=`${classType.name}.${methodName}`;if(methodName.startsWith("@@")){methodName=Symbol[methodName.substring(2)]}if(isPureVirtual){classType.registeredClass.pureVirtualFunctions.push(methodName)}function unboundTypesHandler(){throwUnboundTypeError(`Cannot call ${humanName} due to unbound types`,rawArgTypes)}var proto=classType.registeredClass.instancePrototype;var method=proto[methodName];if(undefined===method||undefined===method.overloadTable&&method.className!==classType.name&&method.argCount===argCount-2){unboundTypesHandler.argCount=argCount-2;unboundTypesHandler.className=classType.name;proto[methodName]=unboundTypesHandler}else{ensureOverloadTable(proto,methodName,humanName);proto[methodName].overloadTable[argCount-2]=unboundTypesHandler}whenDependentTypesAreResolved([],rawArgTypes,argTypes=>{var memberFunction=craftInvokerFunction(humanName,argTypes,classType,rawInvoker,context,isAsync);if(undefined===proto[methodName].overloadTable){memberFunction.argCount=argCount-2;proto[methodName]=memberFunction}else{proto[methodName].overloadTable[argCount-2]=memberFunction}return[]});return[]})};var validateThis=(this_,classType,humanName)=>{if(!(this_ instanceof Object)){throwBindingError(`${humanName} with invalid "this": ${this_}`)}if(!(this_ instanceof classType.registeredClass.constructor)){throwBindingError(`${humanName} incompatible with "this" of type ${this_.constructor.name}`)}if(!this_.$$.ptr){throwBindingError(`cannot call emscripten binding method ${humanName} on deleted object`)}return upcastPointer(this_.$$.ptr,this_.$$.ptrType.registeredClass,classType.registeredClass)};var __embind_register_class_property=(classType,fieldName,getterReturnType,getterSignature,getter,getterContext,setterArgumentType,setterSignature,setter,setterContext)=>{fieldName=readLatin1String(fieldName);getter=embind__requireFunction(getterSignature,getter);whenDependentTypesAreResolved([],[classType],classType=>{classType=classType[0];var humanName=`${classType.name}.${fieldName}`;var desc={get(){throwUnboundTypeError(`Cannot access ${humanName} due to unbound types`,[getterReturnType,setterArgumentType])},enumerable:true,configurable:true};if(setter){desc.set=()=>throwUnboundTypeError(`Cannot access ${humanName} due to unbound types`,[getterReturnType,setterArgumentType])}else{desc.set=v=>throwBindingError(humanName+" is a read-only property")}Object.defineProperty(classType.registeredClass.instancePrototype,fieldName,desc);whenDependentTypesAreResolved([],setter?[getterReturnType,setterArgumentType]:[getterReturnType],types=>{var getterReturnType=types[0];var desc={get(){var ptr=validateThis(this,classType,humanName+" getter");return getterReturnType["fromWireType"](getter(getterContext,ptr))},enumerable:true};if(setter){setter=embind__requireFunction(setterSignature,setter);var setterArgumentType=types[1];desc.set=function(v){var ptr=validateThis(this,classType,humanName+" setter");var destructors=[];setter(setterContext,ptr,setterArgumentType["toWireType"](destructors,v));runDestructors(destructors)}}Object.defineProperty(classType.registeredClass.instancePrototype,fieldName,desc);return[]});return[]})};var emval_freelist=[];var emval_handles=[];var __emval_decref=handle=>{if(handle>9&&0===--emval_handles[handle+1]){emval_handles[handle]=undefined;emval_freelist.push(handle)}};var count_emval_handles=()=>emval_handles.length/2-5-emval_freelist.length;var init_emval=()=>{emval_handles.push(0,1,undefined,1,null,1,true,1,false,1);Module["count_emval_handles"]=count_emval_handles};var Emval={toValue:handle=>{if(!handle){throwBindingError("Cannot use deleted val. handle = "+handle)}return emval_handles[handle]},toHandle:value=>{switch(value){case undefined:return 2;case null:return 4;case true:return 6;case false:return 8;default:{const handle=emval_freelist.pop()||emval_handles.length;emval_handles[handle]=value;emval_handles[handle+1]=1;return handle}}}};var EmValType={name:"emscripten::val",fromWireType:handle=>{var rv=Emval.toValue(handle);__emval_decref(handle);return rv},toWireType:(destructors,value)=>Emval.toHandle(value),argPackAdvance:GenericWireTypeSize,readValueFromPointer:readPointer,destructorFunction:null};var __embind_register_emval=rawType=>registerType(rawType,EmValType);var enumReadValueFromPointer=(name,width,signed)=>{switch(width){case 1:return signed?function(pointer){return this["fromWireType"](HEAP8[pointer])}:function(pointer){return this["fromWireType"](HEAPU8[pointer])};case 2:return signed?function(pointer){return this["fromWireType"](HEAP16[pointer>>1])}:function(pointer){return this["fromWireType"](HEAPU16[pointer>>1])};case 4:return signed?function(pointer){return this["fromWireType"](HEAP32[pointer>>2])}:function(pointer){return this["fromWireType"](HEAPU32[pointer>>2])};default:throw new TypeError(`invalid integer width (${width}): ${name}`)}};var __embind_register_enum=(rawType,name,size,isSigned)=>{name=readLatin1String(name);function ctor(){}ctor.values={};registerType(rawType,{name:name,constructor:ctor,fromWireType:function(c){return this.constructor.values[c]},toWireType:(destructors,c)=>c.value,argPackAdvance:GenericWireTypeSize,readValueFromPointer:enumReadValueFromPointer(name,size,isSigned),destructorFunction:null});exposePublicSymbol(name,ctor)};var requireRegisteredType=(rawType,humanName)=>{var impl=registeredTypes[rawType];if(undefined===impl){throwBindingError(`${humanName} has unknown type ${getTypeName(rawType)}`)}return impl};var __embind_register_enum_value=(rawEnumType,name,enumValue)=>{var enumType=requireRegisteredType(rawEnumType,"enum");name=readLatin1String(name);var Enum=enumType.constructor;var Value=Object.create(enumType.constructor.prototype,{value:{value:enumValue},constructor:{value:createNamedFunction(`${enumType.name}_${name}`,function(){})}});Enum.values[enumValue]=Value;Enum[name]=Value};var embindRepr=v=>{if(v===null){return"null"}var t=typeof v;if(t==="object"||t==="array"||t==="function"){return v.toString()}else{return""+v}};var floatReadValueFromPointer=(name,width)=>{switch(width){case 4:return function(pointer){return this["fromWireType"](HEAPF32[pointer>>2])};case 8:return function(pointer){return this["fromWireType"](HEAPF64[pointer>>3])};default:throw new TypeError(`invalid float width (${width}): ${name}`)}};var __embind_register_float=(rawType,name,size)=>{name=readLatin1String(name);registerType(rawType,{name:name,fromWireType:value=>value,toWireType:(destructors,value)=>value,argPackAdvance:GenericWireTypeSize,readValueFromPointer:floatReadValueFromPointer(name,size),destructorFunction:null})};var __embind_register_function=(name,argCount,rawArgTypesAddr,signature,rawInvoker,fn,isAsync)=>{var argTypes=heap32VectorToArray(argCount,rawArgTypesAddr);name=readLatin1String(name);name=getFunctionName(name);rawInvoker=embind__requireFunction(signature,rawInvoker);exposePublicSymbol(name,function(){throwUnboundTypeError(`Cannot call ${name} due to unbound types`,argTypes)},argCount-1);whenDependentTypesAreResolved([],argTypes,argTypes=>{var invokerArgsArray=[argTypes[0],null].concat(argTypes.slice(1));replacePublicSymbol(name,craftInvokerFunction(name,invokerArgsArray,null,rawInvoker,fn,isAsync),argCount-1);return[]})};var integerReadValueFromPointer=(name,width,signed)=>{switch(width){case 1:return signed?pointer=>HEAP8[pointer]:pointer=>HEAPU8[pointer];case 2:return signed?pointer=>HEAP16[pointer>>1]:pointer=>HEAPU16[pointer>>1];case 4:return signed?pointer=>HEAP32[pointer>>2]:pointer=>HEAPU32[pointer>>2];default:throw new TypeError(`invalid integer width (${width}): ${name}`)}};var __embind_register_integer=(primitiveType,name,size,minRange,maxRange)=>{name=readLatin1String(name);if(maxRange===-1){maxRange=4294967295}var fromWireType=value=>value;if(minRange===0){var bitshift=32-8*size;fromWireType=value=>value<<bitshift>>>bitshift}var isUnsignedType=name.includes("unsigned");var checkAssertions=(value,toTypeName)=>{};var toWireType;if(isUnsignedType){toWireType=function(destructors,value){checkAssertions(value,this.name);return value>>>0}}else{toWireType=function(destructors,value){checkAssertions(value,this.name);return value}}registerType(primitiveType,{name:name,fromWireType:fromWireType,toWireType:toWireType,argPackAdvance:GenericWireTypeSize,readValueFromPointer:integerReadValueFromPointer(name,size,minRange!==0),destructorFunction:null})};var __embind_register_memory_view=(rawType,dataTypeIndex,name)=>{var typeMapping=[Int8Array,Uint8Array,Int16Array,Uint16Array,Int32Array,Uint32Array,Float32Array,Float64Array];var TA=typeMapping[dataTypeIndex];function decodeMemoryView(handle){var size=HEAPU32[handle>>2];var data=HEAPU32[handle+4>>2];return new TA(HEAP8.buffer,data,size)}name=readLatin1String(name);registerType(rawType,{name:name,fromWireType:decodeMemoryView,argPackAdvance:GenericWireTypeSize,readValueFromPointer:decodeMemoryView},{ignoreDuplicateRegistrations:true})};var __embind_register_optional=(rawOptionalType,rawType)=>{__embind_register_emval(rawOptionalType)};var stringToUTF8=(str,outPtr,maxBytesToWrite)=>stringToUTF8Array(str,HEAPU8,outPtr,maxBytesToWrite);var __embind_register_std_string=(rawType,name)=>{name=readLatin1String(name);var stdStringIsUTF8=name==="std::string";registerType(rawType,{name:name,fromWireType(value){var length=HEAPU32[value>>2];var payload=value+4;var str;if(stdStringIsUTF8){var decodeStartPtr=payload;for(var i=0;i<=length;++i){var currentBytePtr=payload+i;if(i==length||HEAPU8[currentBytePtr]==0){var maxRead=currentBytePtr-decodeStartPtr;var stringSegment=UTF8ToString(decodeStartPtr,maxRead);if(str===undefined){str=stringSegment}else{str+=String.fromCharCode(0);str+=stringSegment}decodeStartPtr=currentBytePtr+1}}}else{var a=new Array(length);for(var i=0;i<length;++i){a[i]=String.fromCharCode(HEAPU8[payload+i])}str=a.join("")}_free(value);return str},toWireType(destructors,value){if(value instanceof ArrayBuffer){value=new Uint8Array(value)}var length;var valueIsOfTypeString=typeof value=="string";if(!(valueIsOfTypeString||value instanceof Uint8Array||value instanceof Uint8ClampedArray||value instanceof Int8Array)){throwBindingError("Cannot pass non-string to std::string")}if(stdStringIsUTF8&&valueIsOfTypeString){length=lengthBytesUTF8(value)}else{length=value.length}var base=_malloc(4+length+1);var ptr=base+4;HEAPU32[base>>2]=length;if(stdStringIsUTF8&&valueIsOfTypeString){stringToUTF8(value,ptr,length+1)}else{if(valueIsOfTypeString){for(var i=0;i<length;++i){var charCode=value.charCodeAt(i);if(charCode>255){_free(ptr);throwBindingError("String has UTF-16 code units that do not fit in 8 bits")}HEAPU8[ptr+i]=charCode}}else{for(var i=0;i<length;++i){HEAPU8[ptr+i]=value[i]}}}if(destructors!==null){destructors.push(_free,base)}return base},argPackAdvance:GenericWireTypeSize,readValueFromPointer:readPointer,destructorFunction(ptr){_free(ptr)}})};var UTF16Decoder=typeof TextDecoder!="undefined"?new TextDecoder("utf-16le"):undefined;var UTF16ToString=(ptr,maxBytesToRead)=>{var endPtr=ptr;var idx=endPtr>>1;var maxIdx=idx+maxBytesToRead/2;while(!(idx>=maxIdx)&&HEAPU16[idx])++idx;endPtr=idx<<1;if(endPtr-ptr>32&&UTF16Decoder)return UTF16Decoder.decode(HEAPU8.subarray(ptr,endPtr));var str="";for(var i=0;!(i>=maxBytesToRead/2);++i){var codeUnit=HEAP16[ptr+i*2>>1];if(codeUnit==0)break;str+=String.fromCharCode(codeUnit)}return str};var stringToUTF16=(str,outPtr,maxBytesToWrite)=>{maxBytesToWrite??=2147483647;if(maxBytesToWrite<2)return 0;maxBytesToWrite-=2;var startPtr=outPtr;var numCharsToWrite=maxBytesToWrite<str.length*2?maxBytesToWrite/2:str.length;for(var i=0;i<numCharsToWrite;++i){var codeUnit=str.charCodeAt(i);HEAP16[outPtr>>1]=codeUnit;outPtr+=2}HEAP16[outPtr>>1]=0;return outPtr-startPtr};var lengthBytesUTF16=str=>str.length*2;var UTF32ToString=(ptr,maxBytesToRead)=>{var i=0;var str="";while(!(i>=maxBytesToRead/4)){var utf32=HEAP32[ptr+i*4>>2];if(utf32==0)break;++i;if(utf32>=65536){var ch=utf32-65536;str+=String.fromCharCode(55296|ch>>10,56320|ch&1023)}else{str+=String.fromCharCode(utf32)}}return str};var stringToUTF32=(str,outPtr,maxBytesToWrite)=>{maxBytesToWrite??=2147483647;if(maxBytesToWrite<4)return 0;var startPtr=outPtr;var endPtr=startPtr+maxBytesToWrite-4;for(var i=0;i<str.length;++i){var codeUnit=str.charCodeAt(i);if(codeUnit>=55296&&codeUnit<=57343){var trailSurrogate=str.charCodeAt(++i);codeUnit=65536+((codeUnit&1023)<<10)|trailSurrogate&1023}HEAP32[outPtr>>2]=codeUnit;outPtr+=4;if(outPtr+4>endPtr)break}HEAP32[outPtr>>2]=0;return outPtr-startPtr};var lengthBytesUTF32=str=>{var len=0;for(var i=0;i<str.length;++i){var codeUnit=str.charCodeAt(i);if(codeUnit>=55296&&codeUnit<=57343)++i;len+=4}return len};var __embind_register_std_wstring=(rawType,charSize,name)=>{name=readLatin1String(name);var decodeString,encodeString,readCharAt,lengthBytesUTF;if(charSize===2){decodeString=UTF16ToString;encodeString=stringToUTF16;lengthBytesUTF=lengthBytesUTF16;readCharAt=pointer=>HEAPU16[pointer>>1]}else if(charSize===4){decodeString=UTF32ToString;encodeString=stringToUTF32;lengthBytesUTF=lengthBytesUTF32;readCharAt=pointer=>HEAPU32[pointer>>2]}registerType(rawType,{name:name,fromWireType:value=>{var length=HEAPU32[value>>2];var str;var decodeStartPtr=value+4;for(var i=0;i<=length;++i){var currentBytePtr=value+4+i*charSize;if(i==length||readCharAt(currentBytePtr)==0){var maxReadBytes=currentBytePtr-decodeStartPtr;var stringSegment=decodeString(decodeStartPtr,maxReadBytes);if(str===undefined){str=stringSegment}else{str+=String.fromCharCode(0);str+=stringSegment}decodeStartPtr=currentBytePtr+charSize}}_free(value);return str},toWireType:(destructors,value)=>{if(!(typeof value=="string")){throwBindingError(`Cannot pass non-string to C++ string type ${name}`)}var length=lengthBytesUTF(value);var ptr=_malloc(4+length+charSize);HEAPU32[ptr>>2]=length/charSize;encodeString(value,ptr+4,length+charSize);if(destructors!==null){destructors.push(_free,ptr)}return ptr},argPackAdvance:GenericWireTypeSize,readValueFromPointer:readPointer,destructorFunction(ptr){_free(ptr)}})};var __embind_register_value_array=(rawType,name,constructorSignature,rawConstructor,destructorSignature,rawDestructor)=>{tupleRegistrations[rawType]={name:readLatin1String(name),rawConstructor:embind__requireFunction(constructorSignature,rawConstructor),rawDestructor:embind__requireFunction(destructorSignature,rawDestructor),elements:[]}};var __embind_register_value_array_element=(rawTupleType,getterReturnType,getterSignature,getter,getterContext,setterArgumentType,setterSignature,setter,setterContext)=>{tupleRegistrations[rawTupleType].elements.push({getterReturnType:getterReturnType,getter:embind__requireFunction(getterSignature,getter),getterContext:getterContext,setterArgumentType:setterArgumentType,setter:embind__requireFunction(setterSignature,setter),setterContext:setterContext})};var __embind_register_value_object=(rawType,name,constructorSignature,rawConstructor,destructorSignature,rawDestructor)=>{structRegistrations[rawType]={name:readLatin1String(name),rawConstructor:embind__requireFunction(constructorSignature,rawConstructor),rawDestructor:embind__requireFunction(destructorSignature,rawDestructor),fields:[]}};var __embind_register_value_object_field=(structType,fieldName,getterReturnType,getterSignature,getter,getterContext,setterArgumentType,setterSignature,setter,setterContext)=>{structRegistrations[structType].fields.push({fieldName:readLatin1String(fieldName),getterReturnType:getterReturnType,getter:embind__requireFunction(getterSignature,getter),getterContext:getterContext,setterArgumentType:setterArgumentType,setter:embind__requireFunction(setterSignature,setter),setterContext:setterContext})};var __embind_register_void=(rawType,name)=>{name=readLatin1String(name);registerType(rawType,{isVoid:true,name:name,argPackAdvance:0,fromWireType:()=>undefined,toWireType:(destructors,o)=>undefined})};var nowIsMonotonic=1;var __emscripten_get_now_is_monotonic=()=>nowIsMonotonic;var __emscripten_memcpy_js=(dest,src,num)=>HEAPU8.copyWithin(dest,src,src+num);var emval_returnValue=(returnType,destructorsRef,handle)=>{var destructors=[];var result=returnType["toWireType"](destructors,handle);if(destructors.length){HEAPU32[destructorsRef>>2]=Emval.toHandle(destructors)}return result};var __emval_as=(handle,returnType,destructorsRef)=>{handle=Emval.toValue(handle);returnType=requireRegisteredType(returnType,"emval::as");return emval_returnValue(returnType,destructorsRef,handle)};var __emval_get_property=(handle,key)=>{handle=Emval.toValue(handle);key=Emval.toValue(key);return Emval.toHandle(handle[key])};var __emval_incref=handle=>{if(handle>9){emval_handles[handle+1]+=1}};var emval_symbols={};var getStringOrSymbol=address=>{var symbol=emval_symbols[address];if(symbol===undefined){return readLatin1String(address)}return symbol};var __emval_new_cstring=v=>Emval.toHandle(getStringOrSymbol(v));var __emval_run_destructors=handle=>{var destructors=Emval.toValue(handle);runDestructors(destructors);__emval_decref(handle)};var __emval_take_value=(type,arg)=>{type=requireRegisteredType(type,"_emval_take_value");var v=type["readValueFromPointer"](arg);return Emval.toHandle(v)};var readEmAsmArgsArray=[];var readEmAsmArgs=(sigPtr,buf)=>{readEmAsmArgsArray.length=0;var ch;while(ch=HEAPU8[sigPtr++]){var wide=ch!=105;wide&=ch!=112;buf+=wide&&buf%8?4:0;readEmAsmArgsArray.push(ch==112?HEAPU32[buf>>2]:ch==105?HEAP32[buf>>2]:HEAPF64[buf>>3]);buf+=wide?8:4}return readEmAsmArgsArray};var runEmAsmFunction=(code,sigPtr,argbuf)=>{var args=readEmAsmArgs(sigPtr,argbuf);return ASM_CONSTS[code](...args)};var _emscripten_asm_const_int=(code,sigPtr,argbuf)=>runEmAsmFunction(code,sigPtr,argbuf);var _emscripten_date_now=()=>Date.now();var _emscripten_err=str=>err(UTF8ToString(str));var getHeapMax=()=>2147483648;var _emscripten_get_heap_max=()=>getHeapMax();var _emscripten_get_now;_emscripten_get_now=()=>performance.now();var _emscripten_out=str=>out(UTF8ToString(str));var growMemory=size=>{var b=wasmMemory.buffer;var pages=(size-b.byteLength+65535)/65536;try{wasmMemory.grow(pages);updateMemoryViews();return 1}catch(e){}};var _emscripten_resize_heap=requestedSize=>{var oldSize=HEAPU8.length;requestedSize>>>=0;var maxHeapSize=getHeapMax();if(requestedSize>maxHeapSize){return false}var alignUp=(x,multiple)=>x+(multiple-x%multiple)%multiple;for(var cutDown=1;cutDown<=4;cutDown*=2){var overGrownHeapSize=oldSize*(1+.2/cutDown);overGrownHeapSize=Math.min(overGrownHeapSize,requestedSize+100663296);var newSize=Math.min(maxHeapSize,alignUp(Math.max(requestedSize,overGrownHeapSize),65536));var replacement=growMemory(newSize);if(replacement){return true}}return false};var ENV={};var getExecutableName=()=>thisProgram||"./this.program";var getEnvStrings=()=>{if(!getEnvStrings.strings){var lang=(typeof navigator=="object"&&navigator.languages&&navigator.languages[0]||"C").replace("-","_")+".UTF-8";var env={USER:"web_user",LOGNAME:"web_user",PATH:"/",PWD:"/",HOME:"/home/web_user",LANG:lang,_:getExecutableName()};for(var x in ENV){if(ENV[x]===undefined)delete env[x];else env[x]=ENV[x]}var strings=[];for(var x in env){strings.push(`${x}=${env[x]}`)}getEnvStrings.strings=strings}return getEnvStrings.strings};var stringToAscii=(str,buffer)=>{for(var i=0;i<str.length;++i){HEAP8[buffer++]=str.charCodeAt(i)}HEAP8[buffer]=0};var _environ_get=(__environ,environ_buf)=>{var bufSize=0;getEnvStrings().forEach((string,i)=>{var ptr=environ_buf+bufSize;HEAPU32[__environ+i*4>>2]=ptr;stringToAscii(string,ptr);bufSize+=string.length+1});return 0};var _environ_sizes_get=(penviron_count,penviron_buf_size)=>{var strings=getEnvStrings();HEAPU32[penviron_count>>2]=strings.length;var bufSize=0;strings.forEach(string=>bufSize+=string.length+1);HEAPU32[penviron_buf_size>>2]=bufSize;return 0};function _fd_close(fd){try{var stream=SYSCALLS.getStreamFromFD(fd);FS.close(stream);return 0}catch(e){if(typeof FS=="undefined"||!(e.name==="ErrnoError"))throw e;return e.errno}}var doReadv=(stream,iov,iovcnt,offset)=>{var ret=0;for(var i=0;i<iovcnt;i++){var ptr=HEAPU32[iov>>2];var len=HEAPU32[iov+4>>2];iov+=8;var curr=FS.read(stream,HEAP8,ptr,len,offset);if(curr<0)return-1;ret+=curr;if(curr<len)break;if(typeof offset!="undefined"){offset+=curr}}return ret};function _fd_read(fd,iov,iovcnt,pnum){try{var stream=SYSCALLS.getStreamFromFD(fd);var num=doReadv(stream,iov,iovcnt);HEAPU32[pnum>>2]=num;return 0}catch(e){if(typeof FS=="undefined"||!(e.name==="ErrnoError"))throw e;return e.errno}}var convertI32PairToI53Checked=(lo,hi)=>hi+2097152>>>0<4194305-!!lo?(lo>>>0)+hi*4294967296:NaN;function _fd_seek(fd,offset_low,offset_high,whence,newOffset){var offset=convertI32PairToI53Checked(offset_low,offset_high);try{if(isNaN(offset))return 61;var stream=SYSCALLS.getStreamFromFD(fd);FS.llseek(stream,offset,whence);tempI64=[stream.position>>>0,(tempDouble=stream.position,+Math.abs(tempDouble)>=1?tempDouble>0?+Math.floor(tempDouble/4294967296)>>>0:~~+Math.ceil((tempDouble-+(~~tempDouble>>>0))/4294967296)>>>0:0)],HEAP32[newOffset>>2]=tempI64[0],HEAP32[newOffset+4>>2]=tempI64[1];if(stream.getdents&&offset===0&&whence===0)stream.getdents=null;return 0}catch(e){if(typeof FS=="undefined"||!(e.name==="ErrnoError"))throw e;return e.errno}}var doWritev=(stream,iov,iovcnt,offset)=>{var ret=0;for(var i=0;i<iovcnt;i++){var ptr=HEAPU32[iov>>2];var len=HEAPU32[iov+4>>2];iov+=8;var curr=FS.write(stream,HEAP8,ptr,len,offset);if(curr<0)return-1;ret+=curr;if(typeof offset!="undefined"){offset+=curr}}return ret};function _fd_write(fd,iov,iovcnt,pnum){try{var stream=SYSCALLS.getStreamFromFD(fd);var num=doWritev(stream,iov,iovcnt);HEAPU32[pnum>>2]=num;return 0}catch(e){if(typeof FS=="undefined"||!(e.name==="ErrnoError"))throw e;return e.errno}}var _getentropy=(buffer,size)=>{randomFill(HEAPU8.subarray(buffer,buffer+size));return 0};var webgl_enable_WEBGL_draw_instanced_base_vertex_base_instance=ctx=>!!(ctx.dibvbi=ctx.getExtension("WEBGL_draw_instanced_base_vertex_base_instance"));var webgl_enable_WEBGL_multi_draw_instanced_base_vertex_base_instance=ctx=>!!(ctx.mdibvbi=ctx.getExtension("WEBGL_multi_draw_instanced_base_vertex_base_instance"));var webgl_enable_WEBGL_multi_draw=ctx=>!!(ctx.multiDrawWebgl=ctx.getExtension("WEBGL_multi_draw"));var getEmscriptenSupportedExtensions=ctx=>{var supportedExtensions=["EXT_color_buffer_float","EXT_conservative_depth","EXT_disjoint_timer_query_webgl2","EXT_texture_norm16","NV_shader_noperspective_interpolation","WEBGL_clip_cull_distance","EXT_color_buffer_half_float","EXT_depth_clamp","EXT_float_blend","EXT_texture_compression_bptc","EXT_texture_compression_rgtc","EXT_texture_filter_anisotropic","KHR_parallel_shader_compile","OES_texture_float_linear","WEBGL_blend_func_extended","WEBGL_compressed_texture_astc","WEBGL_compressed_texture_etc","WEBGL_compressed_texture_etc1","WEBGL_compressed_texture_s3tc","WEBGL_compressed_texture_s3tc_srgb","WEBGL_debug_renderer_info","WEBGL_debug_shaders","WEBGL_lose_context","WEBGL_multi_draw"];return(ctx.getSupportedExtensions()||[]).filter(ext=>supportedExtensions.includes(ext))};var GL={counter:1,buffers:[],mappedBuffers:{},programs:[],framebuffers:[],renderbuffers:[],textures:[],shaders:[],vaos:[],contexts:[],offscreenCanvases:{},queries:[],samplers:[],transformFeedbacks:[],syncs:[],byteSizeByTypeRoot:5120,byteSizeByType:[1,1,2,2,4,4,4,2,3,4,8],stringCache:{},stringiCache:{},unpackAlignment:4,recordError:errorCode=>{if(!GL.lastError){GL.lastError=errorCode}},getNewId:table=>{var ret=GL.counter++;for(var i=table.length;i<ret;i++){table[i]=null}return ret},genObject:(n,buffers,createFunction,objectTable)=>{for(var i=0;i<n;i++){var buffer=GLctx[createFunction]();var id=buffer&&GL.getNewId(objectTable);if(buffer){buffer.name=id;objectTable[id]=buffer}else{GL.recordError(1282)}HEAP32[buffers+i*4>>2]=id}},MAX_TEMP_BUFFER_SIZE:2097152,numTempVertexBuffersPerSize:64,log2ceilLookup:i=>32-Math.clz32(i===0?0:i-1),generateTempBuffers:(quads,context)=>{var largestIndex=GL.log2ceilLookup(GL.MAX_TEMP_BUFFER_SIZE);context.tempVertexBufferCounters1=[];context.tempVertexBufferCounters2=[];context.tempVertexBufferCounters1.length=context.tempVertexBufferCounters2.length=largestIndex+1;context.tempVertexBuffers1=[];context.tempVertexBuffers2=[];context.tempVertexBuffers1.length=context.tempVertexBuffers2.length=largestIndex+1;context.tempIndexBuffers=[];context.tempIndexBuffers.length=largestIndex+1;for(var i=0;i<=largestIndex;++i){context.tempIndexBuffers[i]=null;context.tempVertexBufferCounters1[i]=context.tempVertexBufferCounters2[i]=0;var ringbufferLength=GL.numTempVertexBuffersPerSize;context.tempVertexBuffers1[i]=[];context.tempVertexBuffers2[i]=[];var ringbuffer1=context.tempVertexBuffers1[i];var ringbuffer2=context.tempVertexBuffers2[i];ringbuffer1.length=ringbuffer2.length=ringbufferLength;for(var j=0;j<ringbufferLength;++j){ringbuffer1[j]=ringbuffer2[j]=null}}if(quads){context.tempQuadIndexBuffer=GLctx.createBuffer();context.GLctx.bindBuffer(34963,context.tempQuadIndexBuffer);var numIndexes=GL.MAX_TEMP_BUFFER_SIZE>>1;var quadIndexes=new Uint16Array(numIndexes);var i=0,v=0;while(1){quadIndexes[i++]=v;if(i>=numIndexes)break;quadIndexes[i++]=v+1;if(i>=numIndexes)break;quadIndexes[i++]=v+2;if(i>=numIndexes)break;quadIndexes[i++]=v;if(i>=numIndexes)break;quadIndexes[i++]=v+2;if(i>=numIndexes)break;quadIndexes[i++]=v+3;if(i>=numIndexes)break;v+=4}context.GLctx.bufferData(34963,quadIndexes,35044);context.GLctx.bindBuffer(34963,null)}},getTempVertexBuffer:sizeBytes=>{var idx=GL.log2ceilLookup(sizeBytes);var ringbuffer=GL.currentContext.tempVertexBuffers1[idx];var nextFreeBufferIndex=GL.currentContext.tempVertexBufferCounters1[idx];GL.currentContext.tempVertexBufferCounters1[idx]=GL.currentContext.tempVertexBufferCounters1[idx]+1&GL.numTempVertexBuffersPerSize-1;var vbo=ringbuffer[nextFreeBufferIndex];if(vbo){return vbo}var prevVBO=GLctx.getParameter(34964);ringbuffer[nextFreeBufferIndex]=GLctx.createBuffer();GLctx.bindBuffer(34962,ringbuffer[nextFreeBufferIndex]);GLctx.bufferData(34962,1<<idx,35048);GLctx.bindBuffer(34962,prevVBO);return ringbuffer[nextFreeBufferIndex]},getTempIndexBuffer:sizeBytes=>{var idx=GL.log2ceilLookup(sizeBytes);var ibo=GL.currentContext.tempIndexBuffers[idx];if(ibo){return ibo}var prevIBO=GLctx.getParameter(34965);GL.currentContext.tempIndexBuffers[idx]=GLctx.createBuffer();GLctx.bindBuffer(34963,GL.currentContext.tempIndexBuffers[idx]);GLctx.bufferData(34963,1<<idx,35048);GLctx.bindBuffer(34963,prevIBO);return GL.currentContext.tempIndexBuffers[idx]},newRenderingFrameStarted:()=>{if(!GL.currentContext){return}var vb=GL.currentContext.tempVertexBuffers1;GL.currentContext.tempVertexBuffers1=GL.currentContext.tempVertexBuffers2;GL.currentContext.tempVertexBuffers2=vb;vb=GL.currentContext.tempVertexBufferCounters1;GL.currentContext.tempVertexBufferCounters1=GL.currentContext.tempVertexBufferCounters2;GL.currentContext.tempVertexBufferCounters2=vb;var largestIndex=GL.log2ceilLookup(GL.MAX_TEMP_BUFFER_SIZE);for(var i=0;i<=largestIndex;++i){GL.currentContext.tempVertexBufferCounters1[i]=0}},getSource:(shader,count,string,length)=>{var source="";for(var i=0;i<count;++i){var len=length?HEAPU32[length+i*4>>2]:undefined;source+=UTF8ToString(HEAPU32[string+i*4>>2],len)}return source},calcBufLength:(size,type,stride,count)=>{if(stride>0){return count*stride}var typeSize=GL.byteSizeByType[type-GL.byteSizeByTypeRoot];return size*typeSize*count},usedTempBuffers:[],preDrawHandleClientVertexAttribBindings:count=>{GL.resetBufferBinding=false;for(var i=0;i<GL.currentContext.maxVertexAttribs;++i){var cb=GL.currentContext.clientBuffers[i];if(!cb.clientside||!cb.enabled)continue;GL.resetBufferBinding=true;var size=GL.calcBufLength(cb.size,cb.type,cb.stride,count);var buf=GL.getTempVertexBuffer(size);GLctx.bindBuffer(34962,buf);GLctx.bufferSubData(34962,0,HEAPU8.subarray(cb.ptr,cb.ptr+size));cb.vertexAttribPointerAdaptor.call(GLctx,i,cb.size,cb.type,cb.normalized,cb.stride,0)}},postDrawHandleClientVertexAttribBindings:()=>{if(GL.resetBufferBinding){GLctx.bindBuffer(34962,GL.buffers[GLctx.currentArrayBufferBinding])}},createContext:(canvas,webGLContextAttributes)=>{if(!canvas.getContextSafariWebGL2Fixed){canvas.getContextSafariWebGL2Fixed=canvas.getContext;function fixedGetContext(ver,attrs){var gl=canvas.getContextSafariWebGL2Fixed(ver,attrs);return ver=="webgl"==gl instanceof WebGLRenderingContext?gl:null}canvas.getContext=fixedGetContext}var ctx=canvas.getContext("webgl2",webGLContextAttributes);if(!ctx)return 0;var handle=GL.registerContext(ctx,webGLContextAttributes);return handle},registerContext:(ctx,webGLContextAttributes)=>{var handle=GL.getNewId(GL.contexts);var context={handle:handle,attributes:webGLContextAttributes,version:webGLContextAttributes.majorVersion,GLctx:ctx};if(ctx.canvas)ctx.canvas.GLctxObject=context;GL.contexts[handle]=context;if(typeof webGLContextAttributes.enableExtensionsByDefault=="undefined"||webGLContextAttributes.enableExtensionsByDefault){GL.initExtensions(context)}context.maxVertexAttribs=context.GLctx.getParameter(34921);context.clientBuffers=[];for(var i=0;i<context.maxVertexAttribs;i++){context.clientBuffers[i]={enabled:false,clientside:false,size:0,type:0,normalized:0,stride:0,ptr:0,vertexAttribPointerAdaptor:null}}GL.generateTempBuffers(false,context);return handle},makeContextCurrent:contextHandle=>{GL.currentContext=GL.contexts[contextHandle];Module.ctx=GLctx=GL.currentContext?.GLctx;return!(contextHandle&&!GLctx)},getContext:contextHandle=>GL.contexts[contextHandle],deleteContext:contextHandle=>{if(GL.currentContext===GL.contexts[contextHandle]){GL.currentContext=null}if(typeof JSEvents=="object"){JSEvents.removeAllHandlersOnTarget(GL.contexts[contextHandle].GLctx.canvas)}if(GL.contexts[contextHandle]&&GL.contexts[contextHandle].GLctx.canvas){GL.contexts[contextHandle].GLctx.canvas.GLctxObject=undefined}GL.contexts[contextHandle]=null},initExtensions:context=>{context||=GL.currentContext;if(context.initExtensionsDone)return;context.initExtensionsDone=true;var GLctx=context.GLctx;webgl_enable_WEBGL_draw_instanced_base_vertex_base_instance(GLctx);webgl_enable_WEBGL_multi_draw_instanced_base_vertex_base_instance(GLctx);if(context.version>=2){GLctx.disjointTimerQueryExt=GLctx.getExtension("EXT_disjoint_timer_query_webgl2")}if(context.version<2||!GLctx.disjointTimerQueryExt){GLctx.disjointTimerQueryExt=GLctx.getExtension("EXT_disjoint_timer_query")}webgl_enable_WEBGL_multi_draw(GLctx);getEmscriptenSupportedExtensions(GLctx).forEach(ext=>{if(!ext.includes("lose_context")&&!ext.includes("debug")){GLctx.getExtension(ext)}})}};var _glActiveTexture=x0=>GLctx.activeTexture(x0);var _glAttachShader=(program,shader)=>{GLctx.attachShader(GL.programs[program],GL.shaders[shader])};var _glBeginQuery=(target,id)=>{GLctx.beginQuery(target,GL.queries[id])};var _glBindAttribLocation=(program,index,name)=>{GLctx.bindAttribLocation(GL.programs[program],index,UTF8ToString(name))};var _glBindBuffer=(target,buffer)=>{if(target==34962){GLctx.currentArrayBufferBinding=buffer}else if(target==34963){GLctx.currentElementArrayBufferBinding=buffer}if(target==35051){GLctx.currentPixelPackBufferBinding=buffer}else if(target==35052){GLctx.currentPixelUnpackBufferBinding=buffer}GLctx.bindBuffer(target,GL.buffers[buffer])};var _glBindBufferBase=(target,index,buffer)=>{GLctx.bindBufferBase(target,index,GL.buffers[buffer])};var _glBindBufferRange=(target,index,buffer,offset,ptrsize)=>{GLctx.bindBufferRange(target,index,GL.buffers[buffer],offset,ptrsize)};var _glBindFramebuffer=(target,framebuffer)=>{GLctx.bindFramebuffer(target,GL.framebuffers[framebuffer])};var _glBindRenderbuffer=(target,renderbuffer)=>{GLctx.bindRenderbuffer(target,GL.renderbuffers[renderbuffer])};var _glBindSampler=(unit,sampler)=>{GLctx.bindSampler(unit,GL.samplers[sampler])};var _glBindTexture=(target,texture)=>{GLctx.bindTexture(target,GL.textures[texture])};var _glBindVertexArray=vao=>{GLctx.bindVertexArray(GL.vaos[vao]);var ibo=GLctx.getParameter(34965);GLctx.currentElementArrayBufferBinding=ibo?ibo.name|0:0};var _glBlendEquationSeparate=(x0,x1)=>GLctx.blendEquationSeparate(x0,x1);var _glBlendFuncSeparate=(x0,x1,x2,x3)=>GLctx.blendFuncSeparate(x0,x1,x2,x3);var _glBlitFramebuffer=(x0,x1,x2,x3,x4,x5,x6,x7,x8,x9)=>GLctx.blitFramebuffer(x0,x1,x2,x3,x4,x5,x6,x7,x8,x9);var _glBufferData=(target,size,data,usage)=>{if(true){if(data&&size){GLctx.bufferData(target,HEAPU8,usage,data,size)}else{GLctx.bufferData(target,size,usage)}return}GLctx.bufferData(target,data?HEAPU8.subarray(data,data+size):size,usage)};var _glBufferSubData=(target,offset,size,data)=>{if(true){size&&GLctx.bufferSubData(target,offset,HEAPU8,data,size);return}GLctx.bufferSubData(target,offset,HEAPU8.subarray(data,data+size))};var _glClear=x0=>GLctx.clear(x0);var _glClearBufferfi=(x0,x1,x2,x3)=>GLctx.clearBufferfi(x0,x1,x2,x3);var _glClearBufferfv=(buffer,drawbuffer,value)=>{GLctx.clearBufferfv(buffer,drawbuffer,HEAPF32,value>>2)};var _glClearBufferiv=(buffer,drawbuffer,value)=>{GLctx.clearBufferiv(buffer,drawbuffer,HEAP32,value>>2)};var _glClearColor=(x0,x1,x2,x3)=>GLctx.clearColor(x0,x1,x2,x3);var _glClearDepthf=x0=>GLctx.clearDepth(x0);var _glClearStencil=x0=>GLctx.clearStencil(x0);var convertI32PairToI53=(lo,hi)=>(lo>>>0)+hi*4294967296;var _glClientWaitSync=(sync,flags,timeout_low,timeout_high)=>{var timeout=convertI32PairToI53(timeout_low,timeout_high);return GLctx.clientWaitSync(GL.syncs[sync],flags,timeout)};var _glColorMask=(red,green,blue,alpha)=>{GLctx.colorMask(!!red,!!green,!!blue,!!alpha)};var _glCompileShader=shader=>{GLctx.compileShader(GL.shaders[shader])};var _glCompressedTexSubImage2D=(target,level,xoffset,yoffset,width,height,format,imageSize,data)=>{if(true){if(GLctx.currentPixelUnpackBufferBinding||!imageSize){GLctx.compressedTexSubImage2D(target,level,xoffset,yoffset,width,height,format,imageSize,data);return}GLctx.compressedTexSubImage2D(target,level,xoffset,yoffset,width,height,format,HEAPU8,data,imageSize);return}GLctx.compressedTexSubImage2D(target,level,xoffset,yoffset,width,height,format,data?HEAPU8.subarray(data,data+imageSize):null)};var _glCompressedTexSubImage3D=(target,level,xoffset,yoffset,zoffset,width,height,depth,format,imageSize,data)=>{if(GLctx.currentPixelUnpackBufferBinding){GLctx.compressedTexSubImage3D(target,level,xoffset,yoffset,zoffset,width,height,depth,format,imageSize,data)}else{GLctx.compressedTexSubImage3D(target,level,xoffset,yoffset,zoffset,width,height,depth,format,HEAPU8,data,imageSize)}};var _glCopyBufferSubData=(x0,x1,x2,x3,x4)=>GLctx.copyBufferSubData(x0,x1,x2,x3,x4);var _glCreateProgram=()=>{var id=GL.getNewId(GL.programs);var program=GLctx.createProgram();program.name=id;program.maxUniformLength=program.maxAttributeLength=program.maxUniformBlockNameLength=0;program.uniformIdCounter=1;GL.programs[id]=program;return id};var _glCreateShader=shaderType=>{var id=GL.getNewId(GL.shaders);GL.shaders[id]=GLctx.createShader(shaderType);return id};var _glCullFace=x0=>GLctx.cullFace(x0);var _glDeleteBuffers=(n,buffers)=>{for(var i=0;i<n;i++){var id=HEAP32[buffers+i*4>>2];var buffer=GL.buffers[id];if(!buffer)continue;GLctx.deleteBuffer(buffer);buffer.name=0;GL.buffers[id]=null;if(id==GLctx.currentArrayBufferBinding)GLctx.currentArrayBufferBinding=0;if(id==GLctx.currentElementArrayBufferBinding)GLctx.currentElementArrayBufferBinding=0;if(id==GLctx.currentPixelPackBufferBinding)GLctx.currentPixelPackBufferBinding=0;if(id==GLctx.currentPixelUnpackBufferBinding)GLctx.currentPixelUnpackBufferBinding=0}};var _glDeleteFramebuffers=(n,framebuffers)=>{for(var i=0;i<n;++i){var id=HEAP32[framebuffers+i*4>>2];var framebuffer=GL.framebuffers[id];if(!framebuffer)continue;GLctx.deleteFramebuffer(framebuffer);framebuffer.name=0;GL.framebuffers[id]=null}};var _glDeleteProgram=id=>{if(!id)return;var program=GL.programs[id];if(!program){GL.recordError(1281);return}GLctx.deleteProgram(program);program.name=0;GL.programs[id]=null};var _glDeleteQueries=(n,ids)=>{for(var i=0;i<n;i++){var id=HEAP32[ids+i*4>>2];var query=GL.queries[id];if(!query)continue;GLctx.deleteQuery(query);GL.queries[id]=null}};var _glDeleteRenderbuffers=(n,renderbuffers)=>{for(var i=0;i<n;i++){var id=HEAP32[renderbuffers+i*4>>2];var renderbuffer=GL.renderbuffers[id];if(!renderbuffer)continue;GLctx.deleteRenderbuffer(renderbuffer);renderbuffer.name=0;GL.renderbuffers[id]=null}};var _glDeleteSamplers=(n,samplers)=>{for(var i=0;i<n;i++){var id=HEAP32[samplers+i*4>>2];var sampler=GL.samplers[id];if(!sampler)continue;GLctx.deleteSampler(sampler);sampler.name=0;GL.samplers[id]=null}};var _glDeleteShader=id=>{if(!id)return;var shader=GL.shaders[id];if(!shader){GL.recordError(1281);return}GLctx.deleteShader(shader);GL.shaders[id]=null};var _glDeleteSync=id=>{if(!id)return;var sync=GL.syncs[id];if(!sync){GL.recordError(1281);return}GLctx.deleteSync(sync);sync.name=0;GL.syncs[id]=null};var _glDeleteTextures=(n,textures)=>{for(var i=0;i<n;i++){var id=HEAP32[textures+i*4>>2];var texture=GL.textures[id];if(!texture)continue;GLctx.deleteTexture(texture);texture.name=0;GL.textures[id]=null}};var _glDeleteVertexArrays=(n,vaos)=>{for(var i=0;i<n;i++){var id=HEAP32[vaos+i*4>>2];GLctx.deleteVertexArray(GL.vaos[id]);GL.vaos[id]=null}};var _glDepthFunc=x0=>GLctx.depthFunc(x0);var _glDepthMask=flag=>{GLctx.depthMask(!!flag)};var _glDepthRangef=(x0,x1)=>GLctx.depthRange(x0,x1);var _glDetachShader=(program,shader)=>{GLctx.detachShader(GL.programs[program],GL.shaders[shader])};var _glDisable=x0=>GLctx.disable(x0);var _glDisableVertexAttribArray=index=>{var cb=GL.currentContext.clientBuffers[index];cb.enabled=false;GLctx.disableVertexAttribArray(index)};var tempFixedLengthArray=[];var _glDrawBuffers=(n,bufs)=>{var bufArray=tempFixedLengthArray[n];for(var i=0;i<n;i++){bufArray[i]=HEAP32[bufs+i*4>>2]}GLctx.drawBuffers(bufArray)};var _glDrawElements=(mode,count,type,indices)=>{var buf;if(!GLctx.currentElementArrayBufferBinding){var size=GL.calcBufLength(1,type,0,count);buf=GL.getTempIndexBuffer(size);GLctx.bindBuffer(34963,buf);GLctx.bufferSubData(34963,0,HEAPU8.subarray(indices,indices+size));indices=0}GL.preDrawHandleClientVertexAttribBindings(count);GLctx.drawElements(mode,count,type,indices);GL.postDrawHandleClientVertexAttribBindings(count);if(!GLctx.currentElementArrayBufferBinding){GLctx.bindBuffer(34963,null)}};var _glDrawElementsInstanced=(mode,count,type,indices,primcount)=>{GLctx.drawElementsInstanced(mode,count,type,indices,primcount)};var _glEnable=x0=>GLctx.enable(x0);var _glEnableVertexAttribArray=index=>{var cb=GL.currentContext.clientBuffers[index];cb.enabled=true;GLctx.enableVertexAttribArray(index)};var _glEndQuery=x0=>GLctx.endQuery(x0);var _glFenceSync=(condition,flags)=>{var sync=GLctx.fenceSync(condition,flags);if(sync){var id=GL.getNewId(GL.syncs);sync.name=id;GL.syncs[id]=sync;return id}return 0};var _glFinish=()=>GLctx.finish();var _glFlush=()=>GLctx.flush();var _glFramebufferRenderbuffer=(target,attachment,renderbuffertarget,renderbuffer)=>{GLctx.framebufferRenderbuffer(target,attachment,renderbuffertarget,GL.renderbuffers[renderbuffer])};var _glFramebufferTexture2D=(target,attachment,textarget,texture,level)=>{GLctx.framebufferTexture2D(target,attachment,textarget,GL.textures[texture],level)};var _glFramebufferTextureLayer=(target,attachment,texture,level,layer)=>{GLctx.framebufferTextureLayer(target,attachment,GL.textures[texture],level,layer)};var _glFrontFace=x0=>GLctx.frontFace(x0);var _glGenBuffers=(n,buffers)=>{GL.genObject(n,buffers,"createBuffer",GL.buffers)};var _glGenFramebuffers=(n,ids)=>{GL.genObject(n,ids,"createFramebuffer",GL.framebuffers)};var _glGenQueries=(n,ids)=>{GL.genObject(n,ids,"createQuery",GL.queries)};var _glGenRenderbuffers=(n,renderbuffers)=>{GL.genObject(n,renderbuffers,"createRenderbuffer",GL.renderbuffers)};var _glGenSamplers=(n,samplers)=>{GL.genObject(n,samplers,"createSampler",GL.samplers)};var _glGenTextures=(n,textures)=>{GL.genObject(n,textures,"createTexture",GL.textures)};var _glGenVertexArrays=(n,arrays)=>{GL.genObject(n,arrays,"createVertexArray",GL.vaos)};var _glGenerateMipmap=x0=>GLctx.generateMipmap(x0);var _glGetBufferSubData=(target,offset,size,data)=>{if(!data){GL.recordError(1281);return}size&&GLctx.getBufferSubData(target,offset,HEAPU8,data,size)};var _glGetError=()=>{var error=GLctx.getError()||GL.lastError;GL.lastError=0;return error};var writeI53ToI64=(ptr,num)=>{HEAPU32[ptr>>2]=num;var lower=HEAPU32[ptr>>2];HEAPU32[ptr+4>>2]=(num-lower)/4294967296};var webglGetExtensions=function $webglGetExtensions(){var exts=getEmscriptenSupportedExtensions(GLctx);exts=exts.concat(exts.map(e=>"GL_"+e));return exts};var emscriptenWebGLGet=(name_,p,type)=>{if(!p){GL.recordError(1281);return}var ret=undefined;switch(name_){case 36346:ret=1;break;case 36344:if(type!=0&&type!=1){GL.recordError(1280)}return;case 34814:case 36345:ret=0;break;case 34466:var formats=GLctx.getParameter(34467);ret=formats?formats.length:0;break;case 33309:if(GL.currentContext.version<2){GL.recordError(1282);return}ret=webglGetExtensions().length;break;case 33307:case 33308:if(GL.currentContext.version<2){GL.recordError(1280);return}ret=name_==33307?3:0;break}if(ret===undefined){var result=GLctx.getParameter(name_);switch(typeof result){case"number":ret=result;break;case"boolean":ret=result?1:0;break;case"string":GL.recordError(1280);return;case"object":if(result===null){switch(name_){case 34964:case 35725:case 34965:case 36006:case 36007:case 32873:case 34229:case 36662:case 36663:case 35053:case 35055:case 36010:case 35097:case 35869:case 32874:case 36389:case 35983:case 35368:case 34068:{ret=0;break}default:{GL.recordError(1280);return}}}else if(result instanceof Float32Array||result instanceof Uint32Array||result instanceof Int32Array||result instanceof Array){for(var i=0;i<result.length;++i){switch(type){case 0:HEAP32[p+i*4>>2]=result[i];break;case 2:HEAPF32[p+i*4>>2]=result[i];break;case 4:HEAP8[p+i]=result[i]?1:0;break}}return}else{try{ret=result.name|0}catch(e){GL.recordError(1280);err(`GL_INVALID_ENUM in glGet${type}v: Unknown object returned from WebGL getParameter(${name_})! (error: ${e})`);return}}break;default:GL.recordError(1280);err(`GL_INVALID_ENUM in glGet${type}v: Native code calling glGet${type}v(${name_}) and it returns ${result} of type ${typeof result}!`);return}}switch(type){case 1:writeI53ToI64(p,ret);break;case 0:HEAP32[p>>2]=ret;break;case 2:HEAPF32[p>>2]=ret;break;case 4:HEAP8[p]=ret?1:0;break}};var _glGetFloatv=(name_,p)=>emscriptenWebGLGet(name_,p,2);var _glGetIntegerv=(name_,p)=>emscriptenWebGLGet(name_,p,0);var _glGetProgramBinary=(program,bufSize,length,binaryFormat,binary)=>{GL.recordError(1282)};var _glGetProgramInfoLog=(program,maxLength,length,infoLog)=>{var log=GLctx.getProgramInfoLog(GL.programs[program]);if(log===null)log="(unknown error)";var numBytesWrittenExclNull=maxLength>0&&infoLog?stringToUTF8(log,infoLog,maxLength):0;if(length)HEAP32[length>>2]=numBytesWrittenExclNull};var _glGetProgramiv=(program,pname,p)=>{if(!p){GL.recordError(1281);return}if(program>=GL.counter){GL.recordError(1281);return}program=GL.programs[program];if(pname==35716){var log=GLctx.getProgramInfoLog(program);if(log===null)log="(unknown error)";HEAP32[p>>2]=log.length+1}else if(pname==35719){if(!program.maxUniformLength){for(var i=0;i<GLctx.getProgramParameter(program,35718);++i){program.maxUniformLength=Math.max(program.maxUniformLength,GLctx.getActiveUniform(program,i).name.length+1)}}HEAP32[p>>2]=program.maxUniformLength}else if(pname==35722){if(!program.maxAttributeLength){for(var i=0;i<GLctx.getProgramParameter(program,35721);++i){program.maxAttributeLength=Math.max(program.maxAttributeLength,GLctx.getActiveAttrib(program,i).name.length+1)}}HEAP32[p>>2]=program.maxAttributeLength}else if(pname==35381){if(!program.maxUniformBlockNameLength){for(var i=0;i<GLctx.getProgramParameter(program,35382);++i){program.maxUniformBlockNameLength=Math.max(program.maxUniformBlockNameLength,GLctx.getActiveUniformBlockName(program,i).length+1)}}HEAP32[p>>2]=program.maxUniformBlockNameLength}else{HEAP32[p>>2]=GLctx.getProgramParameter(program,pname)}};var _glGetQueryObjectuiv=(id,pname,params)=>{if(!params){GL.recordError(1281);return}var query=GL.queries[id];var param=GLctx.getQueryParameter(query,pname);var ret;if(typeof param=="boolean"){ret=param?1:0}else{ret=param}HEAP32[params>>2]=ret};var _glGetShaderInfoLog=(shader,maxLength,length,infoLog)=>{var log=GLctx.getShaderInfoLog(GL.shaders[shader]);if(log===null)log="(unknown error)";var numBytesWrittenExclNull=maxLength>0&&infoLog?stringToUTF8(log,infoLog,maxLength):0;if(length)HEAP32[length>>2]=numBytesWrittenExclNull};var _glGetShaderiv=(shader,pname,p)=>{if(!p){GL.recordError(1281);return}if(pname==35716){var log=GLctx.getShaderInfoLog(GL.shaders[shader]);if(log===null)log="(unknown error)";var logLength=log?log.length+1:0;HEAP32[p>>2]=logLength}else if(pname==35720){var source=GLctx.getShaderSource(GL.shaders[shader]);var sourceLength=source?source.length+1:0;HEAP32[p>>2]=sourceLength}else{HEAP32[p>>2]=GLctx.getShaderParameter(GL.shaders[shader],pname)}};var stringToNewUTF8=str=>{var size=lengthBytesUTF8(str)+1;var ret=_malloc(size);if(ret)stringToUTF8(str,ret,size);return ret};var _glGetString=name_=>{var ret=GL.stringCache[name_];if(!ret){switch(name_){case 7939:ret=stringToNewUTF8(webglGetExtensions().join(" "));break;case 7936:case 7937:case 37445:case 37446:var s=GLctx.getParameter(name_);if(!s){GL.recordError(1280)}ret=s?stringToNewUTF8(s):0;break;case 7938:var glVersion=GLctx.getParameter(7938);if(true)glVersion=`OpenGL ES 3.0 (${glVersion})`;else{glVersion=`OpenGL ES 2.0 (${glVersion})`}ret=stringToNewUTF8(glVersion);break;case 35724:var glslVersion=GLctx.getParameter(35724);var ver_re=/^WebGL GLSL ES ([0-9]\.[0-9][0-9]?)(?:$| .*)/;var ver_num=glslVersion.match(ver_re);if(ver_num!==null){if(ver_num[1].length==3)ver_num[1]=ver_num[1]+"0";glslVersion=`OpenGL ES GLSL ES ${ver_num[1]} (${glslVersion})`}ret=stringToNewUTF8(glslVersion);break;default:GL.recordError(1280)}GL.stringCache[name_]=ret}return ret};var _glGetUniformBlockIndex=(program,uniformBlockName)=>GLctx.getUniformBlockIndex(GL.programs[program],UTF8ToString(uniformBlockName));var jstoi_q=str=>parseInt(str);var webglGetLeftBracePos=name=>name.slice(-1)=="]"&&name.lastIndexOf("[");var webglPrepareUniformLocationsBeforeFirstUse=program=>{var uniformLocsById=program.uniformLocsById,uniformSizeAndIdsByName=program.uniformSizeAndIdsByName,i,j;if(!uniformLocsById){program.uniformLocsById=uniformLocsById={};program.uniformArrayNamesById={};for(i=0;i<GLctx.getProgramParameter(program,35718);++i){var u=GLctx.getActiveUniform(program,i);var nm=u.name;var sz=u.size;var lb=webglGetLeftBracePos(nm);var arrayName=lb>0?nm.slice(0,lb):nm;var id=program.uniformIdCounter;program.uniformIdCounter+=sz;uniformSizeAndIdsByName[arrayName]=[sz,id];for(j=0;j<sz;++j){uniformLocsById[id]=j;program.uniformArrayNamesById[id++]=arrayName}}}};var _glGetUniformLocation=(program,name)=>{name=UTF8ToString(name);if(program=GL.programs[program]){webglPrepareUniformLocationsBeforeFirstUse(program);var uniformLocsById=program.uniformLocsById;var arrayIndex=0;var uniformBaseName=name;var leftBrace=webglGetLeftBracePos(name);if(leftBrace>0){arrayIndex=jstoi_q(name.slice(leftBrace+1))>>>0;uniformBaseName=name.slice(0,leftBrace)}var sizeAndId=program.uniformSizeAndIdsByName[uniformBaseName];if(sizeAndId&&arrayIndex<sizeAndId[0]){arrayIndex+=sizeAndId[1];if(uniformLocsById[arrayIndex]=uniformLocsById[arrayIndex]||GLctx.getUniformLocation(program,name)){return arrayIndex}}}else{GL.recordError(1281)}return-1};var _glHint=(x0,x1)=>GLctx.hint(x0,x1);var _glInvalidateFramebuffer=(target,numAttachments,attachments)=>{var list=tempFixedLengthArray[numAttachments];for(var i=0;i<numAttachments;i++){list[i]=HEAP32[attachments+i*4>>2]}GLctx.invalidateFramebuffer(target,list)};var _glLinkProgram=program=>{program=GL.programs[program];GLctx.linkProgram(program);program.uniformLocsById=0;program.uniformSizeAndIdsByName={}};var emscriptenWebGLGetBufferBinding=target=>{switch(target){case 34962:target=34964;break;case 34963:target=34965;break;case 35051:target=35053;break;case 35052:target=35055;break;case 35982:target=35983;break;case 36662:target=36662;break;case 36663:target=36663;break;case 35345:target=35368;break}var buffer=GLctx.getParameter(target);if(buffer)return buffer.name|0;else return 0};var emscriptenWebGLValidateMapBufferTarget=target=>{switch(target){case 34962:case 34963:case 36662:case 36663:case 35051:case 35052:case 35882:case 35982:case 35345:return true;default:return false}};var _glMapBufferRange=(target,offset,length,access)=>{if((access&(1|32))!=0){err("glMapBufferRange access does not support MAP_READ or MAP_UNSYNCHRONIZED");return 0}if((access&2)==0){err("glMapBufferRange access must include MAP_WRITE");return 0}if((access&(4|8))==0){err("glMapBufferRange access must include INVALIDATE_BUFFER or INVALIDATE_RANGE");return 0}if(!emscriptenWebGLValidateMapBufferTarget(target)){GL.recordError(1280);err("GL_INVALID_ENUM in glMapBufferRange");return 0}var mem=_malloc(length),binding=emscriptenWebGLGetBufferBinding(target);if(!mem)return 0;if(!GL.mappedBuffers[binding])GL.mappedBuffers[binding]={};binding=GL.mappedBuffers[binding];binding.offset=offset;binding.length=length;binding.mem=mem;binding.access=access;return mem};var _glPixelStorei=(pname,param)=>{if(pname==3317){GL.unpackAlignment=param}GLctx.pixelStorei(pname,param)};var _glPolygonOffset=(x0,x1)=>GLctx.polygonOffset(x0,x1);var _glProgramBinary=(program,binaryFormat,binary,length)=>{GL.recordError(1280)};var computeUnpackAlignedImageSize=(width,height,sizePerPixel,alignment)=>{function roundedToNextMultipleOf(x,y){return x+y-1&-y}var plainRowSize=width*sizePerPixel;var alignedRowSize=roundedToNextMultipleOf(plainRowSize,alignment);return height*alignedRowSize};var colorChannelsInGlTextureFormat=format=>{var colorChannels={5:3,6:4,8:2,29502:3,29504:4,26917:2,26918:2,29846:3,29847:4};return colorChannels[format-6402]||1};var heapObjectForWebGLType=type=>{type-=5120;if(type==0)return HEAP8;if(type==1)return HEAPU8;if(type==2)return HEAP16;if(type==4)return HEAP32;if(type==6)return HEAPF32;if(type==5||type==28922||type==28520||type==30779||type==30782)return HEAPU32;return HEAPU16};var toTypedArrayIndex=(pointer,heap)=>pointer>>>31-Math.clz32(heap.BYTES_PER_ELEMENT);var emscriptenWebGLGetTexPixelData=(type,format,width,height,pixels,internalFormat)=>{var heap=heapObjectForWebGLType(type);var sizePerPixel=colorChannelsInGlTextureFormat(format)*heap.BYTES_PER_ELEMENT;var bytes=computeUnpackAlignedImageSize(width,height,sizePerPixel,GL.unpackAlignment);return heap.subarray(toTypedArrayIndex(pixels,heap),toTypedArrayIndex(pixels+bytes,heap))};var _glReadPixels=(x,y,width,height,format,type,pixels)=>{if(true){if(GLctx.currentPixelPackBufferBinding){GLctx.readPixels(x,y,width,height,format,type,pixels);return}var heap=heapObjectForWebGLType(type);var target=toTypedArrayIndex(pixels,heap);GLctx.readPixels(x,y,width,height,format,type,heap,target);return}var pixelData=emscriptenWebGLGetTexPixelData(type,format,width,height,pixels,format);if(!pixelData){GL.recordError(1280);return}GLctx.readPixels(x,y,width,height,format,type,pixelData)};var _glRenderbufferStorage=(x0,x1,x2,x3)=>GLctx.renderbufferStorage(x0,x1,x2,x3);var _glRenderbufferStorageMultisample=(x0,x1,x2,x3,x4)=>GLctx.renderbufferStorageMultisample(x0,x1,x2,x3,x4);var _glSamplerParameterf=(sampler,pname,param)=>{GLctx.samplerParameterf(GL.samplers[sampler],pname,param)};var _glSamplerParameteri=(sampler,pname,param)=>{GLctx.samplerParameteri(GL.samplers[sampler],pname,param)};var _glScissor=(x0,x1,x2,x3)=>GLctx.scissor(x0,x1,x2,x3);var _glShaderSource=(shader,count,string,length)=>{var source=GL.getSource(shader,count,string,length);GLctx.shaderSource(GL.shaders[shader],source)};var _glStencilFuncSeparate=(x0,x1,x2,x3)=>GLctx.stencilFuncSeparate(x0,x1,x2,x3);var _glStencilMaskSeparate=(x0,x1)=>GLctx.stencilMaskSeparate(x0,x1);var _glStencilOpSeparate=(x0,x1,x2,x3)=>GLctx.stencilOpSeparate(x0,x1,x2,x3);var _glTexImage2D=(target,level,internalFormat,width,height,border,format,type,pixels)=>{if(true){if(GLctx.currentPixelUnpackBufferBinding){GLctx.texImage2D(target,level,internalFormat,width,height,border,format,type,pixels);return}if(pixels){var heap=heapObjectForWebGLType(type);var index=toTypedArrayIndex(pixels,heap);GLctx.texImage2D(target,level,internalFormat,width,height,border,format,type,heap,index);return}}var pixelData=pixels?emscriptenWebGLGetTexPixelData(type,format,width,height,pixels,internalFormat):null;GLctx.texImage2D(target,level,internalFormat,width,height,border,format,type,pixelData)};var _glTexParameterf=(x0,x1,x2)=>GLctx.texParameterf(x0,x1,x2);var _glTexParameteri=(x0,x1,x2)=>GLctx.texParameteri(x0,x1,x2);var _glTexStorage2D=(x0,x1,x2,x3,x4)=>GLctx.texStorage2D(x0,x1,x2,x3,x4);var _glTexStorage3D=(x0,x1,x2,x3,x4,x5)=>GLctx.texStorage3D(x0,x1,x2,x3,x4,x5);var _glTexSubImage2D=(target,level,xoffset,yoffset,width,height,format,type,pixels)=>{if(true){if(GLctx.currentPixelUnpackBufferBinding){GLctx.texSubImage2D(target,level,xoffset,yoffset,width,height,format,type,pixels);return}if(pixels){var heap=heapObjectForWebGLType(type);GLctx.texSubImage2D(target,level,xoffset,yoffset,width,height,format,type,heap,toTypedArrayIndex(pixels,heap));return}}var pixelData=pixels?emscriptenWebGLGetTexPixelData(type,format,width,height,pixels,0):null;GLctx.texSubImage2D(target,level,xoffset,yoffset,width,height,format,type,pixelData)};var _glTexSubImage3D=(target,level,xoffset,yoffset,zoffset,width,height,depth,format,type,pixels)=>{if(GLctx.currentPixelUnpackBufferBinding){GLctx.texSubImage3D(target,level,xoffset,yoffset,zoffset,width,height,depth,format,type,pixels)}else if(pixels){var heap=heapObjectForWebGLType(type);GLctx.texSubImage3D(target,level,xoffset,yoffset,zoffset,width,height,depth,format,type,heap,toTypedArrayIndex(pixels,heap))}else{GLctx.texSubImage3D(target,level,xoffset,yoffset,zoffset,width,height,depth,format,type,null)}};var webglGetUniformLocation=location=>{var p=GLctx.currentProgram;if(p){var webglLoc=p.uniformLocsById[location];if(typeof webglLoc=="number"){p.uniformLocsById[location]=webglLoc=GLctx.getUniformLocation(p,p.uniformArrayNamesById[location]+(webglLoc>0?`[${webglLoc}]`:""))}return webglLoc}else{GL.recordError(1282)}};var _glUniform1f=(location,v0)=>{GLctx.uniform1f(webglGetUniformLocation(location),v0)};var _glUniform1fv=(location,count,value)=>{count&&GLctx.uniform1fv(webglGetUniformLocation(location),HEAPF32,value>>2,count)};var _glUniform1i=(location,v0)=>{GLctx.uniform1i(webglGetUniformLocation(location),v0)};var _glUniform1iv=(location,count,value)=>{count&&GLctx.uniform1iv(webglGetUniformLocation(location),HEAP32,value>>2,count)};var _glUniform2fv=(location,count,value)=>{count&&GLctx.uniform2fv(webglGetUniformLocation(location),HEAPF32,value>>2,count*2)};var _glUniform2iv=(location,count,value)=>{count&&GLctx.uniform2iv(webglGetUniformLocation(location),HEAP32,value>>2,count*2)};var _glUniform3fv=(location,count,value)=>{count&&GLctx.uniform3fv(webglGetUniformLocation(location),HEAPF32,value>>2,count*3)};var _glUniform3iv=(location,count,value)=>{count&&GLctx.uniform3iv(webglGetUniformLocation(location),HEAP32,value>>2,count*3)};var _glUniform4fv=(location,count,value)=>{count&&GLctx.uniform4fv(webglGetUniformLocation(location),HEAPF32,value>>2,count*4)};var _glUniform4iv=(location,count,value)=>{count&&GLctx.uniform4iv(webglGetUniformLocation(location),HEAP32,value>>2,count*4)};var _glUniformBlockBinding=(program,uniformBlockIndex,uniformBlockBinding)=>{program=GL.programs[program];GLctx.uniformBlockBinding(program,uniformBlockIndex,uniformBlockBinding)};var _glUniformMatrix3fv=(location,count,transpose,value)=>{count&&GLctx.uniformMatrix3fv(webglGetUniformLocation(location),!!transpose,HEAPF32,value>>2,count*9)};var _glUniformMatrix4fv=(location,count,transpose,value)=>{count&&GLctx.uniformMatrix4fv(webglGetUniformLocation(location),!!transpose,HEAPF32,value>>2,count*16)};var _glUnmapBuffer=target=>{if(!emscriptenWebGLValidateMapBufferTarget(target)){GL.recordError(1280);err("GL_INVALID_ENUM in glUnmapBuffer");return 0}var buffer=emscriptenWebGLGetBufferBinding(target);var mapping=GL.mappedBuffers[buffer];if(!mapping||!mapping.mem){GL.recordError(1282);err("buffer was never mapped in glUnmapBuffer");return 0}if(!(mapping.access&16)){if(true){GLctx.bufferSubData(target,mapping.offset,HEAPU8,mapping.mem,mapping.length)}else GLctx.bufferSubData(target,mapping.offset,HEAPU8.subarray(mapping.mem,mapping.mem+mapping.length))}_free(mapping.mem);mapping.mem=0;return 1};var _glUseProgram=program=>{program=GL.programs[program];GLctx.useProgram(program);GLctx.currentProgram=program};var _glVertexAttrib4f=(x0,x1,x2,x3,x4)=>GLctx.vertexAttrib4f(x0,x1,x2,x3,x4);var _glVertexAttribI4ui=(x0,x1,x2,x3,x4)=>GLctx.vertexAttribI4ui(x0,x1,x2,x3,x4);var _glVertexAttribIPointer=(index,size,type,stride,ptr)=>{var cb=GL.currentContext.clientBuffers[index];if(!GLctx.currentArrayBufferBinding){cb.size=size;cb.type=type;cb.normalized=false;cb.stride=stride;cb.ptr=ptr;cb.clientside=true;cb.vertexAttribPointerAdaptor=function(index,size,type,normalized,stride,ptr){this.vertexAttribIPointer(index,size,type,stride,ptr)};return}cb.clientside=false;GLctx.vertexAttribIPointer(index,size,type,stride,ptr)};var _glVertexAttribPointer=(index,size,type,normalized,stride,ptr)=>{var cb=GL.currentContext.clientBuffers[index];if(!GLctx.currentArrayBufferBinding){cb.size=size;cb.type=type;cb.normalized=normalized;cb.stride=stride;cb.ptr=ptr;cb.clientside=true;cb.vertexAttribPointerAdaptor=function(index,size,type,normalized,stride,ptr){this.vertexAttribPointer(index,size,type,normalized,stride,ptr)};return}cb.clientside=false;GLctx.vertexAttribPointer(index,size,type,!!normalized,stride,ptr)};var _glViewport=(x0,x1,x2,x3)=>GLctx.viewport(x0,x1,x2,x3);var isLeapYear=year=>year%4===0&&(year%100!==0||year%400===0);var arraySum=(array,index)=>{var sum=0;for(var i=0;i<=index;sum+=array[i++]){}return sum};var MONTH_DAYS_LEAP=[31,29,31,30,31,30,31,31,30,31,30,31];var MONTH_DAYS_REGULAR=[31,28,31,30,31,30,31,31,30,31,30,31];var addDays=(date,days)=>{var newDate=new Date(date.getTime());while(days>0){var leap=isLeapYear(newDate.getFullYear());var currentMonth=newDate.getMonth();var daysInCurrentMonth=(leap?MONTH_DAYS_LEAP:MONTH_DAYS_REGULAR)[currentMonth];if(days>daysInCurrentMonth-newDate.getDate()){days-=daysInCurrentMonth-newDate.getDate()+1;newDate.setDate(1);if(currentMonth<11){newDate.setMonth(currentMonth+1)}else{newDate.setMonth(0);newDate.setFullYear(newDate.getFullYear()+1)}}else{newDate.setDate(newDate.getDate()+days);return newDate}}return newDate};var writeArrayToMemory=(array,buffer)=>{HEAP8.set(array,buffer)};var _strftime=(s,maxsize,format,tm)=>{var tm_zone=HEAPU32[tm+40>>2];var date={tm_sec:HEAP32[tm>>2],tm_min:HEAP32[tm+4>>2],tm_hour:HEAP32[tm+8>>2],tm_mday:HEAP32[tm+12>>2],tm_mon:HEAP32[tm+16>>2],tm_year:HEAP32[tm+20>>2],tm_wday:HEAP32[tm+24>>2],tm_yday:HEAP32[tm+28>>2],tm_isdst:HEAP32[tm+32>>2],tm_gmtoff:HEAP32[tm+36>>2],tm_zone:tm_zone?UTF8ToString(tm_zone):""};var pattern=UTF8ToString(format);var EXPANSION_RULES_1={"%c":"%a %b %d %H:%M:%S %Y","%D":"%m/%d/%y","%F":"%Y-%m-%d","%h":"%b","%r":"%I:%M:%S %p","%R":"%H:%M","%T":"%H:%M:%S","%x":"%m/%d/%y","%X":"%H:%M:%S","%Ec":"%c","%EC":"%C","%Ex":"%m/%d/%y","%EX":"%H:%M:%S","%Ey":"%y","%EY":"%Y","%Od":"%d","%Oe":"%e","%OH":"%H","%OI":"%I","%Om":"%m","%OM":"%M","%OS":"%S","%Ou":"%u","%OU":"%U","%OV":"%V","%Ow":"%w","%OW":"%W","%Oy":"%y"};for(var rule in EXPANSION_RULES_1){pattern=pattern.replace(new RegExp(rule,"g"),EXPANSION_RULES_1[rule])}var WEEKDAYS=["Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"];var MONTHS=["January","February","March","April","May","June","July","August","September","October","November","December"];function leadingSomething(value,digits,character){var str=typeof value=="number"?value.toString():value||"";while(str.length<digits){str=character[0]+str}return str}function leadingNulls(value,digits){return leadingSomething(value,digits,"0")}function compareByDay(date1,date2){function sgn(value){return value<0?-1:value>0?1:0}var compare;if((compare=sgn(date1.getFullYear()-date2.getFullYear()))===0){if((compare=sgn(date1.getMonth()-date2.getMonth()))===0){compare=sgn(date1.getDate()-date2.getDate())}}return compare}function getFirstWeekStartDate(janFourth){switch(janFourth.getDay()){case 0:return new Date(janFourth.getFullYear()-1,11,29);case 1:return janFourth;case 2:return new Date(janFourth.getFullYear(),0,3);case 3:return new Date(janFourth.getFullYear(),0,2);case 4:return new Date(janFourth.getFullYear(),0,1);case 5:return new Date(janFourth.getFullYear()-1,11,31);case 6:return new Date(janFourth.getFullYear()-1,11,30)}}function getWeekBasedYear(date){var thisDate=addDays(new Date(date.tm_year+1900,0,1),date.tm_yday);var janFourthThisYear=new Date(thisDate.getFullYear(),0,4);var janFourthNextYear=new Date(thisDate.getFullYear()+1,0,4);var firstWeekStartThisYear=getFirstWeekStartDate(janFourthThisYear);var firstWeekStartNextYear=getFirstWeekStartDate(janFourthNextYear);if(compareByDay(firstWeekStartThisYear,thisDate)<=0){if(compareByDay(firstWeekStartNextYear,thisDate)<=0){return thisDate.getFullYear()+1}return thisDate.getFullYear()}return thisDate.getFullYear()-1}var EXPANSION_RULES_2={"%a":date=>WEEKDAYS[date.tm_wday].substring(0,3),"%A":date=>WEEKDAYS[date.tm_wday],"%b":date=>MONTHS[date.tm_mon].substring(0,3),"%B":date=>MONTHS[date.tm_mon],"%C":date=>{var year=date.tm_year+1900;return leadingNulls(year/100|0,2)},"%d":date=>leadingNulls(date.tm_mday,2),"%e":date=>leadingSomething(date.tm_mday,2," "),"%g":date=>getWeekBasedYear(date).toString().substring(2),"%G":getWeekBasedYear,"%H":date=>leadingNulls(date.tm_hour,2),"%I":date=>{var twelveHour=date.tm_hour;if(twelveHour==0)twelveHour=12;else if(twelveHour>12)twelveHour-=12;return leadingNulls(twelveHour,2)},"%j":date=>leadingNulls(date.tm_mday+arraySum(isLeapYear(date.tm_year+1900)?MONTH_DAYS_LEAP:MONTH_DAYS_REGULAR,date.tm_mon-1),3),"%m":date=>leadingNulls(date.tm_mon+1,2),"%M":date=>leadingNulls(date.tm_min,2),"%n":()=>"\n","%p":date=>{if(date.tm_hour>=0&&date.tm_hour<12){return"AM"}return"PM"},"%S":date=>leadingNulls(date.tm_sec,2),"%t":()=>"\t","%u":date=>date.tm_wday||7,"%U":date=>{var days=date.tm_yday+7-date.tm_wday;return leadingNulls(Math.floor(days/7),2)},"%V":date=>{var val=Math.floor((date.tm_yday+7-(date.tm_wday+6)%7)/7);if((date.tm_wday+371-date.tm_yday-2)%7<=2){val++}if(!val){val=52;var dec31=(date.tm_wday+7-date.tm_yday-1)%7;if(dec31==4||dec31==5&&isLeapYear(date.tm_year%400-1)){val++}}else if(val==53){var jan1=(date.tm_wday+371-date.tm_yday)%7;if(jan1!=4&&(jan1!=3||!isLeapYear(date.tm_year)))val=1}return leadingNulls(val,2)},"%w":date=>date.tm_wday,"%W":date=>{var days=date.tm_yday+7-(date.tm_wday+6)%7;return leadingNulls(Math.floor(days/7),2)},"%y":date=>(date.tm_year+1900).toString().substring(2),"%Y":date=>date.tm_year+1900,"%z":date=>{var off=date.tm_gmtoff;var ahead=off>=0;off=Math.abs(off)/60;off=off/60*100+off%60;return(ahead?"+":"-")+String("0000"+off).slice(-4)},"%Z":date=>date.tm_zone,"%%":()=>"%"};pattern=pattern.replace(/%%/g,"\0\0");for(var rule in EXPANSION_RULES_2){if(pattern.includes(rule)){pattern=pattern.replace(new RegExp(rule,"g"),EXPANSION_RULES_2[rule](date))}}pattern=pattern.replace(/\0\0/g,"%");var bytes=intArrayFromString(pattern,false);if(bytes.length>maxsize){return 0}writeArrayToMemory(bytes,s);return bytes.length-1};var _strftime_l=(s,maxsize,format,tm,loc)=>_strftime(s,maxsize,format,tm);FS.createPreloadedFile=FS_createPreloadedFile;FS.staticInit();InternalError=Module["InternalError"]=class InternalError extends Error{constructor(message){super(message);this.name="InternalError"}};embind_init_charCodes();BindingError=Module["BindingError"]=class BindingError extends Error{constructor(message){super(message);this.name="BindingError"}};init_ClassHandle();init_embind();init_RegisteredPointer();UnboundTypeError=Module["UnboundTypeError"]=extendError(Error,"UnboundTypeError");init_emval();var GLctx;for(var i=0;i<32;++i)tempFixedLengthArray.push(new Array(i));var wasmImports={Ga:___syscall_fcntl64,Hb:___syscall_ioctl,Ib:___syscall_openat,Db:___syscall_stat64,Nb:__abort_js,v:__embind_finalize_value_array,j:__embind_finalize_value_object,yb:__embind_register_bigint,tb:__embind_register_bool,d:__embind_register_class,h:__embind_register_class_class_function,i:__embind_register_class_constructor,a:__embind_register_class_function,t:__embind_register_class_property,Xa:__embind_register_emval,f:__embind_register_enum,b:__embind_register_enum_value,Da:__embind_register_float,Z:__embind_register_function,z:__embind_register_integer,l:__embind_register_memory_view,Y:__embind_register_optional,Ba:__embind_register_std_string,ia:__embind_register_std_wstring,w:__embind_register_value_array,e:__embind_register_value_array_element,k:__embind_register_value_object,c:__embind_register_value_object_field,vb:__embind_register_void,Kb:__emscripten_get_now_is_monotonic,Mb:__emscripten_memcpy_js,p:__emval_as,g:__emval_decref,q:__emval_get_property,ub:__emval_incref,F:__emval_new_cstring,o:__emval_run_destructors,r:__emval_take_value,ma:_emscripten_asm_const_int,Lb:_emscripten_date_now,sb:_emscripten_err,Cb:_emscripten_get_heap_max,Jb:_emscripten_get_now,rb:_emscripten_out,Bb:_emscripten_resize_heap,Eb:_environ_get,Fb:_environ_sizes_get,na:_fd_close,Gb:_fd_read,wb:_fd_seek,Fa:_fd_write,zb:_getentropy,m:_glActiveTexture,ka:_glAttachShader,db:_glBeginQuery,lb:_glBindAttribLocation,y:_glBindBuffer,sa:_glBindBufferBase,Ua:_glBindBufferRange,P:_glBindFramebuffer,Ma:_glBindRenderbuffer,U:_glBindSampler,I:_glBindTexture,$a:_glBindVertexArray,va:_glBlendEquationSeparate,ua:_glBlendFuncSeparate,$:_glBlitFramebuffer,C:_glBufferData,aa:_glBufferSubData,Rb:_glClear,Wb:_glClearBufferfi,A:_glClearBufferfv,Vb:_glClearBufferiv,Ub:_glClearColor,Tb:_glClearDepthf,Sb:_glClearStencil,xb:_glClientWaitSync,ha:_glColorMask,mb:_glCompileShader,Ja:_glCompressedTexSubImage2D,Ia:_glCompressedTexSubImage3D,fc:_glCopyBufferSubData,Ea:_glCreateProgram,ob:_glCreateShader,wa:_glCullFace,fa:_glDeleteBuffers,Q:_glDeleteFramebuffers,la:_glDeleteProgram,cb:_glDeleteQueries,Ra:_glDeleteRenderbuffers,ya:_glDeleteSamplers,V:_glDeleteShader,Oa:_glDeleteSync,Sa:_glDeleteTextures,ab:_glDeleteVertexArrays,ja:_glDepthFunc,ga:_glDepthMask,pa:_glDepthRangef,W:_glDetachShader,n:_glDisable,Zb:_glDisableVertexAttribArray,ec:_glDrawBuffers,Ha:_glDrawElements,Pa:_glDrawElementsInstanced,u:_glEnable,_b:_glEnableVertexAttribArray,eb:_glEndQuery,ba:_glFenceSync,Ca:_glFinish,hb:_glFlush,G:_glFramebufferRenderbuffer,x:_glFramebufferTexture2D,M:_glFramebufferTextureLayer,xa:_glFrontFace,R:_glGenBuffers,_:_glGenFramebuffers,bb:_glGenQueries,oa:_glGenRenderbuffers,Aa:_glGenSamplers,ea:_glGenTextures,_a:_glGenVertexArrays,hc:_glGenerateMipmap,Qb:_glGetBufferSubData,X:_glGetError,Za:_glGetFloatv,s:_glGetIntegerv,pb:_glGetProgramBinary,ib:_glGetProgramInfoLog,L:_glGetProgramiv,fb:_glGetQueryObjectuiv,jb:_glGetShaderInfoLog,E:_glGetShaderiv,J:_glGetString,Wa:_glGetUniformBlockIndex,da:_glGetUniformLocation,Ya:_glHint,gb:_glInvalidateFramebuffer,kb:_glLinkProgram,Pb:_glMapBufferRange,N:_glPixelStorei,ta:_glPolygonOffset,qb:_glProgramBinary,Qa:_glReadPixels,Xb:_glRenderbufferStorage,Yb:_glRenderbufferStorageMultisample,za:_glSamplerParameterf,H:_glSamplerParameteri,ra:_glScissor,nb:_glShaderSource,T:_glStencilFuncSeparate,D:_glStencilMaskSeparate,S:_glStencilOpSeparate,K:_glTexImage2D,Ta:_glTexParameterf,B:_glTexParameteri,dc:_glTexStorage2D,Na:_glTexStorage3D,La:_glTexSubImage2D,Ka:_glTexSubImage3D,gc:_glUniform1f,rc:_glUniform1fv,ca:_glUniform1i,nc:_glUniform1iv,qc:_glUniform2fv,mc:_glUniform2iv,pc:_glUniform3fv,lc:_glUniform3iv,oc:_glUniform4fv,kc:_glUniform4iv,Va:_glUniformBlockBinding,jc:_glUniformMatrix3fv,ic:_glUniformMatrix4fv,Ob:_glUnmapBuffer,O:_glUseProgram,$b:_glVertexAttrib4f,ac:_glVertexAttribI4ui,cc:_glVertexAttribIPointer,bc:_glVertexAttribPointer,qa:_glViewport,Ab:_strftime_l};var wasmExports=createWasm();var ___wasm_call_ctors=()=>(___wasm_call_ctors=wasmExports["tc"])();var ___getTypeName=a0=>(___getTypeName=wasmExports["uc"])(a0);var _malloc=a0=>(_malloc=wasmExports["wc"])(a0);var _free=a0=>(_free=wasmExports["xc"])(a0);var dynCall_ji=Module["dynCall_ji"]=(a0,a1)=>(dynCall_ji=Module["dynCall_ji"]=wasmExports["yc"])(a0,a1);var dynCall_j=Module["dynCall_j"]=a0=>(dynCall_j=Module["dynCall_j"]=wasmExports["zc"])(a0);var dynCall_vij=Module["dynCall_vij"]=(a0,a1,a2,a3)=>(dynCall_vij=Module["dynCall_vij"]=wasmExports["Ac"])(a0,a1,a2,a3);var dynCall_viij=Module["dynCall_viij"]=(a0,a1,a2,a3,a4)=>(dynCall_viij=Module["dynCall_viij"]=wasmExports["Bc"])(a0,a1,a2,a3,a4);var dynCall_iiiiij=Module["dynCall_iiiiij"]=(a0,a1,a2,a3,a4,a5,a6)=>(dynCall_iiiiij=Module["dynCall_iiiiij"]=wasmExports["Cc"])(a0,a1,a2,a3,a4,a5,a6);var dynCall_jii=Module["dynCall_jii"]=(a0,a1,a2)=>(dynCall_jii=Module["dynCall_jii"]=wasmExports["Dc"])(a0,a1,a2);var dynCall_iiij=Module["dynCall_iiij"]=(a0,a1,a2,a3,a4)=>(dynCall_iiij=Module["dynCall_iiij"]=wasmExports["Ec"])(a0,a1,a2,a3,a4);var dynCall_iiiij=Module["dynCall_iiiij"]=(a0,a1,a2,a3,a4,a5)=>(dynCall_iiiij=Module["dynCall_iiiij"]=wasmExports["Fc"])(a0,a1,a2,a3,a4,a5);var dynCall_vijji=Module["dynCall_vijji"]=(a0,a1,a2,a3,a4,a5,a6)=>(dynCall_vijji=Module["dynCall_vijji"]=wasmExports["Gc"])(a0,a1,a2,a3,a4,a5,a6);var dynCall_jiji=Module["dynCall_jiji"]=(a0,a1,a2,a3,a4)=>(dynCall_jiji=Module["dynCall_jiji"]=wasmExports["Hc"])(a0,a1,a2,a3,a4);var dynCall_viijii=Module["dynCall_viijii"]=(a0,a1,a2,a3,a4,a5,a6)=>(dynCall_viijii=Module["dynCall_viijii"]=wasmExports["Ic"])(a0,a1,a2,a3,a4,a5,a6);var dynCall_iiiiijj=Module["dynCall_iiiiijj"]=(a0,a1,a2,a3,a4,a5,a6,a7,a8)=>(dynCall_iiiiijj=Module["dynCall_iiiiijj"]=wasmExports["Jc"])(a0,a1,a2,a3,a4,a5,a6,a7,a8);var dynCall_iiiiiijj=Module["dynCall_iiiiiijj"]=(a0,a1,a2,a3,a4,a5,a6,a7,a8,a9)=>(dynCall_iiiiiijj=Module["dynCall_iiiiiijj"]=wasmExports["Kc"])(a0,a1,a2,a3,a4,a5,a6,a7,a8,a9);var calledRun;dependenciesFulfilled=function runCaller(){if(!calledRun)run();if(!calledRun)dependenciesFulfilled=runCaller};function run(){if(runDependencies>0){return}preRun();if(runDependencies>0){return}function doRun(){if(calledRun)return;calledRun=true;Module["calledRun"]=true;if(ABORT)return;initRuntime();readyPromiseResolve(Module);if(Module["onRuntimeInitialized"])Module["onRuntimeInitialized"]();postRun()}if(Module["setStatus"]){Module["setStatus"]("Running...");setTimeout(function(){setTimeout(function(){Module["setStatus"]("")},1);doRun()},1)}else{doRun()}}if(Module["preInit"]){if(typeof Module["preInit"]=="function")Module["preInit"]=[Module["preInit"]];while(Module["preInit"].length>0){Module["preInit"].pop()()}}run();moduleRtn=readyPromise;


  return moduleRtn;
}
);
})();
if (typeof exports === 'object' && typeof module === 'object')
  module.exports = Filament;
else if (typeof define === 'function' && define['amd'])
  define([], () => Filament);
/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

Filament.onReadyListeners = [];

Filament.isReady = false;

/// init ::function:: Downloads assets, loads the Filament module, and invokes a callback when done.
///
/// All JavaScript clients must call the init function, passing in a list of asset URL's and a
/// callback. This callback gets invoked only after all assets have been downloaded and the Filament
/// WebAssembly module has been loaded. Clients should only pass asset URL's that absolutely must
/// be ready at initialization time.
///
/// When the callback is called, each downloaded asset is available in the `Filament.assets` global
/// object, which contains a mapping from URL's to Uint8Array objects.
///
/// assets ::argument:: Array of strings containing URL's of required assets.
/// onready ::argument:: callback that gets invoked after all assets have been downloaded and the \
/// Filament WebAssembly module has been loaded.
Filament.init = (assets, onready) => {
    if (onready) {
        Filament.onReadyListeners.push(onready);
    }
    if (Filament.initialized) {
        console.assert(!assets || assets.length == 0, "Assets can be specified only with the first call to init.");
        return;
    };
    Filament.initialized = true;

    Filament.assets = {};

    // Usage of glmatrix is optional. If it exists, then go ahead and augment it with some
    // useful math functions.
    if (typeof glMatrix !== 'undefined') {
        Filament.loadMathExtensions();
    }

    // One task for compiling & loading the wasm file, plus one task for each asset.
    let remainingTasks = 1 + assets.length;
    const taskFinished = () => {
        if (--remainingTasks == 0) {
            for (const callback of Filament.onReadyListeners) {
                callback();
            }
            Filament.isReady = true;
        }
    };

    // Issue a fetch for each asset.
    Filament.fetch(assets, null, taskFinished);

    // Emscripten creates a global function called "Filament" that returns a promise that
    // resolves to a module. Here we replace the function with the module. Note that our
    // TypeScript bindings assume that Filament is a namespace, not a function.
    Filament().then(module => {

        // Merge our extension functions into the emscripten module, not the other
        // way around, because Emscripten potentially replaces the HEAPU8 views in
        // the original module object (e.g. if it needs to grow the heap).
        Filament = Object.assign(module, Filament);

        // At this point, emscripten has finished compiling and instancing the WebAssembly module.
        // The JS classes that correspond to core Filament classes (e.g., Engine) are not guaranteed
        // to exist until now.

        Filament.loadClassExtensions();
        taskFinished();
    });
};

Filament.clearAssetCache = () => {
    for (const key in Filament.assets) delete Filament.assets[key];
};

/// fetch ::function:: Downloads assets and invokes a callback when done.
///
/// This utility consumes an array of URI strings and invokes callbacks after each asset is
/// downloaded. Additionally, each downloaded asset becomes available in the `Filament.assets`
/// global object, which is a mapping from URI strings to `Uint8Array`. If desired, clients can
/// pre-populate entries in `Filament.assets` to circumvent HTTP requests (this should be done after
/// calling `Filament.init`).
///
/// This function is used internally by `Filament.init` and `gltfio$FilamentAsset.loadResources`.
///
/// assets ::argument:: Array of strings containing URL's of required assets.
/// onDone ::argument:: callback that gets invoked after all assets have been downloaded.
/// onFetched ::argument:: optional callback that's invoked after each asset is downloaded.
Filament.fetch = (assets, onDone, onFetched) => {
    let remainingAssets = assets.length;
    assets.forEach(name => {

        // Check if a buffer already exists in case the client wishes
        // to provide its own data rather than using a HTTP request.
        if (Filament.assets[name]) {
            if (onFetched) {
                onFetched(name);
            }
            if (--remainingAssets === 0 && onDone) {
                onDone();
            }
        } else {
            fetch(name).then(response => {
                if (!response.ok) {
                    throw new Error(name);
                }
                return response.arrayBuffer();
            }).then(arrayBuffer => {
                Filament.assets[name] = new Uint8Array(arrayBuffer);
                if (onFetched) {
                    onFetched(name);
                }
                if (--remainingAssets === 0 && onDone) {
                    onDone();
                }
            });
        }
    });
};

// This file has been generated by beamsplitter

Filament.loadGeneratedExtensions = function() {

    Filament.View.prototype.setDynamicResolutionOptionsDefaults = function(overrides) {
        const options = {
            minScale: [0.5, 0.5],
            maxScale: [1.0, 1.0],
            sharpness: 0.9,
            enabled: false,
            homogeneousScaling: false,
            quality: Filament.View$QualityLevel.LOW,
        };
        return Object.assign(options, overrides);
    };

    Filament.View.prototype.setBloomOptionsDefaults = function(overrides) {
        const options = {
            // JavaScript binding for dirt is not yet supported, must use default value.
            // JavaScript binding for dirtStrength is not yet supported, must use default value.
            strength: 0.10,
            resolution: 384,
            levels: 6,
            blendMode: Filament.View$BloomOptions$BlendMode.ADD,
            threshold: true,
            enabled: false,
            highlight: 1000.0,
            quality: Filament.View$QualityLevel.LOW,
            lensFlare: false,
            starburst: true,
            chromaticAberration: 0.005,
            ghostCount: 4,
            ghostSpacing: 0.6,
            ghostThreshold: 10.0,
            haloThickness: 0.1,
            haloRadius: 0.4,
            haloThreshold: 10.0,
        };
        return Object.assign(options, overrides);
    };

    Filament.View.prototype.setFogOptionsDefaults = function(overrides) {
        const options = {
            distance: 0.0,
            cutOffDistance: Infinity,
            maximumOpacity: 1.0,
            height: 0.0,
            heightFalloff: 1.0,
            color: [ 1.0, 1.0, 1.0 ],
            density: 0.1,
            inScatteringStart: 0.0,
            inScatteringSize: -1.0,
            fogColorFromIbl: false,
            // JavaScript binding for skyColor is not yet supported, must use default value.
            enabled: false,
        };
        return Object.assign(options, overrides);
    };

    Filament.View.prototype.setDepthOfFieldOptionsDefaults = function(overrides) {
        const options = {
            cocScale: 1.0,
            cocAspectRatio: 1.0,
            maxApertureDiameter: 0.01,
            enabled: false,
            filter: Filament.View$DepthOfFieldOptions$Filter.MEDIAN,
            nativeResolution: false,
            foregroundRingCount: 0,
            backgroundRingCount: 0,
            fastGatherRingCount: 0,
            maxForegroundCOC: 0,
            maxBackgroundCOC: 0,
        };
        return Object.assign(options, overrides);
    };

    Filament.View.prototype.setVignetteOptionsDefaults = function(overrides) {
        const options = {
            midPoint: 0.5,
            roundness: 0.5,
            feather: 0.5,
            color: [0.0, 0.0, 0.0, 1.0],
            enabled: false,
        };
        return Object.assign(options, overrides);
    };

    Filament.View.prototype.setRenderQualityDefaults = function(overrides) {
        const options = {
            hdrColorBuffer: Filament.View$QualityLevel.HIGH,
        };
        return Object.assign(options, overrides);
    };

    Filament.View.prototype.setSsctDefaults = function(overrides) {
        const options = {
            lightConeRad: 1.0,
            shadowDistance: 0.3,
            contactDistanceMax: 1.0,
            intensity: 0.8,
            lightDirection: [ 0, -1, 0 ],
            depthBias: 0.01,
            depthSlopeBias: 0.01,
            sampleCount: 4,
            rayCount: 1,
            enabled: false,
        };
        return Object.assign(options, overrides);
    };

    Filament.View.prototype.setAmbientOcclusionOptionsDefaults = function(overrides) {
        const options = {
            radius: 0.3,
            power: 1.0,
            bias: 0.0005,
            resolution: 0.5,
            intensity: 1.0,
            bilateralThreshold: 0.05,
            quality: Filament.View$QualityLevel.LOW,
            lowPassFilter: Filament.View$QualityLevel.MEDIUM,
            upsampling: Filament.View$QualityLevel.LOW,
            enabled: false,
            bentNormals: false,
            minHorizonAngleRad: 0.0,
            // JavaScript binding for ssct is not yet supported, must use default value.
        };
        return Object.assign(options, overrides);
    };

    Filament.View.prototype.setMultiSampleAntiAliasingOptionsDefaults = function(overrides) {
        const options = {
            enabled: false,
            sampleCount: 4,
            customResolve: false,
        };
        return Object.assign(options, overrides);
    };

    Filament.View.prototype.setTemporalAntiAliasingOptionsDefaults = function(overrides) {
        const options = {
            filterWidth: 1.0,
            feedback: 0.12,
            lodBias: -1.0,
            sharpness: 0.0,
            enabled: false,
            upscaling: false,
            filterHistory: true,
            filterInput: true,
            useYCoCg: false,
            boxType: Filament.View$TemporalAntiAliasingOptions$BoxType.AABB,
            boxClipping: Filament.View$TemporalAntiAliasingOptions$BoxClipping.ACCURATE,
            jitterPattern: Filament.View$TemporalAntiAliasingOptions$JitterPattern.HALTON_23_X16,
            varianceGamma: 1.0,
            preventFlickering: false,
            historyReprojection: true,
        };
        return Object.assign(options, overrides);
    };

    Filament.View.prototype.setScreenSpaceReflectionsOptionsDefaults = function(overrides) {
        const options = {
            thickness: 0.1,
            bias: 0.01,
            maxDistance: 3.0,
            stride: 2.0,
            enabled: false,
        };
        return Object.assign(options, overrides);
    };

    Filament.View.prototype.setGuardBandOptionsDefaults = function(overrides) {
        const options = {
            enabled: false,
        };
        return Object.assign(options, overrides);
    };

    Filament.View.prototype.setVsmShadowOptionsDefaults = function(overrides) {
        const options = {
            anisotropy: 0,
            mipmapping: false,
            msaaSamples: 1,
            highPrecision: false,
            minVarianceScale: 0.5,
            lightBleedReduction: 0.15,
        };
        return Object.assign(options, overrides);
    };

    Filament.View.prototype.setSoftShadowOptionsDefaults = function(overrides) {
        const options = {
            penumbraScale: 1.0,
            penumbraRatioScale: 1.0,
        };
        return Object.assign(options, overrides);
    };

    Filament.View.prototype.setStereoscopicOptionsDefaults = function(overrides) {
        const options = {
            enabled: false,
        };
        return Object.assign(options, overrides);
    };

};

/*
* Copyright (C) 2018 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

// Private utility that converts an asset string or Uint8Array into a low-level buffer descriptor.
// Note that the low-level buffer descriptor must be manually deleted.
function getBufferDescriptor(buffer) {
    if ('string' == typeof buffer || buffer instanceof String) {
        buffer = Filament.assets[buffer];
    }
    if (buffer.buffer instanceof ArrayBuffer) {
        buffer = Filament.Buffer(buffer);
    }
    return buffer;
}

function isTexture(uri) {
    // TODO: This is not a great way to determine if a resource is a texture, but we can
    // remove it after gltfio gains support for concurrent downloading of vertex data:
    // https://github.com/google/filament/issues/5909
    if (uri.endsWith(".png")) {
        return true;
    }
    if (uri.endsWith(".ktx2")) {
        return true;
    }
    if (uri.endsWith(".jpg") || uri.endsWith(".jpeg")) {
        return true;
    }
    return false;
}

Filament.vectorToArray = function(vector) {
    const result = [];
    for (let i = 0; i < vector.size(); i++) {
        result.push(vector.get(i));
    }
    return result;
};

Filament.shadowOptions = function(overrides) {
    const options = {
        mapSize: 1024,
        shadowCascades: 1,
        constantBias: 0.001,
        normalBias: 1.0,
        shadowFar: 0.0,
        shadowNearHint: 1.0,
        shadowFarHint: 100.0,
        stable: false,
        polygonOffsetConstant: 0.5,
        polygonOffsetSlope: 2.0,
        screenSpaceContactShadows: false,
        stepCount: 8,
        maxShadowDistance: 0.3
    };
    return Object.assign(options, overrides);
};

Filament.loadClassExtensions = function() {

    Filament.loadGeneratedExtensions();

    /// Engine ::core class::

    /// create ::static method:: Creates an Engine instance for the given canvas.
    /// canvas ::argument:: the canvas DOM element
    /// options ::argument:: optional WebGL 2.0 context configuration
    /// ::retval:: an instance of [Engine]
    Filament.Engine.create = function(canvas, options) {
        const defaults = {
            majorVersion: 2,
            minorVersion: 0,
            antialias: false,
            depth: true,
            alpha: false
        };
        options = Object.assign(defaults, options);

        // Create the WebGL 2.0 context.
        const ctx = canvas.getContext("webgl2", options);

        // Enable all desired extensions by calling getExtension on each one.
        ctx.getExtension('WEBGL_compressed_texture_s3tc');
        ctx.getExtension('WEBGL_compressed_texture_s3tc_srgb');
        ctx.getExtension('WEBGL_compressed_texture_astc');
        ctx.getExtension('WEBGL_compressed_texture_etc');

        // These transient globals are used temporarily during Engine construction.
        window.filament_glOptions = options;
        window.filament_glContext = ctx;

        // Register the GL context with emscripten and create the Engine.
        const engine = Filament.Engine._create();

        // Annotate the engine with the GL context to support multiple canvases.
        engine.context = window.filament_glContext;
        engine.handle = window.filament_contextHandle;

        // Ensure that we do not pollute the global namespace.
        delete window.filament_glOptions;
        delete window.filament_glContext;
        delete window.filament_contextHandle;

        return engine;
    };

    Filament.Engine.prototype.execute = function() {
        window.filament_contextHandle = this.handle;
        this._execute();
        delete window.filament_contextHandle;
    };

    /// createMaterial ::method::
    /// package ::argument:: asset string, or Uint8Array, or [Buffer] with filamat contents
    /// ::retval:: an instance of [createMaterial]
    Filament.Engine.prototype.createMaterial = function(buffer) {
        buffer = getBufferDescriptor(buffer);
        const result = this._createMaterial(buffer);
        buffer.delete();
        return result;
    };

    /// createTextureFromKtx1 ::method:: Utility function that creates a [Texture] from a KTX1 file.
    /// buffer ::argument:: asset string, or Uint8Array, or [Buffer] with KTX1 file contents
    /// options ::argument:: Options dictionary.
    /// ::retval:: [Texture]
    Filament.Engine.prototype.createTextureFromKtx1 = function(buffer, options) {
        buffer = getBufferDescriptor(buffer);
        const result = Filament._createTextureFromKtx1(buffer, this, options);
        buffer.delete();
        return result;
    };

    /// createTextureFromKtx2 ::method:: Utility function that creates a [Texture] from a KTX2 file.
    /// buffer ::argument:: asset string, or Uint8Array, or [Buffer] with KTX2 file contents
    /// options ::argument:: Options dictionary.
    /// ::retval:: [Texture]
    Filament.Engine.prototype.createTextureFromKtx2 = function(buffer, options) {
        options = options || {};
        buffer = getBufferDescriptor(buffer);

        const engine = this;
        const quiet = false;
        const reader = new Filament.Ktx2Reader(engine, quiet);

        reader.requestFormat(Filament.Texture$InternalFormat.RGBA8);
        reader.requestFormat(Filament.Texture$InternalFormat.SRGB8_A8);

        const formats = options.formats || [];
        for (const format of formats) {
            reader.requestFormat(format);
        }

        result = reader.load(buffer, options.srgb ? Filament.Ktx2Reader$TransferFunction.sRGB :
            Filament.Ktx2Reader$TransferFunction.LINEAR);

        reader.delete();
        buffer.delete();
        return result;
    };

    /// createIblFromKtx1 ::method:: Utility that creates an [IndirectLight] from a KTX file.
    /// NOTE: To prevent a leak, please be sure to destroy the associated reflections texture.
    /// buffer ::argument:: asset string, or Uint8Array, or [Buffer] with KTX file contents
    /// options ::argument:: Options dictionary.
    /// ::retval:: [IndirectLight]
    Filament.Engine.prototype.createIblFromKtx1 = function(buffer, options) {
        buffer = getBufferDescriptor(buffer);
        const result = Filament._createIblFromKtx1(buffer, this, options);
        buffer.delete();
        return result;
    };

    /// createSkyFromKtx1 ::method:: Utility function that creates a [Skybox] from a KTX file.
    /// NOTE: To prevent a leak, please be sure to destroy the associated texture.
    /// buffer ::argument:: asset string, or Uint8Array, or [Buffer] with KTX file contents
    /// options ::argument:: Options dictionary.
    /// ::retval:: [Skybox]
    Filament.Engine.prototype.createSkyFromKtx1 = function(buffer, options) {
        const skytex = this.createTextureFromKtx1(buffer, options);
        return Filament.Skybox.Builder().environment(skytex).build(this);
    };

    /// createTextureFromPng ::method:: Creates a 2D [Texture] from the raw contents of a PNG file.
    /// buffer ::argument:: asset string, or Uint8Array, or [Buffer] with PNG file contents
    /// options ::argument:: object with optional `srgb`, `noalpha`, and `nomips` keys.
    /// ::retval:: [Texture]
    Filament.Engine.prototype.createTextureFromPng = function(buffer, options) {
        buffer = getBufferDescriptor(buffer);
        const result = Filament._createTextureFromImageFile(buffer, this, options);
        buffer.delete();
        return result;
    };

    /// createTextureFromJpeg ::method:: Creates a 2D [Texture] from the contents of a JPEG file.
    /// buffer ::argument:: asset string, or Uint8Array, or [Buffer] with JPEG file contents
    /// options ::argument:: JavaScript object with optional `srgb` and `nomips` keys.
    /// ::retval:: [Texture]
    Filament.Engine.prototype.createTextureFromJpeg = function(buffer, options) {
        buffer = getBufferDescriptor(buffer);
        const result = Filament._createTextureFromImageFile(buffer, this, options);
        buffer.delete();
        return result;
    };

    /// loadFilamesh ::method:: Consumes the contents of a filamesh file and creates a renderable.
    /// buffer ::argument:: asset string, or Uint8Array, or [Buffer] with filamesh contents
    /// definstance ::argument:: Optional default [MaterialInstance]
    /// matinstances ::argument:: Optional in-out object that gets populated with a \
    /// name-to-[MaterialInstance] mapping. Clients can also optionally provide individual \
    /// material instances using this argument.
    /// ::retval:: JavaScript object with keys `renderable`, `vertexBuffer`, and `indexBuffer`. \
    /// These are of type [Entity], [VertexBuffer], and [IndexBuffer].
    Filament.Engine.prototype.loadFilamesh = function(buffer, definstance, matinstances) {
        buffer = getBufferDescriptor(buffer);
        const result = Filament._loadFilamesh(this, buffer, definstance, matinstances);
        buffer.delete();
        return result;
    };

    /// createAssetLoader ::method::
    /// ::retval:: an instance of [AssetLoader]
    /// Clients should create only one asset loader for the lifetime of their app, this prevents
    /// memory leaks and duplication of Material objects.
    Filament.Engine.prototype.createAssetLoader = function() {
        const materials = new Filament.gltfio$UbershaderProvider(this);
        return new Filament.gltfio$AssetLoader(this, materials);
    };

    /// addEntities ::method::
    /// entities ::argument:: array of entities
    /// This method is equivalent to calling `addEntity` on each item in the array.
    Filament.Scene.prototype.addEntities = function(entities) {
        const vector = new Filament.EntityVector();
        for (const entity of entities) {
            vector.push_back(entity);
        }
        this._addEntities(vector);
    };

    /// removeEntities ::method::
    /// entities ::argument:: array of entities
    /// This method is equivalent to calling `remove` on each item in the array.
    Filament.Scene.prototype.removeEntities = function(entities) {
        const vector = new Filament.EntityVector();
        for (const entity of entities) {
            vector.push_back(entity);
        }
        this._removeEntities(vector);
    };

    /// setShadowOptions ::method::
    /// instance ::argument:: Instance of a light component obtained from `getInstance`.
    /// overrides ::argument:: Dictionary with one or more of the following properties: \
    /// mapSize, shadowCascades, constantBias, normalBias, shadowFar, shadowNearHint, \
    /// shadowFarHint, stable, polygonOffsetConstant, polygonOffsetSlope, \
    // screenSpaceContactShadows, stepCount, maxShadowDistance.
    Filament.LightManager.prototype.setShadowOptions = function(instance, overrides) {
        this._setShadowOptions(instance, Filament.shadowOptions(overrides));
    };

    /// setClearOptions ::method::
    /// overrides ::argument:: Dictionary with one or more of the following properties: \
    /// clearColor, clear, discard.
    Filament.Renderer.prototype.setClearOptions = function(overrides) {
        const options = {
            clearColor: [0, 0, 0, 0],
            clear: false,
            discard: true
        };
        Object.assign(options, overrides);
        this._setClearOptions(options);
    };

    /// setAmbientOcclusionOptions ::method::
    Filament.View.prototype.setAmbientOcclusionOptions = function(overrides) {
        const options = this.setAmbientOcclusionOptionsDefaults(overrides);
        this._setAmbientOcclusionOptions(options);
    };

    /// setDepthOfFieldOptions ::method::
    Filament.View.prototype.setDepthOfFieldOptions = function(overrides) {
        const options = this.setDepthOfFieldOptionsDefaults(overrides);
        this._setDepthOfFieldOptions(options);
    };

    /// setMultiSampleAntiAliasingOptions ::method::
    Filament.View.prototype.setMultiSampleAntiAliasingOptions = function(overrides) {
        const options = this.setMultiSampleAntiAliasingOptionsDefaults(overrides);
        this._setMultiSampleAntiAliasingOptions(options);
    };

    /// setTemporalAntiAliasingOptions ::method::
    Filament.View.prototype.setTemporalAntiAliasingOptions = function(overrides) {
        const options = this.setTemporalAntiAliasingOptionsDefaults(overrides);
        this._setTemporalAntiAliasingOptions(options);
    };

    /// setScreenSpaceReflectionsOptions ::method::
    Filament.View.prototype.setScreenSpaceReflectionsOptions = function(overrides) {
        const options = this.setScreenSpaceReflectionsOptionsDefaults(overrides);
        this._setScreenSpaceReflectionsOptions(options);
    };

    /// setBloomOptions ::method::
    Filament.View.prototype.setBloomOptions = function(overrides) {
        const options = this.setBloomOptionsDefaults(overrides);
        this._setBloomOptions(options);
    };

    /// setFogOptions ::method::
    Filament.View.prototype.setFogOptions = function(overrides) {
        const options = this.setFogOptionsDefaults(overrides);
        this._setFogOptions(options);
    };

    /// setVignetteOptions ::method::
    Filament.View.prototype.setVignetteOptions = function(overrides) {
        const options = this.setVignetteOptionsDefaults(overrides);
        this._setVignetteOptions(options);
    };

    /// setGuardBandOptions ::method::
    Filament.View.prototype.setGuardBandOptions = function(overrides) {
        const options = this.setGuardBandOptionsDefaults(overrides);
        this._setGuardBandOptions(options);
    };

    /// setStereoscopicOptions ::method::
    Filament.View.prototype.setStereoscopicOptions = function(overrides) {
        const options = this.setStereoscopicOptionsDefaults(overrides);
        this._setStereoscopicOptions(options);
    }

    /// BufferObject ::core class::

    /// setBuffer ::method::
    /// engine ::argument:: [Engine]
    /// buffer ::argument:: asset string, or Uint8Array, or [Buffer]
    /// byteOffset ::argument:: non-negative integer
    Filament.BufferObject.prototype.setBuffer = function(engine, buffer, byteOffset = 0) {
        buffer = getBufferDescriptor(buffer);
        this._setBuffer(engine, buffer, byteOffset);
        buffer.delete();
    };

    /// VertexBuffer ::core class::

    /// setBufferAt ::method::
    /// engine ::argument:: [Engine]
    /// bufferIndex ::argument:: non-negative integer
    /// buffer ::argument:: asset string, or Uint8Array, or [Buffer]
    /// byteOffset ::argument:: non-negative integer
    Filament.VertexBuffer.prototype.setBufferAt = function(engine, bufferIndex, buffer, byteOffset = 0) {
        buffer = getBufferDescriptor(buffer);
        this._setBufferAt(engine, bufferIndex, buffer, byteOffset);
        buffer.delete();
    };

    /// IndexBuffer ::core class::

    /// setBuffer ::method::
    /// engine ::argument:: [Engine]
    /// buffer ::argument:: asset string, or Uint8Array, or [Buffer]
    /// byteOffset ::argument:: non-negative integer
    Filament.IndexBuffer.prototype.setBuffer = function(engine, buffer, byteOffset = 0) {
        buffer = getBufferDescriptor(buffer);
        this._setBuffer(engine, buffer, byteOffset);
        buffer.delete();
    };

    Filament.LightManager$Builder.prototype.shadowOptions = function(overrides) {
        return this._shadowOptions(Filament.shadowOptions(overrides));
    };

    Filament.RenderableManager$Builder.prototype.build =
    Filament.LightManager$Builder.prototype.build =
        function(engine, entity) {
            const result = this._build(engine, entity);
            this.delete();
            return result;
        };

    Filament.ColorGrading$Builder.prototype.build =
    Filament.RenderTarget$Builder.prototype.build =
    Filament.VertexBuffer$Builder.prototype.build =
    Filament.IndexBuffer$Builder.prototype.build =
    Filament.Texture$Builder.prototype.build =
    Filament.IndirectLight$Builder.prototype.build =
    Filament.Skybox$Builder.prototype.build =
        function(engine) {
            const result = this._build(engine);
            this.delete();
            return result;
        };

    Filament.Ktx1Bundle.prototype.getBlob = function(index) {
        const blob = this._getBlob(index);
        const result = blob.getBytes();
        blob.delete();
        return result;
    }

    Filament.Ktx1Bundle.prototype.getCubeBlob = function(miplevel) {
        const blob = this._getCubeBlob(miplevel);
        const result = blob.getBytes();
        blob.delete();
        return result;
    }

    Filament.Texture.prototype.setImage = function(engine, level, pbd) {
        this._setImage(engine, level, pbd);
        pbd.delete();
    }

    Filament.Texture.prototype.setImageCube = function(engine, level, pbd) {
        this._setImageCube(engine, level, pbd);
        pbd.delete();
    }

    Filament.Texture.prototype.getWidth = function(engine, level = 0) {
        return this._getWidth(engine, level);
    }

    Filament.Texture.prototype.getHeight = function(engine, level = 0) {
        return this._getHeight(engine, level);
    }

    Filament.Texture.prototype.getDepth = function(engine, level = 0) {
        return this._getDepth(engine, level);
    }

    Filament.Texture.prototype.getLevels = function(engine) {
        return this._getLevels(engine);
    }

    Filament.SurfaceOrientation$Builder.prototype.normals = function(buffer, stride = 0) {
        buffer = new Uint8Array(buffer.buffer, buffer.byteOffset, buffer.byteLength);
        this.norPointer = Filament._malloc(buffer.byteLength);
        Filament.HEAPU8.set(buffer, this.norPointer);
        this._normals(this.norPointer, stride);
    };

    Filament.SurfaceOrientation$Builder.prototype.uvs = function(buffer, stride = 0) {
        buffer = new Uint8Array(buffer.buffer, buffer.byteOffset, buffer.byteLength);
        this.uvsPointer = Filament._malloc(buffer.byteLength);
        Filament.HEAPU8.set(buffer, this.uvsPointer);
        this._uvs(this.uvsPointer, stride);
    };

    Filament.SurfaceOrientation$Builder.prototype.positions = function(buffer, stride = 0) {
        buffer = new Uint8Array(buffer.buffer, buffer.byteOffset, buffer.byteLength);
        this.posPointer = Filament._malloc(buffer.byteLength);
        Filament.HEAPU8.set(buffer, this.posPointer);
        this._positions(this.posPointer, stride);
    };

    Filament.SurfaceOrientation$Builder.prototype.triangles16 = function(buffer) {
        buffer = new Uint8Array(buffer.buffer, buffer.byteOffset, buffer.byteLength);
        this.t16Pointer = Filament._malloc(buffer.byteLength);
        Filament.HEAPU8.set(buffer, this.t16Pointer);
        this._triangles16(this.t16Pointer);
    };

    Filament.SurfaceOrientation$Builder.prototype.triangles32 = function(buffer) {
        buffer = new Uint8Array(buffer.buffer, buffer.byteOffset, buffer.byteLength);
        this.t32Pointer = Filament._malloc(buffer.byteLength);
        Filament.HEAPU8.set(buffer, this.t32Pointer);
        this._triangles32(this.t32Pointer);
    };

    Filament.SurfaceOrientation$Builder.prototype.build = function() {
        const result = this._build();
        this.delete();
        if ('norPointer' in this) Filament._free(this.norPointer);
        if ('uvsPointer' in this) Filament._free(this.uvsPointer);
        if ('posPointer' in this) Filament._free(this.posPointer);
        if ('t16Pointer' in this) Filament._free(this.t16Pointer);
        if ('t32Pointer' in this) Filament._free(this.t32Pointer);
        return result;
    };

    Filament.SurfaceOrientation.prototype.getQuats = function(nverts) {
        const attribType = Filament.VertexBuffer$AttributeType.SHORT4;
        const quatsBufferSize = 8 * nverts;
        const quatsBuffer = Filament._malloc(quatsBufferSize);
        this._getQuats(quatsBuffer, nverts, attribType);
        const arrayBuffer = Filament.HEAPU8.subarray(quatsBuffer, quatsBuffer + quatsBufferSize).slice().buffer;
        Filament._free(quatsBuffer);
        return new Int16Array(arrayBuffer);
    };

    Filament.SurfaceOrientation.prototype.getQuatsHalf4 = function (nverts) {
        const attribType = Filament.VertexBuffer$AttributeType.HALF4;
        const quatsBufferSize = 8 * nverts;
        const quatsBuffer = Filament._malloc(quatsBufferSize);
        this._getQuats(quatsBuffer, nverts, attribType);
        const arrayBuffer = Filament.HEAPU8.subarray(quatsBuffer, quatsBuffer + quatsBufferSize).slice().buffer;
        Filament._free(quatsBuffer);
        return new Uint16Array(arrayBuffer);
    };

    Filament.SurfaceOrientation.prototype.getQuatsFloat4 = function (nverts) {
        const attribType = Filament.VertexBuffer$AttributeType.FLOAT4;
        const quatsBufferSize = 16 * nverts;
        const quatsBuffer = Filament._malloc(quatsBufferSize);
        this._getQuats(quatsBuffer, nverts, attribType);
        const arrayBuffer = Filament.HEAPU8.subarray(quatsBuffer, quatsBuffer + quatsBufferSize).slice().buffer;
        Filament._free(quatsBuffer);
        return new Float32Array(arrayBuffer);
    };

    Filament.gltfio$AssetLoader.prototype.createAsset = function(buffer) {
        buffer = getBufferDescriptor(buffer);
        const result = this._createAsset(buffer);
        buffer.delete();
        return result;
    };

    Filament.gltfio$AssetLoader.prototype.createInstancedAsset = function(buffer, instances) {
        buffer = getBufferDescriptor(buffer);
        const asset = this._createInstancedAsset(buffer, instances.length);
        buffer.delete();
        const instancesVector = asset._getAssetInstances();
        for (let i = 0; i < instancesVector.size(); i++) {
            instances[i] = instancesVector.get(i);
        }
        return asset;
    };

    // See the C++ documentation for ResourceLoader and AssetLoader. The JavaScript API differs in
    // that it takes two optional callbacks:
    //
    // - onDone is called after all resources have been downloaded and decoded.
    // - onFetched is called after each resource has finished downloading.
    //
    // Takes an optional base path for resolving the URI strings in the glTF file, which is
    // typically the path to the parent glTF file. The given base path cannot itself be a relative
    // URL, but clients can do the following to resolve a relative URL:
    //    const basePath = '' + new URL(myRelativeUrl, document.location);
    // If the given base path is null, document.location is used as the base.
    //
    // The optional asyncInterval argument allows clients to control how decoding is amortized
    // over time. It represents the number of milliseconds between each texture decoding task.
    //
    // The optional config argument is an object with boolean field `normalizeSkinningWeights`.
    Filament.gltfio$FilamentAsset.prototype.loadResources = function(onDone, onFetched, basePath,
            asyncInterval, config) {
        const asset = this;
        const engine = this.getEngine();
        const interval = asyncInterval || 30;
        const defaults = {
            normalizeSkinningWeights: true,
        };
        config = Object.assign(defaults, config || {});

        basePath = basePath || document.location;
        onFetched = onFetched || ((name) => {});
        onDone = onDone || (() => {});

        // Construct two lists of URI strings to fetch: textures and non-textures.
        let textureUris = new Set();
        let bufferUris = new Set();
        const absoluteToRelativeUri = {};
        for (const relativeUri of this.getResourceUris()) {
            const absoluteUri = '' + new URL(relativeUri, basePath);
            absoluteToRelativeUri[absoluteUri] = relativeUri;
            if (isTexture(relativeUri)) {
                textureUris.add(absoluteUri);
                continue;
            }
            bufferUris.add(absoluteUri);
        }
        textureUris = Array.from(textureUris);
        bufferUris = Array.from(bufferUris);

        // Construct a resource loader and start decoding after all textures are fetched.
        const resourceLoader = new Filament.gltfio$ResourceLoader(engine,
                config.normalizeSkinningWeights);

        const stbProvider = new Filament.gltfio$StbProvider(engine);
        const ktx2Provider = new Filament.gltfio$Ktx2Provider(engine);

        resourceLoader.addStbProvider("image/jpeg", stbProvider);
        resourceLoader.addStbProvider("image/png", stbProvider);
        resourceLoader.addKtx2Provider("image/ktx2", ktx2Provider);

        const onComplete = () => {
            resourceLoader.asyncBeginLoad(asset);

            // NOTE: This decodes in the wasm layer instead of using Canvas2D, which allows Filament
            // to have more control (handling of alpha, srgb, etc) and improves parity with native
            // platforms. In the future we may wish to offload this to web workers.

            // Decode a single PNG or JPG every 30 milliseconds, or at the specified interval.
            const timer = setInterval(() => {
                resourceLoader.asyncUpdateLoad();
                const progress = resourceLoader.asyncGetLoadProgress();
                if (progress >= 1) {
                    clearInterval(timer);
                    resourceLoader.delete();
                    stbProvider.delete();
                    onDone();
                }
            }, interval);
        };

        // Download all non-texture resources and invoke the callback when done.
        if (bufferUris.length == 0) {
            onComplete();
        } else {
            Filament.fetch(bufferUris, onComplete, function(absoluteUri) {
                const buffer = getBufferDescriptor(absoluteUri);
                const relativeUri = absoluteToRelativeUri[absoluteUri];
                resourceLoader.addResourceData(relativeUri, buffer);
                buffer.delete();
                onFetched(relativeUri);
            });
        }

        // Begin downloading all texture resources, no completion callback necessary.
        Filament.fetch(textureUris, null, function(absoluteUri) {
            const buffer = getBufferDescriptor(absoluteUri);
            const relativeUri = absoluteToRelativeUri[absoluteUri];
            resourceLoader.addResourceData(relativeUri, buffer);
            buffer.delete();
            onFetched(relativeUri);
        });
    };

    Filament.gltfio$FilamentAsset.prototype.getEntities = function() {
        return Filament.vectorToArray(this._getEntities());
    };

    Filament.gltfio$FilamentAsset.prototype.getEntitiesByName = function(name) {
        return Filament.vectorToArray(this._getEntitiesByName(name));
    };

    Filament.gltfio$FilamentAsset.prototype.getEntitiesByPrefix = function(prefix) {
        return Filament.vectorToArray(this._getEntitiesByPrefix(prefix));
    };

    Filament.gltfio$FilamentAsset.prototype.getLightEntities = function() {
        return Filament.vectorToArray(this._getLightEntities());
    };

    Filament.gltfio$FilamentAsset.prototype.getRenderableEntities = function() {
        return Filament.vectorToArray(this._getRenderableEntities());
    };

    Filament.gltfio$FilamentAsset.prototype.getCameraEntities = function() {
        return Filament.vectorToArray(this._getCameraEntities());
    };

    Filament.gltfio$FilamentAsset.prototype.getResourceUris = function() {
        return Filament.vectorToArray(this._getResourceUris());
    }

    Filament.gltfio$FilamentAsset.prototype.getAssetInstances = function() {
        return Filament.vectorToArray(this._getAssetInstances());
    }

    Filament.gltfio$FilamentInstance.prototype.getMaterialVariantNames = function() {
        return Filament.vectorToArray(this._getMaterialVariantNames());
    }
};

/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// ---------------
// Buffer Wrappers
// ---------------

// These wrappers make it easy for JavaScript clients to pass large swaths of data to Filament. They
// copy the contents of the given typed array into the WASM heap, then return a low-level buffer
// descriptor object. If the given array was taken from the WASM heap, then they create a temporary
// copy because the input pointer becomes invalidated after allocating heap memory for the buffer
// descriptor.

/// Buffer ::function:: Constructs a [BufferDescriptor] by copying a typed array into the WASM heap.
/// typedarray ::argument:: Data to consume (e.g. Uint8Array, Uint16Array, Float32Array)
/// ::retval:: [BufferDescriptor]
Filament.Buffer = function(typedarray) {
    console.assert(typedarray.buffer instanceof ArrayBuffer);
    console.assert(typedarray.byteLength > 0);

    // The only reason we need to create a copy here is that emscripten might "grow" its entire heap
    // (i.e. destroy and recreate) during the allocation of the BufferDescriptor, which would cause
    // detachment if the source array happens to be view into the old emscripten heap.
    const ta = typedarray.slice();

    const bd = new Filament.driver$BufferDescriptor(ta.byteLength);
    const uint8array = new Uint8Array(ta.buffer, ta.byteOffset, ta.byteLength);

    // getBytes() returns a view into the emscripten heap, this just does a memcpy into it.
    bd.getBytes().set(uint8array);

    return bd;
};

/// PixelBuffer ::function:: Constructs a [PixelBufferDescriptor] by copying a typed array into \
/// the WASM heap.
/// typedarray ::argument:: Data to consume (e.g. Uint8Array, Uint16Array, Float32Array)
/// format ::argument:: [PixelDataFormat]
/// datatype ::argument:: [PixelDataType]
/// ::retval:: [PixelBufferDescriptor]
Filament.PixelBuffer = function(typedarray, format, datatype) {
    console.assert(typedarray.buffer instanceof ArrayBuffer);
    console.assert(typedarray.byteLength > 0);
    const ta = typedarray.slice();
    const bd = new Filament.driver$PixelBufferDescriptor(ta.byteLength, format, datatype);
    const uint8array = new Uint8Array(ta.buffer, ta.byteOffset, ta.byteLength);
    bd.getBytes().set(uint8array);
    return bd;
};

/// CompressedPixelBuffer ::function:: Constructs a [PixelBufferDescriptor] for compressed texture
/// data by copying a typed array into the WASM heap.
/// typedarray ::argument:: Data to consume (e.g. Uint8Array, Uint16Array, Float32Array)
/// cdatatype ::argument:: [CompressedPixelDataType]
/// faceSize ::argument:: Number of bytes in each face (cubemaps only)
/// ::retval:: [PixelBufferDescriptor]
Filament.CompressedPixelBuffer = function(typedarray, cdatatype, faceSize) {
    console.assert(typedarray.buffer instanceof ArrayBuffer);
    console.assert(typedarray.byteLength > 0);
    faceSize = faceSize || typedarray.byteLength;
    const ta = typedarray.slice();
    const bd = new Filament.driver$PixelBufferDescriptor(ta.byteLength, cdatatype, faceSize, true);
    const uint8array = new Uint8Array(ta.buffer, ta.byteOffset, ta.byteLength);
    bd.getBytes().set(uint8array);
    return bd;
};

Filament._loadFilamesh = function(engine, buffer, definstance, matinstances) {
    matinstances = matinstances || {};
    const registry = new Filament.MeshReader$MaterialRegistry();
    for (let key in matinstances) {
        registry.set(key, matinstances[key]);
    }
    if (definstance) {
        registry.set("DefaultMaterial", definstance);
    }
    const mesh = Filament.MeshReader.loadMeshFromBuffer(engine, buffer, registry);
    const keys = registry.keys();
    for (let i = 0; i < keys.size(); i++) {
        const key = keys.get(i);
        const minstance = registry.get(key);
        matinstances[key] = minstance;
    }
    return {
        "renderable": mesh.renderable(),
        "vertexBuffer": mesh.vertexBuffer(),
        "indexBuffer": mesh.indexBuffer(),
    }
}

// ------------------
// Geometry Utilities
// ------------------

/// IcoSphere ::class:: Utility class for constructing spheres (requires glMatrix).
///
/// The constructor takes an integer subdivision level, with 0 being an icosahedron.
///
/// Exposes three arrays as properties:
///
/// - `icosphere.vertices` Float32Array of XYZ coordinates.
/// - `icosphere.tangents` Uint16Array (interpreted as half-floats) encoding the surface orientation
/// as quaternions.
/// - `icosphere.triangles` Uint16Array with triangle indices.
///
Filament.IcoSphere = function(nsubdivs) {
    const X = .525731112119133606;
    const Z = .850650808352039932;
    const N = 0.;
    this.vertices = new Float32Array([
        -X, +N, +Z, +X, +N, +Z, -X, +N, -Z, +X, +N, -Z ,
        +N, +Z, +X, +N, +Z, -X, +N, -Z, +X, +N, -Z, -X ,
        +Z, +X, +N, -Z, +X, +N, +Z, -X, +N, -Z, -X, +N ,
    ]);
    this.triangles = new Uint16Array([
        1,   4, 0,  4,  9,  0, 4,   5, 9, 8, 5,   4 ,  1,  8,  4 ,
        1,  10, 8, 10,  3,  8, 8,   3, 5, 3, 2,   5 ,  3,  7,  2 ,
        3,  10, 7, 10,  6,  7, 6,  11, 7, 6, 0,  11 ,  6,  1,  0 ,
        10,   1, 6, 11,  0,  9, 2,  11, 9, 5, 2,   9 , 11,  2,  7 ,
    ]);

    nsubdivs = nsubdivs || 0;
    while (nsubdivs-- > 0) {
        this.subdivide();
    }

    const nverts = this.vertices.length / 3;

    // This is a unit sphere, so normals = positions.
    const normals = this.vertices;

    // Perform computations.
    const sob = new Filament.SurfaceOrientation$Builder();
    sob.vertexCount(nverts);
    sob.normals(normals, 0)
    const orientation = sob.build();

    // Copy the results out of the helper.
    this.tangents = orientation.getQuats(nverts);

    // Free up the surface orientation helper now that we're done with it.
    orientation.delete();
}

Filament.IcoSphere.prototype.subdivide = function() {
    const srctris = this.triangles;
    const srcverts = this.vertices;
    const nsrctris = srctris.length / 3;
    const ndsttris = nsrctris * 4;
    const nsrcverts = srcverts.length / 3;
    const ndstverts = nsrcverts + nsrctris * 3;
    const dsttris = new Uint16Array(ndsttris * 3);
    const dstverts = new Float32Array(ndstverts * 3);
    dstverts.set(srcverts);
    let srcind = 0, dstind = 0, i3 = nsrcverts * 3, i4 = i3 + 3, i5 = i4 + 3;
    for (let tri = 0; tri < nsrctris; tri++, i3 += 9, i4 += 9, i5 += 9) {
        const i0 = srctris[srcind++] * 3;
        const i1 = srctris[srcind++] * 3;
        const i2 = srctris[srcind++] * 3;
        const v0 = srcverts.subarray(i0, i0 + 3);
        const v1 = srcverts.subarray(i1, i1 + 3);
        const v2 = srcverts.subarray(i2, i2 + 3);
        const v3 = dstverts.subarray(i3, i3 + 3);
        const v4 = dstverts.subarray(i4, i4 + 3);
        const v5 = dstverts.subarray(i5, i5 + 3);
        vec3.normalize(v3, vec3.add(v3, v0, v1));
        vec3.normalize(v4, vec3.add(v4, v1, v2));
        vec3.normalize(v5, vec3.add(v5, v2, v0));
        dsttris[dstind++] = i0 / 3;
        dsttris[dstind++] = i3 / 3;
        dsttris[dstind++] = i5 / 3;
        dsttris[dstind++] = i3 / 3;
        dsttris[dstind++] = i1 / 3;
        dsttris[dstind++] = i4 / 3;
        dsttris[dstind++] = i5 / 3;
        dsttris[dstind++] = i3 / 3;
        dsttris[dstind++] = i4 / 3;
        dsttris[dstind++] = i2 / 3;
        dsttris[dstind++] = i5 / 3;
        dsttris[dstind++] = i4 / 3;
    }
    this.triangles = dsttris;
    this.vertices = dstverts;
}

// ---------------
// Math Extensions
// ---------------

function clamp(v, least, most) {
    return Math.max(Math.min(most, v), least);
}

/// packSnorm16 ::function:: Converts a float in [-1, +1] into a half-float.
/// value ::argument:: float
/// ::retval:: half-float
Filament.packSnorm16 = function(value) {
    return Math.round(clamp(value, -1.0, 1.0) * 32767.0);
}

/// loadMathExtensions ::function:: Extends the [glMatrix](http://glmatrix.net/) math library.
/// Filament does not require its clients to use glMatrix, but if its usage is detected then
/// the [init] function will automatically call `loadMathExtensions`.
/// This defines the following functions:
/// - **vec4.packSnorm16** can be used to create half-floats (see [packSnorm16])
/// - **mat3.fromRotation** now takes an arbitrary axis
Filament.loadMathExtensions = function() {
    vec4.packSnorm16 = function(out, src) {
        out[0] = Filament.packSnorm16(src[0]);
        out[1] = Filament.packSnorm16(src[1]);
        out[2] = Filament.packSnorm16(src[2]);
        out[3] = Filament.packSnorm16(src[3]);
        return out;
    }
    // In gl-matrix, mat3 rotation assumes rotation about the Z axis, so here we add a function
    // to allow an arbitrary axis.
    const fromRotationZ = mat3.fromRotation;
    mat3.fromRotation = function(out, radians, axis) {
        if (axis) {
            return mat3.fromMat4(out, mat4.fromRotation(mat4.create(), radians, axis));
        }
        return fromRotationZ(out, radians);
    };
};

// ---------------
// Texture helpers
// ---------------

Filament._createTextureFromKtx1 = function(ktxdata, engine, options) {
    options = options || {};
    const ktx = options['ktx'] || new Filament.Ktx1Bundle(ktxdata);
    const srgb = !!options['srgb'];
    return Filament.ktx1reader$createTexture(engine, ktx, srgb);
};

Filament._createIblFromKtx1 = function(ktxdata, engine, options) {
    options = options || {};
    const iblktx = options['ktx'] = new Filament.Ktx1Bundle(ktxdata);

    const format = iblktx.info().glInternalFormat;
    //if (format != this.ctx.R11F_G11F_B10F && format != this.ctx.RGB16F && format != this.ctx.RGB32F) {
    if (format != 35898 && format != 33327 && format != 34837) {
        console.warn('IBL texture format is 0x' + format.toString(16) +
            ' which is not an expected floating-point format. Please use cmgen to generate IBL.');
    }

    const ibltex = Filament._createTextureFromKtx1(ktxdata, engine, options);
    const shstring = iblktx.getMetadata("sh");
    const ibl = Filament.IndirectLight.Builder()
        .reflections(ibltex)
        .build(engine);
    ibl.shfloats = shstring.split(/\s/, 9 * 3).map(parseFloat);
    return ibl;
};

Filament._createTextureFromImageFile = function(fileContents, engine, options) {
    const Sampler = Filament.Texture$Sampler;
    const TextureFormat = Filament.Texture$InternalFormat;
    const PixelDataFormat = Filament.PixelDataFormat;

    options = options || {};
    const srgb = !!options['srgb'];
    const noalpha = !!options['noalpha'];
    const nomips = !!options['nomips'];

    const decodedImage = Filament.decodeImage(fileContents, noalpha ? 3 : 4);

    let texformat, pbformat, pbtype;
    if (noalpha) {
        texformat = srgb ? TextureFormat.SRGB8 : TextureFormat.RGB8;
        pbformat = PixelDataFormat.RGB;
        pbtype = Filament.PixelDataType.UBYTE;
    } else {
        texformat = srgb ? TextureFormat.SRGB8_A8 : TextureFormat.RGBA8;
        pbformat = PixelDataFormat.RGBA;
        pbtype = Filament.PixelDataType.UBYTE;
    }

    const tex = Filament.Texture.Builder()
        .width(decodedImage.width)
        .height(decodedImage.height)
        .levels(nomips ? 1 : 0xff)
        .sampler(Sampler.SAMPLER_2D)
        .format(texformat)
        .build(engine);

    const pixelbuffer = Filament.PixelBuffer(decodedImage.data.getBytes(), pbformat, pbtype);
    tex.setImage(engine, 0, pixelbuffer);
    if (!nomips) {
        tex.generateMipmaps(engine);
    }
    return tex;
};

/// getSupportedFormats ::function:: Queries WebGL to check which compressed formats are supported.
/// ::retval:: object with boolean values and the following keys: s3tc, astc, etc
Filament.getSupportedFormats = function() {
    if (Filament.supportedFormats) {
        return Filament.supportedFormats;
    }
    const options = { majorVersion: 2, minorVersion: 0 };
    let ctx = document.createElement('canvas').getContext('webgl2', options);
    const result = {
        s3tc: false,
        s3tc_srgb: false,
        astc: false,
        etc: false,
    }
    let exts = ctx.getSupportedExtensions(), nexts = exts.length, i;
    for (i = 0; i < nexts; i++) {
        let ext = exts[i];
        if (ext == "WEBGL_compressed_texture_s3tc") {
            result.s3tc = true;
        } else if (ext == "WEBGL_compressed_texture_s3tc_srgb") {
            result.s3tc_srgb = true;
        } else if (ext == "WEBGL_compressed_texture_astc") {
            result.astc = true;
        } else if (ext == "WEBGL_compressed_texture_etc") {
            result.etc = true;
        }
    }
    return Filament.supportedFormats = result;
}

/// getSupportedFormatSuffix ::function:: Generate a file suffix according to the texture format.
/// Consumes a string describing desired formats and produces a file suffix depending on
/// which (if any) of the formats are actually supported by the WebGL implementation. This is
/// useful for compressed textures. For example, some platforms accept ETC and others accept S3TC.
/// desiredFormats ::argument:: space-delimited string of desired formats
/// ::retval:: empty string if there is no intersection of supported and desired formats.
Filament.getSupportedFormatSuffix = function(desiredFormats) {
    desiredFormats = desiredFormats.split(' ');
    let exts = Filament.getSupportedFormats();
    for (let key in exts) {
        if (exts[key] && desiredFormats.includes(key)) {
            return '_' + key;
        }
    }
    return '';
}
